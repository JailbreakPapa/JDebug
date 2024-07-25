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

class nsObjectAccessorBase;
class nsObjectCommandAccessor;
class nsEditorInputContext;
class nsAbstractObjectNode;

struct NS_TOOLSFOUNDATION_DLL nsObjectAccessorChangeEvent
{
  nsDocument* m_pDocument;
  nsObjectAccessorBase* m_pOldObjectAccessor;
  nsObjectAccessorBase* m_pNewObjectAccessor;
};

class NS_TOOLSFOUNDATION_DLL nsDocumentObjectMetaData : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsDocumentObjectMetaData, nsReflectedClass);

public:
  enum ModifiedFlags : unsigned int
  {
    HiddenFlag = NS_BIT(0),
    PrefabFlag = NS_BIT(1),

    AllFlags = 0xFFFFFFFF
  };

  nsDocumentObjectMetaData() { m_bHidden = false; }

  bool m_bHidden;            /// Whether the object should be rendered in the editor view (no effect on the runtime)
  nsUuid m_CreateFromPrefab; /// The asset GUID of the prefab from which this object was created. Invalid GUID, if this is not a prefab instance.
  nsUuid m_PrefabSeedGuid;   /// The seed GUID used to remap the object GUIDs from the prefab asset into this instance.
  nsString m_sBasePrefab;    /// The prefab from which this instance was created as complete DDL text (this describes the entire object!). Necessary for
                             /// three-way-merging the prefab instances.
};

enum class nsManipulatorSearchStrategy
{
  None,
  SelectedObject,
  ChildrenOfSelectedObject
};

class NS_TOOLSFOUNDATION_DLL nsDocument : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsDocument, nsReflectedClass);

public:
  nsDocument(nsStringView sPath, nsDocumentObjectManager* pDocumentObjectManagerImpl);
  virtual ~nsDocument();

  /// \name Document State Functions
  ///@{

  bool IsModified() const { return m_bModified; }
  bool IsReadOnly() const { return m_bReadOnly; }
  const nsUuid GetGuid() const { return m_pDocumentInfo ? m_pDocumentInfo->m_DocumentID : nsUuid(); }

  const nsDocumentObjectManager* GetObjectManager() const { return m_pObjectManager.Borrow(); }
  nsDocumentObjectManager* GetObjectManager() { return m_pObjectManager.Borrow(); }
  nsSelectionManager* GetSelectionManager() const { return m_pSelectionManager.Borrow(); }
  nsCommandHistory* GetCommandHistory() const { return m_pCommandHistory.Borrow(); }
  virtual nsObjectAccessorBase* GetObjectAccessor() const;

  ///@}
  /// \name Main / Sub-Document Functions
  ///@{

  /// \brief Returns whether this document is a main document, i.e. self contained.
  bool IsMainDocument() const { return m_pHostDocument == this; }
  /// \brief Returns whether this document is a sub-document, i.e. is part of another document.
  bool IsSubDocument() const { return m_pHostDocument != this; }
  /// \brief In case this is a sub-document, returns the main document this belongs to. Otherwise 'this' is returned.
  const nsDocument* GetMainDocument() const { return m_pHostDocument; }
  /// \brief At any given time, only the active sub-document can be edited. This returns the active sub-document which can also be this document itself. Changes to the active sub-document are generally triggered by nsDocumentObjectStructureEvent::Type::AfterReset.
  const nsDocument* GetActiveSubDocument() const { return m_pActiveSubDocument; }
  nsDocument* GetMainDocument() { return m_pHostDocument; }
  nsDocument* GetActiveSubDocument() { return m_pActiveSubDocument; }

protected:
  nsDocument* m_pHostDocument = nullptr;
  nsDocument* m_pActiveSubDocument = nullptr;

  ///@}
  /// \name Document Management Functions
  ///@{

