#pragma once

#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/IO/DirectoryWatcher.h>
#include <Foundation/IO/Implementation/Shared/FileSystemMirror.h>
#include <Foundation/IO/Implementation/Win/DosDevicePath_win.h>
#include <Foundation/Logging/Log.h>

// Comment in to get verbose output on the function of the directory watcher
// #define DEBUG_FILE_WATCHER

#ifdef DEBUG_FILE_WATCHER
#  define DEBUG_LOG(...) wdLog::Debug(__VA_ARGS__)
#else
#  define DEBUG_LOG(...)
#endif

namespace
{
  struct MoveEvent
  {
    wdString path;
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

  using wdFileSystemMirrorType = wdFileSystemMirror<bool>;
} // namespace

struct wdDirectoryWatcherImpl
{
  void DoRead();

  HANDLE m_directoryHandle;
  DWORD m_filter;
  OVERLAPPED m_overlapped;
  HANDLE m_overlappedEvent;
  wdDynamicArray<wdUInt8> m_buffer;
  wdBitflags<wdDirectoryWatcher::Watch> m_whatToWatch;
  wdUniquePtr<wdFileSystemMirrorType> m_mirror; // store the last modification timestamp alongside each file
};

wdDirectoryWatcher::wdDirectoryWatcher()
  : m_pImpl(WD_DEFAULT_NEW(wdDirectoryWatcherImpl))
{
  m_pImpl->m_buffer.SetCountUninitialized(1024 * 1024);
}

wdResult wdDirectoryWatcher::OpenDirectory(wdStringView sAbsolutePath, wdBitflags<Watch> whatToWatch)
{
  WD_ASSERT_DEV(m_sDirectoryPath.IsEmpty(), "Directory already open, call CloseDirectory first!");
  wdStringBuilder sPath(sAbsolutePath);
  sPath.MakeCleanPath();
  sPath.Trim("/");

  m_pImpl->m_whatToWatch = whatToWatch;
  m_pImpl->m_filter = FILE_NOTIFY_CHANGE_FILE_NAME;
  if (whatToWatch.IsSet(Watch::Writes) || whatToWatch.AreAllSet(Watch::Deletes | Watch::Subdirectories))
  {
    m_pImpl->m_filter |= FILE_NOTIFY_CHANGE_LAST_WRITE;
    m_pImpl->m_mirror = WD_DEFAULT_NEW(wdFileSystemMirrorType);
    m_pImpl->m_mirror->AddDirectory(sPath).AssertSuccess();
  }

  if (whatToWatch.IsAnySet(Watch::Deletes | Watch::Creates | Watch::Renames))
  {
    m_pImpl->m_filter |= FILE_NOTIFY_CHANGE_DIR_NAME;
  }

  m_pImpl->m_directoryHandle = CreateFileW(wdDosDevicePath(sPath), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);
  if (m_pImpl->m_directoryHandle == INVALID_HANDLE_VALUE)
  {
    return WD_FAILURE;
  }

  m_pImpl->m_overlappedEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
  if (m_pImpl->m_overlappedEvent == INVALID_HANDLE_VALUE)
  {
    return WD_FAILURE;
  }

  m_pImpl->DoRead();
  m_sDirectoryPath = sPath;

  return WD_SUCCESS;
}

void wdDirectoryWatcher::CloseDirectory()
{
  if (!m_sDirectoryPath.IsEmpty())
  {
    CancelIo(m_pImpl->m_directoryHandle);
    CloseHandle(m_pImpl->m_overlappedEvent);
    CloseHandle(m_pImpl->m_directoryHandle);
    m_sDirectoryPath.Clear();
  }
}

wdDirectoryWatcher::~wdDirectoryWatcher()
{
  CloseDirectory();
  WD_DEFAULT_DELETE(m_pImpl);
}

void wdDirectoryWatcherImpl::DoRead()
{
  ResetEvent(m_overlappedEvent);
  memset(&m_overlapped, 0, sizeof(m_overlapped));
  m_overlapped.hEvent = m_overlappedEvent;
  BOOL success =
    ReadDirectoryChangesExW(m_directoryHandle, m_buffer.GetData(), m_buffer.GetCount(), m_whatToWatch.IsSet(wdDirectoryWatcher::Watch::Subdirectories), m_filter, nullptr, &m_overlapped, nullptr, ReadDirectoryNotifyExtendedInformation);
  WD_ASSERT_DEV(success, "ReadDirectoryChangesW failed.");
}

void wdDirectoryWatcher::EnumerateChanges(EnumerateChangesFunction func, wdTime waitUpTo)
{
  WD_ASSERT_DEV(!m_sDirectoryPath.IsEmpty(), "No directory opened!");
  while (WaitForSingleObject(m_pImpl->m_overlappedEvent, static_cast<DWORD>(waitUpTo.GetMilliseconds())) == WAIT_OBJECT_0)
  {
    waitUpTo = wdTime::Zero(); // only wait on the first call to GetQueuedCompletionStatus

    DWORD numberOfBytes = 0;
    GetOverlappedResult(m_pImpl->m_directoryHandle, &m_pImpl->m_overlapped, &numberOfBytes, FALSE);

    // Copy the buffer
    wdHybridArray<wdUInt8, 4096> buffer;
    buffer.SetCountUninitialized(numberOfBytes);
    buffer.GetArrayPtr().CopyFrom(m_pImpl->m_buffer.GetArrayPtr().GetSubArray(0, numberOfBytes));

    // Reissue the read request
    m_pImpl->DoRead();

    if (numberOfBytes == 0)
    {
      return;
    }

    const wdBitflags<wdDirectoryWatcher::Watch> whatToWatch = m_pImpl->m_whatToWatch;

    wdFileSystemMirrorType* mirror = m_pImpl->m_mirror.Borrow();

    MoveEvent lastMoveFrom;

    // Progress the messages
    auto info = (const FILE_NOTIFY_EXTENDED_INFORMATION*)buffer.GetData();
    while (true)
    {
      auto directory = wdArrayPtr<const WCHAR>(info->FileName, info->FileNameLength / sizeof(WCHAR));
      int bytesNeeded = WideCharToMultiByte(CP_UTF8, 0, directory.GetPtr(), directory.GetCount(), nullptr, 0, nullptr, nullptr);
      if (bytesNeeded > 0)
      {
        wdHybridArray<char, 1024> dir;
        dir.SetCountUninitialized(bytesNeeded);
        WideCharToMultiByte(CP_UTF8, 0, directory.GetPtr(), directory.GetCount(), dir.GetData(), dir.GetCount(), nullptr, nullptr);
        wdDirectoryWatcherAction action = wdDirectoryWatcherAction::None;
        bool fireEvent = false;

        wdStringBuilder eventFilePath = m_sDirectoryPath;
        eventFilePath.AppendPath(wdStringView(dir.GetData(), dir.GetCount()));
        eventFilePath.MakeCleanPath();

        if ((info->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
          switch (info->Action)
          {
            case FILE_ACTION_ADDED:
              DEBUG_LOG("FILE_ACTION_ADDED {} ({})", eventFilePath, info->LastModificationTime.QuadPart);
              action = wdDirectoryWatcherAction::Added;
              fireEvent = whatToWatch.IsSet(wdDirectoryWatcher::Watch::Creates);
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
              action = wdDirectoryWatcherAction::Removed;
              fireEvent = whatToWatch.IsSet(wdDirectoryWatcher::Watch::Deletes);
              if (mirror)
              {
                mirror->RemoveFile(eventFilePath.GetData()).AssertSuccess();
              }
              break;
            case FILE_ACTION_MODIFIED:
            {
              DEBUG_LOG("FILE_ACTION_MODIFIED {} ({})", eventFilePath, info->LastModificationTime.QuadPart);
              action = wdDirectoryWatcherAction::Modified;
              fireEvent = whatToWatch.IsAnySet(wdDirectoryWatcher::Watch::Writes);
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
              WD_ASSERT_DEV(lastMoveFrom.IsEmpty(), "there should be no pending move from");
              action = wdDirectoryWatcherAction::RenamedOldName;
              fireEvent = whatToWatch.IsAnySet(wdDirectoryWatcher::Watch::Renames);
              WD_ASSERT_DEV(lastMoveFrom.IsEmpty(), "there should be no pending last move from");
              lastMoveFrom = {eventFilePath, false};
              break;
            case FILE_ACTION_RENAMED_NEW_NAME:
              DEBUG_LOG("FILE_ACTION_RENAMED_NEW_NAME {} ({})", eventFilePath, info->LastModificationTime.QuadPart);
              action = wdDirectoryWatcherAction::RenamedNewName;
              fireEvent = whatToWatch.IsAnySet(wdDirectoryWatcher::Watch::Renames);
              WD_ASSERT_DEV(!lastMoveFrom.IsEmpty() && !lastMoveFrom.isDirectory, "last move from doesn't match");
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
            func(eventFilePath, action, wdDirectoryWatcherType::File);
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
                func(eventFilePath, wdDirectoryWatcherAction::Added, wdDirectoryWatcherType::Directory);
              }

              // Whenever we add a directory we might be "to late" to see changes inside it.
              // So iterate the file system and make sure we track all files / subdirectories
              wdFileSystemIterator subdirIt;

              subdirIt.StartSearch(eventFilePath.GetData(),
                whatToWatch.IsSet(wdDirectoryWatcher::Watch::Subdirectories)
                  ? wdFileSystemIteratorFlags::ReportFilesAndFoldersRecursive
                  : wdFileSystemIteratorFlags::ReportFiles);

              wdStringBuilder tmpPath2;
              for (; subdirIt.IsValid(); subdirIt.Next())
              {
                const wdFileStats& stats = subdirIt.GetStats();
                stats.GetFullPath(tmpPath2);
                if (stats.m_bIsDirectory)
                {
                  directoryAlreadyKnown = false;
                  if (mirror)
                  {
                    mirror->AddDirectory(tmpPath2, &directoryAlreadyKnown).AssertSuccess();
                  }
                  if (whatToWatch.IsSet(wdDirectoryWatcher::Watch::Creates) && !directoryAlreadyKnown)
                  {
                    func(tmpPath2, wdDirectoryWatcherAction::Added, wdDirectoryWatcherType::Directory);
                  }
                }
                else
                {
                  bool fileExistsAlready = false;
                  if (mirror)
                  {
                    mirror->AddFile(tmpPath2, false, &fileExistsAlready, nullptr).AssertSuccess();
                  }
                  if (whatToWatch.IsSet(wdDirectoryWatcher::Watch::Creates) && !fileExistsAlready)
                  {
                    func(tmpPath2, wdDirectoryWatcherAction::Added, wdDirectoryWatcherType::File);
                  }
                }
              }
            }
            break;
            case FILE_ACTION_REMOVED:
              DEBUG_LOG("DIR_ACTION_REMOVED {}", eventFilePath);
              if (whatToWatch.IsSet(Watch::Deletes))
              {
                if (mirror && whatToWatch.IsSet(Watch::Subdirectories))
                {
                  mirror->Enumerate(eventFilePath, [&](const wdStringBuilder& sPath, wdFileSystemMirrorType::Type type) {
                          func(sPath, wdDirectoryWatcherAction::Removed, (type == wdFileSystemMirrorType::Type::File) ? wdDirectoryWatcherType::File : wdDirectoryWatcherType::Directory);
                        })
                    .AssertSuccess();
                }
                func(eventFilePath, wdDirectoryWatcherAction::Removed, wdDirectoryWatcherType::Directory);
              }
              if (mirror)
              {
                mirror->RemoveDirectory(eventFilePath).AssertSuccess();
              }
              break;
            case FILE_ACTION_RENAMED_OLD_NAME:
              DEBUG_LOG("DIR_ACTION_OLD_NAME {}", eventFilePath);
              WD_ASSERT_DEV(lastMoveFrom.IsEmpty(), "there should be no pending move from");
              lastMoveFrom = {eventFilePath, true};
              break;
            case FILE_ACTION_RENAMED_NEW_NAME:
              DEBUG_LOG("DIR_ACTION_NEW_NAME {}", eventFilePath);
              WD_ASSERT_DEV(!lastMoveFrom.IsEmpty(), "rename old name and rename new name should always appear in pairs");
              if (mirror)
              {
                mirror->MoveDirectory(lastMoveFrom.path, eventFilePath).AssertSuccess();
              }
              if (whatToWatch.IsSet(Watch::Renames))
              {
                func(lastMoveFrom.path, wdDirectoryWatcherAction::RenamedOldName, wdDirectoryWatcherType::Directory);
                func(eventFilePath, wdDirectoryWatcherAction::RenamedNewName, wdDirectoryWatcherType::Directory);
              }
              lastMoveFrom.Clear();
              break;
            default:
              break;
          }
        }
      }
      if (info->NextEntryOffset == 0)
        break;
      else
        info = (const FILE_NOTIFY_EXTENDED_INFORMATION*)(((wdUInt8*)info) + info->NextEntryOffset);
    }
  }
}


void wdDirectoryWatcher::EnumerateChanges(wdArrayPtr<wdDirectoryWatcher*> watchers, EnumerateChangesFunction func, wdTime waitUpTo)
{
  wdHybridArray<HANDLE, 16> events;
  events.SetCount(watchers.GetCount());

  for (wdUInt32 i = 0; i < watchers.GetCount(); ++i)
  {
    events[i] = watchers[i]->m_pImpl->m_overlappedEvent;
  }

  // Wait for any of the watchers to have some data ready
  if (WaitForMultipleObjects(events.GetCount(), events.GetData(), FALSE, static_cast<DWORD>(waitUpTo.GetMilliseconds())) == WAIT_TIMEOUT)
  {
    return;
  }

  // Iterate all of them to make sure we report all changes up to this point.
  for (wdDirectoryWatcher* watcher : watchers)
  {
    watcher->EnumerateChanges(func);
  }
}
