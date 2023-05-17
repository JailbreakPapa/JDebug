#pragma once

#include <Foundation/Communication/Event.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Threading/Implementation/TaskSystemDeclarations.h>
#include <Foundation/Types/Status.h>
#include <Foundation/Types/UniquePtr.h>
#include <ToolsFoundation/CommandHistory/CommandHistory.h>
#include <ToolsFoundation/Document/Implementation/Declarations.h>
#include <ToolsFoundation/Object/ObjectMetaData.h>
#include <ToolsFoundation/Selection/SelectionManager.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class wdObjectAccessorBase;
class wdObjectCommandAccessor;
class wdEditorInputContext;
class wdAbstractObjectNode;

struct WD_TOOLSFOUNDATION_DLL wdObjectAccessorChangeEvent
{
  wdDocument* m_pDocument;
  wdObjectAccessorBase* m_pOldObjectAccessor;
  wdObjectAccessorBase* m_pNewObjectAccessor;
};

class WD_TOOLSFOUNDATION_DLL wdDocumentObjectMetaData : public wdReflectedClass
{
  WD_ADD_DYNAMIC_REFLECTION(wdDocumentObjectMetaData, wdReflectedClass);

public:
  enum ModifiedFlags : unsigned int
  {
    HiddenFlag = WD_BIT(0),
    PrefabFlag = WD_BIT(1),

    AllFlags = 0xFFFFFFFF
  };

  wdDocumentObjectMetaData() { m_bHidden = false; }

  bool m_bHidden;            /// Whether the object should be rendered in the editor view (no effect on the runtime)
  wdUuid m_CreateFromPrefab; /// The asset GUID of the prefab from which this object was created. Invalid GUID, if this is not a prefab instance.
  wdUuid m_PrefabSeedGuid;   /// The seed GUID used to remap the object GUIDs from the prefab asset into this instance.
  wdString m_sBasePrefab;    /// The prefab from which this instance was created as complete DDL text (this describes the entire object!). Necessary for
                             /// three-way-merging the prefab instances.
};

enum class wdManipulatorSearchStrategy
{
  None,
  SelectedObject,
  ChildrenOfSelectedObject
};

class WD_TOOLSFOUNDATION_DLL wdDocument : public wdReflectedClass
{
  WD_ADD_DYNAMIC_REFLECTION(wdDocument, wdReflectedClass);

public:
  wdDocument(const char* szPath, wdDocumentObjectManager* pDocumentObjectManagerImpl);
  virtual ~wdDocument();

  /// \name Document State Functions
  ///@{

  bool IsModified() const { return m_bModified; }
  bool IsReadOnly() const { return m_bReadOnly; }
  const wdUuid GetGuid() const { return m_pDocumentInfo ? m_pDocumentInfo->m_DocumentID : wdUuid(); }

  const wdDocumentObjectManager* GetObjectManager() const { return m_pObjectManager.Borrow(); }
  wdDocumentObjectManager* GetObjectManager() { return m_pObjectManager.Borrow(); }
  wdSelectionManager* GetSelectionManager() const { return m_pSelectionManager.Borrow(); }
  wdCommandHistory* GetCommandHistory() const { return m_pCommandHistory.Borrow(); }
  virtual wdObjectAccessorBase* GetObjectAccessor() const;

  ///@}
  /// \name Main / Sub-Document Functions
  ///@{

  /// \brief Returns whether this document is a main document, i.e. self contained.
  bool IsMainDocument() const { return m_pHostDocument == this; }
  /// \brief Returns whether this document is a sub-document, i.e. is part of another document.
  bool IsSubDocument() const { return m_pHostDocument != this; }
  /// \brief In case this is a sub-document, returns the main document this belongs to. Otherwise 'this' is returned.
  const wdDocument* GetMainDocument() const { return m_pHostDocument; }
  /// @brief At any given time, only the active sub-document can be edited. This returns the active sub-document which can also be this document itself. Changes to the active sub-document are generally triggered by wdDocumentObjectStructureEvent::Type::AfterReset.
  const wdDocument* GetActiveSubDocument() const { return m_pActiveSubDocument; }
  wdDocument* GetMainDocument() { return m_pHostDocument; }
  wdDocument* GetActiveSubDocument() { return m_pActiveSubDocument; }

protected:
  wdDocument* m_pHostDocument = nullptr;
  wdDocument* m_pActiveSubDocument = nullptr;

  ///@}
  /// \name Document Management Functions
  ///@{

public:
  /// \brief Returns the absolute path to the document.
  const char* GetDocumentPath() const { return m_sDocumentPath; }

  /// \brief Saves the document, if it is modified.
  /// If bForce is true, the document will be written, even if it is not considered modified.
  wdStatus SaveDocument(bool bForce = false);
  typedef wdDelegate<void(wdDocument* doc, wdStatus res)> AfterSaveCallback;
  wdTaskGroupID SaveDocumentAsync(AfterSaveCallback callback, bool bForce = false);

