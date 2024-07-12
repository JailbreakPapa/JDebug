#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Types/UniquePtr.h>

// A general problem when implementing a directory watcher is, that moving a folder out of the watched directory only communicates
// Which folder was moved (but not to where, nor its contents). This means when a folder is moved out of view,
// this needs to be treated as a delete. At the point of the move, it is no longer possible to query the contents of the folder.
// So a in memory copy of the file system is required in order to correctly implement a directory watcher.
template <typename T>
class nsFileSystemMirror
{
public:
  enum class Type
  {
    File,
    Directory
  };

  struct DirEntry
  {
    nsMap<nsString, DirEntry> m_subDirectories;
    nsMap<nsString, T> m_files;
  };

  nsFileSystemMirror();
  ~nsFileSystemMirror();

  // \brief Adds the directory, and all files in it recursively.
  nsResult AddDirectory(nsStringView sPath, bool* out_pDirectoryExistsAlready = nullptr);

  // \brief Adds a file. Creates directories if they do not exist.
  nsResult AddFile(nsStringView sPath, const T& value, bool* out_pFileExistsAlready, T* out_pOldValue);

  // \brief Removes a file.
  nsResult RemoveFile(nsStringView sPath);

  // \brief Removes a directory. Deletes any files & directories inside.
  nsResult RemoveDirectory(nsStringView sPath);

  // \brief Moves a directory. Any files & folders inside are moved with it.
  nsResult MoveDirectory(nsStringView sFromPath, nsStringView sToPath);

  using EnumerateFunc = nsDelegate<void(const nsStringBuilder& path, Type type)>;

  // \brief Enumerates the files & directories under the given path
  nsResult Enumerate(nsStringView sPath, EnumerateFunc callbackFunc);

private:
  DirEntry* FindDirectory(nsStringBuilder& path);

private:
  DirEntry m_TopLevelDir;
  nsString m_sTopLevelDirPath;
};

namespace
{
  void EnsureTrailingSlash(nsStringBuilder& ref_sBuilder)
  {
    if (!ref_sBuilder.EndsWith("/"))
    {
      ref_sBuilder.Append("/");
    }
  }

  void RemoveTrailingSlash(nsStringBuilder& ref_sBuilder)
  {
    if (ref_sBuilder.EndsWith("/"))
    {
      ref_sBuilder.Shrink(0, 1);
    }
  }
} // namespace

template <typename T>
nsFileSystemMirror<T>::nsFileSystemMirror() = default;

template <typename T>
nsFileSystemMirror<T>::~nsFileSystemMirror() = default;

