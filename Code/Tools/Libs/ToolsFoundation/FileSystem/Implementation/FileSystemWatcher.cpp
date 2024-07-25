#include <ToolsFoundation/ToolsFoundationDLL.h>

#if NS_ENABLED(NS_SUPPORTS_DIRECTORY_WATCHER)

#  include <ToolsFoundation/FileSystem/FileSystemWatcher.h>

#  include <Foundation/IO/DirectoryWatcher.h>
#  include <Foundation/IO/FileSystem/FileSystem.h>
#  include <Foundation/Logging/Log.h>
#  include <Foundation/Threading/DelegateTask.h>

////////////////////////////////////////////////////////////////////////
// nsAssetWatcher
////////////////////////////////////////////////////////////////////////

nsFileSystemWatcher::nsFileSystemWatcher(const nsApplicationFileSystemConfig& fileSystemConfig)
{
  m_FileSystemConfig = fileSystemConfig;
}


nsFileSystemWatcher::~nsFileSystemWatcher() = default;

void nsFileSystemWatcher::Initialize()
{
  NS_PROFILE_SCOPE("Initialize");

  for (auto& dd : m_FileSystemConfig.m_DataDirs)
  {
    nsStringBuilder sTemp;
    if (nsFileSystem::ResolveSpecialDirectory(dd.m_sDataDirSpecialPath, sTemp).Failed())
    {
      nsLog::Error("Failed to init directory watcher for dir '{0}'", dd.m_sDataDirSpecialPath);
      continue;
    }

    nsDirectoryWatcher* pWatcher = NS_DEFAULT_NEW(nsDirectoryWatcher);
    nsResult res = pWatcher->OpenDirectory(sTemp, nsDirectoryWatcher::Watch::Deletes | nsDirectoryWatcher::Watch::Writes | nsDirectoryWatcher::Watch::Creates | nsDirectoryWatcher::Watch::Renames | nsDirectoryWatcher::Watch::Subdirectories);

    if (res.Failed())
    {
      NS_DEFAULT_DELETE(pWatcher);
      nsLog::Error("Failed to init directory watcher for dir '{0}'", sTemp);
      continue;
    }

    m_Watchers.PushBack(pWatcher);
  }

  m_pWatcherTask = NS_DEFAULT_NEW(nsDelegateTask<void>, "Watcher Changes", nsTaskNesting::Never, [this]()
    {
      nsHybridArray<WatcherResult, 16> watcherResults;
      for (nsDirectoryWatcher* pWatcher : m_Watchers)
      {
        pWatcher->EnumerateChanges([pWatcher, &watcherResults](nsStringView sFilename, nsDirectoryWatcherAction action, nsDirectoryWatcherType type)
          { watcherResults.PushBack({sFilename, action, type}); });
      }
      for (const WatcherResult& res : watcherResults)
      {
        HandleWatcherChange(res);
      } //
    });
  // This is a separate task as these trigger callbacks which can potentially take a long time and we can't have the watcher changes task be blocked for so long or notifications might get lost.
  m_pNotifyTask = NS_DEFAULT_NEW(nsDelegateTask<void>, "Watcher Notify", nsTaskNesting::Never, [this]()
    { NotifyChanges(); });
}

void nsFileSystemWatcher::Deinitialize()
{
  m_bShutdown = true;
  nsTaskGroupID watcherGroup;
  nsTaskGroupID notifyGroup;
  {
    NS_LOCK(m_WatcherMutex);
    watcherGroup = m_WatcherGroup;
    notifyGroup = m_NotifyGroup;
  }
  nsTaskSystem::WaitForGroup(watcherGroup);
  nsTaskSystem::WaitForGroup(notifyGroup);
  {
    NS_LOCK(m_WatcherMutex);
    m_pWatcherTask.Clear();
    m_pNotifyTask.Clear();
    for (nsDirectoryWatcher* pWatcher : m_Watchers)
    {
      NS_DEFAULT_DELETE(pWatcher);
    }
    m_Watchers.Clear();
  }
}

void nsFileSystemWatcher::MainThreadTick()
{
  NS_PROFILE_SCOPE("nsAssetWatcherTick");
  NS_LOCK(m_WatcherMutex);
  if (!m_bShutdown && m_pWatcherTask && nsTaskSystem::IsTaskGroupFinished(m_WatcherGroup))
  {
    m_WatcherGroup = nsTaskSystem::StartSingleTask(m_pWatcherTask, nsTaskPriority::LongRunningHighPriority);
  }
  if (!m_bShutdown && m_pNotifyTask && nsTaskSystem::IsTaskGroupFinished(m_NotifyGroup))
  {
    m_NotifyGroup = nsTaskSystem::StartSingleTask(m_pNotifyTask, nsTaskPriority::LongRunningHighPriority);
  }
}


