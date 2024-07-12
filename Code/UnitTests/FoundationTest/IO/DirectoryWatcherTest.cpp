#include <FoundationTest/FoundationTestPCH.h>

#if NS_ENABLED(NS_SUPPORTS_DIRECTORY_WATCHER)

#  include <Foundation/IO/DirectoryWatcher.h>
#  include <Foundation/IO/OSFile.h>
#  include <Foundation/Threading/ThreadUtils.h>

namespace DirectoryWatcherTestHelpers
{

  struct ExpectedEvent
  {
    ~ExpectedEvent(){}; // NOLINT: To make it non-pod

    const char* path;
    nsDirectoryWatcherAction action;
    nsDirectoryWatcherType type;

    bool operator==(const ExpectedEvent& other) const
    {
      return nsStringView(path) == nsStringView(other.path) && action == other.action && type == other.type;
    }
  };

  struct ExpectedEventStorage
  {
    nsString path;
    nsDirectoryWatcherAction action;
    nsDirectoryWatcherType type;
  };

  void TickWatcher(nsDirectoryWatcher& ref_watcher)
  {
    ref_watcher.EnumerateChanges([&](nsStringView sPath, nsDirectoryWatcherAction action, nsDirectoryWatcherType type) {},
      nsTime::MakeFromMilliseconds(100));
  }
} // namespace DirectoryWatcherTestHelpers

