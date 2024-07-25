#pragma once

#include <Foundation/Containers/Set.h>
#include <ToolsFoundation/Reflection/PhantomRttiManager.h>

class nsReflectedTypeStorageAccessor;
class nsDocumentObject;

/// \brief Manages all nsReflectedTypeStorageAccessor instances.
///
/// This class takes care of patching all nsReflectedTypeStorageAccessor instances when their
/// nsRTTI is modified. It also provides the mapping from property name to the data
/// storage index of the corresponding nsVariant in the nsReflectedTypeStorageAccessor.
class NS_TOOLSFOUNDATION_DLL nsReflectedTypeStorageManager
{
public:
  nsReflectedTypeStorageManager();

private:
  struct ReflectedTypeStorageMapping
  {
    struct StorageInfo
    {
      StorageInfo()
        : m_uiIndex(0)
        , m_Type(nsVariant::Type::Invalid)
      {
      }
      StorageInfo(nsUInt16 uiIndex, nsVariant::Type::Enum type, const nsVariant& defaultValue)
        : m_uiIndex(uiIndex)
        , m_Type(type)
        , m_DefaultValue(defaultValue)
      {
      }

      nsUInt16 m_uiIndex;
      nsEnum<nsVariant::Type> m_Type;
      nsVariant m_DefaultValue;
    };

    /// \brief Flattens all POD type properties of the given nsRTTI into m_PathToStorageInfoTable.
    ///
    /// The functions first adds all parent class properties and then adds its own properties.
    /// POD type properties are added under the current path.
    void AddProperties(const nsRTTI* pType);
    void AddPropertiesRecursive(const nsRTTI* pType, nsSet<const nsDocumentObject*>& ref_requiresPatchingEmbeddedClass);

    void UpdateInstances(nsUInt32 uiIndex, const nsAbstractProperty* pProperty, nsSet<const nsDocumentObject*>& ref_requiresPatchingEmbeddedClass);
    void AddPropertyToInstances(nsUInt32 uiIndex, const nsAbstractProperty* pProperty, nsSet<const nsDocumentObject*>& ref_requiresPatchingEmbeddedClass);

    nsSet<nsReflectedTypeStorageAccessor*> m_Instances;
    nsHashTable<nsString, StorageInfo> m_PathToStorageInfoTable;
  };

  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(ToolsFoundation, ReflectedTypeStorageManager);
  friend class nsReflectedTypeStorageAccessor;

  static void Startup();
  static void Shutdown();

  static const ReflectedTypeStorageMapping* AddStorageAccessor(nsReflectedTypeStorageAccessor* pInstance);
  static void RemoveStorageAccessor(nsReflectedTypeStorageAccessor* pInstance);

  static ReflectedTypeStorageMapping* GetTypeStorageMapping(const nsRTTI* pType);
  static void TypeEventHandler(const nsPhantomRttiManagerEvent& e);

private:
  static nsMap<const nsRTTI*, ReflectedTypeStorageMapping*> s_ReflectedTypeToStorageMapping;
};
