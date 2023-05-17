#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Strings/HashedString.h>

class wdRawMemoryStreamReader;

/// \brief Compression modes for wdArchive file entries
enum class wdArchiveCompressionMode : wdUInt8
{
  Uncompressed,
  Compressed_zstd,
  Compressed_zip,
};

/// \brief Data for a single file entry in an wdArchive file
class WD_FOUNDATION_DLL wdArchiveEntry
{
public:
  wdUInt64 m_uiDataStartOffset = 0;      ///< Byte offset for where the file's (compressed) data stream starts in the wdArchive
  wdUInt64 m_uiUncompressedDataSize = 0; ///< Size of the original uncompressed data.
  wdUInt64 m_uiStoredDataSize = 0;       ///< The amount of (compressed) bytes actually stored in the wdArchive.
  wdUInt32 m_uiPathStringOffset = 0;     ///< Byte offset into wdArchiveTOC::m_AllPathStrings where the path string for this entry resides.
  wdArchiveCompressionMode m_CompressionMode = wdArchiveCompressionMode::Uncompressed;

  wdResult Serialize(wdStreamWriter& inout_stream) const;
  wdResult Deserialize(wdStreamReader& inout_stream);
};

/// \brief Helper class to store a hashed string for quick lookup in the archive TOC
///
/// Stores a hash of the lower case string for quick comparison.
/// Additionally stores an offset into the wdArchiveTOC::m_AllPathStrings array for final validation, to prevent hash collisions.
/// The proper string lookup with hash collision check only works together with wdArchiveLookupString, which has the necessary context
/// to index the wdArchiveTOC::m_AllPathStrings array.
class WD_FOUNDATION_DLL wdArchiveStoredString
{
public:
  WD_DECLARE_POD_TYPE();

  wdArchiveStoredString() = default;

  wdArchiveStoredString(wdUInt64 uiLowerCaseHash, wdUInt32 uiSrcStringOffset)
    : m_uiLowerCaseHash(wdHashingUtils::StringHashTo32(uiLowerCaseHash))
    , m_uiSrcStringOffset(uiSrcStringOffset)
  {
  }

  wdUInt32 m_uiLowerCaseHash;
  wdUInt32 m_uiSrcStringOffset;
};

void operator<<(wdStreamWriter& inout_stream, const wdArchiveStoredString& value);
void operator>>(wdStreamReader& inout_stream, wdArchiveStoredString& value);

/// \brief Helper class for looking up path strings in wdArchiveTOC::FindEntry()
///
/// Only works together with wdArchiveStoredString.
class wdArchiveLookupString
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdArchiveLookupString);

public:
  WD_DECLARE_POD_TYPE();

  wdArchiveLookupString(wdUInt64 uiLowerCaseHash, const char* szString, const wdDynamicArray<wdUInt8>& archiveAllPathStrings)
    : m_uiLowerCaseHash(wdHashingUtils::StringHashTo32(uiLowerCaseHash))
    , m_szString(szString)
    , m_ArchiveAllPathStrings(archiveAllPathStrings)
  {
  }

  wdUInt32 m_uiLowerCaseHash;
  const char* m_szString = nullptr;
  const wdDynamicArray<wdUInt8>& m_ArchiveAllPathStrings;
};

/// \brief Functions to enable wdHashTable to 1) store wdArchiveStoredString and 2) lookup strings efficiently with a wdArchiveLookupString
template <>
struct wdHashHelper<wdArchiveStoredString>
{
  WD_ALWAYS_INLINE static wdUInt32 Hash(const wdArchiveStoredString& hs) { return hs.m_uiLowerCaseHash; }
  WD_ALWAYS_INLINE static wdUInt32 Hash(const wdArchiveLookupString& hs) { return hs.m_uiLowerCaseHash; }

  WD_ALWAYS_INLINE static bool Equal(const wdArchiveStoredString& a, const wdArchiveStoredString& b) { return a.m_uiSrcStringOffset == b.m_uiSrcStringOffset; }

  WD_ALWAYS_INLINE static bool Equal(const wdArchiveStoredString& a, const wdArchiveLookupString& b)
  {
    // in case that we want to lookup a string using a wdArchiveLookupString, we validate
    // that the stored string is actually equal to the lookup string, to enable handling of hash collisions
    return wdStringUtils::IsEqual_NoCase(reinterpret_cast<const char*>(&b.m_ArchiveAllPathStrings[a.m_uiSrcStringOffset]), b.m_szString);
  }
};

/// \brief Table-of-contents for an wdArchive file
class WD_FOUNDATION_DLL wdArchiveTOC
{
public:
  /// all files stored in the wdArchive
  wdDynamicArray<wdArchiveEntry> m_Entries;
  /// allows to map a hashed string to the index of the file entry for the file path
  wdHashTable<wdArchiveStoredString, wdUInt32> m_PathToEntryIndex;
  /// one large array holding all path strings for the file entries, to reduce allocations
  wdDynamicArray<wdUInt8> m_AllPathStrings;

  /// \brief Returns the entry index for the given file or wdInvalidIndex, if not found.
  wdUInt32 FindEntry(const char* szFile) const;

  const char* GetEntryPathString(wdUInt32 uiEntryIdx) const;

  wdResult Serialize(wdStreamWriter& inout_stream) const;
  wdResult Deserialize(wdStreamReader& inout_stream, wdUInt8 uiArchiveVersion);
};
