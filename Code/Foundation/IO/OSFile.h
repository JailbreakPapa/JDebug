#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Communication/Event.h>
#include <Foundation/IO/FileEnums.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Threading/AtomicInteger.h>
#include <Foundation/Threading/Mutex.h>
#include <Foundation/Time/Timestamp.h>

struct wdOSFileData;

#if WD_ENABLED(WD_USE_POSIX_FILE_API)
#  include <Foundation/IO/Implementation/Posix/OSFileDeclarations_posix.h>
#elif WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/IO/Implementation/Win/OSFileDeclarations_win.h>
#endif

/// \brief Defines in which mode to open a file.
struct wdFileOpenMode
{
  enum Enum
  {
    None,   ///< None, only used internally.
    Read,   ///< Open file for reading.
    Write,  ///< Open file for writing (already existing data is discarded).
    Append, ///< Open file for appending (writing, but always only at the end, already existing data is preserved).
  };
};

/// \brief Holds the stats for a file.
struct WD_FOUNDATION_DLL wdFileStats
{
  wdFileStats();
  ~wdFileStats();

  /// \brief Stores the concatenated m_sParentPath and m_sName in \a path.
  void GetFullPath(wdStringBuilder& ref_sPath) const;

  /// \brief Path to the parent folder.
  /// Append m_sName to m_sParentPath to obtain the full path.
  wdStringBuilder m_sParentPath;

  /// \brief The name of the file or folder that the stats are for. Does not include the parent path to it.
  /// Append m_sName to m_sParentPath to obtain the full path.
  wdString m_sName;

  /// \brief The last modification time as an UTC timestamp since Unix epoch.
  wdTimestamp m_LastModificationTime;

  /// \brief The size of the file in bytes.
  wdUInt64 m_uiFileSize = 0;

  /// \brief Whether the file object is a file or folder.
  bool m_bIsDirectory = false;
};

#if WD_ENABLED(WD_SUPPORTS_FILE_ITERATORS) || defined(WD_DOCS)

struct wdFileIterationData;

struct wdFileSystemIteratorFlags
{
  using StorageType = wdUInt8;

  enum Enum : wdUInt8
  {
    Recursive = WD_BIT(0),
    ReportFiles = WD_BIT(1),
    ReportFolders = WD_BIT(2),

    ReportFilesRecursive = Recursive | ReportFiles,
    ReportFoldersRecursive = Recursive | ReportFolders,
    ReportFilesAndFoldersRecursive = Recursive | ReportFiles | ReportFolders,

    Default = ReportFilesAndFoldersRecursive,
  };

  struct Bits
  {
    StorageType Recursive : 1;
    StorageType ReportFiles : 1;
    StorageType ReportFolders : 1;
  };
};

WD_DECLARE_FLAGS_OPERATORS(wdFileSystemIteratorFlags);

/// \brief An wdFileSystemIterator allows to iterate over all files in a certain directory.
///
/// The search can be recursive, and it can contain wildcards (* and ?) to limit the search to specific file types.
class WD_FOUNDATION_DLL wdFileSystemIterator
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdFileSystemIterator);

