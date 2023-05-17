#pragma once

#include <Foundation/Communication/Event.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/IO/FileSystem/Implementation/DataDirType.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Threading/Mutex.h>

/// \brief The wdFileSystem provides high-level functionality to manage files in a virtual file system.
///
/// There are two sides at which the file system can be extended:
/// Data directories are the 'sources' of data. These can be simple folders, zip files, data-bases, HTTP servers, etc.
/// Different wdDataDirectoryType's can implement these different 'decoding methods', i.e. they handle how to actually
/// access the data and they use their own readers/writers to implement a common interface for passing data streams
/// to and from the data directory.
/// On the other end there are the actual file readers/writers, which implement policies how to optimize these reads/writes.
/// The default wdFileReader and wdFileWriter implement a buffering policy, i.e. they use an internal cache to only
/// sporadically read or write to the actual data stream.
/// A 'threaded' or 'parallel' file reader/writer could implement a different policy, where a file is read/written
/// in a thread and thus allows to have non-blocking file accesses.
///
/// Which policy to use is defined by the user every time he needs to access a file, by simply using the desired
/// reader/writer class.
/// How to mount data directories (i.e. with which wdDataDirectoryType) is defined by the 'DataDirFactories', which
/// are functions that create wdDataDirectoryType's. This way one can mount the same data directory (e.g. "MyTestDir")
/// differently, depending on which Factories have been registered previously.
/// This allows to easily configure how to set up data directories.
/// E.g. by default ordinary folders will be mounted to be read from the local file system.
/// However, by registering a different Factory, the same directory could also be mounted over a network on a remote
/// file serving machine.
///
/// Additionally wdFileSystem also broadcasts events about which files are (about to be) accessed.
/// This allows to hook into the system and implement stuff like automatic asset transformations before/after certain
/// file accesses, checking out files from revision control systems, or simply logging all file activity.
///
/// All operations that go through the wdFileSystem are protected by a mutex, which means that opening, closing, deleting
/// files, as well as adding or removing data directories etc. will be synchronized and cannot happen in parallel.
/// Reading/writing file streams can happen in parallel, only the administrative tasks need to be protected.
/// File events are broadcast as they occur, that means they will be executed on whichever thread triggered them.
/// Since they are executed from within the filesystem mutex, they cannot occur in parallel.
class WD_FOUNDATION_DLL wdFileSystem
{
public:
  /// \brief Enum that describes the type of file-event that occurred.
  struct FileEventType;

  /// \brief The data that is sent through the event interface.
  struct FileEvent;

  /// \brief Registers an Event Handler that will be informed about all the events that the file system broadcasts.
  static wdEventSubscriptionID RegisterEventHandler(wdEvent<const FileEvent&>::Handler handler);

  /// \brief Unregisters a previously registered Event Handler.
  static void UnregisterEventHandler(wdEvent<const FileEvent&>::Handler handler);
  static void UnregisterEventHandler(wdEventSubscriptionID subscriptionId);

public:
  /// \brief Describes in which mode a data directory is mounted.
  enum DataDirUsage
  {
    ReadOnly,
    AllowWrites,
  };

  /// \name Data Directory Modifications
  ///
  /// All functions that add / remove data directories are not thread safe and require that this is done
  /// on a single thread with no other thread accessing anything in wdFileSystem simultaneously.
  ///@{

  /// \brief This factory creates a data directory type, if it can handle the given data directory. Otherwise it returns nullptr.
  ///
  /// Every time a data directory is supposed to be added, the file system will query its data dir factories, which one
  /// can successfully create an wdDataDirectoryType. In this process the last factory added has the highest priority.
  /// Once a factory is found that was able to create a wdDataDirectoryType, that one is used.
  /// Different factories can be used to mount different types of data directories. But the same directory can also be
  /// mounted in different ways. For example a simple folder could be mounted on the local system, or via a HTTP server
  /// over a network (lets call it a 'FileServer'). Thus depending on which type of factories are registered, the file system
  /// can provide data from very different sources.
  using wdDataDirFactory = wdDataDirectoryType* (*)(wdStringView, wdStringView, wdStringView, wdFileSystem::DataDirUsage);

  /// \brief This function allows to register another data directory factory, which might be invoked when a new data directory is to be added.
  static void RegisterDataDirectoryFactory(wdDataDirFactory factory, float fPriority = 0); // [tested]

  /// \brief Will remove all known data directory factories.
  static void ClearAllDataDirectoryFactories() { s_pData->m_DataDirFactories.Clear(); } // [tested]

