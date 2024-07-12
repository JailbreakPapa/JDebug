#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/Archive/Archive.h>
#include <Foundation/Logging/Log.h>

void operator<<(nsStreamWriter& inout_stream, const nsArchiveStoredString& value)
{
  inout_stream << value.m_uiLowerCaseHash;
  inout_stream << value.m_uiSrcStringOffset;
}

void operator>>(nsStreamReader& inout_stream, nsArchiveStoredString& value)
{
  inout_stream >> value.m_uiLowerCaseHash;
  inout_stream >> value.m_uiSrcStringOffset;
}

nsUInt32 nsArchiveTOC::FindEntry(nsStringView sFile) const
{
  nsStringBuilder sLowerCasePath = sFile;
  sLowerCasePath.ToLower();

  nsUInt32 uiIndex;

  nsArchiveLookupString lookup(nsHashingUtils::StringHash(sLowerCasePath.GetView()), sLowerCasePath, m_AllPathStrings);

  if (!m_PathToEntryIndex.TryGetValue(lookup, uiIndex))
    return nsInvalidIndex;

  NS_ASSERT_DEBUG(sFile.IsEqual_NoCase(GetEntryPathString(uiIndex)), "Hash table corruption detected.");
  return uiIndex;
}

nsUInt32 nsArchiveTOC::AddPathString(nsStringView sPathString)
{
  const nsUInt32 offset = m_AllPathStrings.GetCount();
  const nsUInt32 numNewBytesNeeded = sPathString.GetElementCount() + 1;
  m_AllPathStrings.Reserve(m_AllPathStrings.GetCount() + numNewBytesNeeded);
  m_AllPathStrings.PushBackRange(nsArrayPtr<const nsUInt8>(reinterpret_cast<const nsUInt8*>(sPathString.GetStartPointer()), sPathString.GetElementCount()));
  m_AllPathStrings.PushBackUnchecked('\0');
  return offset;
}

void nsArchiveTOC::RebuildPathToEntryHashes()
{
  const nsUInt32 uiNumEntries = m_Entries.GetCount();
  m_PathToEntryIndex.Clear();
  m_PathToEntryIndex.Reserve(uiNumEntries);

  nsStringBuilder sLowerCasePath;

  for (nsUInt32 i = 0; i < uiNumEntries; i++)
  {
    const nsUInt32 uiSrcStringOffset = m_Entries[i].m_uiPathStringOffset;
    nsStringView sEntryString = GetEntryPathString(i);
    sLowerCasePath = sEntryString;
    sLowerCasePath.ToLower();

    // cut off the upper 32 bit, we don't need them here
    const nsUInt32 uiLowerCaseHash = nsHashingUtils::StringHashTo32(nsHashingUtils::StringHash(sLowerCasePath.GetView()) & 0xFFFFFFFFllu);

    m_PathToEntryIndex.Insert(nsArchiveStoredString(uiLowerCaseHash, uiSrcStringOffset), i);

    // Verify that the conversion worked
    NS_ASSERT_DEBUG(FindEntry(sEntryString) == i, "Hashed path retrieval did not yield inserted index");
  }
}

nsStringView nsArchiveTOC::GetEntryPathString(nsUInt32 uiEntryIdx) const
{
  return reinterpret_cast<const char*>(&m_AllPathStrings[m_Entries[uiEntryIdx].m_uiPathStringOffset]);
}

nsResult nsArchiveTOC::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream.WriteVersion(2);

  NS_SUCCEED_OR_RETURN(inout_stream.WriteArray(m_Entries));

  // write the hash of a known string to the archive, to detect hash function changes
  nsUInt64 uiStringHash = nsHashingUtils::StringHash("nsArchive");
  inout_stream << uiStringHash;

  NS_SUCCEED_OR_RETURN(inout_stream.WriteHashTable(m_PathToEntryIndex));

  NS_SUCCEED_OR_RETURN(inout_stream.WriteArray(m_AllPathStrings));

  return NS_SUCCESS;
}

struct nsOldTempHashedString
{
  nsUInt32 m_uiHash = 0;

  nsResult Deserialize(nsStreamReader& r)
  {
    r >> m_uiHash;
    return NS_SUCCESS;
  }

  bool operator==(const nsOldTempHashedString& rhs) const
  {
    return m_uiHash == rhs.m_uiHash;
  }
};

template <>
struct nsHashHelper<nsOldTempHashedString>
{
  static nsUInt32 Hash(const nsOldTempHashedString& value)
  {
    return value.m_uiHash;
  }

  static bool Equal(const nsOldTempHashedString& a, const nsOldTempHashedString& b) { return a == b; }
};

nsResult nsArchiveTOC::Deserialize(nsStreamReader& inout_stream, nsUInt8 uiArchiveVersion)
{
  NS_ASSERT_ALWAYS(uiArchiveVersion <= 4, "Unsupported archive version {}", uiArchiveVersion);

  // we don't use the TOC version anymore, but the archive version instead
  const nsTypeVersion version = inout_stream.ReadVersion(2);

  NS_SUCCEED_OR_RETURN(inout_stream.ReadArray(m_Entries));

  bool bRecreateStringHashes = true;

  if (version == 1)
  {
    // read and discard the data, it is regenerated below
    nsHashTable<nsOldTempHashedString, nsUInt32> m_PathToIndex;
    NS_SUCCEED_OR_RETURN(inout_stream.ReadHashTable(m_PathToIndex));
  }
  else
  {
    if (uiArchiveVersion >= 4)
    {
      // read the hash of a known string from the archive, to detect hash function changes
      nsUInt64 uiStringHash = 0;
      inout_stream >> uiStringHash;

      if (uiStringHash == nsHashingUtils::StringHash("nsArchive"))
      {
        bRecreateStringHashes = false;
      }
    }

    NS_SUCCEED_OR_RETURN(inout_stream.ReadHashTable(m_PathToEntryIndex));
  }

  NS_SUCCEED_OR_RETURN(inout_stream.ReadArray(m_AllPathStrings));

  if (bRecreateStringHashes)
  {
    nsLog::Info("Archive uses older string hashing, recomputing hashes.");

    // version 1 stores an older way for the path/hash -> entry lookup table, which is prone to hash collisions
    // in this case, rebuild the new hash table on the fly
    //
    // version 2 used MurmurHash
    // version 3 switched to 32 bit xxHash
    // version 4 switched to 64 bit hashes

    RebuildPathToEntryHashes();
  }

  // path strings mustn't be empty and must be zero-terminated
  if (m_AllPathStrings.IsEmpty() || m_AllPathStrings.PeekBack() != '\0')
  {
    nsLog::Error("Archive is corrupt. Invalid string data.");
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsResult nsArchiveEntry::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream << m_uiDataStartOffset;
  inout_stream << m_uiUncompressedDataSize;
  inout_stream << m_uiStoredDataSize;
  inout_stream << (nsUInt8)m_CompressionMode;
  inout_stream << m_uiPathStringOffset;

  return NS_SUCCESS;
}

nsResult nsArchiveEntry::Deserialize(nsStreamReader& inout_stream)
{
  inout_stream >> m_uiDataStartOffset;
  inout_stream >> m_uiUncompressedDataSize;
  inout_stream >> m_uiStoredDataSize;
  nsUInt8 uiCompressionMode = 0;
  inout_stream >> uiCompressionMode;
  m_CompressionMode = (nsArchiveCompressionMode)uiCompressionMode;
  inout_stream >> m_uiPathStringOffset;

  return NS_SUCCESS;
}
