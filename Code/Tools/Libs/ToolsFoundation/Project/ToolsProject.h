#pragma once

#include <Foundation/Communication/Event.h>
#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Types/Status.h>
#include <Foundation/Types/Uuid.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class nsToolsProject;
class nsDocument;

struct nsToolsProjectEvent
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

  nsToolsProject* m_pProject;
  Type m_Type;
};

struct nsToolsProjectRequest
{
  nsToolsProjectRequest();

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
  nsDynamicArray<nsDocument*> m_Documents; ///< In case of 'CanCloseDocuments', these will be the documents in question.
  nsInt32
    m_iContainerWindowUniqueIdentifier;    ///< In case of 'SuggestContainerWindow', the ID of the container to be used for the docs in m_Documents.

  nsUuid m_documentGuid;
  nsStringBuilder m_sAbsDocumentPath;
};

class NS_TOOLSFOUNDATION_DLL nsToolsProject
{
  NS_DECLARE_SINGLETON(nsToolsProject);

public:
  static nsEvent<const nsToolsProjectEvent&> s_Events;
  static nsEvent<nsToolsProjectRequest&> s_Requests;

public:
  static bool IsProjectOpen() { return GetSingleton() != nullptr; }
  static bool IsProjectClosing() { return (GetSingleton() != nullptr && GetSingleton()->m_bIsClosing); }
  static void CloseProject();
  static void SaveProjectState();
  /// \brief Returns true when the project can be closed. Uses nsToolsProjectRequest::Type::CanCloseProject event.
  static bool CanCloseProject();
  /// \brief Returns true when the given list of documents can be closed. Uses nsToolsProjectRequest::Type::CanCloseDocuments event.
  static bool CanCloseDocuments(nsArrayPtr<nsDocument*> documents);
  /// \brief Returns the unique ID of the container window this document should use for its window. Uses
  /// nsToolsProjectRequest::Type::SuggestContainerWindow event.
  static nsInt32 SuggestContainerWindow(nsDocument* pDoc);
  /// \brief Resolve document GUID into an absolute path.
  nsStringBuilder GetPathForDocumentGuid(const nsUuid& guid);
  static nsStatus OpenProject(nsStringView sProjectPath);
  static nsStatus CreateProject(nsStringView sProjectPath);

  /// \brief Broadcasts the SaveAll event, though otherwise has no direct effect.
  static void BroadcastSaveAll();

  /// \brief Sent when global project configuration data was changed and thus certain menus would need to update their content (or just deselect any
  /// item, forcing the user to reselect and thus update state)
  static void BroadcastConfigChanged();

  /// \brief Returns the path to the 'nsProject' file
  const nsString& GetProjectFile() const { return m_sProjectPath; }

  /// \brief Returns the short name of the project (extracted from the path).
  ///
  /// \param bSanitize Whether to replace whitespace and other problematic characters, such that it can be used in code.
  const nsString GetProjectName(bool bSanitize) const;

  /// \brief Returns the path in which the 'nsProject' file is stored
  nsString GetProjectDirectory() const;

  /// \brief Returns the directory path in which project settings etc. should be stored
  nsString GetProjectDataFolder() const;

  /// \brief Starts at the  given document and then searches the tree upwards until it finds an nsProject file.
  static nsString FindProjectDirectoryForDocument(nsStringView sDocumentPath);

  bool IsDocumentInAllowedRoot(nsStringView sDocumentPath, nsString* out_pRelativePath = nullptr) const;

  void AddAllowedDocumentRoot(nsStringView sPath);

  /// \brief Makes sure the given sub-folder exists inside the project directory
  void CreateSubFolder(nsStringView sFolder) const;

private:
  static nsStatus CreateOrOpenProject(nsStringView sProjectPath, bool bCreate);

private:
  nsToolsProject(nsStringView sProjectPath);
  ~nsToolsProject();

  nsStatus Create();
  nsStatus Open();

private:
  bool m_bIsClosing;
  nsString m_sProjectPath;
  nsHybridArray<nsString, 4> m_AllowedDocumentRoots;
};
