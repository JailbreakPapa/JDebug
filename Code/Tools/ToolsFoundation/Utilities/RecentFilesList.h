#include <Foundation/Containers/Deque.h>
#include <Foundation/Strings/String.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

/// \brief Maintains a list of recently used files and the container window ID they previously resided in.
class WD_TOOLSFOUNDATION_DLL wdRecentFilesList
{
public:
  wdRecentFilesList(wdUInt32 uiMaxElements) { m_uiMaxElements = uiMaxElements; }

  /// \brief Struct that defines the file and container window of the recent file list.
  struct RecentFile
  {
    RecentFile()
      : m_iContainerWindow(0)
    {
    }
    RecentFile(wdStringView sFile, wdInt32 iContainerWindow)
      : m_File(sFile)
      , m_iContainerWindow(iContainerWindow)
    {
    }

    wdString m_File;
    wdInt32 m_iContainerWindow;
  };
  /// \brief Moves the inserted file to the front with the given container ID.
  void Insert(const char* szFile, wdInt32 iContainerWindow);

  /// \brief Returns all files in the list.
  const wdDeque<RecentFile>& GetFileList() const { return m_Files; }

  /// \brief Clears the list
  void Clear() { m_Files.Clear(); }

  /// \brief Saves the recent files list to the given file. Uses a simple text file format (one line per item).
  void Save(const char* szFile);

  /// \brief Loads the recent files list from the given file. Uses a simple text file format (one line per item).
  void Load(const char* szFile);

private:
  wdUInt32 m_uiMaxElements;
  wdDeque<RecentFile> m_Files;
};