  /// \brief Adds a data directory. It will try all the registered factories to find a data directory type that can handle the given path.
  ///
  /// If Usage is ReadOnly, writing to the data directory is not allowed. This is independent of whether the data directory type
  /// COULD write anything.
  /// szGroup defines to what 'group' of data directories this data dir belongs. This is only used in calls to RemoveDataDirectoryGroup,
  /// to remove all data directories of the same group.
  /// You could use groups such as 'Base', 'Project', 'Settings', 'Level', 'Temp' to distinguish between different sets of data directories.
  /// You can also specify the exact same string as szDataDirectory for szGroup, and thus uniquely identify the data dir, to be able to remove just
  /// that one. szRootName is optional for read-only data dirs, but mandatory for writable ones. It has to be unique to clearly identify a file within
  /// that data directory. It must be used when writing to a file in this directory. For instance, if a data dir root name is "mydata", then the path
  /// ":mydata/SomeFile.txt" can be used to write to the top level folder of this data directory. The same can be used for reading exactly that file
  /// and ignoring the other data dirs.
  static wdResult AddDataDirectory(wdStringView sDataDirectory, wdStringView sGroup = {}, wdStringView sRootName = {}, wdFileSystem::DataDirUsage usage = ReadOnly); // [tested]

  /// \brief Searches for a data directory with the given root name and removes it
  ///
  /// Returns true, if one was found and removed, false if no such data dir existed.
  static bool RemoveDataDirectory(wdStringView sRootName);

  /// \brief Removes all data directories that belong to the given group. Returns the number of data directories that were removed.
  static wdUInt32 RemoveDataDirectoryGroup(wdStringView sGroup); // [tested]

  /// \brief Removes all data directories.
  static void ClearAllDataDirectories(); // [tested]

  /// \brief If a data directory with the given root name already exists, it will be returned, nullptr otherwise.
  static wdDataDirectoryType* FindDataDirectoryWithRoot(wdStringView sRootName);

  /// \brief Returns the number of currently active data directories.
  static wdUInt32 GetNumDataDirectories(); // [tested]

  /// \brief Returns the n-th currently active data directory.
  static wdDataDirectoryType* GetDataDirectory(wdUInt32 uiDataDirIndex); // [tested]

  /// \brief Calls wdDataDirectoryType::ReloadExternalConfigs() on all active data directories.
  static void ReloadAllExternalDataDirectoryConfigs();

  ///@}
  /// \name Special Directories
  ///@{

  /// \brief Searches for a directory to use as the "Sdk" special directory
  ///
  /// It does so by starting at the directory where the application binary is located and then goes up
  /// until it finds a folder that contains the given sub-folder. The sub-folder is usually where the
  /// engine loads the most basic data from, so it should exist.
  ///
  /// Additionally the 'redirection file' feature of wdFileSystem::FindFolderWithSubPath() is used
  /// to allow finding a relocated SDK folder. To do that, place a file called 'wdSdkRoot.txt'
  /// in your top level folder. It should contain the relative path pointing to the SDK folder.
  ///
  /// Upon success SetSdkRootDirectory() is called with the resulting path.
  ///
  /// \note If the Sdk root directory has been set before, this function does nothing!
  /// It will not override a previously set value. If that is desired, call SetSdkRootDirectory("") first.
  ///
  /// \sa wdFileSystem::FindFolderWithSubPath()
  static wdResult DetectSdkRootDirectory(wdStringView sExpectedSubFolder = "Data/Base");

  /// \brief the special directory ">Sdk" is the root folder of the SDK data, it is often used as the main reference
  /// from where other data directories are found. For higher level code (e.g. wdApplication) it is often vital that this is set early at startup.
  ///
  /// \sa DetectSdkRootDirectory()
  static void SetSdkRootDirectory(wdStringView sSdkDir);

  /// \brief Returns the previously set Sdk root directory.
  ///
  /// \note Asserts that the path is not empty!
  ///
  /// \sa SetSdkRootDirectory
  /// \sa DetectSdkRootDirectory
  static const char* GetSdkRootDirectory();

  /// \brief Special directories are used when mounting data directories as basic references.
  ///
  /// They are indicated with a ">", ie. ">sdk/Test", but using them is only allowed in few places, e.g. in AddDataDirectory().
  /// Special directories are needed to be able to set up other paths relative to them and to be able to use different
  /// ones on different PCs. For instance when using file-serve functionality, the special directories may be different
  /// on the host and client machines, but the paths used to mount data directories can stay the same because of this.
  static void SetSpecialDirectory(wdStringView sName, wdStringView sReplacement);

  /// \brief Returns the absolute path to \a szDirectory.
  ///
  /// If the path starts with a known special directory marker (">marker/") it is replaced accordingly.
  /// See SetSpecialDirectory() for setting custom special directories.
  ///
  /// Built-in special directories (always available) are:
  ///
  /// ">sdk/" - Resolves to what GetSdkRootDirectory() returns.
  /// ">user/" - Resolves to what wdOSFile::GetUserDataFolder() returns.
  /// ">temp/" - Resolves to what wdOSFile::GetTempDataFolder() returns.
  /// ">appdir/" - Resolves to what wdOSFile::GetApplicationDirectory() returns.
  ///
  /// Returns WD_FAILURE if \a szDirectory starts with an unknown special directory.
  static wdResult ResolveSpecialDirectory(wdStringView sDirectory, wdStringBuilder& out_sPath);