NS_CREATE_SIMPLE_TEST(IO, DirectoryWatcher)
{
  using namespace DirectoryWatcherTestHelpers;

  nsStringBuilder tmp, tmp2;
  nsStringBuilder sTestRootPath = nsTestFramework::GetInstance()->GetAbsOutputPath();
  sTestRootPath.AppendPath("DirectoryWatcher/");

  auto CheckExpectedEvents = [&](nsDirectoryWatcher& ref_watcher, nsArrayPtr<ExpectedEvent> events)
  {
    nsDynamicArray<ExpectedEventStorage> firedEvents;
    nsUInt32 i = 0;
    ref_watcher.EnumerateChanges([&](nsStringView sPath, nsDirectoryWatcherAction action, nsDirectoryWatcherType type)
      {
      tmp = sPath;
      tmp.Shrink(sTestRootPath.GetCharacterCount(), 0);
      firedEvents.PushBack({tmp, action, type});
      if (i < events.GetCount())
      {
        NS_TEST_BOOL_MSG(tmp == events[i].path, "Expected event at index %d path mismatch: '%s' vs '%s'", i, tmp.GetData(), events[i].path);
        NS_TEST_BOOL_MSG(action == events[i].action, "Expected event at index %d action", i);
        NS_TEST_BOOL_MSG(type == events[i].type, "Expected event at index %d type mismatch", i);
      }
      i++; },
      nsTime::MakeFromMilliseconds(100));
    NS_TEST_BOOL_MSG(firedEvents.GetCount() == events.GetCount(), "Directory watcher did not fire expected amount of events");
  };

  auto CheckExpectedEventsUnordered = [&](nsDirectoryWatcher& ref_watcher, nsArrayPtr<ExpectedEvent> events)
  {
    nsDynamicArray<ExpectedEventStorage> firedEvents;
    nsUInt32 i = 0;
    nsDynamicArray<bool> eventFired;
    eventFired.SetCount(events.GetCount());
    ref_watcher.EnumerateChanges([&](nsStringView sPath, nsDirectoryWatcherAction action, nsDirectoryWatcherType type)
      {
        tmp = sPath;
        tmp.Shrink(sTestRootPath.GetCharacterCount(), 0);
        firedEvents.PushBack({tmp, action, type});
        auto index = events.IndexOf({tmp, action, type});
        NS_TEST_BOOL_MSG(index != nsInvalidIndex, "Event %d (%s, %d, %d) not found in expected events list", i, tmp.GetData(), (int)action, (int)type);
        if (index != nsInvalidIndex)
        {
          eventFired[index] = true;
        }
        i++;
        //
      },
      nsTime::MakeFromMilliseconds(100));
    for (auto& fired : eventFired)
    {
      NS_TEST_BOOL(fired);
    }
    NS_TEST_BOOL_MSG(firedEvents.GetCount() == events.GetCount(), "Directory watcher did not fire expected amount of events");
  };

  auto CheckExpectedEventsMultiple = [&](nsArrayPtr<nsDirectoryWatcher*> watchers, nsArrayPtr<ExpectedEvent> events)
  {
    nsDynamicArray<ExpectedEventStorage> firedEvents;
    nsUInt32 i = 0;
    nsDirectoryWatcher::EnumerateChanges(
      watchers, [&](nsStringView sPath, nsDirectoryWatcherAction action, nsDirectoryWatcherType type)
      {
        tmp = sPath;
        tmp.Shrink(sTestRootPath.GetCharacterCount(), 0);
        firedEvents.PushBack({tmp, action, type});
        if (i < events.GetCount())
        {
          NS_TEST_BOOL_MSG(tmp == events[i].path, "Expected event at index %d path mismatch: '%s' vs '%s'", i, tmp.GetData(), events[i].path);
          NS_TEST_BOOL_MSG(action == events[i].action, "Expected event at index %d action", i);
          NS_TEST_BOOL_MSG(type == events[i].type, "Expected event at index %d type mismatch", i);
        }
        i++;
        //
      },
      nsTime::MakeFromMilliseconds(100));
    NS_TEST_BOOL_MSG(firedEvents.GetCount() == events.GetCount(), "Directory watcher did not fire expected amount of events");
  };

  auto CreateFile = [&](const char* szRelPath)
  {
    tmp = sTestRootPath;
    tmp.AppendPath(szRelPath);

    nsOSFile file;
    NS_TEST_BOOL(file.Open(tmp, nsFileOpenMode::Write).Succeeded());
    NS_TEST_BOOL(file.Write("Hello World", 11).Succeeded());
  };

  auto ModifyFile = [&](const char* szRelPath)
  {
    tmp = sTestRootPath;
    tmp.AppendPath(szRelPath);

    nsOSFile file;
    NS_TEST_BOOL(file.Open(tmp, nsFileOpenMode::Append).Succeeded());
    NS_TEST_BOOL(file.Write("Hello World", 11).Succeeded());
  };

  auto DeleteFile = [&](const char* szRelPath)
  {
    tmp = sTestRootPath;
    tmp.AppendPath(szRelPath);
    NS_TEST_BOOL(nsOSFile::DeleteFile(tmp).Succeeded());
  };

  auto CreateDirectory = [&](const char* szRelPath)
  {
    tmp = sTestRootPath;
    tmp.AppendPath(szRelPath);
    NS_TEST_BOOL(nsOSFile::CreateDirectoryStructure(tmp).Succeeded());
  };

  auto Rename = [&](const char* szFrom, const char* szTo)
  {
    tmp = sTestRootPath;
    tmp.AppendPath(szFrom);

    tmp2 = sTestRootPath;
    tmp2.AppendPath(szTo);

    NS_TEST_BOOL(nsOSFile::MoveFileOrDirectory(tmp, tmp2).Succeeded());
  };

  auto DeleteDirectory = [&](const char* szRelPath, bool bTest = true)
  {
    tmp = sTestRootPath;
    tmp.AppendPath(szRelPath);
    tmp.MakeCleanPath();

    if (bTest)
    {
      NS_TEST_BOOL(nsOSFile::DeleteFolder(tmp).Succeeded());
    }
    else
    {
      nsOSFile::DeleteFolder(tmp).IgnoreResult();
    }
  };

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Simple Create File")
  {
    nsOSFile::DeleteFolder(sTestRootPath).IgnoreResult();
    NS_TEST_BOOL(nsOSFile::CreateDirectoryStructure(sTestRootPath).Succeeded());

    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(sTestRootPath, nsDirectoryWatcher::Watch::Creates | nsDirectoryWatcher::Watch::Writes).Succeeded());

    CreateFile("test.file");

    ExpectedEvent expectedEvents[] = {
      {"test.file", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
    };
    CheckExpectedEvents(watcher, expectedEvents);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Simple delete file")
  {
    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(sTestRootPath, nsDirectoryWatcher::Watch::Deletes).Succeeded());

    DeleteFile("test.file");

    ExpectedEvent expectedEvents[] = {
      {"test.file", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::File},
    };
    CheckExpectedEvents(watcher, expectedEvents);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Simple modify file")
  {
    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(sTestRootPath, nsDirectoryWatcher::Watch::Writes).Succeeded());

    CreateFile("test.file");

    TickWatcher(watcher);

    ModifyFile("test.file");

    ExpectedEvent expectedEvents[] = {
      {"test.file", nsDirectoryWatcherAction::Modified, nsDirectoryWatcherType::File},
    };
    CheckExpectedEvents(watcher, expectedEvents);

    DeleteFile("test.file");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Simple rename file")
  {
    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(sTestRootPath, nsDirectoryWatcher::Watch::Renames | nsDirectoryWatcher::Watch::Creates | nsDirectoryWatcher::Watch::Deletes | nsDirectoryWatcher::Watch::Writes | nsDirectoryWatcher::Watch::Subdirectories).Succeeded());

    CreateFile("test.file");
    Rename("test.file", "supertest.file");

    ExpectedEvent expectedEvents[] = {
      {"test.file", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
      {"test.file", nsDirectoryWatcherAction::RenamedOldName, nsDirectoryWatcherType::File},
      {"supertest.file", nsDirectoryWatcherAction::RenamedNewName, nsDirectoryWatcherType::File},
    };
    CheckExpectedEvents(watcher, expectedEvents);

    DeleteFile("supertest.file");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Change file casing")
  {
    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(sTestRootPath, nsDirectoryWatcher::Watch::Renames | nsDirectoryWatcher::Watch::Creates | nsDirectoryWatcher::Watch::Deletes | nsDirectoryWatcher::Watch::Writes | nsDirectoryWatcher::Watch::Subdirectories).Succeeded());

    CreateFile("rename.file");
    Rename("rename.file", "Rename.file");

    ExpectedEvent expectedEvents[] = {
      {"rename.file", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
      {"rename.file", nsDirectoryWatcherAction::RenamedOldName, nsDirectoryWatcherType::File},
      {"Rename.file", nsDirectoryWatcherAction::RenamedNewName, nsDirectoryWatcherType::File},
    };
    CheckExpectedEvents(watcher, expectedEvents);

    DeleteFile("Rename.file");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Windows check for correct handling of pending file remove event #1")
  {
    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(sTestRootPath, nsDirectoryWatcher::Watch::Renames | nsDirectoryWatcher::Watch::Creates | nsDirectoryWatcher::Watch::Deletes | nsDirectoryWatcher::Watch::Writes | nsDirectoryWatcher::Watch::Subdirectories).Succeeded());

    CreateFile("rename.file");
    DeleteFile("rename.file");

    ExpectedEvent expectedEvents[] = {
      {"rename.file", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
      {"rename.file", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::File},
    };
    CheckExpectedEvents(watcher, expectedEvents);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Windows check for correct handling of pending file remove event #2")
  {
    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(sTestRootPath, nsDirectoryWatcher::Watch::Renames | nsDirectoryWatcher::Watch::Creates | nsDirectoryWatcher::Watch::Deletes | nsDirectoryWatcher::Watch::Writes | nsDirectoryWatcher::Watch::Subdirectories).Succeeded());

    CreateFile("rename.file");
    DeleteFile("rename.file");
    CreateFile("Rename.file");
    DeleteFile("Rename.file");

    ExpectedEvent expectedEvents[] = {
      {"rename.file", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
      {"rename.file", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::File},
      {"Rename.file", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
      {"Rename.file", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::File},
    };
    CheckExpectedEvents(watcher, expectedEvents);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Simple create directory")
  {
    nsOSFile::DeleteFolder(sTestRootPath).IgnoreResult();
    NS_TEST_BOOL(nsOSFile::CreateDirectoryStructure(sTestRootPath).Succeeded());

    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(sTestRootPath, nsDirectoryWatcher::Watch::Creates).Succeeded());

    CreateDirectory("testDir");

    ExpectedEvent expectedEvents[] = {
      {"testDir", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::Directory},
    };
    CheckExpectedEvents(watcher, expectedEvents);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Simple delete directory")
  {
    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(sTestRootPath, nsDirectoryWatcher::Watch::Deletes).Succeeded());

    DeleteDirectory("testDir");

    ExpectedEvent expectedEvents[] = {
      {"testDir", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::Directory},
    };
    CheckExpectedEvents(watcher, expectedEvents);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Simple rename directory")
  {
    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(sTestRootPath, nsDirectoryWatcher::Watch::Renames).Succeeded());

    CreateDirectory("testDir");
    Rename("testDir", "supertestDir");

    ExpectedEvent expectedEvents[] = {
      {"testDir", nsDirectoryWatcherAction::RenamedOldName, nsDirectoryWatcherType::Directory},
      {"supertestDir", nsDirectoryWatcherAction::RenamedNewName, nsDirectoryWatcherType::Directory},
    };
    CheckExpectedEvents(watcher, expectedEvents);

    DeleteDirectory("supertestDir");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Change directory casing")
  {
    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(sTestRootPath, nsDirectoryWatcher::Watch::Renames | nsDirectoryWatcher::Watch::Creates | nsDirectoryWatcher::Watch::Deletes | nsDirectoryWatcher::Watch::Writes | nsDirectoryWatcher::Watch::Subdirectories).Succeeded());

    CreateDirectory("renameDir");
    Rename("renameDir", "RenameDir");

    ExpectedEvent expectedEvents[] = {
      {"renameDir", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::Directory},
      {"renameDir", nsDirectoryWatcherAction::RenamedOldName, nsDirectoryWatcherType::Directory},
      {"RenameDir", nsDirectoryWatcherAction::RenamedNewName, nsDirectoryWatcherType::Directory},
    };
    CheckExpectedEvents(watcher, expectedEvents);

    DeleteDirectory("RenameDir");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Windows check for correct handling of pending directory remove event #1")
  {
    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(sTestRootPath, nsDirectoryWatcher::Watch::Renames | nsDirectoryWatcher::Watch::Creates | nsDirectoryWatcher::Watch::Deletes | nsDirectoryWatcher::Watch::Writes | nsDirectoryWatcher::Watch::Subdirectories).Succeeded());

    CreateDirectory("renameDir");
    DeleteDirectory("renameDir");

    ExpectedEvent expectedEvents[] = {
      {"renameDir", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::Directory},
      {"renameDir", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::Directory},
    };
    CheckExpectedEvents(watcher, expectedEvents);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Windows check for correct handling of pending directory remove event #2")
  {
    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(sTestRootPath, nsDirectoryWatcher::Watch::Renames | nsDirectoryWatcher::Watch::Creates | nsDirectoryWatcher::Watch::Deletes | nsDirectoryWatcher::Watch::Writes | nsDirectoryWatcher::Watch::Subdirectories).Succeeded());

    CreateDirectory("renameDir");
    DeleteDirectory("renameDir");
    CreateDirectory("RenameDir");
    DeleteDirectory("RenameDir");

    ExpectedEvent expectedEvents[] = {
      {"renameDir", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::Directory},
      {"renameDir", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::Directory},
      {"RenameDir", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::Directory},
      {"RenameDir", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::Directory},
    };
    CheckExpectedEvents(watcher, expectedEvents);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Subdirectory Create File")
  {
    tmp = sTestRootPath;
    tmp.AppendPath("subdir");
    NS_TEST_BOOL(nsOSFile::CreateDirectoryStructure(tmp).Succeeded());

    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(sTestRootPath, nsDirectoryWatcher::Watch::Creates | nsDirectoryWatcher::Watch::Subdirectories).Succeeded());

    CreateFile("subdir/test.file");

    ExpectedEvent expectedEvents[] = {
      {"subdir/test.file", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
    };
    CheckExpectedEvents(watcher, expectedEvents);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Subdirectory delete file")
  {

    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(sTestRootPath, nsDirectoryWatcher::Watch::Deletes | nsDirectoryWatcher::Watch::Subdirectories).Succeeded());

    DeleteFile("subdir/test.file");

    ExpectedEvent expectedEvents[] = {
      {"subdir/test.file", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::File},
    };
    CheckExpectedEvents(watcher, expectedEvents);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Subdirectory modify file")
  {
    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(sTestRootPath, nsDirectoryWatcher::Watch::Writes | nsDirectoryWatcher::Watch::Subdirectories).Succeeded());

    CreateFile("subdir/test.file");

    TickWatcher(watcher);

    ModifyFile("subdir/test.file");

    ExpectedEvent expectedEvents[] = {
      {"subdir/test.file", nsDirectoryWatcherAction::Modified, nsDirectoryWatcherType::File},
    };
    CheckExpectedEvents(watcher, expectedEvents);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GUI Create Folder & file")
  {
    DeleteDirectory("sub", false);
    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(
                          sTestRootPath,
                          nsDirectoryWatcher::Watch::Creates | nsDirectoryWatcher::Watch::Deletes |
                            nsDirectoryWatcher::Watch::Writes | nsDirectoryWatcher::Watch::Subdirectories)
                   .Succeeded());

    CreateDirectory("New Folder");

    ExpectedEvent expectedEvents1[] = {
      {"New Folder", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::Directory},
    };
    CheckExpectedEvents(watcher, expectedEvents1);

    Rename("New Folder", "sub");

    CreateFile("sub/bla");

    ExpectedEvent expectedEvents2[] = {
      {"sub/bla", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
    };
    CheckExpectedEvents(watcher, expectedEvents2);

    ModifyFile("sub/bla");
    DeleteFile("sub/bla");

    ExpectedEvent expectedEvents3[] = {
      {"sub/bla", nsDirectoryWatcherAction::Modified, nsDirectoryWatcherType::File},
      {"sub/bla", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::File},
    };
    CheckExpectedEvents(watcher, expectedEvents3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GUI Create Folder & file fast")
  {
    DeleteDirectory("sub", false);
    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(
                          sTestRootPath,
                          nsDirectoryWatcher::Watch::Creates | nsDirectoryWatcher::Watch::Deletes |
                            nsDirectoryWatcher::Watch::Writes | nsDirectoryWatcher::Watch::Subdirectories)
                   .Succeeded());

    CreateDirectory("New Folder");
    Rename("New Folder", "sub");

    ExpectedEvent expectedEvents1[] = {
      {"New Folder", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::Directory},
    };
    CheckExpectedEvents(watcher, expectedEvents1);

    CreateFile("sub/bla");

    ExpectedEvent expectedEvents2[] = {
      {"sub/bla", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
    };
    CheckExpectedEvents(watcher, expectedEvents2);

    ModifyFile("sub/bla");
    DeleteFile("sub/bla");

    ExpectedEvent expectedEvents3[] = {
      {"sub/bla", nsDirectoryWatcherAction::Modified, nsDirectoryWatcherType::File},
      {"sub/bla", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::File},
    };
    CheckExpectedEvents(watcher, expectedEvents3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GUI Create Folder & file fast subdir")
  {
    DeleteDirectory("sub", false);

    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(
                          sTestRootPath,
                          nsDirectoryWatcher::Watch::Creates | nsDirectoryWatcher::Watch::Deletes |
                            nsDirectoryWatcher::Watch::Writes | nsDirectoryWatcher::Watch::Subdirectories)
                   .Succeeded());

    CreateDirectory("New Folder/subsub");
    Rename("New Folder", "sub");

    TickWatcher(watcher);

    CreateFile("sub/subsub/bla");

    ExpectedEvent expectedEvents2[] = {
      {"sub/subsub/bla", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
    };
    CheckExpectedEvents(watcher, expectedEvents2);

    ModifyFile("sub/subsub/bla");
    DeleteFile("sub/subsub/bla");

    ExpectedEvent expectedEvents3[] = {
      {"sub/subsub/bla", nsDirectoryWatcherAction::Modified, nsDirectoryWatcherType::File},
      {"sub/subsub/bla", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::File},
    };
    CheckExpectedEvents(watcher, expectedEvents3);

    DeleteDirectory("sub");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GUI Delete Folder")
  {
    DeleteDirectory("sub2", false);
    DeleteDirectory("../sub2", false);

    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(
                          sTestRootPath,
                          nsDirectoryWatcher::Watch::Creates | nsDirectoryWatcher::Watch::Deletes |
                            nsDirectoryWatcher::Watch::Writes | nsDirectoryWatcher::Watch::Subdirectories)
                   .Succeeded());

    CreateDirectory("sub2/subsub2");
    CreateFile("sub2/file1");
    CreateFile("sub2/subsub2/file2.txt");

    ExpectedEvent expectedEvents1[] = {
      {"sub2", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::Directory},
      {"sub2/file1", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
      {"sub2/subsub2", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::Directory},
      {"sub2/subsub2/file2.txt", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
    };
    CheckExpectedEventsUnordered(watcher, expectedEvents1);

    Rename("sub2", "../sub2");

    ExpectedEvent expectedEvents2[] = {
      {"sub2/subsub2/file2.txt", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::File},
      {"sub2/subsub2", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::Directory},
      {"sub2/file1", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::File},
      {"sub2", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::Directory},
    };
    // Issue here: After moving sub2 out of view, it remains in m_pathToWd
    CheckExpectedEvents(watcher, expectedEvents2);

    Rename("../sub2", "sub2");

    ExpectedEvent expectedEvents3[] = {
      {"sub2", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::Directory},
      {"sub2/file1", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
      {"sub2/subsub2", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::Directory},
      {"sub2/subsub2/file2.txt", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
    };
    CheckExpectedEventsUnordered(watcher, expectedEvents3);

    DeleteDirectory("sub2");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Create, Delete, Create")
  {
    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(
                          sTestRootPath,
                          nsDirectoryWatcher::Watch::Creates | nsDirectoryWatcher::Watch::Deletes |
                            nsDirectoryWatcher::Watch::Writes | nsDirectoryWatcher::Watch::Subdirectories)
                   .Succeeded());

    CreateDirectory("sub2/subsub2");
    CreateFile("sub2/file1");
    CreateFile("sub2/subsub2/file2.txt");

    ExpectedEvent expectedEvents1[] = {
      {"sub2", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::Directory},
      {"sub2/file1", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
      {"sub2/subsub2", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::Directory},
      {"sub2/subsub2/file2.txt", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
    };
    CheckExpectedEventsUnordered(watcher, expectedEvents1);

    DeleteDirectory("sub2");

    ExpectedEvent expectedEvents2[] = {
      {"sub2/file1", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::File},
      {"sub2/subsub2/file2.txt", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::File},
      {"sub2/subsub2", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::Directory},
      {"sub2", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::Directory},
    };
    CheckExpectedEventsUnordered(watcher, expectedEvents2);

    CreateDirectory("sub2/subsub2");
    CreateFile("sub2/file1");
    CreateFile("sub2/subsub2/file2.txt");

    ExpectedEvent expectedEvents3[] = {
      {"sub2", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::Directory},
      {"sub2/file1", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
      {"sub2/subsub2", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::Directory},
      {"sub2/subsub2/file2.txt", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
    };
    CheckExpectedEventsUnordered(watcher, expectedEvents3);

    DeleteDirectory("sub2");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GUI Create file & delete")
  {
    DeleteDirectory("sub", false);
    nsDirectoryWatcher watcher;
    NS_TEST_BOOL(watcher.OpenDirectory(
                          sTestRootPath,
                          nsDirectoryWatcher::Watch::Creates | nsDirectoryWatcher::Watch::Deletes |
                            nsDirectoryWatcher::Watch::Writes | nsDirectoryWatcher::Watch::Renames)
                   .Succeeded());

    CreateFile("file2.txt");

    ExpectedEvent expectedEvents1[] = {
      {"file2.txt", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
    };
    CheckExpectedEvents(watcher, expectedEvents1);

    Rename("file2.txt", "datei2.txt");

    ExpectedEvent expectedEvents2[] = {
      {"file2.txt", nsDirectoryWatcherAction::RenamedOldName, nsDirectoryWatcherType::File},
      {"datei2.txt", nsDirectoryWatcherAction::RenamedNewName, nsDirectoryWatcherType::File},
    };
    CheckExpectedEvents(watcher, expectedEvents2);

    DeleteFile("datei2.txt");

    ExpectedEvent expectedEvents3[] = {
      {"datei2.txt", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::File},
    };
    CheckExpectedEvents(watcher, expectedEvents3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Enumerate multiple")
  {
    DeleteDirectory("watch1", false);
    DeleteDirectory("watch2", false);
    DeleteDirectory("watch3", false);
    nsDirectoryWatcher watchers[3];

    nsDirectoryWatcher* pWatchers[] = {watchers + 0, watchers + 1, watchers + 2};

    CreateDirectory("watch1");
    CreateDirectory("watch2");
    CreateDirectory("watch3");

    nsStringBuilder watchPath;

    watchPath = sTestRootPath;
    watchPath.AppendPath("watch1");
    NS_TEST_BOOL(watchers[0].OpenDirectory(
                              watchPath,
                              nsDirectoryWatcher::Watch::Creates | nsDirectoryWatcher::Watch::Deletes |
                                nsDirectoryWatcher::Watch::Writes | nsDirectoryWatcher::Watch::Renames)
                   .Succeeded());

    watchPath = sTestRootPath;
    watchPath.AppendPath("watch2");
    NS_TEST_BOOL(watchers[1].OpenDirectory(
                              watchPath,
                              nsDirectoryWatcher::Watch::Creates | nsDirectoryWatcher::Watch::Deletes |
                                nsDirectoryWatcher::Watch::Writes | nsDirectoryWatcher::Watch::Renames)
                   .Succeeded());

    watchPath = sTestRootPath;
    watchPath.AppendPath("watch3");
    NS_TEST_BOOL(watchers[2].OpenDirectory(
                              watchPath,
                              nsDirectoryWatcher::Watch::Creates | nsDirectoryWatcher::Watch::Deletes |
                                nsDirectoryWatcher::Watch::Writes | nsDirectoryWatcher::Watch::Renames)
                   .Succeeded());

    CreateFile("watch1/file2.txt");

    ExpectedEvent expectedEvents1[] = {
      {"watch1/file2.txt", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
    };
    CheckExpectedEventsMultiple(pWatchers, expectedEvents1);

    CreateFile("watch2/file2.txt");

    ExpectedEvent expectedEvents2[] = {
      {"watch2/file2.txt", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
    };
    CheckExpectedEventsMultiple(pWatchers, expectedEvents2);

    CreateFile("watch3/file2.txt");

    ExpectedEvent expectedEvents3[] = {
      {"watch3/file2.txt", nsDirectoryWatcherAction::Added, nsDirectoryWatcherType::File},
    };
    CheckExpectedEventsMultiple(pWatchers, expectedEvents3);

    ModifyFile("watch1/file2.txt");
    ModifyFile("watch2/file2.txt");

    ExpectedEvent expectedEvents4[] = {
      {"watch1/file2.txt", nsDirectoryWatcherAction::Modified, nsDirectoryWatcherType::File},
      {"watch2/file2.txt", nsDirectoryWatcherAction::Modified, nsDirectoryWatcherType::File},
    };
    CheckExpectedEventsMultiple(pWatchers, expectedEvents4);

    DeleteFile("watch1/file2.txt");
    DeleteFile("watch2/file2.txt");
    DeleteFile("watch3/file2.txt");

    ExpectedEvent expectedEvents5[] = {
      {"watch1/file2.txt", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::File},
      {"watch2/file2.txt", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::File},
      {"watch3/file2.txt", nsDirectoryWatcherAction::Removed, nsDirectoryWatcherType::File},
    };
    CheckExpectedEventsMultiple(pWatchers, expectedEvents5);
  }

  nsOSFile::DeleteFolder(sTestRootPath).IgnoreResult();
}

#endif
