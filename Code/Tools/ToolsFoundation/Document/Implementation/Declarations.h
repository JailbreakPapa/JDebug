#pragma once

class wdDocument;
class wdDocumentManager;
class wdDocumentObjectManager;
class wdAbstractObjectGraph;

struct wdDocumentFlags
{
  typedef wdUInt8 StorageType;

  enum Enum
  {
    None = 0,
    RequestWindow = WD_BIT(0),        ///< Open the document visibly (not just internally)
    AddToRecentFilesList = WD_BIT(1), ///< Add the document path to the recently used list for users
    AsyncSave = WD_BIT(2),            ///<
    EmptyDocument = WD_BIT(3),        ///< Don't populate a new document with default state (templates etc)
    Default = None,
  };

  struct Bits
  {
    StorageType RequestWindow : 1;
    StorageType AddToRecentFilesList : 1;
    StorageType AsyncSave : 1;
  };
};

WD_DECLARE_FLAGS_OPERATORS(wdDocumentFlags);


struct WD_TOOLSFOUNDATION_DLL wdDocumentTypeDescriptor
{
  wdString m_sFileExtension;
  wdString m_sDocumentTypeName;
  bool m_bCanCreate = true;
  wdString m_sIcon;
  const wdRTTI* m_pDocumentType = nullptr;
  wdDocumentManager* m_pManager = nullptr;

  /// This list is used to decide which asset types can be picked from the asset browser for a property.
  /// The strings are arbitrary and don't need to be registered anywhere else.
  /// An asset may be compatible for multiple scenarios, e.g. a skinned mesh may also be used as a static mesh, but not the other way round.
  /// In such a case the skinned mesh is set to be compatible to both "CompatibleAsset_Mesh_Static" and "CompatibleAsset_Mesh_Skinned", but the non-skinned mesh only to "CompatibleAsset_Mesh_Static".
  /// A component then only needs to specify that it takes an "CompatibleAsset_Mesh_Static" as input, and all asset types that are compatible to that will be browseable.
  wdHybridArray<wdString, 1> m_CompatibleTypes;
};


struct wdDocumentEvent
{
  enum class Type
  {
    ModifiedChanged,
    ReadOnlyChanged,
    EnsureVisible,
    DocumentSaved,
    DocumentStatusMsg,
  };

  Type m_Type;
  const wdDocument* m_pDocument;

  const char* m_szStatusMsg;
};

class WD_TOOLSFOUNDATION_DLL wdDocumentInfo : public wdReflectedClass
{
  WD_ADD_DYNAMIC_REFLECTION(wdDocumentInfo, wdReflectedClass);

public:
  wdDocumentInfo();

  wdUuid m_DocumentID;
};