public:
  wdFileSystemIterator();
  ~wdFileSystemIterator();

  /// \brief Starts a search at the given folder. Use * and ? as wildcards.
  ///
  /// To iterate all files from one folder, use '/Some/Folder'
  /// To iterate over all files of a certain type (in one folder) use '/Some/Folder/*.ext'
  /// Only the final path segment can use placeholders, folders in between must be fully named.
  /// If bRecursive is false, the iterator will only iterate over the files in the start folder, and will not recurse into subdirectories.
  /// If bReportFolders is false, only files will be reported, folders will be skipped (though they will be recursed into, if bRecursive is true).
  ///
  /// If WD_SUCCESS is returned, the iterator points to a valid file, and the functions GetCurrentPath() and GetStats() will return
  /// the information about that file. To advance to the next file, use Next() or SkipFolder().
  /// When no iteration is possible (the directory does not exist or the wild-cards are used incorrectly), WD_FAILURE is returned.
  void StartSearch(wdStringView sSearchTerm, wdBitflags<wdFileSystemIteratorFlags> flags = wdFileSystemIteratorFlags::Default); // [tested]

  /// \brief The same as StartSearch() but executes the same search on multiple folders.
  ///
  /// The search term is appended to each start folder and they are searched one after the other.
  void StartMultiFolderSearch(wdArrayPtr<wdString> startFolders, wdStringView sSearchTerm, wdBitflags<wdFileSystemIteratorFlags> flags = wdFileSystemIteratorFlags::Default);

  /// \brief Returns the search string with which StartSearch() was called.
  ///
  /// If StartMultiFolderSearch() is used, every time a new top-level folder is entered, StartSearch() is executed. In this case GetCurrentSearchTerm() can be used to know in which top-level folder the search is currently running.
  const wdStringView GetCurrentSearchTerm() const { return m_sSearchTerm; }

  /// \brief Returns the current path in which files are searched. Changes when 'Next' moves in or out of a sub-folder.
  ///
  /// You can use this to get the full path of the current file, by appending this value and the filename from 'GetStats'
  const wdStringBuilder& GetCurrentPath() const { return m_sCurPath; } // [tested]

  /// \brief Returns the file stats of the current object that the iterator points to.
  const wdFileStats& GetStats() const { return m_CurFile; } // [tested]

  /// \brief Advances the iterator to the next file object. Might recurse into sub-folders.
  void Next(); // [tested]

  /// \brief The same as 'Next' only that the current folder will not be recursed into.
  void SkipFolder(); // [tested]

  /// \brief Returns true if the iterator currently points to a valid file entry.
  bool IsValid() const;

private:
  wdInt32 InternalNext();

  /// \brief The current path of the folder, in which the iterator currently is.
  wdStringBuilder m_sCurPath;

  wdBitflags<wdFileSystemIteratorFlags> m_Flags;

  /// \brief The stats about the file that the iterator currently points to.
  wdFileStats m_CurFile;

  /// \brief Platform specific data, required by the implementation.
  wdFileIterationData m_Data;

  wdString m_sSearchTerm;
  wdString m_sMultiSearchTerm;
  wdUInt32 m_uiCurrentStartFolder = 0;
  wdHybridArray<wdString, 8> m_StartFolders;
};

#endif

/// \brief This is an abstraction for the most important file operations.
///
/// Instances of wdOSFile can be used for reading and writing files.
/// All paths must be absolute paths, relative paths and current working directories are not supported,
/// since that cannot be guaranteed to work equally on all platforms under all circumstances.
/// A few static functions allow to query the most important data about files, to delete files and create directories.
class WD_FOUNDATION_DLL wdOSFile
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdOSFile);

public:
  wdOSFile();
  ~wdOSFile();

  /// \brief Opens a file for reading or writing. Returns WD_SUCCESS if the file could be opened successfully.
  wdResult Open(wdStringView sFile, wdFileOpenMode::Enum openMode, wdFileShareMode::Enum fileShareMode = wdFileShareMode::Default); // [tested]

  /// \brief Returns true if a file is currently open.
  bool IsOpen() const; // [tested]

  /// \brief Closes the file, if it is currently opened.
  void Close(); // [tested]

  /// \brief Writes the given number of bytes from the buffer into the file. Returns true if all data was successfully written.
  wdResult Write(const void* pBuffer, wdUInt64 uiBytes); // [tested]

  /// \brief Reads up to the given number of bytes from the file. Returns the actual number of bytes that was read.
  wdUInt64 Read(void* pBuffer, wdUInt64 uiBytes); // [tested]

  /// \brief Reads the entire file content into the given array
  wdUInt64 ReadAll(wdDynamicArray<wdUInt8>& out_fileContent); // [tested]

  /// \brief Returns the name of the file that is currently opened. Returns an empty string, if no file is open.
  wdStringView GetOpenFileName() const { return m_sFileName; } // [tested]

  /// \brief Returns the position in the file at which read/write operations will occur.
  wdUInt64 GetFilePosition() const; // [tested]

  /// \brief Sets the position where in the file to read/write next.
  void SetFilePosition(wdInt64 iDistance, wdFileSeekMode::Enum pos) const; // [tested]

  /// \brief Returns the current total size of the file.
  wdUInt64 GetFileSize() const; // [tested]

  /// \brief This will return the platform specific file data (handles etc.), if you really want to be able to wreak havoc.
  const wdOSFileData& GetFileData() const { return m_FileData; }

  /// \brief Returns the processes current working directory (CWD).
  ///
  /// The value typically depends on the directory from which the application was launched.
  /// Since this is a process wide global variable, other code can modify it at any time.
  ///
  /// \note wd does not use the CWD for any file resolution. This function is provided to enable
  /// tools to work with relative paths from the command-line, but every application has to implement
  /// such behavior individually.
  static const wdString GetCurrentWorkingDirectory(); // [tested]

  /// \brief If szPath is a relative path, this function prepends GetCurrentWorkingDirectory().
  ///
  /// In either case, MakeCleanPath() is used before the string is returned.
  static const wdString MakePathAbsoluteWithCWD(wdStringView sPath); // [tested]

  /// \brief Checks whether the given file exists.
  static bool ExistsFile(wdStringView sFile); // [tested]

  /// \brief Checks whether the given file exists.
  static bool ExistsDirectory(wdStringView sDirectory); // [tested]

  /// \brief Deletes the given file. Returns WD_SUCCESS, if the file was deleted or did not exist in the first place. Returns WD_FAILURE
  static wdResult DeleteFile(wdStringView sFile); // [tested]

  /// \brief Creates the given directory structure (meaning all directories in the path, that do not exist). Returns false, if any directory could not
  /// be created.
  static wdResult CreateDirectoryStructure(wdStringView sDirectory); // [tested]

  /// \brief Renames / Moves an existing directory. The file / directory at szFrom must exist. The parent directory of szTo must exist.
  /// Returns WD_FAILURE if the move failed.
  static wdResult MoveFileOrDirectory(wdStringView sFrom, wdStringView sTo);

  /// \brief Copies the source file into the destination file.
  static wdResult CopyFile(wdStringView sSource, wdStringView sDestination); // [tested]