  static wdStatus ReadDocument(const char* szDocumentPath, wdUniquePtr<wdAbstractObjectGraph>& ref_pHeader, wdUniquePtr<wdAbstractObjectGraph>& ref_pObjects,
    wdUniquePtr<wdAbstractObjectGraph>& ref_pTypes);
  static wdStatus ReadAndRegisterTypes(const wdAbstractObjectGraph& types);

  wdStatus LoadDocument() { return InternalLoadDocument(); }

  /// \brief Brings the corresponding window to the front.
  void EnsureVisible();

  wdDocumentManager* GetDocumentManager() const { return m_pDocumentManager; }

  bool HasWindowBeenRequested() const { return m_bWindowRequested; }

  const wdDocumentTypeDescriptor* GetDocumentTypeDescriptor() const { return m_pTypeDescriptor; }

  /// \brief Returns the document's type name. Same as GetDocumentTypeDescriptor()->m_sDocumentTypeName.
  const char* GetDocumentTypeName() const { return m_pTypeDescriptor->m_sDocumentTypeName; }

  const wdDocumentInfo* GetDocumentInfo() const { return m_pDocumentInfo; }

  /// \brief Asks the document whether a restart of the engine process is allowed at this time.
  ///
  /// Documents that are currently interacting with the engine process (active play-the-game mode) should return false.
  /// All others should return true.
  /// As long as any document returns false, automatic engine process reload is suppressed.
  virtual bool CanEngineProcessBeRestarted() const { return true; }

  ///@}
  /// \name Clipboard Functions
  ///@{

  struct PasteInfo
  {
    WD_DECLARE_POD_TYPE();

    wdDocumentObject* m_pObject = nullptr;
    wdDocumentObject* m_pParent = nullptr;
    wdInt32 m_Index = -1;
  };

  /// \brief Whether this document supports pasting the given mime format into it
  virtual void GetSupportedMimeTypesForPasting(wdHybridArray<wdString, 4>& out_mimeTypes) const {}
  /// \brief Creates the abstract graph of data to be copied and returns the mime type for the clipboard to identify the data
  virtual bool CopySelectedObjects(wdAbstractObjectGraph& out_objectGraph, wdStringBuilder& out_sMimeType) const { return false; };
  virtual bool Paste(const wdArrayPtr<PasteInfo>& info, const wdAbstractObjectGraph& objectGraph, bool bAllowPickedPosition, const char* szMimeType)
  {
    return false;
  };

  ///@}
  /// \name Inter Document Communication
  ///@{

  /// \brief This will deliver the message to all open documents. The documents may respond, e.g. by modifying the content of the message.
  void BroadcastInterDocumentMessage(wdReflectedClass* pMessage, wdDocument* pSender);

  /// \brief Called on all documents when BroadcastInterDocumentMessage() is called.
  ///
  /// Use the RTTI information to identify whether the message is of interest.
  virtual void OnInterDocumentMessage(wdReflectedClass* pMessage, wdDocument* pSender) {}

  ///@}
  /// \name Editing Functionality
  ///@{

  /// \brief Allows to return a single input context that currently overrides all others (in priority).
  ///
  /// Used to implement custom tools that need to have priority over selection and camera movement.
  virtual wdEditorInputContext* GetEditorInputContextOverride() { return nullptr; }

  ///@}
  /// \name Misc Functions
  ///@{

  virtual void DeleteSelectedObjects() const;

  const wdSet<wdString>& GetUnknownObjectTypes() const { return m_UnknownObjectTypes; }
  wdUInt32 GetUnknownObjectTypeInstances() const { return m_uiUnknownObjectTypeInstances; }

  /// \brief If disabled, this document will not be put into the recent files list.
  void SetAddToResetFilesList(bool b) { m_bAddToRecentFilesList = b; }

  /// \brief Whether this document shall be put into the recent files list.
  bool GetAddToRecentFilesList() const { return m_bAddToRecentFilesList; }

  /// \brief Broadcasts a status message event. The window that displays the document may show this in some form, e.g. in the status bar.
  void ShowDocumentStatus(const wdFormatString& msg) const;

  /// \brief Tries to compute the position and rotation for an object in the document. Returns WD_SUCCESS if it was possible.
  virtual wdResult ComputeObjectTransformation(const wdDocumentObject* pObject, wdTransform& out_result) const;

  /// \brief Needed by wdManipulatorManager to know where to look for the manipulator attributes.
  ///
  /// Override this function for document types that use manipulators.
  /// The wdManipulatorManager will assert that the document type doesn't return 'None' once it is in use.
  virtual wdManipulatorSearchStrategy GetManipulatorSearchStrategy() const { return wdManipulatorSearchStrategy::None; }

  ///@}
  /// \name Prefab Functions
  ///@{

  /// \brief Whether the document allows to create prefabs in it. This may note be allowed for prefab documents themselves, to prevent nested prefabs.
  virtual bool ArePrefabsAllowed() const { return true; }

