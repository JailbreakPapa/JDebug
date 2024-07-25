#pragma once

#include <ToolsFoundation/ToolsFoundationDLL.h>

#if NS_ENABLED(NS_SUPPORTS_DIRECTORY_WATCHER)

#  include <Foundation/Application/Config/FileSystemConfig.h>
#  include <Foundation/IO/DirectoryWatcher.h>
#  include <Foundation/Threading/TaskSystem.h>

class nsTask;

/// \brief Event fired by nsFileSystemWatcher::m_Events.
struct nsFileSystemWatcherEvent
{
  enum class Type
  {
    FileAdded,
    FileRemoved,
    FileChanged,
    DirectoryAdded,
    DirectoryRemoved,
  };

  nsStringView m_sPath;
  Type m_Type;
};

/// \brief Creates a file system watcher for the given filesystem config and fires any changes on a worker task via an event.
class NS_TOOLSFOUNDATION_DLL nsFileSystemWatcher
{
public:
  nsFileSystemWatcher(const nsApplicationFileSystemConfig& fileSystemConfig);
  ~nsFileSystemWatcher();

  /// \brief Once called, file system watchers are created for each data directory and changes are observed.
  void Initialize();

  /// \brief Waits for all pending tasks to complete and then stops observing changes and destroys file system watchers.
  void Deinitialize();

  /// \brief Needs to be called at regular intervals (e.g. each frame) to restart background tasks.
  void MainThreadTick();

public:
  nsEvent<const nsFileSystemWatcherEvent&, nsMutex> m_Events;

private:
  // On file move / rename operations we want the new file to be seen first before the old file delete event so that we can correctly detect this as a move instead of a delete operation. We achieve this by delaying each event by a fixed number of frames.
  static constexpr nsUInt32 s_AddedFrameDelay = 5;
  static constexpr nsUInt32 s_RemovedFrameDelay = 10;
  // Sometimes moving a file triggers a modified event on the old file. To prevent this from triggering the removal to be seen before the addition, we also delay modified events by the same amount as remove events.
  static constexpr nsUInt32 s_ModifiedFrameDelay = 10;

  struct WatcherResult
  {
    nsString m_sFile;
    nsDirectoryWatcherAction m_Action;
    nsDirectoryWatcherType m_Type;
  };

  struct PendingUpdate
  {
    nsString m_sAbsPath;
    nsUInt32 m_uiFrameDelay = 0;
  };

  /// \brief Handles a single change notification by a directory watcher.
  void HandleWatcherChange(const WatcherResult& res);
  /// \brief Handles update delays to allow compacting multiple changes.
  void NotifyChanges();
  /// \brief Adds a change with the given delay to the container. If the entry is already present, only its delay is increased.
  void AddEntry(nsDynamicArray<PendingUpdate>& container, const nsStringView sAbsPath, nsUInt32 uiFrameDelay);
  /// \brief Reduces the delay counter of every item in the container. If a delay reaches zero, it is removed and the callback is fired.
  void ConsumeEntry(nsDynamicArray<PendingUpdate>& container, nsFileSystemWatcherEvent::Type type, const nsDelegate<void(const nsString& sAbsPath, nsFileSystemWatcherEvent::Type type)>& consume);

private:
  // Immutable data after StartInitialize
  nsApplicationFileSystemConfig m_FileSystemConfig;

  // Watchers
  mutable nsMutex m_WatcherMutex;
  nsHybridArray<nsDirectoryWatcher*, 6> m_Watchers;
  nsSharedPtr<nsTask> m_pWatcherTask;
  nsSharedPtr<nsTask> m_pNotifyTask;
  nsTaskGroupID m_WatcherGroup;
  nsTaskGroupID m_NotifyGroup;
  nsAtomicBool m_bShutdown = false;

  // Pending ops
  nsHybridArray<PendingUpdate, 4> m_FileAdded;
  nsHybridArray<PendingUpdate, 4> m_FileRemoved;
  nsHybridArray<PendingUpdate, 4> m_FileChanged;
  nsHybridArray<PendingUpdate, 4> m_DirectoryAdded;
  nsHybridArray<PendingUpdate, 4> m_DirectoryRemoved;
};

#endif
