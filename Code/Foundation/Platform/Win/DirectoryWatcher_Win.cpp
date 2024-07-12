#include <Foundation/FoundationPCH.h>

#if (NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP) && NS_ENABLED(NS_SUPPORTS_DIRECTORY_WATCHER))

#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#  include <Foundation/Containers/DynamicArray.h>
#  include <Foundation/IO/DirectoryWatcher.h>
#  include <Foundation/IO/Implementation/FileSystemMirror.h>
#  include <Foundation/Logging/Log.h>
#  include <Foundation/Platform/Win/DosDevicePath_Win.h>

// Comment in to get verbose output on the function of the directory watcher
// #define DEBUG_FILE_WATCHER

#  ifdef DEBUG_FILE_WATCHER
#    define DEBUG_LOG(...) nsLog::Warning(__VA_ARGS__)
#  else
#    define DEBUG_LOG(...)
#  endif

namespace
{
  struct MoveEvent
  {
    nsString path;
    bool isDirectory = false;

    void Clear()
    {
      path.Clear();
    }

    bool IsEmpty()
    {
      return path.IsEmpty();
    }
  };

  using nsFileSystemMirrorType = nsFileSystemMirror<bool>;
} // namespace

struct nsDirectoryWatcherImpl
{
  void DoRead();

  HANDLE m_directoryHandle;
  DWORD m_filter;
  OVERLAPPED m_overlapped;
  HANDLE m_overlappedEvent;
  nsDynamicArray<nsUInt8> m_buffer;
  nsBitflags<nsDirectoryWatcher::Watch> m_whatToWatch;
  nsUniquePtr<nsFileSystemMirrorType> m_mirror; // store the last modification timestamp alongside each file
};

nsDirectoryWatcher::nsDirectoryWatcher()
  : m_pImpl(NS_DEFAULT_NEW(nsDirectoryWatcherImpl))
{
  m_pImpl->m_buffer.SetCountUninitialized(1024 * 1024);
}

nsResult nsDirectoryWatcher::OpenDirectory(nsStringView sAbsolutePath, nsBitflags<Watch> whatToWatch)
{
  NS_ASSERT_DEV(m_sDirectoryPath.IsEmpty(), "Directory already open, call CloseDirectory first!");
  nsStringBuilder sPath(sAbsolutePath);
  sPath.MakeCleanPath();
  sPath.Trim("/");

  m_pImpl->m_whatToWatch = whatToWatch;
  m_pImpl->m_filter = FILE_NOTIFY_CHANGE_FILE_NAME;
  if (whatToWatch.IsSet(Watch::Writes) || whatToWatch.AreAllSet(Watch::Deletes | Watch::Subdirectories))
  {
    m_pImpl->m_filter |= FILE_NOTIFY_CHANGE_LAST_WRITE;
    m_pImpl->m_mirror = NS_DEFAULT_NEW(nsFileSystemMirrorType);
    m_pImpl->m_mirror->AddDirectory(sPath).AssertSuccess();
  }

  if (whatToWatch.IsAnySet(Watch::Deletes | Watch::Creates | Watch::Renames))
  {
    m_pImpl->m_filter |= FILE_NOTIFY_CHANGE_DIR_NAME;
  }

  m_pImpl->m_directoryHandle = CreateFileW(nsDosDevicePath(sPath), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);
  if (m_pImpl->m_directoryHandle == INVALID_HANDLE_VALUE)
  {
    return NS_FAILURE;
  }

  m_pImpl->m_overlappedEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
  if (m_pImpl->m_overlappedEvent == INVALID_HANDLE_VALUE)
  {
    return NS_FAILURE;
  }

  m_pImpl->DoRead();
  m_sDirectoryPath = sPath;

  return NS_SUCCESS;
}

void nsDirectoryWatcher::CloseDirectory()
{
  if (!m_sDirectoryPath.IsEmpty())
  {
    CancelIo(m_pImpl->m_directoryHandle);
    CloseHandle(m_pImpl->m_overlappedEvent);
    CloseHandle(m_pImpl->m_directoryHandle);
    m_sDirectoryPath.Clear();
  }
}

nsDirectoryWatcher::~nsDirectoryWatcher()
{
  CloseDirectory();
  NS_DEFAULT_DELETE(m_pImpl);
}