  ///@}

  /// \name Misc
  ///@{

  /// \brief Returns the (recursive) mutex that is used internally by the file system which can be used to guard bundled operations on the file
  /// system.
  static wdMutex& GetMutex();

#if WD_ENABLED(WD_SUPPORTS_FILE_ITERATORS)
  /// \brief Starts a multi-folder search for \a szSearchTerm on all current data directories.
  static void StartSearch(wdFileSystemIterator& ref_iterator, wdStringView sSearchTerm, wdBitflags<wdFileSystemIteratorFlags> flags = wdFileSystemIteratorFlags::Default);
#endif

  ///@}

  static wdResult CreateDirectoryStructure(wdStringView sPath);

public:
  /// \brief Deletes the given file from all data directories, if possible.
  ///
  /// The path must be absolute or rooted, to uniquely identify which file to delete.
  /// For example ":appdata/SomeData.txt", assuming a writable data directory has been mounted with the "appdata" root name.
  static void DeleteFile(wdStringView sFile); // [tested]

  /// \brief Checks whether the given file exists in any data directory.
  ///
  /// The search can be restricted to directories of certain categories (see AddDataDirectory).
  static bool ExistsFile(wdStringView sFile); // [tested]

  /// \brief Tries to get the wdFileStats for the given file.
  /// Typically should give the same results as wdOSFile::GetFileStats, but some data dir implementations may not support
  /// retrieving all data (e.g. GetFileStats on folders might not always work).
  static wdResult GetFileStats(wdStringView sFileOrFolder, wdFileStats& out_stats);

  /// \brief Tries to resolve the given path and returns the absolute and relative path to the final file.
  ///
  /// If the given path is a rooted path, for instance something like ":appdata/UserData.txt", (which is necessary for writing to files),
  /// the path can be converted easily and the file does not need to exist. Only the data directory with the given root name must be mounted.
  ///
  /// If the path is relative, it is attempted to open the specified file, which means it is searched in all available
  /// data directories. The path to the file that is found will be returned.
  ///
  /// \param out_sAbsolutePath will contain the absolute path to the file. Can be nullptr.
  /// \param out_sDataDirRelativePath will contain the relative path to the file (from the data directory in which it might end up in). Can be
  /// nullptr.
  /// \param szPath can be a relative, an absolute or a rooted path. This can also be used to find the relative location to the data
  /// directory that would handle it.
  /// \param out_ppDataDir If not null, it will be set to the data directory that would handle this path.
  ///
  /// \returns The function will return WD_FAILURE if it was not able to determine any location where the file could be read from or written to.
  static wdResult ResolvePath(wdStringView sPath, wdStringBuilder* out_pAbsolutePath, wdStringBuilder* out_pDataDirRelativePath, wdDataDirectoryType** out_pDataDir = nullptr); // [tested]

  /// \brief Starts at szStartDirectory and goes up until it finds a folder that contains the given sub folder structure.
  ///
  /// Returns WD_FAILURE if nothing is found. Otherwise \a result is the absolute path to the existing folder that has a given sub-folder.
  ///
  /// \param result If successful, this contains the folder path in which szSubPath exists.
  /// \param szStartDirectory The directory in which to start the search and iterate upwards.
  /// \param szSubPath the relative path to look for in each visited directory. The function succeeds if such a file or folder is found.
  /// \param szRedirectionFileName An optional file name for a redirection file. If in any visited folder a file with this name is found, it will be opened, read entirely, and appended to the current search path, and it is checked whether \a szSubPath can be found there. This step is not recursive and can't result in an endless loop. It allows to relocate the SDK folder and still have it found, by placing such a redirection file. A common use case, is when wdEngine is used as a Git submodule and therefore the overall file structure is slightly different.
  static wdResult FindFolderWithSubPath(wdStringBuilder& ref_sResult, wdStringView sStartDirectory, wdStringView sSubPath, wdStringView sRedirectionFileName = {}); // [tested]

  /// \brief Returns true, if any data directory knows how to redirect the given path. Otherwise the original string is returned in out_sRedirection.
  static bool ResolveAssetRedirection(wdStringView sPathOrAssetGuid, wdStringBuilder& out_sRedirection);

