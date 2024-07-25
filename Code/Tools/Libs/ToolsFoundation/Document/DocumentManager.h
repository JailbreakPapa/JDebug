#pragma once

#include <Foundation/Types/Status.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class NS_TOOLSFOUNDATION_DLL nsDocumentManager : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsDocumentManager, nsReflectedClass);

public:
  virtual ~nsDocumentManager() = default;

  static const nsHybridArray<nsDocumentManager*, 16>& GetAllDocumentManagers() { return s_AllDocumentManagers; }

  static nsResult FindDocumentTypeFromPath(nsStringView sPath, bool bForCreation, const nsDocumentTypeDescriptor*& out_pTypeDesc);

  nsStatus CanOpenDocument(nsStringView sFilePath) const;

  /// \brief Creates a new document.
  /// \param szDocumentTypeName Document type to create. See nsDocumentTypeDescriptor.
  /// \param szPath Absolute path to the document to be created.
  /// \param out_pDocument Out parameter for the resulting nsDocument. Will be nullptr on failure.
  /// \param flags Flags to define various options like whether a window should be created.
  /// \param pOpenContext An generic context object. Allows for custom data to be passed along into the construction. E.g. inform a sub-document which main document it belongs to.
  /// \return Returns the error in case the operations failed.
  nsStatus CreateDocument(
    nsStringView sDocumentTypeName, nsStringView sPath, nsDocument*& out_pDocument, nsBitflags<nsDocumentFlags> flags = nsDocumentFlags::None, const nsDocumentObject* pOpenContext = nullptr);

  /// \brief Opens an existing document.
  /// \param szDocumentTypeName Document type to open. See nsDocumentTypeDescriptor.
  /// \param szPath Absolute path to the document to be opened.
  /// \param out_pDocument Out parameter for the resulting nsDocument. Will be nullptr on failure.
  /// \param flags Flags to define various options like whether a window should be created.
  /// \param pOpenContext  An generic context object. Allows for custom data to be passed along into the construction. E.g. inform a sub-document which main document it belongs to.
  /// \return Returns the error in case the operations failed.
  /// \return Returns the error in case the operations failed.
  nsStatus OpenDocument(nsStringView sDocumentTypeName, nsStringView sPath, nsDocument*& out_pDocument,
    nsBitflags<nsDocumentFlags> flags = nsDocumentFlags::AddToRecentFilesList | nsDocumentFlags::RequestWindow,
    const nsDocumentObject* pOpenContext = nullptr);
  virtual nsStatus CloneDocument(nsStringView sPath, nsStringView sClonePath, nsUuid& inout_cloneGuid);
  void CloseDocument(nsDocument* pDocument);
  void EnsureWindowRequested(nsDocument* pDocument, const nsDocumentObject* pOpenContext = nullptr);

  /// \brief Returns a list of all currently open documents that are managed by this document manager
  const nsDynamicArray<nsDocument*>& GetAllOpenDocuments() const { return m_AllOpenDocuments; }

  nsDocument* GetDocumentByPath(nsStringView sPath) const;

  static nsDocument* GetDocumentByGuid(const nsUuid& guid);

  /// \brief If the given document is open, it will be closed. User is not asked about it, unsaved changes are discarded. Returns true if the document
  /// was open and needed to be closed.
  static bool EnsureDocumentIsClosedInAllManagers(nsStringView sPath);

  /// \brief If the given document is open, it will be closed. User is not asked about it, unsaved changes are discarded. Returns true if the document
  /// was open and needed to be closed. This function only operates on documents opened by this manager. Use EnsureDocumentIsClosedInAllManagers() to
  /// close documents of any type.
  bool EnsureDocumentIsClosed(nsStringView sPath);

  void CloseAllDocumentsOfManager();
  static void CloseAllDocuments();

  struct Event
  {
    enum class Type
    {
      DocumentTypesRemoved,
      DocumentTypesAdded,
      DocumentOpened,
      DocumentWindowRequested,      ///< Sent when the window for a document is needed. Each plugin should check this and see if it can create the desired
                                    ///< window type
      AfterDocumentWindowRequested, ///< Sent after a document window was requested. Can be used to do things after the new window has been opened
      DocumentClosing,
      DocumentClosing2,             // sent after DocumentClosing but before removing the document, use this to do stuff that depends on code executed during
                                    // DocumentClosing
      DocumentClosed,               // this will not point to a valid document anymore, as the document is deleted, use DocumentClosing to get the event before it
                                    // is deleted
    };

    Type m_Type;
    nsDocument* m_pDocument = nullptr;
    const nsDocumentObject* m_pOpenContext = nullptr;
  };

  struct Request
  {
    enum class Type
    {
      DocumentAllowedToOpen,
    };

    Type m_Type;
    nsString m_sDocumentType;
    nsString m_sDocumentPath;
    nsStatus m_RequestStatus;
  };

  static nsCopyOnBroadcastEvent<const Event&> s_Events;
  static nsEvent<Request&> s_Requests;

  static const nsDocumentTypeDescriptor* GetDescriptorForDocumentType(nsStringView sDocumentType);
  static const nsMap<nsString, const nsDocumentTypeDescriptor*>& GetAllDocumentDescriptors();

  void GetSupportedDocumentTypes(nsDynamicArray<const nsDocumentTypeDescriptor*>& inout_documentTypes) const;

  using CustomAction = nsVariant (*)(const nsDocument*);
  static nsMap<nsString, CustomAction> s_CustomActions;

protected:
  virtual void InternalCloneDocument(nsStringView sPath, nsStringView sClonePath, const nsUuid& documentId, const nsUuid& seedGuid, const nsUuid& cloneGuid, nsAbstractObjectGraph* pHeader, nsAbstractObjectGraph* pObjects, nsAbstractObjectGraph* pTypes);

private:
  virtual void InternalCreateDocument(nsStringView sDocumentTypeName, nsStringView sPath, bool bCreateNewDocument, nsDocument*& out_pDocument, const nsDocumentObject* pOpenContext) = 0;
  virtual void InternalGetSupportedDocumentTypes(nsDynamicArray<const nsDocumentTypeDescriptor*>& inout_DocumentTypes) const = 0;

private:
  nsStatus CreateOrOpenDocument(bool bCreate, nsStringView sDocumentTypeName, nsStringView sPath, nsDocument*& out_pDocument,
    nsBitflags<nsDocumentFlags> flags, const nsDocumentObject* pOpenContext = nullptr);

private:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(ToolsFoundation, DocumentManager);

  static void OnPluginEvent(const nsPluginEvent& e);

  static void UpdateBeforeUnloadingPlugins(const nsPluginEvent& e);
  static void UpdatedAfterLoadingPlugins();

  nsDynamicArray<nsDocument*> m_AllOpenDocuments;

  static nsSet<const nsRTTI*> s_KnownManagers;
  static nsHybridArray<nsDocumentManager*, 16> s_AllDocumentManagers;

  static nsMap<nsString, const nsDocumentTypeDescriptor*> s_AllDocumentDescriptors;
};
