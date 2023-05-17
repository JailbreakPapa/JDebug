#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Types/UniquePtr.h>

// A general problem when implementing a directory watcher is, that moving a folder out of the watched directory only communicates
// Which folder was moved (but not to where, nor its contents). This means when a folder is moved out of view,
// this needs to be treated as a delete. At the point of the move, it is no longer possible to query the contents of the folder.
// So a in memory copy of the file system is required in order to correctly implement a directory watcher.
template <typename T>
class wdFileSystemMirror
{
public:
  enum class Type
  {
    File,
    Directory
  };

  struct DirEntry
  {
    wdMap<wdString, DirEntry> m_subDirectories;
    wdMap<wdString, T> m_files;
  };

  wdFileSystemMirror();
  ~wdFileSystemMirror();

  // \brief Adds the directory, and all files in it recursively.
  wdResult AddDirectory(const char* path, bool* outDirectoryExistsAlready = nullptr);

  // \brief Adds a file. Creates directories if they do not exist.
  wdResult AddFile(const char* path, const T& value, bool* outFileExistsAlready, T* outOldValue);

  // \brief Removes a file.
  wdResult RemoveFile(const char* path);

  // \brief Removes a directory. Deletes any files & directories inside.
  wdResult RemoveDirectory(const char* path);

  // \brief Moves a directory. Any files & folders inside are moved with it.
  wdResult MoveDirectory(const char* fromPath, const char* toPath);

  using EnumerateFunc = wdDelegate<void(const wdStringBuilder& path, Type type)>;

  // \brief Enumerates the files & directories under the given path
  wdResult Enumerate(const char* path, EnumerateFunc callbackFunc);

private:
  DirEntry* FindDirectory(wdStringBuilder& path);
  DirEntry* AddDirectoryImpl(DirEntry* startDir, wdStringBuilder& path);

private:
  DirEntry m_TopLevelDir;
  wdString m_sTopLevelDirPath;
};

namespace
{
  void EnsureTrailingSlash(wdStringBuilder& ref_sBuilder)
  {
    if (!ref_sBuilder.EndsWith("/"))
    {
      ref_sBuilder.Append("/");
    }
  }

  void RemoveTrailingSlash(wdStringBuilder& ref_sBuilder)
  {
    if (ref_sBuilder.EndsWith("/"))
    {
      ref_sBuilder.Shrink(0, 1);
    }
  }
} // namespace

template <typename T>
wdFileSystemMirror<T>::wdFileSystemMirror() = default;

template <typename T>
wdFileSystemMirror<T>::~wdFileSystemMirror() = default;

template <typename T>
wdResult wdFileSystemMirror<T>::AddDirectory(const char* path, bool* outDirectoryExistsAlready)
{
  wdStringBuilder currentDirAbsPath = path;
  currentDirAbsPath.MakeCleanPath();
  EnsureTrailingSlash(currentDirAbsPath);

  if (m_sTopLevelDirPath.IsEmpty())
  {
    m_sTopLevelDirPath = currentDirAbsPath;
    currentDirAbsPath.Shrink(0, 1); // remove trailing /

    DirEntry* currentDir = &m_TopLevelDir;

    wdHybridArray<DirEntry*, 16> m_dirStack;

    wdFileSystemIterator files;
    files.StartSearch(currentDirAbsPath.GetData(), wdFileSystemIteratorFlags::ReportFilesAndFoldersRecursive);
    for (; files.IsValid(); files.Next())
    {
      const wdFileStats& stats = files.GetStats();

      // In case we are done with a directory, move back up
      while (currentDirAbsPath != stats.m_sParentPath)
      {
        WD_ASSERT_DEV(m_dirStack.GetCount() > 0, "Unexpected file iteration order");
        currentDir = m_dirStack.PeekBack();
        m_dirStack.PopBack();
        currentDirAbsPath.PathParentDirectory();
        RemoveTrailingSlash(currentDirAbsPath);
      }

      if (stats.m_bIsDirectory)
      {
        m_dirStack.PushBack(currentDir);
        wdStringBuilder subdirName = stats.m_sName;
        EnsureTrailingSlash(subdirName);
        auto insertIt = currentDir->m_subDirectories.Insert(subdirName, DirEntry());
        currentDir = &insertIt.Value();
        currentDirAbsPath.AppendPath(stats.m_sName);
      }
      else
      {
        currentDir->m_files.Insert(std::move(stats.m_sName), T{});
      }
    }
    if (outDirectoryExistsAlready != nullptr)
    {
      *outDirectoryExistsAlready = false;
    }
  }
  else
  {
    DirEntry* parentDir = FindDirectory(currentDirAbsPath);
    if (parentDir == nullptr)
    {
      return WD_FAILURE;
    }

    if (outDirectoryExistsAlready != nullptr)
    {
      *outDirectoryExistsAlready = currentDirAbsPath.IsEmpty();
    }

    while (!currentDirAbsPath.IsEmpty())
    {
      const char* dirEnd = currentDirAbsPath.FindSubString("/");
      wdStringView subdirName(currentDirAbsPath.GetData(), dirEnd + 1);
      auto insertIt = parentDir->m_subDirectories.Insert(subdirName, DirEntry());
      parentDir = &insertIt.Value();
      currentDirAbsPath.Shrink(wdStringUtils::GetCharacterCount(subdirName.GetStartPointer(), subdirName.GetEndPointer()), 0);
    }
  }

  return WD_SUCCESS;
}

