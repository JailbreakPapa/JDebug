#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Strings/HashedString.h>

class nsRawMemoryStreamReader;

/// \brief Compression modes for nsArchive file entries
enum class nsArchiveCompressionMode : nsUInt8
{
  Uncompressed,
  Compressed_zstd,
  Compressed_zip,
};

/// \brief Data for a single file entry in an nsArchive file
class NS_FOUNDATION_DLL nsArchiveEntry
{
public:
  nsUInt64 m_uiDataStartOffset = 0;      ///< Byte offset for where the file's (compressed) data stream starts in the nsArchive
  nsUInt64 m_uiUncompressedDataSize = 0; ///< Size of the original uncompressed data.
  nsUInt64 m_uiStoredDataSize = 0;       ///< The amount of (compressed) bytes actually stored in the nsArchive.
  nsUInt32 m_uiPathStringOffset = 0;     ///< Byte offset into nsArchiveTOC::m_AllPathStrings where the path string for this entry resides.
  nsArchiveCompressionMode m_CompressionMode = nsArchiveCompressionMode::Uncompressed;

  nsResult Serialize(nsStreamWriter& inout_stream) const;
  nsResult Deserialize(nsStreamReader& inout_stream);
};

/// \brief Helper class to store a hashed string for quick lookup in the archive TOC
///
/// Stores a hash of the lower case string for quick comparison.
/// Additionally stores an offset into the nsArchiveTOC::m_AllPathStrings array for final validation, to prevent hash collisions.
/// The proper string lookup with hash collision check only works together with nsArchiveLookupString, which has the necessary context
/// to index the nsArchiveTOC::m_AllPathStrings array.
class NS_FOUNDATION_DLL nsArchiveStoredString
{
public:
  NS_DECLARE_POD_TYPE();

  nsArchiveStoredString() = default;

  nsArchiveStoredString(nsUInt64 uiLowerCaseHash, nsUInt32 uiSrcStringOffset)
    : m_uiLowerCaseHash(nsHashingUtils::StringHashTo32(uiLowerCaseHash))
    , m_uiSrcStringOffset(uiSrcStringOffset)
  {
  }

  nsUInt32 m_uiLowerCaseHash;
  nsUInt32 m_uiSrcStringOffset;
};

void operator<<(nsStreamWriter& inout_stream, const nsArchiveStoredString& value);
void operator>>(nsStreamReader& inout_stream, nsArchiveStoredString& value);

/// \brief Helper class for looking up path strings in nsArchiveTOC::FindEntry()
///
/// Only works together with nsArchiveStoredString.
class nsArchiveLookupString
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsArchiveLookupString);

public:
  NS_DECLARE_POD_TYPE();

  nsArchiveLookupString(nsUInt64 uiLowerCaseHash, nsStringView sString, const nsDynamicArray<nsUInt8>& archiveAllPathStrings)
    : m_uiLowerCaseHash(nsHashingUtils::StringHashTo32(uiLowerCaseHash))
    , m_sString(sString)
    , m_ArchiveAllPathStrings(archiveAllPathStrings)
  {
  }

  nsUInt32 m_uiLowerCaseHash;
  nsStringView m_sString;
  const nsDynamicArray<nsUInt8>& m_ArchiveAllPathStrings;
};

/// \brief Functions to enable nsHashTable to 1) store nsArchiveStoredString and 2) lookup strings efficiently with a nsArchiveLookupString
template <>
struct nsHashHelper<nsArchiveStoredString>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(const nsArchiveStoredString& hs) { return hs.m_uiLowerCaseHash; }
  NS_ALWAYS_INLINE static nsUInt32 Hash(const nsArchiveLookupString& hs) { return hs.m_uiLowerCaseHash; }

  NS_ALWAYS_INLINE static bool Equal(const nsArchiveStoredString& a, const nsArchiveStoredString& b) { return a.m_uiSrcStringOffset == b.m_uiSrcStringOffset; }

  NS_ALWAYS_INLINE static bool Equal(const nsArchiveStoredString& a, const nsArchiveLookupString& b)
  {
    // in case that we want to lookup a string using a nsArchiveLookupString, we validate
    // that the stored string is actually equal to the lookup string, to enable handling of hash collisions
    return b.m_sString.IsEqual_NoCase(reinterpret_cast<const char*>(&b.m_ArchiveAllPathStrings[a.m_uiSrcStringOffset]));
  }
};

/// \brief Table-of-contents for an nsArchive file
class NS_FOUNDATION_DLL nsArchiveTOC
{
public:
  /// all files stored in the nsArchive
  nsDynamicArray<nsArchiveEntry> m_Entries;
  /// allows to map a hashed string to the index of the file entry for the file path
  nsHashTable<nsArchiveStoredString, nsUInt32> m_PathToEntryIndex;
  /// one large array holding all path strings for the file entries, to reduce allocations
  nsDynamicArray<nsUInt8> m_AllPathStrings;

  /// \brief Returns the entry index for the given file or nsInvalidIndex, if not found.
  nsUInt32 FindEntry(nsStringView sFile) const;

  nsUInt32 AddPathString(nsStringView sPathString);

  void RebuildPathToEntryHashes();

  nsStringView GetEntryPathString(nsUInt32 uiEntryIdx) const;

  nsResult Serialize(nsStreamWriter& inout_stream) const;
  nsResult Deserialize(nsStreamReader& inout_stream, nsUInt8 uiArchiveVersion);
};