void nsFileSystemWatcher::NotifyChanges()
{
  auto NotifyChange = [this](const nsString& sAbsPath, nsFileSystemWatcherEvent::Type type)
  {
    nsFileSystemWatcherEvent e;
    e.m_sPath = sAbsPath;
    e.m_Type = type;
    m_Events.Broadcast(e);
  };

  // Files
  ConsumeEntry(m_FileAdded, nsFileSystemWatcherEvent::Type::FileAdded, NotifyChange);
  ConsumeEntry(m_FileChanged, nsFileSystemWatcherEvent::Type::FileChanged, NotifyChange);
  ConsumeEntry(m_FileRemoved, nsFileSystemWatcherEvent::Type::FileRemoved, NotifyChange);

  // Directories
  ConsumeEntry(m_DirectoryAdded, nsFileSystemWatcherEvent::Type::DirectoryAdded, NotifyChange);
  ConsumeEntry(m_DirectoryRemoved, nsFileSystemWatcherEvent::Type::DirectoryRemoved, NotifyChange);
}

void nsFileSystemWatcher::HandleWatcherChange(const WatcherResult& res)
{
  switch (res.m_Action)
  {
    case nsDirectoryWatcherAction::None:
      NS_ASSERT_DEV(false, "None event should never happen");
      break;
    case nsDirectoryWatcherAction::RenamedNewName:
    case nsDirectoryWatcherAction::Added:
    {
      if (res.m_Type == nsDirectoryWatcherType::Directory)
      {
        AddEntry(m_DirectoryAdded, res.m_sFile, s_AddedFrameDelay);
      }
      else
      {
        AddEntry(m_FileAdded, res.m_sFile, s_AddedFrameDelay);
      }
    }
    break;
    case nsDirectoryWatcherAction::RenamedOldName:
    case nsDirectoryWatcherAction::Removed:
    {
      if (res.m_Type == nsDirectoryWatcherType::Directory)
      {
        AddEntry(m_DirectoryRemoved, res.m_sFile, s_RemovedFrameDelay);
      }
      else
      {
        AddEntry(m_FileRemoved, res.m_sFile, s_RemovedFrameDelay);
      }
    }
    break;
    case nsDirectoryWatcherAction::Modified:
    {
      if (res.m_Type == nsDirectoryWatcherType::Directory)
      {
        // Can a directory even be modified? In any case, we ignore this change.
        // UpdateEntry(m_DirectoryRemoved, res.sFile, s_RemovedFrameDelay);
      }
      else
      {
        AddEntry(m_FileChanged, res.m_sFile, s_ModifiedFrameDelay);
      }
    }
    break;
  }
}

void nsFileSystemWatcher::AddEntry(nsDynamicArray<PendingUpdate>& container, const nsStringView sAbsPath, nsUInt32 uiFrameDelay)
{
  NS_LOCK(m_WatcherMutex);
  for (PendingUpdate& update : container)
  {
    if (update.m_sAbsPath == sAbsPath)
    {
      update.m_uiFrameDelay = uiFrameDelay;
      return;
    }
  }
  PendingUpdate& update = container.ExpandAndGetRef();
  update.m_uiFrameDelay = uiFrameDelay;
  update.m_sAbsPath = sAbsPath;
}

void nsFileSystemWatcher::ConsumeEntry(nsDynamicArray<PendingUpdate>& container, nsFileSystemWatcherEvent::Type type, const nsDelegate<void(const nsString& sAbsPath, nsFileSystemWatcherEvent::Type type)>& consume)
{
  nsHybridArray<PendingUpdate, 16> updates;
  {
    NS_LOCK(m_WatcherMutex);
    for (nsUInt32 i = container.GetCount(); i > 0; --i)
    {
      PendingUpdate& update = container[i - 1];
      --update.m_uiFrameDelay;
      if (update.m_uiFrameDelay == 0)
      {
        updates.PushBack(update);
        container.RemoveAtAndSwap(i - 1);
      }
    }
  }
  for (const PendingUpdate& update : updates)
  {
    consume(update.m_sAbsPath, type);
  }
}

#endif
