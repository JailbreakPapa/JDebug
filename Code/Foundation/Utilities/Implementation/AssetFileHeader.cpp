#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Utilities/AssetFileHeader.h>

static const char* g_szAssetTag = "nsAsset";

nsAssetFileHeader::nsAssetFileHeader() = default;

enum nsAssetFileHeaderVersion : nsUInt8
{
  Version1 = 1,
  Version2,
  Version3,

  VersionCount,
  VersionCurrent = VersionCount - 1
};

nsResult nsAssetFileHeader::Write(nsStreamWriter& inout_stream) const
{
  NS_ASSERT_DEBUG(m_uiHash != 0xFFFFFFFFFFFFFFFF, "Cannot write an invalid hash to file");

  // 8 Bytes for identification + version
  NS_SUCCEED_OR_RETURN(inout_stream.WriteBytes(g_szAssetTag, 7));

  const nsUInt8 uiVersion = nsAssetFileHeaderVersion::VersionCurrent;
  inout_stream << uiVersion;

  // 8 Bytes for the hash
  inout_stream << m_uiHash;
  // 2 for the type version
  inout_stream << m_uiVersion;

  inout_stream << m_sGenerator;
  return NS_SUCCESS;
}

nsResult nsAssetFileHeader::Read(nsStreamReader& inout_stream)
{
  // initialize to 'invalid'
  m_uiHash = 0xFFFFFFFFFFFFFFFF;
  m_uiVersion = 0;

  char szTag[8] = {0};
  if (inout_stream.ReadBytes(szTag, 7) < 7)
  {
    NS_REPORT_FAILURE("The stream does not contain a valid asset file header");
    return NS_FAILURE;
  }

  szTag[7] = '\0';

  // invalid asset file ... this is not going to end well
  NS_ASSERT_DEBUG(nsStringUtils::IsEqual(szTag, g_szAssetTag), "The stream does not contain a valid asset file header");

  if (!nsStringUtils::IsEqual(szTag, g_szAssetTag))
    return NS_FAILURE;

  nsUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  nsUInt64 uiHash = 0;
  inout_stream >> uiHash;

  // future version?
  NS_ASSERT_DEV(uiVersion <= nsAssetFileHeaderVersion::VersionCurrent, "Unknown asset header version {0}", uiVersion);

  if (uiVersion >= nsAssetFileHeaderVersion::Version2)
  {
    inout_stream >> m_uiVersion;
  }

  if (uiVersion >= nsAssetFileHeaderVersion::Version3)
  {
    inout_stream >> m_sGenerator;
  }

  // older version? set the hash to 'invalid'
  if (uiVersion != nsAssetFileHeaderVersion::VersionCurrent)
    return NS_FAILURE;

  m_uiHash = uiHash;

  return NS_SUCCESS;
}