public:
  /// \brief Returns the absolute path to the document.
  nsStringView GetDocumentPath() const { return m_sDocumentPath; }

  /// \brief Saves the document, if it is modified.
  /// If bForce is true, the document will be written, even if it is not considered modified.
  nsStatus SaveDocument(bool bForce = false);
  using AfterSaveCallback = nsDelegate<void(nsDocument*, nsStatus)>;
  nsTaskGroupID SaveDocumentAsync(AfterSaveCallback callback, bool bForce = false);
  void DocumentRenamed(nsStringView sNewDocumentPath);

  static nsStatus ReadDocument(nsStringView sDocumentPath, nsUniquePtr<nsAbstractObjectGraph>& ref_pHeader, nsUniquePtr<nsAbstractObjectGraph>& ref_pObjects,
    nsUniquePtr<nsAbstractObjectGraph>& ref_pTypes);
  static nsStatus ReadAndRegisterTypes(const nsAbstractObjectGraph& types);

  nsStatus LoadDocument() { return InternalLoadDocument(); }

  /// \brief Brings the corresponding window to the front.
  void EnsureVisible();

  nsDocumentManager* GetDocumentManager() const { return m_pDocumentManager; }

  bool HasWindowBeenRequested() const { return m_bWindowRequested; }

  const nsDocumentTypeDescriptor* GetDocumentTypeDescriptor() const { return m_pTypeDescriptor; }

  /// \brief Returns the document's type name. Same as GetDocumentTypeDescriptor()->m_sDocumentTypeName.
  nsStringView GetDocumentTypeName() const
  {
    if (m_pTypeDescriptor == nullptr)
    {
      // if this is a document without a type descriptor, use the RTTI type name as a fallback
      return GetDynamicRTTI()->GetTypeName();
    }

    return m_pTypeDescriptor->m_sDocumentTypeName;
  }

  const nsDocumentInfo* GetDocumentInfo() const { return m_pDocumentInfo; }

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
    NS_DECLARE_POD_TYPE();

    nsDocumentObject* m_pObject = nullptr;
    nsDocumentObject* m_pParent = nullptr;
    nsInt32 m_Index = -1;
  };

  /// \brief Whether this document supports pasting the given mime format into it
  virtual void GetSupportedMimeTypesForPasting(nsHybridArray<nsString, 4>& out_mimeTypes) const {}
  /// \brief Creates the abstract graph of data to be copied and returns the mime type for the clipboard to identify the data
  virtual bool CopySelectedObjects(nsAbstractObjectGraph& out_objectGraph, nsStringBuilder& out_sMimeType) const { return false; };
  virtual bool Paste(const nsArrayPtr<PasteInfo>& info, const nsAbstractObjectGraph& objectGraph, bool bAllowPickedPosition, nsStringView sMimeType)
  {
    return false;
  };

  ///@}
  /// \name Inter Document Communication
  ///@{

  /// \brief This will deliver the message to all open documents. The documents may respond, e.g. by modifying the content of the message.
  void BroadcastInterDocumentMessage(nsReflectedClass* pMessage, nsDocument* pSender);

  /// \brief Called on all documents when BroadcastInterDocumentMessage() is called.
  ///
  /// Use the RTTI information to identify whether the message is of interest.
  virtual void OnInterDocumentMessage(nsReflectedClass* pMessage, nsDocument* pSender) {}

  ///@}
  /// \name Editing Functionality
  ///@{

  /// \brief Allows to return a single input context that currently overrides all others (in priority).
  ///
  /// Used to implement custom tools that need to have priority over selection and camera movement.
  virtual nsEditorInputContext* GetEditorInputContextOverride() { return nullptr; }

  ///@}
  /// \name Misc Functions
  ///@{

  virtual void DeleteSelectedObjects() const;

  const nsSet<nsString>& GetUnknownObjectTypes() const { return m_UnknownObjectTypes; }
  nsUInt32 GetUnknownObjectTypeInstances() const { return m_uiUnknownObjectTypeInstances; }

  /// \brief If disabled, this document will not be put into the recent files list.
  void SetAddToResetFilesList(bool b) { m_bAddToRecentFilesList = b; }

  /// \brief Whether this document shall be put into the recent files list.
  bool GetAddToRecentFilesList() const { return m_bAddToRecentFilesList; }

  /// \brief Broadcasts a status message event. The window that displays the document may show this in some form, e.g. in the status bar.
  void ShowDocumentStatus(const nsFormatString& msg) const;

  /// \brief Tries to compute the position and rotation for an object in the document. Returns NS_SUCCESS if it was possible.
  virtual nsResult ComputeObjectTransformation(const nsDocumentObject* pObject, nsTransform& out_result) const;

  /// \brief Needed by nsManipulatorManager to know where to look for the manipulator attributes.
  ///
  /// Override this function for document types that use manipulators.
  /// The nsManipulatorManager will assert that the document type doesn't return 'None' once it is in use.
  virtual nsManipulatorSearchStrategy GetManipulatorSearchStrategy() const { return nsManipulatorSearchStrategy::None; }

  ///@}
  /// \name Prefab Functions
  ///@{

  /// \brief Whether the document allows to create prefabs in it. This may note be allowed for prefab documents themselves, to prevent nested prefabs.
  virtual bool ArePrefabsAllowed() const { return true; }

  /// \brief Updates ALL prefabs in the document with the latest changes. Merges the current prefab templates with the instances in the document.
  virtual void UpdatePrefabs();

  /// \brief Resets the given objects to their template prefab state, if they have local modifications.
  void RevertPrefabs(nsArrayPtr<const nsDocumentObject*> selection);

  /// \brief Removes the link between a prefab instance and its template, turning the instance into a regular object.
  virtual void UnlinkPrefabs(nsArrayPtr<const nsDocumentObject*> selection);

  virtual nsStatus CreatePrefabDocumentFromSelection(nsStringView sFile, const nsRTTI* pRootType, nsDelegate<void(nsAbstractObjectNode*)> adjustGraphNodeCB = {}, nsDelegate<void(nsDocumentObject*)> adjustNewNodesCB = {}, nsDelegate<void(nsAbstractObjectGraph& graph, nsDynamicArray<nsAbstractObjectNode*>& graphRootNodes)> finalizeGraphCB = {});
  virtual nsStatus CreatePrefabDocument(nsStringView sFile, nsArrayPtr<const nsDocumentObject*> rootObjects, const nsUuid& invPrefabSeed, nsUuid& out_newDocumentGuid, nsDelegate<void(nsAbstractObjectNode*)> adjustGraphNodeCB = {}, bool bKeepOpen = false, nsDelegate<void(nsAbstractObjectGraph& graph, nsDynamicArray<nsAbstractObjectNode*>& graphRootNodes)> finalizeGraphCB = {});

  // Returns new guid of replaced object.
  virtual nsUuid ReplaceByPrefab(const nsDocumentObject* pRootObject, nsStringView sPrefabFile, const nsUuid& prefabAsset, const nsUuid& prefabSeed, bool bEnginePrefab);
  // Returns new guid of reverted object.
  virtual nsUuid RevertPrefab(const nsDocumentObject* pObject);

  ///@}