template <typename T>
wdResult wdFileSystemMirror<T>::AddFile(const char* path, const T& value, bool* outFileExistsAlready, T* outOldValue)
{
  wdStringBuilder sPath = path;
  DirEntry* dir = FindDirectory(sPath);
  if (dir == nullptr)
  {
    return WD_FAILURE; // file not under top level directory
  }

  const char* szSlashPos = sPath.FindSubString("/");

  while (szSlashPos != nullptr)
  {
    wdStringView subdirName(sPath.GetData(), szSlashPos + 1);
    auto insertIt = dir->m_subDirectories.Insert(subdirName, DirEntry());
    dir = &insertIt.Value();
    sPath.Shrink(wdStringUtils::GetCharacterCount(subdirName.GetStartPointer(), subdirName.GetEndPointer()), 0);
    szSlashPos = sPath.FindSubString("/");
  }

  auto it = dir->m_files.Find(sPath);
  // Do not add the file twice
  if (!it.IsValid())
  {
    dir->m_files.Insert(sPath, value);
    if (outFileExistsAlready != nullptr)
    {
      *outFileExistsAlready = false;
    }
  }
  else
  {
    if (outFileExistsAlready != nullptr)
    {
      *outFileExistsAlready = true;
    }
    if (outOldValue != nullptr)
    {
      *outOldValue = it.Value();
    }
    it.Value() = value;
  }
  return WD_SUCCESS;
}

template <typename T>
wdResult wdFileSystemMirror<T>::RemoveFile(const char* path)
{
  wdStringBuilder sPath = path;
  DirEntry* dir = FindDirectory(sPath);
  if (dir == nullptr)
  {
    return WD_FAILURE; // file not under top level directory
  }

  if (sPath.FindSubString("/") != nullptr)
  {
    return WD_FAILURE; // file does not exist
  }

  if (dir->m_files.GetCount() == 0)
  {
    return WD_FAILURE; // there are no files in this directory
  }

  auto it = dir->m_files.Find(sPath);
  if (!it.IsValid())
  {
    return WD_FAILURE; // file does not exist
  }

  dir->m_files.Remove(it);
  return WD_SUCCESS;
}

template <typename T>
wdResult wdFileSystemMirror<T>::RemoveDirectory(const char* path)
{
  wdStringBuilder parentPath = path;
  wdStringBuilder dirName = path;
  parentPath.PathParentDirectory();
  EnsureTrailingSlash(parentPath);
  dirName.Shrink(parentPath.GetCharacterCount(), 0);
  EnsureTrailingSlash(dirName);

  DirEntry* parentDir = FindDirectory(parentPath);
  if (parentDir == nullptr || !parentPath.IsEmpty())
  {
    return WD_FAILURE;
  }

  if (!parentDir->m_subDirectories.Remove(dirName))
  {
    return WD_FAILURE;
  }

  return WD_SUCCESS;
}