void nsDirectoryWatcherImpl::DoRead()
{
  ResetEvent(m_overlappedEvent);
  memset(&m_overlapped, 0, sizeof(m_overlapped));
  m_overlapped.hEvent = m_overlappedEvent;
  BOOL success =
    ReadDirectoryChangesExW(m_directoryHandle, m_buffer.GetData(), m_buffer.GetCount(), m_whatToWatch.IsSet(nsDirectoryWatcher::Watch::Subdirectories), m_filter, nullptr, &m_overlapped, nullptr, ReadDirectoryNotifyExtendedInformation);
  NS_ASSERT_DEV(success, "ReadDirectoryChangesW failed.");
}

void nsDirectoryWatcher::EnumerateChanges(EnumerateChangesFunction func, nsTime waitUpTo)
{
  MoveEvent pendingRemoveOrRename;
  const nsBitflags<nsDirectoryWatcher::Watch> whatToWatch = m_pImpl->m_whatToWatch;
  nsFileSystemMirrorType* mirror = m_pImpl->m_mirror.Borrow();
  // Renaming a file to the same filename with different casing triggers the events REMOVED (old casing) -> RENAMED_OLD_NAME -> _RENAMED_NEW_NAME.
  // Thus, we need to cache every remove event to make sure the very next event is not a rename of the exact same file.
  auto FirePendingRemove = [&]()
  {
    if (!pendingRemoveOrRename.IsEmpty())
    {
      if (pendingRemoveOrRename.isDirectory)
      {
        if (whatToWatch.IsSet(Watch::Deletes))
        {
          if (mirror && whatToWatch.IsSet(Watch::Subdirectories))
          {
            mirror->Enumerate(pendingRemoveOrRename.path, [&](const nsStringBuilder& sPath, nsFileSystemMirrorType::Type type)
                    { func(sPath, nsDirectoryWatcherAction::Removed, (type == nsFileSystemMirrorType::Type::File) ? nsDirectoryWatcherType::File : nsDirectoryWatcherType::Directory); })
              .AssertSuccess();
          }
          func(pendingRemoveOrRename.path, nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::Directory);
        }
        if (mirror)
        {
          mirror->RemoveDirectory(pendingRemoveOrRename.path).AssertSuccess();
        }
      }
      else
      {
        if (mirror)
        {
          mirror->RemoveFile(pendingRemoveOrRename.path).AssertSuccess();
        }
        if (whatToWatch.IsSet(nsDirectoryWatcher::Watch::Deletes))
        {
          func(pendingRemoveOrRename.path, nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::File);
        }
      }
      pendingRemoveOrRename.Clear();
    }
  };

  NS_SCOPE_EXIT(FirePendingRemove());


  NS_ASSERT_DEV(!m_sDirectoryPath.IsEmpty(), "No directory opened!");
  while (WaitForSingleObject(m_pImpl->m_overlappedEvent, static_cast<DWORD>(waitUpTo.GetMilliseconds())) == WAIT_OBJECT_0)
  {
    waitUpTo = nsTime::MakeZero(); // only wait on the first call to GetQueuedCompletionStatus

    DWORD numberOfBytes = 0;
    GetOverlappedResult(m_pImpl->m_directoryHandle, &m_pImpl->m_overlapped, &numberOfBytes, FALSE);

    // Copy the buffer
    nsHybridArray<nsUInt8, 4096> buffer;
    buffer.SetCountUninitialized(numberOfBytes);
    buffer.GetArrayPtr().CopyFrom(m_pImpl->m_buffer.GetArrayPtr().GetSubArray(0, numberOfBytes));

    // Reissue the read request
    m_pImpl->DoRead();

    if (numberOfBytes == 0)
    {
      return;
    }

    MoveEvent lastMoveFrom;

    // Progress the messages
    auto info = (const FILE_NOTIFY_EXTENDED_INFORMATION*)buffer.GetData();
    while (true)
    {
      auto directory = nsArrayPtr<const WCHAR>(info->FileName, info->FileNameLength / sizeof(WCHAR));
      int bytesNeeded = WideCharToMultiByte(CP_UTF8, 0, directory.GetPtr(), directory.GetCount(), nullptr, 0, nullptr, nullptr);
      if (bytesNeeded > 0)
      {
        nsHybridArray<char, 1024> dir;
        dir.SetCountUninitialized(bytesNeeded);
        WideCharToMultiByte(CP_UTF8, 0, directory.GetPtr(), directory.GetCount(), dir.GetData(), dir.GetCount(), nullptr, nullptr);
        nsDirectoryWatcherAction action = nsDirectoryWatcherAction::None;
        bool fireEvent = false;

        nsStringBuilder eventFilePath = m_sDirectoryPath;
        eventFilePath.AppendPath(nsStringView(dir.GetData(), dir.GetCount()));
        eventFilePath.MakeCleanPath();

        const bool isFile = (info->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
        if (!pendingRemoveOrRename.IsEmpty() && isFile == !pendingRemoveOrRename.isDirectory && info->Action == FILE_ACTION_RENAMED_OLD_NAME && pendingRemoveOrRename.path == eventFilePath)
        {
          // This is the bogus removed event because we changed the casing of a file / directory, ignore.
          pendingRemoveOrRename.Clear();
        }
        FirePendingRemove();

        if (isFile)
        {
          switch (info->Action)
          {
            case FILE_ACTION_ADDED:
              DEBUG_LOG("FILE_ACTION_ADDED {} ({})", eventFilePath, info->LastModificationTime.QuadPart);
              action = nsDirectoryWatcherAction::Added;
              fireEvent = whatToWatch.IsSet(nsDirectoryWatcher::Watch::Creates);
              if (mirror)
              {
                bool fileAlreadyExists = false;
                mirror->AddFile(eventFilePath.GetData(), true, &fileAlreadyExists, nullptr).AssertSuccess();
                if (fileAlreadyExists)
                {
                  fireEvent = false;
                }
              }
              break;
            case FILE_ACTION_REMOVED:
              DEBUG_LOG("FILE_ACTION_REMOVED {} ({})", eventFilePath, info->LastModificationTime.QuadPart);
              action = nsDirectoryWatcherAction::Removed;
              fireEvent = false;
              pendingRemoveOrRename = {eventFilePath, false};
              break;
            case FILE_ACTION_MODIFIED:
            {
              DEBUG_LOG("FILE_ACTION_MODIFIED {} ({})", eventFilePath, info->LastModificationTime.QuadPart);
              action = nsDirectoryWatcherAction::Modified;
              fireEvent = whatToWatch.IsAnySet(nsDirectoryWatcher::Watch::Writes);
              bool fileAreadyKnown = false;
              bool addPending = false;
              if (mirror)
              {
                mirror->AddFile(eventFilePath.GetData(), false, &fileAreadyKnown, &addPending).AssertSuccess();
              }
              if (fileAreadyKnown && addPending)
              {
                fireEvent = false;
              }
            }
            break;
            case FILE_ACTION_RENAMED_OLD_NAME:
              DEBUG_LOG("FILE_ACTION_RENAMED_OLD_NAME {} ({})", eventFilePath, info->LastModificationTime.QuadPart);
              NS_ASSERT_DEV(lastMoveFrom.IsEmpty(), "there should be no pending move from");
              action = nsDirectoryWatcherAction::RenamedOldName;
              fireEvent = whatToWatch.IsAnySet(nsDirectoryWatcher::Watch::Renames);
              NS_ASSERT_DEV(lastMoveFrom.IsEmpty(), "there should be no pending last move from");
              lastMoveFrom = {eventFilePath, false};
              break;
            case FILE_ACTION_RENAMED_NEW_NAME:
              DEBUG_LOG("FILE_ACTION_RENAMED_NEW_NAME {} ({})", eventFilePath, info->LastModificationTime.QuadPart);
              action = nsDirectoryWatcherAction::RenamedNewName;
              fireEvent = whatToWatch.IsAnySet(nsDirectoryWatcher::Watch::Renames);
              NS_ASSERT_DEV(!lastMoveFrom.IsEmpty() && !lastMoveFrom.isDirectory, "last move from doesn't match");
              if (mirror)
              {
                mirror->RemoveFile(lastMoveFrom.path).AssertSuccess();
                mirror->AddFile(eventFilePath, false, nullptr, nullptr).AssertSuccess();
              }
              lastMoveFrom.Clear();
              break;
          }

          if (fireEvent)
          {
            func(eventFilePath, action, nsDirectoryWatcherType::File);
          }
        }
        else
        {
          switch (info->Action)
          {
            case FILE_ACTION_ADDED:
            {
              DEBUG_LOG("DIR_ACTION_ADDED {}", eventFilePath);
              bool directoryAlreadyKnown = false;
              if (mirror)
              {
                mirror->AddDirectory(eventFilePath, &directoryAlreadyKnown).AssertSuccess();
              }

              if (whatToWatch.IsSet(Watch::Creates) && !directoryAlreadyKnown)
              {
                func(eventFilePath, nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::Directory);
              }

              // Whenever we add a directory we might be "to late" to see changes inside it.
              // So iterate the file system and make sure we track all files / subdirectories
              nsFileSystemIterator subdirIt;

              subdirIt.StartSearch(eventFilePath.GetData(),
                whatToWatch.IsSet(nsDirectoryWatcher::Watch::Subdirectories)
                  ? nsFileSystemIteratorFlags::ReportFilesAndFoldersRecursive
                  : nsFileSystemIteratorFlags::ReportFiles);

              nsStringBuilder tmpPath2;
              for (; subdirIt.IsValid(); subdirIt.Next())
              {
                const nsFileStats& stats = subdirIt.GetStats();
                stats.GetFullPath(tmpPath2);
                if (stats.m_bIsDirectory)
                {
                  directoryAlreadyKnown = false;
                  if (mirror)
                  {
                    mirror->AddDirectory(tmpPath2, &directoryAlreadyKnown).AssertSuccess();
                  }
                  if (whatToWatch.IsSet(nsDirectoryWatcher::Watch::Creates) && !directoryAlreadyKnown)
                  {
                    func(tmpPath2, nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::Directory);
                  }
                }
                else
                {
                  bool fileExistsAlready = false;
                  if (mirror)
                  {
                    mirror->AddFile(tmpPath2, false, &fileExistsAlready, nullptr).AssertSuccess();
                  }
                  if (whatToWatch.IsSet(nsDirectoryWatcher::Watch::Creates) && !fileExistsAlready)
                  {
                    func(tmpPath2, nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File);
                  }
                }
              }
            }
            break;
            case FILE_ACTION_REMOVED:
              DEBUG_LOG("DIR_ACTION_REMOVED {}", eventFilePath);
              pendingRemoveOrRename = {eventFilePath, true};
              break;
            case FILE_ACTION_RENAMED_OLD_NAME:
              DEBUG_LOG("DIR_ACTION_OLD_NAME {}", eventFilePath);
              NS_ASSERT_DEV(lastMoveFrom.IsEmpty(), "there should be no pending move from");
              lastMoveFrom = {eventFilePath, true};
              break;
            case FILE_ACTION_RENAMED_NEW_NAME:
              DEBUG_LOG("DIR_ACTION_NEW_NAME {}", eventFilePath);
              NS_ASSERT_DEV(!lastMoveFrom.IsEmpty(), "rename old name and rename new name should always appear in pairs");
              if (mirror)
              {
                mirror->MoveDirectory(lastMoveFrom.path, eventFilePath).AssertSuccess();
              }
              if (whatToWatch.IsSet(Watch::Renames))
              {
                func(lastMoveFrom.path, nsDirectoryWatcherAction::RenamedOldName, nsDirectoryWatcherType::Directory);
                func(eventFilePath, nsDirectoryWatcherAction::RenamedNewName, nsDirectoryWatcherType::Directory);
              }
              lastMoveFrom.Clear();
              break;
            default:
              break;
          }
        }
      }
      if (info->NextEntryOffset == 0)
      {
        break;
      }
      else
        info = (const FILE_NOTIFY_EXTENDED_INFORMATION*)(((nsUInt8*)info) + info->NextEntryOffset);
    }
  }
}


void nsDirectoryWatcher::EnumerateChanges(nsArrayPtr<nsDirectoryWatcher*> watchers, EnumerateChangesFunction func, nsTime waitUpTo)
{
  nsHybridArray<HANDLE, 16> events;
  events.SetCount(watchers.GetCount());

  for (nsUInt32 i = 0; i < watchers.GetCount(); ++i)
  {
    events[i] = watchers[i]->m_pImpl->m_overlappedEvent;
  }

  // Wait for any of the watchers to have some data ready
  if (WaitForMultipleObjects(events.GetCount(), events.GetData(), FALSE, static_cast<DWORD>(waitUpTo.GetMilliseconds())) == WAIT_TIMEOUT)
  {
    return;
  }

  // Iterate all of them to make sure we report all changes up to this point.
  for (nsDirectoryWatcher* watcher : watchers)
  {
    watcher->EnumerateChanges(func);
  }
}

#endif