  /// \brief Migrates a file from an old location to a new one, and returns the path that should be used to open it (either the old or the new path).
  ///
  /// If the file does not exist in the old location, nothing is done, and the new location is returned.
  /// Otherwise, it is attempted to move the file from the old location to the new location.
  /// In case that fails (target not writeable or so), the old path is returned, so that code that needs to read that file, finds it in the correct location.
  /// If it succeeds, the new location is returned. Afterwards, the file does not exist in the old location anymore.
  static wdStringView MigrateFileLocation(wdStringView sOldLocation, wdStringView sNewLocation);

private:
  friend class wdDataDirectoryReaderWriterBase;
  friend class wdFileReaderBase;
  friend class wdFileWriterBase;

  /// \brief This is used by the actual file readers (like wdFileReader) to get an abstract file reader.
  ///
  /// It tries all data directories, to find the given file.
  /// szFile can be an absolute or relative path.
  /// If bAllowFileEvents is true, the file system will broadcast events about its activity.
  /// This should usually be set to true, unless code is already acting on a file event and needs to do a file operation
  /// itself, which should not trigger an endless recursion of file events.
  static wdDataDirectoryReader* GetFileReader(wdStringView sFile, wdFileShareMode::Enum FileShareMode, bool bAllowFileEvents);

  /// \brief This is used by the actual file writers (like wdFileWriter) to get an abstract file writer.
  ///
  /// It tries all data directories, to find where the given file could be written to.
  /// szFile can be an absolute or relative path.
  /// If bAllowFileEvents is true, the file system will broadcast events about its activity.
  /// This should usually be set to true, unless code is already acting on a file event and needs to do a file operation
  /// itself, which should not trigger an endless recursion of file events.
  static wdDataDirectoryWriter* GetFileWriter(wdStringView sFile, wdFileShareMode::Enum FileShareMode, bool bAllowFileEvents);


private:
  WD_MAKE_SUBSYSTEM_STARTUP_FRIEND(Foundation, FileSystem);

  /// \brief Called by the Startup System to initialize the file system
  static void Startup();

  /// \brief Called by the Startup System to shutdown the file system
  static void Shutdown();

private:
  struct DataDirectory
  {
    DataDirUsage m_Usage;

    wdString m_sRootName;
    wdString m_sGroup;
    wdDataDirectoryType* m_pDataDirectory;
  };

  struct Factory
  {
    WD_DECLARE_POD_TYPE();

    float m_fPriority;
    wdDataDirFactory m_Factory;
  };

  struct FileSystemData
  {
    wdHybridArray<Factory, 4> m_DataDirFactories;
    wdHybridArray<DataDirectory, 16> m_DataDirectories;

    wdEvent<const FileEvent&, wdMutex> m_Event;
    wdMutex m_FsMutex;
  };

  /// \brief Returns a list of data directory categories that were embedded in the path.
  static wdStringView ExtractRootName(wdStringView sFile, wdString& rootName);

  /// \brief Returns the given path relative to its data directory. The path must be inside the given data directory.
  static wdStringView GetDataDirRelativePath(wdStringView sFile, wdUInt32 uiDataDir);

  static DataDirectory* GetDataDirForRoot(const wdString& sRoot);

  static void CleanUpRootName(wdStringBuilder& sRoot);

  static wdString s_sSdkRootDir;
  static wdMap<wdString, wdString> s_SpecialDirectories;
  static FileSystemData* s_pData;
};

/// \brief Describes the type of events that are broadcast by the wdFileSystem.
struct wdFileSystem::FileEventType
{
  enum Enum
  {
    None,                      ///< None. Should not occur.
    OpenFileAttempt,           ///< A file is about to be opened for reading.
    OpenFileSucceeded,         ///< A file has been successfully opened for reading.
    OpenFileFailed,            ///< Opening a file for reading failed. Probably because it doesn't exist.
    CreateFileAttempt,         ///< A file is about to be opened for writing.
    CreateFileSucceeded,       ///< A file has been successfully opened for writing.
    CreateFileFailed,          ///< Opening a file for writing failed.
    CloseFile,                 ///< A file was closed.
    AddDataDirectorySucceeded, ///< A data directory was successfully added.
    AddDataDirectoryFailed,    ///< Adding a data directory failed. No factory could handle it (or there were none).
    RemoveDataDirectory,       ///< A data directory was removed. IMPORTANT: This is where ResourceManagers should check if some loaded resources need to be
                               ///< purged.
    DeleteFile                 ///< A file is about to be deleted.
  };
};

/// \brief The event data that is broadcast by the wdFileSystem upon certain file operations.
struct wdFileSystem::FileEvent
{
  /// \brief The exact event that occurred.
  wdFileSystem::FileEventType::Enum m_EventType = FileEventType::None;

  /// \brief Path to the file or directory that was involved.
  wdStringView m_sFileOrDirectory;

  /// \brief Additional Path / Name that might be of interest.
  wdStringView m_sOther;

  /// \brief The data-directory, that was involved.
  const wdDataDirectoryType* m_pDataDir = nullptr;
};
