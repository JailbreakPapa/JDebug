#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/Archive/Archive.h>
#include <Foundation/Logging/Log.h>

void operator<<(wdStreamWriter& inout_stream, const wdArchiveStoredString& value)
{
  inout_stream << value.m_uiLowerCaseHash;
  inout_stream << value.m_uiSrcStringOffset;
}

void operator>>(wdStreamReader& inout_stream, wdArchiveStoredString& value)
{
  inout_stream >> value.m_uiLowerCaseHash;
  inout_stream >> value.m_uiSrcStringOffset;
}

wdUInt32 wdArchiveTOC::FindEntry(const char* szFile) const
{
  wdStringBuilder sLowerCasePath = szFile;
  sLowerCasePath.ToLower();

  wdUInt32 uiIndex;

  wdArchiveLookupString lookup(wdHashingUtils::StringHash(sLowerCasePath.GetView()), sLowerCasePath, m_AllPathStrings);

  if (!m_PathToEntryIndex.TryGetValue(lookup, uiIndex))
    return wdInvalidIndex;

  WD_ASSERT_DEBUG(wdStringUtils::IsEqual_NoCase(szFile, GetEntryPathString(uiIndex)), "Hash table corruption detected.");
  return uiIndex;
}

const char* wdArchiveTOC::GetEntryPathString(wdUInt32 uiEntryIdx) const
{
  return reinterpret_cast<const char*>(&m_AllPathStrings[m_Entries[uiEntryIdx].m_uiPathStringOffset]);
}

wdResult wdArchiveTOC::Serialize(wdStreamWriter& inout_stream) const
{
  inout_stream.WriteVersion(2);

  WD_SUCCEED_OR_RETURN(inout_stream.WriteArray(m_Entries));

  // write the hash of a known string to the archive, to detect hash function changes
  wdUInt64 uiStringHash = wdHashingUtils::StringHash("wdArchive");
  inout_stream << uiStringHash;

  WD_SUCCEED_OR_RETURN(inout_stream.WriteHashTable(m_PathToEntryIndex));

  WD_SUCCEED_OR_RETURN(inout_stream.WriteArray(m_AllPathStrings));

  return WD_SUCCESS;
}

struct wdOldTempHashedString
{
  wdUInt32 m_uiHash = 0;

  wdResult Deserialize(wdStreamReader& r)
  {
    r >> m_uiHash;
    return WD_SUCCESS;
  }

  bool operator==(const wdOldTempHashedString& rhs) const
  {
    return m_uiHash == rhs.m_uiHash;
  }
};

template <>
struct wdHashHelper<wdOldTempHashedString>
{
  static wdUInt32 Hash(const wdOldTempHashedString& value)
  {
    return value.m_uiHash;
  }

  static bool Equal(const wdOldTempHashedString& a, const wdOldTempHashedString& b) { return a == b; }
};

wdResult wdArchiveTOC::Deserialize(wdStreamReader& inout_stream, wdUInt8 uiArchiveVersion)
{
  WD_ASSERT_ALWAYS(uiArchiveVersion <= 4, "Unsupported archive version {}", uiArchiveVersion);

  // we don't use the TOC version anymore, but the archive version instead
  const wdTypeVersion version = inout_stream.ReadVersion(2);

  WD_SUCCEED_OR_RETURN(inout_stream.ReadArray(m_Entries));

  bool bRecreateStringHashes = true;

  if (version == 1)
  {
    // read and discard the data, it is regenerated below
    wdHashTable<wdOldTempHashedString, wdUInt32> m_PathToIndex;
    WD_SUCCEED_OR_RETURN(inout_stream.ReadHashTable(m_PathToIndex));
  }
  else
  {
    if (uiArchiveVersion >= 4)
    {
      // read the hash of a known string from the archive, to detect hash function changes
      wdUInt64 uiStringHash = 0;
      inout_stream >> uiStringHash;

      if (uiStringHash == wdHashingUtils::StringHash("wdArchive"))
      {
        bRecreateStringHashes = false;
      }
    }

    WD_SUCCEED_OR_RETURN(inout_stream.ReadHashTable(m_PathToEntryIndex));
  }

  WD_SUCCEED_OR_RETURN(inout_stream.ReadArray(m_AllPathStrings));

  if (bRecreateStringHashes)
  {
    wdLog::Info("Archive uses older string hashing, recomputing hashes.");

    // version 1 stores an older way for the path/hash -> entry lookup table, which is prone to hash collisions
    // in this case, rebuild the new hash table on the fly
    //
    // version 2 used MurmurHash
    // version 3 switched to 32 bit xxHash
    // version 4 switched to 64 bit hashes

    const wdUInt32 uiNumEntries = m_Entries.GetCount();
    m_PathToEntryIndex.Clear();
    m_PathToEntryIndex.Reserve(uiNumEntries);

    wdStringBuilder sLowerCasePath;

    for (wdUInt32 i = 0; i < uiNumEntries; i++)
    {
      const wdUInt32 uiSrcStringOffset = m_Entries[i].m_uiPathStringOffset;

      const char* szEntryString = GetEntryPathString(i);

      sLowerCasePath = szEntryString;
      sLowerCasePath.ToLower();

      // cut off the upper 32 bit, we don't need them here
      const wdUInt32 uiLowerCaseHash = wdHashingUtils::StringHashTo32(wdHashingUtils::StringHash(sLowerCasePath.GetView()) & 0xFFFFFFFFllu);

      m_PathToEntryIndex.Insert(wdArchiveStoredString(uiLowerCaseHash, uiSrcStringOffset), i);

      // Verify that the conversion worked
      WD_ASSERT_DEBUG(FindEntry(szEntryString) == i, "Hashed path retrieval did not yield inserted index");
    }
  }

  // path strings mustn't be empty and must be zero-terminated
  if (m_AllPathStrings.IsEmpty() || m_AllPathStrings.PeekBack() != '\0')
  {
    wdLog::Error("Archive is corrupt. Invalid string data.");
    return WD_FAILURE;
  }

  return WD_SUCCESS;
}

wdResult wdArchiveEntry::Serialize(wdStreamWriter& inout_stream) const
{
  inout_stream << m_uiDataStartOffset;
  inout_stream << m_uiUncompressedDataSize;
  inout_stream << m_uiStoredDataSize;
  inout_stream << (wdUInt8)m_CompressionMode;
  inout_stream << m_uiPathStringOffset;

  return WD_SUCCESS;
}

wdResult wdArchiveEntry::Deserialize(wdStreamReader& inout_stream)
{
  inout_stream >> m_uiDataStartOffset;
  inout_stream >> m_uiUncompressedDataSize;
  inout_stream >> m_uiStoredDataSize;
  wdUInt8 uiCompressionMode = 0;
  inout_stream >> uiCompressionMode;
  m_CompressionMode = (wdArchiveCompressionMode)uiCompressionMode;
  inout_stream >> m_uiPathStringOffset;

  return WD_SUCCESS;
}


WD_STATICLINK_FILE(Foundation, Foundation_IO_Archive_Implementation_Archive);
