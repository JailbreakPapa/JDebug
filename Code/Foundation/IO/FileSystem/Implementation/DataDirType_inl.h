#pragma once

#include <Foundation/Strings/StringBuilder.h>

inline wdDataDirectoryReaderWriterBase::wdDataDirectoryReaderWriterBase(wdInt32 iDataDirUserData, bool bIsReader)
{
  m_iDataDirUserData = iDataDirUserData;
  m_pDataDirectory = nullptr;
  m_bIsReader = bIsReader;
}

inline wdResult wdDataDirectoryReaderWriterBase::Open(wdStringView sFile, wdDataDirectoryType* pDataDirectory, wdFileShareMode::Enum fileShareMode)
{
  m_pDataDirectory = pDataDirectory;
  m_sFilePath = sFile;

  return InternalOpen(fileShareMode);
}

inline const wdString128& wdDataDirectoryReaderWriterBase::GetFilePath() const
{
  return m_sFilePath;
}

inline wdDataDirectoryType* wdDataDirectoryReaderWriterBase::GetDataDirectory() const
{
  return m_pDataDirectory;
}
