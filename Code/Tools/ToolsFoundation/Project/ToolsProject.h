#pragma once

#include <Foundation/Communication/Event.h>
#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Types/Status.h>
#include <Foundation/Types/Uuid.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class wdToolsProject;
class wdDocument;

struct wdToolsProjectEvent
{
  enum class Type
  {
    ProjectCreated,
    ProjectOpened,
    ProjectSaveState,
    ProjectClosing,
    ProjectClosed,
    ProjectConfigChanged, ///< Sent when global project configuration data was changed and thus certain menus would need to update their content (or
                          ///< just deselect any item, forcing the user to reselect and thus update state)
    SaveAll,              ///< When sent, this shall save all outstanding modifications
  };

  wdToolsProject* m_pProject;
  Type m_Type;
};

struct wdToolsProjectRequest
{
  wdToolsProjectRequest();

  enum class Type
  {
    CanCloseProject,        ///< Can we close the project? Listener needs to set m_bCanClose if not.
    CanCloseDocuments,      ///< Can we close the documents in m_Documents? Listener needs to set m_bCanClose if not.
    SuggestContainerWindow, ///< m_Documents contains one element that a container window should be suggested for and written to
                            ///< m_iContainerWindowUniqueIdentifier.
    GetPathForDocumentGuid,
  };

  Type m_Type;
  bool m_bCanClose;                        ///< When the event is sent, interested code can set this to false to prevent closing.
  wdDynamicArray<wdDocument*> m_Documents; ///< In case of 'CanCloseDocuments', these will be the documents in question.
  wdInt32
    m_iContainerWindowUniqueIdentifier; ///< In case of 'SuggestContainerWindow', the ID of the container to be used for the docs in m_Documents.

  wdUuid m_documentGuid;
  wdStringBuilder m_sAbsDocumentPath;
};

class WD_TOOLSFOUNDATION_DLL wdToolsProject
{
  WD_DECLARE_SINGLETON(wdToolsProject);

public:
  static wdEvent<const wdToolsProjectEvent&> s_Events;
  static wdEvent<wdToolsProjectRequest&> s_Requests;

public:
  static bool IsProjectOpen() { return GetSingleton() != nullptr; }
  static bool IsProjectClosing() { return (GetSingleton() != nullptr && GetSingleton()->m_bIsClosing); }
  static void CloseProject();
  static void SaveProjectState();
  /// \brief Returns true when the project can be closed. Uses wdToolsProjectRequest::Type::CanCloseProject event.
  static bool CanCloseProject();
  /// \brief Returns true when the given list of documents can be closed. Uses wdToolsProjectRequest::Type::CanCloseDocuments event.
  static bool CanCloseDocuments(wdArrayPtr<wdDocument*> documents);
  /// \brief Returns the unique ID of the container window this document should use for its window. Uses
  /// wdToolsProjectRequest::Type::SuggestContainerWindow event.
  static wdInt32 SuggestContainerWindow(wdDocument* pDoc);
  /// \brief Resolve document GUID into an absolute path.
  wdStringBuilder GetPathForDocumentGuid(const wdUuid& guid);
  static wdStatus OpenProject(const char* szProjectPath);
  static wdStatus CreateProject(const char* szProjectPath);

  /// \brief Broadcasts the SaveAll event, though otherwise has no direct effect.
  static void BroadcastSaveAll();

  /// \brief Sent when global project configuration data was changed and thus certain menus would need to update their content (or just deselect any
  /// item, forcing the user to reselect and thus update state)
  static void BroadcastConfigChanged();

  /// \brief Returns the path to the 'wdProject' file
  const wdString& GetProjectFile() const { return m_sProjectPath; }

  /// \brief Returns the short name of the project (extracted from the path).
  ///
  /// \param bSanitize Whether to replace whitespace and other problematic characters, such that it can be used in code.
  const wdString GetProjectName(bool bSanitize) const;

  /// \brief Returns the path in which the 'wdProject' file is stored
  wdString GetProjectDirectory() const;

  /// \brief Returns the directory path in which project settings etc. should be stored
  wdString GetProjectDataFolder() const;

  /// \brief Starts at the  given document and then searches the tree upwards until it finds an wdProject file.
  static wdString FindProjectDirectoryForDocument(const char* szDocumentPath);

  bool IsDocumentInAllowedRoot(const char* szDocumentPath, wdString* out_pRelativePath = nullptr) const;

  void AddAllowedDocumentRoot(const char* szPath);

  /// \brief Makes sure the given sub-folder exists inside the project directory
  void CreateSubFolder(const char* szFolder) const;

private:
  static wdStatus CreateOrOpenProject(const char* szProjectPath, bool bCreate);

private:
  wdToolsProject(const char* szProjectPath);
  ~wdToolsProject();

  wdStatus Create();
  wdStatus Open();

private:
  bool m_bIsClosing;
  wdString m_sProjectPath;
  wdHybridArray<wdString, 4> m_AllowedDocumentRoots;
};