template <typename T>
nsResult nsFileSystemMirror<T>::AddDirectory(nsStringView sPath, bool* out_pDirectoryExistsAlready)
{
  nsStringBuilder currentDirAbsPath = sPath;
  currentDirAbsPath.MakeCleanPath();
  EnsureTrailingSlash(currentDirAbsPath);

  if (m_sTopLevelDirPath.IsEmpty())
  {
    m_sTopLevelDirPath = currentDirAbsPath;
    currentDirAbsPath.Shrink(0, 1); // remove trailing /

    DirEntry* currentDir = &m_TopLevelDir;

    nsHybridArray<DirEntry*, 16> m_dirStack;

    nsFileSystemIterator files;
    files.StartSearch(currentDirAbsPath.GetData(), nsFileSystemIteratorFlags::ReportFilesAndFoldersRecursive);
    for (; files.IsValid(); files.Next())
    {
      const nsFileStats& stats = files.GetStats();

      // In case we are done with a directory, move back up
      while (currentDirAbsPath != stats.m_sParentPath)
      {
        NS_ASSERT_DEV(m_dirStack.GetCount() > 0, "Unexpected file iteration order");
        currentDir = m_dirStack.PeekBack();
        m_dirStack.PopBack();
        currentDirAbsPath.PathParentDirectory();
        RemoveTrailingSlash(currentDirAbsPath);
      }

      if (stats.m_bIsDirectory)
      {
        m_dirStack.PushBack(currentDir);
        nsStringBuilder subdirName = stats.m_sName;
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
    if (out_pDirectoryExistsAlready != nullptr)
    {
      *out_pDirectoryExistsAlready = false;
    }
  }
  else
  {
    DirEntry* parentDir = FindDirectory(currentDirAbsPath);
    if (parentDir == nullptr)
    {
      return NS_FAILURE;
    }

    if (out_pDirectoryExistsAlready != nullptr)
    {
      *out_pDirectoryExistsAlready = currentDirAbsPath.IsEmpty();
    }

    while (!currentDirAbsPath.IsEmpty())
    {
      const char* dirEnd = currentDirAbsPath.FindSubString("/");
      nsStringView subdirName(currentDirAbsPath.GetData(), dirEnd + 1);
      auto insertIt = parentDir->m_subDirectories.Insert(subdirName, DirEntry());
      parentDir = &insertIt.Value();
      currentDirAbsPath.Shrink(nsStringUtils::GetCharacterCount(subdirName.GetStartPointer(), subdirName.GetEndPointer()), 0);
    }
  }

  return NS_SUCCESS;
}

template <typename T>
nsResult nsFileSystemMirror<T>::AddFile(nsStringView sPath0, const T& value, bool* out_pFileExistsAlready, T* out_pOldValue)
{
  nsStringBuilder sPath = sPath0;
  DirEntry* dir = FindDirectory(sPath);
  if (dir == nullptr)
  {
    return NS_FAILURE; // file not under top level directory
  }

  const char* szSlashPos = sPath.FindSubString("/");

  while (szSlashPos != nullptr)
  {
    nsStringView subdirName(sPath.GetData(), szSlashPos + 1);
    auto insertIt = dir->m_subDirectories.Insert(subdirName, DirEntry());
    dir = &insertIt.Value();
    sPath.Shrink(nsStringUtils::GetCharacterCount(subdirName.GetStartPointer(), subdirName.GetEndPointer()), 0);
    szSlashPos = sPath.FindSubString("/");
  }

  auto it = dir->m_files.Find(sPath);
  // Do not add the file twice
  if (!it.IsValid())
  {
    dir->m_files.Insert(sPath, value);
    if (out_pFileExistsAlready != nullptr)
    {
      *out_pFileExistsAlready = false;
    }
  }
  else
  {
    if (out_pFileExistsAlready != nullptr)
    {
      *out_pFileExistsAlready = true;
    }
    if (out_pOldValue != nullptr)
    {
      *out_pOldValue = it.Value();
    }
    it.Value() = value;
  }
  return NS_SUCCESS;
}

template <typename T>
nsResult nsFileSystemMirror<T>::RemoveFile(nsStringView sPath0)
{
  nsStringBuilder sPath = sPath0;
  DirEntry* dir = FindDirectory(sPath);
  if (dir == nullptr)
  {
    return NS_FAILURE; // file not under top level directory
  }

  if (sPath.FindSubString("/") != nullptr)
  {
    return NS_FAILURE; // file does not exist
  }

  if (dir->m_files.GetCount() == 0)
  {
    return NS_FAILURE; // there are no files in this directory
  }

  auto it = dir->m_files.Find(sPath);
  if (!it.IsValid())
  {
    return NS_FAILURE; // file does not exist
  }

  dir->m_files.Remove(it);
  return NS_SUCCESS;
}

template <typename T>
nsResult nsFileSystemMirror<T>::RemoveDirectory(nsStringView sPath)
{
  nsStringBuilder parentPath = sPath;
  nsStringBuilder dirName = sPath;
  parentPath.PathParentDirectory();
  EnsureTrailingSlash(parentPath);
  dirName.Shrink(parentPath.GetCharacterCount(), 0);
  EnsureTrailingSlash(dirName);

  DirEntry* parentDir = FindDirectory(parentPath);
  if (parentDir == nullptr || !parentPath.IsEmpty())
  {
    return NS_FAILURE;
  }

  if (!parentDir->m_subDirectories.Remove(dirName))
  {
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

template <typename T>
nsResult nsFileSystemMirror<T>::MoveDirectory(nsStringView sFromPath0, nsStringView sToPath0)
{
  nsStringBuilder sFromPath = sFromPath0;
  nsStringBuilder sFromName = sFromPath0;
  sFromPath.PathParentDirectory();
  EnsureTrailingSlash(sFromPath);
  sFromName.Shrink(sFromPath.GetCharacterCount(), 0);
  EnsureTrailingSlash(sFromName);


  nsStringBuilder sToPath = sToPath0;
  nsStringBuilder sToName = sToPath0;
  sToPath.PathParentDirectory();
  EnsureTrailingSlash(sToPath);
  sToName.Shrink(sToPath.GetCharacterCount(), 0);
  EnsureTrailingSlash(sToName);

  DirEntry* moveFromDir = FindDirectory(sFromPath);
  if (!moveFromDir)
  {
    return NS_FAILURE;
  }
  NS_ASSERT_DEV(sFromPath.IsEmpty(), "move from directory should fully exist");

  DirEntry* moveToDir = FindDirectory(sToPath);
  if (!moveToDir)
  {
    return NS_FAILURE;
  }

  if (!sToPath.IsEmpty())
  {
    do
    {
      const char* dirEnd = sToPath.FindSubString("/");
      nsStringView subdirName(sToPath.GetData(), dirEnd + 1);
      auto insertIt = moveToDir->m_subDirectories.Insert(subdirName, DirEntry());
      moveToDir = &insertIt.Value();
      sToPath.Shrink(0, nsStringUtils::GetCharacterCount(subdirName.GetStartPointer(), subdirName.GetEndPointer()));
    } while (!sToPath.IsEmpty());
  }

  DirEntry movedDir;
  {
    auto fromIt = moveFromDir->m_subDirectories.Find(sFromName);
    if (!fromIt.IsValid())
    {
      return NS_FAILURE;
    }

    movedDir = std::move(fromIt.Value());
    moveFromDir->m_subDirectories.Remove(fromIt);
  }

  moveToDir->m_subDirectories.Insert(sToName, std::move(movedDir));

  return NS_SUCCESS;
}

namespace
{
  template <typename T>
  struct nsDirEnumerateState
  {
    typename nsFileSystemMirror<T>::DirEntry* dir;
    typename nsMap<nsString, typename nsFileSystemMirror<T>::DirEntry>::Iterator subDirIt;
  };
} // namespace

template <typename T>
nsResult nsFileSystemMirror<T>::Enumerate(nsStringView sPath0, EnumerateFunc callbackFunc)
{
  nsHybridArray<nsDirEnumerateState<T>, 16> dirStack;
  nsStringBuilder sPath = sPath0;
  if (!sPath.EndsWith("/"))
  {
    sPath.Append("/");
  }
  DirEntry* dirToEnumerate = FindDirectory(sPath);
  if (dirToEnumerate == nullptr)
  {
    return NS_FAILURE;
  }
  if (!sPath.IsEmpty())
  {
    return NS_FAILURE; // requested folder to enumerate doesn't exist
  }
  DirEntry* currentDir = dirToEnumerate;
  typename nsMap<nsString, nsFileSystemMirror::DirEntry>::Iterator currentSubDirIt = currentDir->m_subDirectories.GetIterator();
  sPath = sPath0;

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
      nsStringBuilder sFilePath;
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

  return NS_SUCCESS;
}

template <typename T>
typename nsFileSystemMirror<T>::DirEntry* nsFileSystemMirror<T>::FindDirectory(nsStringBuilder& path)
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
