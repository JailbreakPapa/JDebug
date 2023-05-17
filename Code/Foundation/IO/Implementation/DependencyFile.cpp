#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/DependencyFile.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>

enum class wdDependencyFileVersion : wdUInt8
{
  Version0 = 0,
  Version1,
  Version2, ///< added 'sum' time

  ENUM_COUNT,
  Current = ENUM_COUNT - 1,
};

wdMap<wdString, wdDependencyFile::FileCheckCache> wdDependencyFile::s_FileTimestamps;

wdDependencyFile::wdDependencyFile()
{
  Clear();
}

void wdDependencyFile::Clear()
{
  m_iMaxTimeStampStored = 0;
  m_uiSumTimeStampStored = 0;
  m_AssetTransformDependencies.Clear();
}

void wdDependencyFile::AddFileDependency(wdStringView sFile)
{
  if (sFile.IsEmpty())
    return;

  m_AssetTransformDependencies.PushBack(sFile);
}

void wdDependencyFile::StoreCurrentTimeStamp()
{
  WD_LOG_BLOCK("wdDependencyFile::StoreCurrentTimeStamp");

  m_iMaxTimeStampStored = 0;
  m_uiSumTimeStampStored = 0;

#if WD_DISABLED(WD_SUPPORTS_FILE_STATS)
  wdLog::Warning("Trying to retrieve file time stamps on a platform that does not support it");
  return;
#endif

  for (const auto& sFile : m_AssetTransformDependencies)
  {
    wdTimestamp ts;
    if (RetrieveFileTimeStamp(sFile, ts).Failed())
      continue;

    const wdInt64 time = ts.GetInt64(wdSIUnitOfTime::Second);
    m_iMaxTimeStampStored = wdMath::Max<wdInt64>(m_iMaxTimeStampStored, time);
    m_uiSumTimeStampStored += (wdUInt64)time;
  }
}

bool wdDependencyFile::HasAnyFileChanged() const
{
#if WD_DISABLED(WD_SUPPORTS_FILE_STATS)
  wdLog::Warning("Trying to retrieve file time stamps on a platform that does not support it");
  return true;
#endif

  wdUInt64 uiSumTs = 0;

  for (const auto& sFile : m_AssetTransformDependencies)
  {
    wdTimestamp ts;
    if (RetrieveFileTimeStamp(sFile, ts).Failed())
      continue;

    const wdInt64 time = ts.GetInt64(wdSIUnitOfTime::Second);

    if (time > m_iMaxTimeStampStored)
    {
      wdLog::Dev("Detected file change in '{0}' (TimeStamp {1} > MaxTimeStamp {2})", wdArgSensitive(sFile, "File"),
        ts.GetInt64(wdSIUnitOfTime::Second), m_iMaxTimeStampStored);
      return true;
    }

    uiSumTs += (wdUInt64)time;
  }

  if (uiSumTs != m_uiSumTimeStampStored)
  {
    wdLog::Dev("Detected file change, but exact file is not known.");
    return true;
  }

  return false;
}

wdResult wdDependencyFile::WriteDependencyFile(wdStreamWriter& inout_stream) const
{
  inout_stream << (wdUInt8)wdDependencyFileVersion::Current;

  inout_stream << m_iMaxTimeStampStored;
  inout_stream << m_uiSumTimeStampStored;
  inout_stream << m_AssetTransformDependencies.GetCount();

  for (const auto& sFile : m_AssetTransformDependencies)
    inout_stream << sFile;

  return WD_SUCCESS;
}

wdResult wdDependencyFile::ReadDependencyFile(wdStreamReader& inout_stream)
{
  wdUInt8 uiVersion = (wdUInt8)wdDependencyFileVersion::Version0;
  inout_stream >> uiVersion;

  if (uiVersion > (wdUInt8)wdDependencyFileVersion::Current)
  {
    wdLog::Error("Dependency file has incorrect file version ({0})", uiVersion);
    return WD_FAILURE;
  }

  WD_ASSERT_DEV(uiVersion <= (wdUInt8)wdDependencyFileVersion::Current, "Invalid file version {0}", uiVersion);

  inout_stream >> m_iMaxTimeStampStored;

  if (uiVersion >= (wdUInt8)wdDependencyFileVersion::Version2)
  {
    inout_stream >> m_uiSumTimeStampStored;
  }

  wdUInt32 count = 0;
  inout_stream >> count;
  m_AssetTransformDependencies.SetCount(count);

  for (wdUInt32 i = 0; i < m_AssetTransformDependencies.GetCount(); ++i)
    inout_stream >> m_AssetTransformDependencies[i];

  return WD_SUCCESS;
}

wdResult wdDependencyFile::RetrieveFileTimeStamp(wdStringView sFile, wdTimestamp& out_Result)
{
#if WD_ENABLED(WD_SUPPORTS_FILE_STATS)

  bool bExisted = false;
  auto it = s_FileTimestamps.FindOrAdd(sFile, &bExisted);

  if (!bExisted || it.Value().m_LastCheck + wdTime::Seconds(2.0) < wdTime::Now())
  {
    it.Value().m_LastCheck = wdTime::Now();

    wdFileStats stats;
    if (wdFileSystem::GetFileStats(sFile, stats).Failed())
    {
      wdLog::Error("Could not query the file stats for '{0}'", wdArgSensitive(sFile, "File"));
      return WD_FAILURE;
    }

    it.Value().m_FileTimestamp = stats.m_LastModificationTime;
  }

  out_Result = it.Value().m_FileTimestamp;

#else

  out_Result.SetInt64(0, wdSIUnitOfTime::Second);
  wdLog::Warning("Trying to retrieve a file time stamp on a platform that does not support it (file: '{0}')", wdArgSensitive(szFile, "File"));

#endif

  return out_Result.IsValid() ? WD_SUCCESS : WD_FAILURE;
}

wdResult wdDependencyFile::WriteDependencyFile(wdStringView sFile) const
{
  WD_LOG_BLOCK("wdDependencyFile::WriteDependencyFile", sFile);

  wdFileWriter file;
  if (file.Open(sFile).Failed())
    return WD_FAILURE;

  return WriteDependencyFile(file);
}

wdResult wdDependencyFile::ReadDependencyFile(wdStringView sFile)
{
  WD_LOG_BLOCK("wdDependencyFile::ReadDependencyFile", sFile);

  wdFileReader file;
  if (file.Open(sFile).Failed())
    return WD_FAILURE;

  return ReadDependencyFile(file);
}



WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_DependencyFile);