#if WD_ENABLED(WD_SUPPORTS_FILE_STATS) || defined(WD_DOCS)
  /// \brief Gets the stats about the given file or folder. Returns false, if the stats could not be determined.
  static wdResult GetFileStats(wdStringView sFileOrFolder, wdFileStats& out_stats); // [tested]

#  if (WD_ENABLED(WD_SUPPORTS_CASE_INSENSITIVE_PATHS) && WD_ENABLED(WD_SUPPORTS_UNRESTRICTED_FILE_ACCESS)) || defined(WD_DOCS)
  /// \brief Useful on systems that are not strict about the casing of file names. Determines the correct name of a file.
  static wdResult GetFileCasing(wdStringView sFileOrFolder, wdStringBuilder& out_sCorrectSpelling); // [tested]
#  endif

#endif

#if (WD_ENABLED(WD_SUPPORTS_FILE_ITERATORS) && WD_ENABLED(WD_SUPPORTS_FILE_STATS)) || defined(WD_DOCS)

  /// \brief Returns the wdFileStats for all files and folders in the given folder
  static void GatherAllItemsInFolder(wdDynamicArray<wdFileStats>& out_itemList, wdStringView sFolder, wdBitflags<wdFileSystemIteratorFlags> flags = wdFileSystemIteratorFlags::Default);

  /// \brief Copies \a szSourceFolder to \a szDestinationFolder. Overwrites existing files.
  ///
  /// If \a out_FilesCopied is provided, the destination path of every successfully copied file is appended to it.
  static wdResult CopyFolder(wdStringView sSourceFolder, wdStringView sDestinationFolder, wdDynamicArray<wdString>* out_pFilesCopied = nullptr);

  /// \brief Deletes all files recursively in \a szFolder.
  ///
  /// \note The current implementation does not remove the (empty) folders themselves.
  static wdResult DeleteFolder(wdStringView sFolder);

#endif

  /// \brief Returns the path in which the applications binary file is located.
  static const char* GetApplicationDirectory();

  /// \brief Returns the folder into which user data may be safely written.
  /// Append a sub-folder for your application.
  ///
  /// On Windows this is the '%appdata%' directory.
  /// On Posix systems this is the '~' (home) directory.
  ///
  /// If szSubFolder is specified, it will be appended to the result.
  static wdString GetUserDataFolder(wdStringView sSubFolder = {});

  /// \brief Returns the folder into which temp data may be written.
  ///
  /// On Windows this is the '%localappdata%/Temp' directory.
  /// On Posix systems this is the '~/.cache' directory.
  ///
  /// If szSubFolder is specified, it will be appended to the result.
  static wdString GetTempDataFolder(wdStringView sSubFolder = {});

