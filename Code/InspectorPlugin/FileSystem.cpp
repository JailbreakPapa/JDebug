#include <InspectorPlugin/InspectorPluginPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <Foundation/IO/FileSystem/FileSystem.h>

static nsInt32 s_iDataDirCounter = 0;
static nsMap<nsString, nsInt32, nsCompareHelper<nsString>, nsStaticsAllocatorWrapper> s_KnownDataDirs;

static void FileSystemEventHandler(const nsFileSystem::FileEvent& e)
{
  switch (e.m_EventType)
  {
    case nsFileSystem::FileEventType::AddDataDirectorySucceeded:
    {
      bool bExisted = false;
      auto it = s_KnownDataDirs.FindOrAdd(e.m_sFileOrDirectory, &bExisted);

      if (!bExisted)
      {
        it.Value() = s_iDataDirCounter;
        ++s_iDataDirCounter;
      }

      nsStringBuilder sName;
      sName.SetFormat("IO/DataDirs/Dir{0}", nsArgI(it.Value(), 2, true));

      nsStats::SetStat(sName.GetData(), e.m_sFileOrDirectory);
    }
    break;

    case nsFileSystem::FileEventType::RemoveDataDirectory:
    {
      auto it = s_KnownDataDirs.Find(e.m_sFileOrDirectory);

      if (!it.IsValid())
        break;

      nsStringBuilder sName;
      sName.SetFormat("IO/DataDirs/Dir{0}", nsArgI(it.Value(), 2, true));

      nsStats::RemoveStat(sName.GetData());
    }
    break;

    default:
      break;
  }
}

void AddFileSystemEventHandler()
{
  nsFileSystem::RegisterEventHandler(FileSystemEventHandler);
}

void RemoveFileSystemEventHandler()
{
  nsFileSystem::UnregisterEventHandler(FileSystemEventHandler);
}
