#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/DependencyFile.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>

enum class nsDependencyFileVersion : nsUInt8
{
  Version0 = 0,
  Version1,
  Version2, ///< added 'sum' time

  ENUM_COUNT,
  Current = ENUM_COUNT - 1,
};

nsMap<nsString, nsDependencyFile::FileCheckCache> nsDependencyFile::s_FileTimestamps;

nsDependencyFile::nsDependencyFile()
{
  Clear();
}

void nsDependencyFile::Clear()
{
  m_iMaxTimeStampStored = 0;
  m_uiSumTimeStampStored = 0;
  m_AssetTransformDependencies.Clear();
}

void nsDependencyFile::AddFileDependency(nsStringView sFile)
{
  if (sFile.IsEmpty())
    return;

  m_AssetTransformDependencies.PushBack(sFile);
}

void nsDependencyFile::StoreCurrentTimeStamp()
{
  NS_LOG_BLOCK("nsDependencyFile::StoreCurrentTimeStamp");

  m_iMaxTimeStampStored = 0;
  m_uiSumTimeStampStored = 0;

#if NS_DISABLED(NS_SUPPORTS_FILE_STATS)
  nsLog::Warning("Trying to retrieve file time stamps on a platform that does not support it");
  return;
#endif

  for (const auto& sFile : m_AssetTransformDependencies)
  {
    nsTimestamp ts;
    if (RetrieveFileTimeStamp(sFile, ts).Failed())
      continue;

    const nsInt64 time = ts.GetInt64(nsSIUnitOfTime::Second);
    m_iMaxTimeStampStored = nsMath::Max<nsInt64>(m_iMaxTimeStampStored, time);
    m_uiSumTimeStampStored += (nsUInt64)time;
  }
}

bool nsDependencyFile::HasAnyFileChanged() const
{
#if NS_DISABLED(NS_SUPPORTS_FILE_STATS)
  nsLog::Warning("Trying to retrieve file time stamps on a platform that does not support it");
  return true;
#endif

  nsUInt64 uiSumTs = 0;

  for (const auto& sFile : m_AssetTransformDependencies)
  {
    nsTimestamp ts;
    if (RetrieveFileTimeStamp(sFile, ts).Failed())
      continue;

    const nsInt64 time = ts.GetInt64(nsSIUnitOfTime::Second);

    if (time > m_iMaxTimeStampStored)
    {
      nsLog::Dev("Detected file change in '{0}' (TimeStamp {1} > MaxTimeStamp {2})", nsArgSensitive(sFile, "File"),
        ts.GetInt64(nsSIUnitOfTime::Second), m_iMaxTimeStampStored);
      return true;
    }

    uiSumTs += (nsUInt64)time;
  }

  if (uiSumTs != m_uiSumTimeStampStored)
  {
    nsLog::Dev("Detected file change, but exact file is not known.");
    return true;
  }

  return false;
}

nsResult nsDependencyFile::WriteDependencyFile(nsStreamWriter& inout_stream) const
{
  inout_stream << (nsUInt8)nsDependencyFileVersion::Current;

  inout_stream << m_iMaxTimeStampStored;
  inout_stream << m_uiSumTimeStampStored;
  inout_stream << m_AssetTransformDependencies.GetCount();

  for (const auto& sFile : m_AssetTransformDependencies)
    inout_stream << sFile;

  return NS_SUCCESS;
}

nsResult nsDependencyFile::ReadDependencyFile(nsStreamReader& inout_stream)
{
  nsUInt8 uiVersion = (nsUInt8)nsDependencyFileVersion::Version0;
  inout_stream >> uiVersion;

  if (uiVersion > (nsUInt8)nsDependencyFileVersion::Current)
  {
    nsLog::Error("Dependency file has incorrect file version ({0})", uiVersion);
    return NS_FAILURE;
  }

  NS_ASSERT_DEV(uiVersion <= (nsUInt8)nsDependencyFileVersion::Current, "Invalid file version {0}", uiVersion);

  inout_stream >> m_iMaxTimeStampStored;

  if (uiVersion >= (nsUInt8)nsDependencyFileVersion::Version2)
  {
    inout_stream >> m_uiSumTimeStampStored;
  }

  nsUInt32 count = 0;
  inout_stream >> count;
  m_AssetTransformDependencies.SetCount(count);

  for (nsUInt32 i = 0; i < m_AssetTransformDependencies.GetCount(); ++i)
    inout_stream >> m_AssetTransformDependencies[i];

  return NS_SUCCESS;
}

nsResult nsDependencyFile::RetrieveFileTimeStamp(nsStringView sFile, nsTimestamp& out_Result)
{
#if NS_ENABLED(NS_SUPPORTS_FILE_STATS)

  bool bExisted = false;
  auto it = s_FileTimestamps.FindOrAdd(sFile, &bExisted);

  if (!bExisted || it.Value().m_LastCheck + nsTime::MakeFromSeconds(2.0) < nsTime::Now())
  {
    it.Value().m_LastCheck = nsTime::Now();

    nsFileStats stats;
    if (nsFileSystem::GetFileStats(sFile, stats).Failed())
    {
      nsLog::Error("Could not query the file stats for '{0}'", nsArgSensitive(sFile, "File"));
      return NS_FAILURE;
    }

    it.Value().m_FileTimestamp = stats.m_LastModificationTime;
  }

  out_Result = it.Value().m_FileTimestamp;

#else

  out_Result = nsTimestamp::MakeFromInt(0, nsSIUnitOfTime::Second);
  nsLog::Warning("Trying to retrieve a file time stamp on a platform that does not support it (file: '{0}')", nsArgSensitive(sFile, "File"));

#endif

  return out_Result.IsValid() ? NS_SUCCESS : NS_FAILURE;
}

nsResult nsDependencyFile::WriteDependencyFile(nsStringView sFile) const
{
  NS_LOG_BLOCK("nsDependencyFile::WriteDependencyFile", sFile);

  nsFileWriter file;
  if (file.Open(sFile).Failed())
    return NS_FAILURE;

  return WriteDependencyFile(file);
}

nsResult nsDependencyFile::ReadDependencyFile(nsStringView sFile)
{
  NS_LOG_BLOCK("nsDependencyFile::ReadDependencyFile", sFile);

  nsFileReader file;
  if (file.Open(sFile).Failed())
    return NS_FAILURE;

  return ReadDependencyFile(file);
}