template <typename T>
wdResult wdFileSystemMirror<T>::MoveDirectory(const char* fromPath, const char* toPath)
{
  wdStringBuilder sFromPath = fromPath;
  wdStringBuilder sFromName = fromPath;
  sFromPath.PathParentDirectory();
  EnsureTrailingSlash(sFromPath);
  sFromName.Shrink(sFromPath.GetCharacterCount(), 0);
  EnsureTrailingSlash(sFromName);


  wdStringBuilder sToPath = toPath;
  wdStringBuilder sToName = toPath;
  sToPath.PathParentDirectory();
  EnsureTrailingSlash(sToPath);
  sToName.Shrink(sToPath.GetCharacterCount(), 0);
  EnsureTrailingSlash(sToName);

  DirEntry* moveFromDir = FindDirectory(sFromPath);
  if (!moveFromDir)
  {
    return WD_FAILURE;
  }
  WD_ASSERT_DEV(sFromPath.IsEmpty(), "move from directory should fully exist");

  DirEntry* moveToDir = FindDirectory(sToPath);
  if (!moveToDir)
  {
    return WD_FAILURE;
  }

  if (!sToPath.IsEmpty())
  {
    do
    {
      const char* dirEnd = sToPath.FindSubString("/");
      wdStringView subdirName(sToPath.GetData(), dirEnd + 1);
      auto insertIt = moveToDir->m_subDirectories.Insert(subdirName, DirEntry());
      moveToDir = &insertIt.Value();
      sToPath.Shrink(0, wdStringUtils::GetCharacterCount(subdirName.GetStartPointer(), subdirName.GetEndPointer()));
    } while (!sToPath.IsEmpty());
  }

  DirEntry movedDir;
  {
    auto fromIt = moveFromDir->m_subDirectories.Find(sFromName);
    if (!fromIt.IsValid())
    {
      return WD_FAILURE;
    }

    movedDir = std::move(fromIt.Value());
    moveFromDir->m_subDirectories.Remove(fromIt);
  }

  moveToDir->m_subDirectories.Insert(sToName, std::move(movedDir));

  return WD_SUCCESS;
}

namespace
{
  template <typename T>
  struct wdDirEnumerateState
  {
    typename wdFileSystemMirror<T>::DirEntry* dir;
    typename wdMap<wdString, typename wdFileSystemMirror<T>::DirEntry>::Iterator subDirIt;
  };
} // namespace

template <typename T>
wdResult wdFileSystemMirror<T>::Enumerate(const char* path, EnumerateFunc callbackFunc)
{
  wdHybridArray<wdDirEnumerateState<T>, 16> dirStack;
  wdStringBuilder sPath = path;
  if (!sPath.EndsWith("/"))
  {
    sPath.Append("/");
  }
  DirEntry* dirToEnumerate = FindDirectory(sPath);
  if (dirToEnumerate == nullptr)
  {
    return WD_FAILURE;
  }
  if (!sPath.IsEmpty())
  {
    return WD_FAILURE; // requested folder to enumerate doesn't exist
  }
  DirEntry* currentDir = dirToEnumerate;
  typename wdMap<wdString, wdFileSystemMirror::DirEntry>::Iterator currentSubDirIt = currentDir->m_subDirectories.GetIterator();
  sPath = path;

  while (currentDir != nullptr)
  {
    if (currentSubDirIt.IsValid())
    {
      DirEntry* nextDir = &currentSubDirIt.Value();
      sPath.AppendPath(currentSubDirIt.Key());
      currentSubDirIt.Next();
      dirStack.PushBack({currentDir, currentSubDirIt});
      currentDir = nextDir;
    }
    else
    {
      wdStringBuilder sFilePath;
      for (auto& file : currentDir->m_files)
      {
        sFilePath = sPath;
        sFilePath.AppendPath(file.Key());
        callbackFunc(sFilePath, Type::File);
      }

      if (currentDir != dirToEnumerate)
      {
        if (sPath.EndsWith("/") && sPath.GetElementCount() > 1)
        {
          sPath.Shrink(0, 1);
        }
        callbackFunc(sPath, Type::Directory);
      }

      if (dirStack.IsEmpty())
      {
        currentDir = nullptr;
      }
      else
      {
        currentDir = dirStack.PeekBack().dir;
        currentSubDirIt = dirStack.PeekBack().subDirIt;
        dirStack.PopBack();
        sPath.PathParentDirectory();
        if (sPath.GetElementCount() > 1 && sPath.EndsWith("/"))
        {
          sPath.Shrink(0, 1);
        }
      }
    }
  }

  return WD_SUCCESS;
}

template <typename T>
typename wdFileSystemMirror<T>::DirEntry* wdFileSystemMirror<T>::FindDirectory(wdStringBuilder& path)
{
  if (!path.StartsWith(m_sTopLevelDirPath))
  {
    return nullptr;
  }
  path.TrimWordStart(m_sTopLevelDirPath);

  DirEntry* currentDir = &m_TopLevelDir;

  bool found = false;
  do
  {
    found = false;
    for (auto& dir : currentDir->m_subDirectories)
    {
      if (path.StartsWith(dir.Key()))
      {
        currentDir = &dir.Value();
        path.TrimWordStart(dir.Key());
        path.TrimWordStart("/");
        found = true;
        break;
      }
    }
  } while (found);

  return currentDir;
}
