#include <Foundation/Containers/Deque.h>
#include <Foundation/Strings/String.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

/// \brief Maintains a list of recently used files and the container window ID they previously resided in.
class NS_TOOLSFOUNDATION_DLL nsRecentFilesList
{
public:
  nsRecentFilesList(nsUInt32 uiMaxElements) { m_uiMaxElements = uiMaxElements; }

  /// \brief Struct that defines the file and container window of the recent file list.
  struct RecentFile
  {
    RecentFile()
      : m_iContainerWindow(0)
    {
    }
    RecentFile(nsStringView sFile, nsInt32 iContainerWindow)
      : m_File(sFile)
      , m_iContainerWindow(iContainerWindow)
    {
    }

    nsString m_File;
    nsInt32 m_iContainerWindow;
  };
  /// \brief Moves the inserted file to the front with the given container ID.
  void Insert(nsStringView sFile, nsInt32 iContainerWindow);

  /// \brief Returns all files in the list.
  const nsDeque<RecentFile>& GetFileList() const { return m_Files; }

  /// \brief Clears the list
  void Clear() { m_Files.Clear(); }

  /// \brief Saves the recent files list to the given file. Uses a simple text file format (one line per item).
  void Save(nsStringView sFile);

  /// \brief Loads the recent files list from the given file. Uses a simple text file format (one line per item).
  void Load(nsStringView sFile);

private:
  nsUInt32 m_uiMaxElements;
  nsDeque<RecentFile> m_Files;
};
