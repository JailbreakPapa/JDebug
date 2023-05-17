#pragma once

#include <Foundation/Types/Status.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class WD_TOOLSFOUNDATION_DLL wdDocumentManager : public wdReflectedClass
{
  WD_ADD_DYNAMIC_REFLECTION(wdDocumentManager, wdReflectedClass);

public:
  virtual ~wdDocumentManager() {}

  static const wdHybridArray<wdDocumentManager*, 16>& GetAllDocumentManagers() { return s_AllDocumentManagers; }

  static wdResult FindDocumentTypeFromPath(const char* szPath, bool bForCreation, const wdDocumentTypeDescriptor*& out_pTypeDesc);

  wdStatus CanOpenDocument(const char* szFilePath) const;

  /// \brief Creates a new document.
  /// \param szDocumentTypeName Document type to create. See wdDocumentTypeDescriptor.
  /// \param szPath Absolute path to the document to be created.
  /// \param out_pDocument Out parameter for the resulting wdDocument. Will be nullptr on failure.
  /// \param flags Flags to define various options like whether a window should be created.
  /// \param pOpenContext An generic context object. Allows for custom data to be passed along into the construction. E.g. inform a sub-document which main document it belongs to.
  /// \return Returns the error in case the operations failed.
  wdStatus CreateDocument(
    const char* szDocumentTypeName, const char* szPath, wdDocument*& out_pDocument, wdBitflags<wdDocumentFlags> flags = wdDocumentFlags::None, const wdDocumentObject* pOpenContext = nullptr);

  /// \brief Opens an existing document.
  /// \param szDocumentTypeName Document type to open. See wdDocumentTypeDescriptor.
  /// \param szPath Absolute path to the document to be opened.
  /// \param out_pDocument Out parameter for the resulting wdDocument. Will be nullptr on failure.
  /// \param flags Flags to define various options like whether a window should be created.
  /// \param pOpenContext  An generic context object. Allows for custom data to be passed along into the construction. E.g. inform a sub-document which main document it belongs to.
  /// \return Returns the error in case the operations failed.
  /// \return Returns the error in case the operations failed.
  wdStatus OpenDocument(const char* szDocumentTypeName, const char* szPath, wdDocument*& out_pDocument,
    wdBitflags<wdDocumentFlags> flags = wdDocumentFlags::AddToRecentFilesList | wdDocumentFlags::RequestWindow,
    const wdDocumentObject* pOpenContext = nullptr);
  virtual wdStatus CloneDocument(const char* szPath, const char* szClonePath, wdUuid& inout_cloneGuid);
  void CloseDocument(wdDocument* pDocument);
  void EnsureWindowRequested(wdDocument* pDocument, const wdDocumentObject* pOpenContext = nullptr);

  /// \brief Returns a list of all currently open documents that are managed by this document manager
  const wdDynamicArray<wdDocument*>& GetAllOpenDocuments() const { return m_AllOpenDocuments; }

  wdDocument* GetDocumentByPath(const char* szPath) const;

  static wdDocument* GetDocumentByGuid(const wdUuid& guid);

  /// \brief If the given document is open, it will be closed. User is not asked about it, unsaved changes are discarded. Returns true if the document
  /// was open and needed to be closed.
  static bool EnsureDocumentIsClosedInAllManagers(const char* szPath);

  /// \brief If the given document is open, it will be closed. User is not asked about it, unsaved changes are discarded. Returns true if the document
  /// was open and needed to be closed. This function only operates on documents opened by this manager. Use EnsureDocumentIsClosedInAllManagers() to
  /// close documents of any type.
  bool EnsureDocumentIsClosed(const char* szPath);

  void CloseAllDocumentsOfManager();
  static void CloseAllDocuments();

  struct Event
  {
    enum class Type
    {
      DocumentTypesRemoved,
      DocumentTypesAdded,
      DocumentOpened,
      DocumentWindowRequested, ///< Sent when the window for a document is needed. Each plugin should check this and see if it can create the desired
                               ///< window type
      AfterDocumentWindowRequested, ///< Sent after a document window was requested. Can be used to do things after the new window has been opened
      DocumentClosing,
      DocumentClosing2, // sent after DocumentClosing but before removing the document, use this to do stuff that depends on code executed during
                        // DocumentClosing
      DocumentClosed,   // this will not point to a valid document anymore, as the document is deleted, use DocumentClosing to get the event before it
                        // is deleted
    };

    Type m_Type;
    wdDocument* m_pDocument = nullptr;
    const wdDocumentObject* m_pOpenContext = nullptr;
  };

  struct Request
  {
    enum class Type
    {
      DocumentAllowedToOpen,
    };

    Type m_Type;
    wdString m_sDocumentType;
    wdString m_sDocumentPath;
    wdStatus m_RequestStatus;
  };

  static wdCopyOnBroadcastEvent<const Event&> s_Events;
  static wdEvent<Request&> s_Requests;

  static const wdDocumentTypeDescriptor* GetDescriptorForDocumentType(const char* szDocumentType);
  static const wdMap<wdString, const wdDocumentTypeDescriptor*>& GetAllDocumentDescriptors();

  void GetSupportedDocumentTypes(wdDynamicArray<const wdDocumentTypeDescriptor*>& inout_documentTypes) const;

  using CustomAction = wdVariant (*)(const wdDocument*);
  static wdMap<wdString, CustomAction> s_CustomActions;

protected:
  virtual void InternalCloneDocument(const char* szPath, const char* szClonePath, const wdUuid& documentId, const wdUuid& seedGuid, const wdUuid& cloneGuid, wdAbstractObjectGraph* pHeader, wdAbstractObjectGraph* pObjects, wdAbstractObjectGraph* pTypes);

private:
  virtual void InternalCreateDocument(const char* szDocumentTypeName, const char* szPath, bool bCreateNewDocument, wdDocument*& out_pDocument, const wdDocumentObject* pOpenContext) = 0;
  virtual void InternalGetSupportedDocumentTypes(wdDynamicArray<const wdDocumentTypeDescriptor*>& inout_DocumentTypes) const = 0;

private:
  wdStatus CreateOrOpenDocument(bool bCreate, const char* szDocumentTypeName, const char* szPath, wdDocument*& out_pDocument,
    wdBitflags<wdDocumentFlags> flags, const wdDocumentObject* pOpenContext = nullptr);

private:
  WD_MAKE_SUBSYSTEM_STARTUP_FRIEND(ToolsFoundation, DocumentManager);

  static void OnPluginEvent(const wdPluginEvent& e);

  static void UpdateBeforeUnloadingPlugins(const wdPluginEvent& e);
  static void UpdatedAfterLoadingPlugins();

  wdDynamicArray<wdDocument*> m_AllOpenDocuments;

  static wdSet<const wdRTTI*> s_KnownManagers;
  static wdHybridArray<wdDocumentManager*, 16> s_AllDocumentManagers;

  static wdMap<wdString, const wdDocumentTypeDescriptor*> s_AllDocumentDescriptors;
};
