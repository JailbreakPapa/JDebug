#pragma once

class nsDocument;
class nsDocumentManager;
class nsDocumentObjectManager;
class nsAbstractObjectGraph;

struct nsDocumentFlags
{
  using StorageType = nsUInt8;

  enum Enum
  {
    None = 0,
    RequestWindow = NS_BIT(0),        ///< Open the document visibly (not just internally)
    AddToRecentFilesList = NS_BIT(1), ///< Add the document path to the recently used list for users
    AsyncSave = NS_BIT(2),            ///<
    EmptyDocument = NS_BIT(3),        ///< Don't populate a new document with default state (templates etc)
    Default = None,
  };

  struct Bits
  {
    StorageType RequestWindow : 1;
    StorageType AddToRecentFilesList : 1;
    StorageType AsyncSave : 1;
    StorageType EmptyDocument : 1;
  };
};

NS_DECLARE_FLAGS_OPERATORS(nsDocumentFlags);


struct NS_TOOLSFOUNDATION_DLL nsDocumentTypeDescriptor
{
  nsString m_sFileExtension;
  nsString m_sDocumentTypeName;
  bool m_bCanCreate = true;
  nsString m_sIcon;
  const nsRTTI* m_pDocumentType = nullptr;
  nsDocumentManager* m_pManager = nullptr;
  nsStringView m_sAssetCategory; // passed to nsColorScheme::GetCategoryColor() with CategoryColorUsage::AssetMenuIcon

  /// This list is used to decide which asset types can be picked from the asset browser for a property.
  /// The strings are arbitrary and don't need to be registered anywhere else.
  /// An asset may be compatible for multiple scenarios, e.g. a skinned mesh may also be used as a static mesh, but not the other way round.
  /// In such a case the skinned mesh is set to be compatible to both "CompatibleAsset_Mesh_Static" and "CompatibleAsset_Mesh_Skinned", but the non-skinned mesh only to "CompatibleAsset_Mesh_Static".
  /// A component then only needs to specify that it takes an "CompatibleAsset_Mesh_Static" as input, and all asset types that are compatible to that will be browseable.
  nsHybridArray<nsString, 1> m_CompatibleTypes;
};


struct nsDocumentEvent
{
  enum class Type
  {
    ModifiedChanged,
    ReadOnlyChanged,
    EnsureVisible,
    DocumentSaved,
    DocumentRenamed,
    DocumentStatusMsg,
  };

  Type m_Type;
  const nsDocument* m_pDocument;

  nsStringView m_sStatusMsg;
};

class NS_TOOLSFOUNDATION_DLL nsDocumentInfo : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsDocumentInfo, nsReflectedClass);

public:
  nsDocumentInfo();

  nsUuid m_DocumentID;
};
