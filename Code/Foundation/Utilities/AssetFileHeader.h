#pragma once

#include <Foundation/IO/Stream.h>
#include <Foundation/Strings/HashedString.h>

/// \brief Simple class to handle asset file headers (the very first bytes in all transformed asset files)
class NS_FOUNDATION_DLL nsAssetFileHeader
{
public:
  nsAssetFileHeader();

  /// \brief Reads the hash from file. If the file is outdated, the hash is set to 0xFFFFFFFFFFFFFFFF.
  nsResult Read(nsStreamReader& inout_stream);

  /// \brief Writes the asset hash to file (plus a little version info)
  nsResult Write(nsStreamWriter& inout_stream) const;

  /// \brief Checks whether the stored file contains the same hash.
  bool IsFileUpToDate(nsUInt64 uiExpectedHash, nsUInt16 uiVersion) const { return (m_uiHash == uiExpectedHash && m_uiVersion == uiVersion); }

  /// \brief Returns the asset file hash
  nsUInt64 GetFileHash() const { return m_uiHash; }

  /// \brief Sets the asset file hash
  void SetFileHashAndVersion(nsUInt64 uiHash, nsUInt16 v)
  {
    m_uiHash = uiHash;
    m_uiVersion = v;
  }

  /// \brief Returns the asset type version
  nsUInt16 GetFileVersion() const { return m_uiVersion; }

  /// \brief Returns the generator which was used to produce the asset file
  const nsHashedString& GetGenerator() { return m_sGenerator; }

  /// \brief Allows to set the generator string
  void SetGenerator(nsStringView sGenerator) { m_sGenerator.Assign(sGenerator); }

private:
  // initialize to a 'valid' hash
  // this may get stored, unless someone sets the hash
  nsUInt64 m_uiHash = 0;
  nsUInt16 m_uiVersion = 0;
  nsHashedString m_sGenerator;
};