  /// \brief Updates ALL prefabs in the document with the latest changes. Merges the current prefab templates with the instances in the document.
  virtual void UpdatePrefabs();

  /// \brief Resets the given objects to their template prefab state, if they have local modifications.
  void RevertPrefabs(const wdDeque<const wdDocumentObject*>& selection);

  /// \brief Removes the link between a prefab instance and its template, turning the instance into a regular object.
  virtual void UnlinkPrefabs(const wdDeque<const wdDocumentObject*>& selection);

  virtual wdStatus CreatePrefabDocumentFromSelection(const char* szFile, const wdRTTI* pRootType, wdDelegate<void(wdAbstractObjectNode*)> adjustGraphNodeCB = {}, wdDelegate<void(wdDocumentObject*)> adjustNewNodesCB = {}, wdDelegate<void(wdAbstractObjectGraph& graph, wdDynamicArray<wdAbstractObjectNode*>& graphRootNodes)> finalizeGraphCB = {});
  virtual wdStatus CreatePrefabDocument(const char* szFile, wdArrayPtr<const wdDocumentObject*> rootObjects, const wdUuid& invPrefabSeed, wdUuid& out_newDocumentGuid, wdDelegate<void(wdAbstractObjectNode*)> adjustGraphNodeCB = {}, bool bKeepOpen = false, wdDelegate<void(wdAbstractObjectGraph& graph, wdDynamicArray<wdAbstractObjectNode*>& graphRootNodes)> finalizeGraphCB = {});

  // Returns new guid of replaced object.
  virtual wdUuid ReplaceByPrefab(const wdDocumentObject* pRootObject, const char* szPrefabFile, const wdUuid& prefabAsset, const wdUuid& prefabSeed, bool bEnginePrefab);
  // Returns new guid of reverted object.
  virtual wdUuid RevertPrefab(const wdDocumentObject* pObject);

  ///@}

public:
  wdUniquePtr<wdObjectMetaData<wdUuid, wdDocumentObjectMetaData>> m_DocumentObjectMetaData;

  mutable wdEvent<const wdDocumentEvent&> m_EventsOne;
  static wdEvent<const wdDocumentEvent&> s_EventsAny;

  mutable wdEvent<const wdObjectAccessorChangeEvent&> m_ObjectAccessorChangeEvents;

protected:
  void SetModified(bool b);
  void SetReadOnly(bool b);
  virtual wdTaskGroupID InternalSaveDocument(AfterSaveCallback callback);
  virtual wdStatus InternalLoadDocument();
  virtual wdDocumentInfo* CreateDocumentInfo() = 0;

  /// \brief A hook to execute additional code after SUCCESSFULLY saving a document. E.g. manual asset transform can be done here.
  virtual void InternalAfterSaveDocument() {}

  virtual void AttachMetaDataBeforeSaving(wdAbstractObjectGraph& graph) const;
  virtual void RestoreMetaDataAfterLoading(const wdAbstractObjectGraph& graph, bool bUndoable);

  virtual void InitializeAfterLoading(bool bFirstTimeCreation) {}
  virtual void InitializeAfterLoadingAndSaving() {}

  virtual void BeforeClosing();

  void SetUnknownObjectTypes(const wdSet<wdString>& Types, wdUInt32 uiInstances);

  /// \name Prefab Functions
  ///@{

  virtual void UpdatePrefabsRecursive(wdDocumentObject* pObject);
  virtual void UpdatePrefabObject(wdDocumentObject* pObject, const wdUuid& PrefabAsset, const wdUuid& PrefabSeed, const char* szBasePrefab);

  ///@}

  wdUniquePtr<wdDocumentObjectManager> m_pObjectManager;
  mutable wdUniquePtr<wdCommandHistory> m_pCommandHistory;
  mutable wdUniquePtr<wdSelectionManager> m_pSelectionManager;
  mutable wdUniquePtr<wdObjectCommandAccessor> m_pObjectAccessor; ///< Default object accessor used by every doc.

  wdDocumentInfo* m_pDocumentInfo = nullptr;
  const wdDocumentTypeDescriptor* m_pTypeDescriptor = nullptr;

private:
  friend class wdDocumentManager;
  friend class wdCommandHistory;
  friend class wdSaveDocumentTask;
  friend class wdAfterSaveDocumentTask;

  void SetupDocumentInfo(const wdDocumentTypeDescriptor* pTypeDescriptor);

  wdDocumentManager* m_pDocumentManager = nullptr;

  wdString m_sDocumentPath;
  bool m_bModified;
  bool m_bReadOnly;
  bool m_bWindowRequested;
  bool m_bAddToRecentFilesList;

  wdSet<wdString> m_UnknownObjectTypes;
  wdUInt32 m_uiUnknownObjectTypeInstances;

  wdTaskGroupID m_ActiveSaveTask;
  wdStatus m_LastSaveResult;
};