public:
  /// \brief Describes the types of events that wdOSFile sends.
  struct EventType
  {
    enum Enum
    {
      None,
      FileOpen,        ///< A file has been (attempted) to open.
      FileClose,       ///< An open file has been closed.
      FileExists,      ///< A check whether a file exists has been done.
      DirectoryExists, ///< A check whether a directory exists has been done.
      FileDelete,      ///< A file was attempted to be deleted.
      FileRead,        ///< From an open file data was read.
      FileWrite,       ///< Data was written to an open file.
      MakeDir,         ///< A path has been created (recursive directory creation).
      FileCopy,        ///< A file has been copied to another location.
      FileStat,        ///< The stats of a file are queried
      FileCasing,      ///< The exact spelling of a file/path is requested
    };
  };

  /// \brief The data that is sent through the event interface.
  struct EventData
  {
    /// \brief The type of information that is sent.
    EventType::Enum m_EventType = EventType::None;

    /// \brief A unique ID for each file access. Reads and writes to the same open file use the same ID. If the same file is opened multiple times,
    /// different IDs are used.
    wdInt32 m_iFileID = 0;

    /// \brief The name of the file that was operated upon.
    wdStringView m_sFile;

    /// \brief If a second file was operated upon (FileCopy), that is the second file name.
    wdStringView m_sFile2;

    /// \brief Mode that a file has been opened in.
    wdFileOpenMode::Enum m_FileMode = wdFileOpenMode::None;

    /// \brief Whether the operation succeeded (reading, writing, etc.)
    bool m_bSuccess = true;

    /// \brief How long the operation took.
    wdTime m_Duration;

    /// \brief How many bytes were transfered (reading, writing)
    wdUInt64 m_uiBytesAccessed = 0;
  };

  using Event = wdEvent<const EventData&, wdMutex>;

  /// \brief Allows to register a function as an event receiver. All receivers will be notified in the order that they registered.
  static void AddEventHandler(Event::Handler handler) { s_FileEvents.AddEventHandler(handler); }

  /// \brief Unregisters a previously registered receiver. It is an error to unregister a receiver that was not registered.
  static void RemoveEventHandler(Event::Handler handler) { s_FileEvents.RemoveEventHandler(handler); }

private:
  /// \brief Manages all the Event Handlers for the OSFile events.
  static Event s_FileEvents;

  // *** Internal Functions that do the platform specific work ***

  wdResult InternalOpen(wdStringView sFile, wdFileOpenMode::Enum OpenMode, wdFileShareMode::Enum FileShareMode);
  void InternalClose();
  wdResult InternalWrite(const void* pBuffer, wdUInt64 uiBytes);
  wdUInt64 InternalRead(void* pBuffer, wdUInt64 uiBytes);
  wdUInt64 InternalGetFilePosition() const;
  void InternalSetFilePosition(wdInt64 iDistance, wdFileSeekMode::Enum Pos) const;

  static bool InternalExistsFile(wdStringView sFile);
  static bool InternalExistsDirectory(wdStringView sDirectory);
  static wdResult InternalDeleteFile(wdStringView sFile);
  static wdResult InternalDeleteDirectory(wdStringView sDirectory);
  static wdResult InternalCreateDirectory(wdStringView sFile);
  static wdResult InternalMoveFileOrDirectory(wdStringView sDirectoryFrom, wdStringView sDirectoryTo);

#if WD_ENABLED(WD_SUPPORTS_FILE_STATS)
  static wdResult InternalGetFileStats(wdStringView sFileOrFolder, wdFileStats& out_Stats);
#endif

  // *************************************************************

  /// \brief Stores the mode with which the file was opened.
  wdFileOpenMode::Enum m_FileMode;

  /// [internal] On win32 when a file is already open, and this is true, wdOSFile will wait until the file becomes available
  bool m_bRetryOnSharingViolation = true;

  /// \brief Stores the (cleaned up) filename that was used to open the file.
  wdStringBuilder m_sFileName;

  /// \brief Stores the value of s_FileCounter when the wdOSFile is created.
  wdInt32 m_iFileID;

  /// \brief Platform specific data about the open file.
  wdOSFileData m_FileData;

  /// \brief The application binaries' path.
  static wdString64 s_sApplicationPath;

  /// \brief The path where user data is stored on this OS
  static wdString64 s_sUserDataPath;

  /// \brief The path where temp data is stored on this OS
  static wdString64 s_sTempDataPath;

  /// \brief Counts how many different files are touched.225
  static wdAtomicInteger32 s_iFileCounter;
};