public:
  nsUniquePtr<nsObjectMetaData<nsUuid, nsDocumentObjectMetaData>> m_DocumentObjectMetaData;

  mutable nsEvent<const nsDocumentEvent&> m_EventsOne;
  static nsEvent<const nsDocumentEvent&> s_EventsAny;

  mutable nsEvent<const nsObjectAccessorChangeEvent&> m_ObjectAccessorChangeEvents;

protected:
  void SetModified(bool b);
  void SetReadOnly(bool b);
  virtual nsTaskGroupID InternalSaveDocument(AfterSaveCallback callback);
  virtual nsStatus InternalLoadDocument();
  virtual nsDocumentInfo* CreateDocumentInfo() = 0;

  /// \brief A hook to execute additional code after SUCCESSFULLY saving a document. E.g. manual asset transform can be done here.
  virtual void InternalAfterSaveDocument() {}

  virtual void AttachMetaDataBeforeSaving(nsAbstractObjectGraph& graph) const;
  virtual void RestoreMetaDataAfterLoading(const nsAbstractObjectGraph& graph, bool bUndoable);

  virtual void InitializeAfterLoading(bool bFirstTimeCreation) {}
  virtual void InitializeAfterLoadingAndSaving() {}

  virtual void BeforeClosing();

  void SetUnknownObjectTypes(const nsSet<nsString>& Types, nsUInt32 uiInstances);

  /// \name Prefab Functions
  ///@{

  virtual void UpdatePrefabsRecursive(nsDocumentObject* pObject);
  virtual void UpdatePrefabObject(nsDocumentObject* pObject, const nsUuid& PrefabAsset, const nsUuid& PrefabSeed, nsStringView sBasePrefab);

  ///@}

  nsUniquePtr<nsDocumentObjectManager> m_pObjectManager;
  mutable nsUniquePtr<nsCommandHistory> m_pCommandHistory;
  mutable nsUniquePtr<nsSelectionManager> m_pSelectionManager;
  mutable nsUniquePtr<nsObjectCommandAccessor> m_pObjectAccessor; ///< Default object accessor used by every doc.

  nsDocumentInfo* m_pDocumentInfo = nullptr;
  const nsDocumentTypeDescriptor* m_pTypeDescriptor = nullptr;

private:
  friend class nsDocumentManager;
  friend class nsCommandHistory;
  friend class nsSaveDocumentTask;
  friend class nsAfterSaveDocumentTask;

  void SetupDocumentInfo(const nsDocumentTypeDescriptor* pTypeDescriptor);

  nsDocumentManager* m_pDocumentManager = nullptr;

  nsString m_sDocumentPath;
  bool m_bModified;
  bool m_bReadOnly;
  bool m_bWindowRequested;
  bool m_bAddToRecentFilesList;

  nsSet<nsString> m_UnknownObjectTypes;
  nsUInt32 m_uiUnknownObjectTypeInstances;

  nsTaskGroupID m_ActiveSaveTask;
  nsStatus m_LastSaveResult;
};
