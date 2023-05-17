#pragma once

#include <Foundation/Containers/Set.h>
#include <ToolsFoundation/Reflection/PhantomRttiManager.h>

class wdReflectedTypeStorageAccessor;
class wdDocumentObject;

/// \brief Manages all wdReflectedTypeStorageAccessor instances.
///
/// This class takes care of patching all wdReflectedTypeStorageAccessor instances when their
/// wdRTTI is modified. It also provides the mapping from property name to the data
/// storage index of the corresponding wdVariant in the wdReflectedTypeStorageAccessor.
class WD_TOOLSFOUNDATION_DLL wdReflectedTypeStorageManager
{
public:
  wdReflectedTypeStorageManager();

private:
  struct ReflectedTypeStorageMapping
  {
    struct StorageInfo
    {
      StorageInfo()
        : m_uiIndex(0)
        , m_Type(wdVariant::Type::Invalid)
      {
      }
      StorageInfo(wdUInt16 uiIndex, wdVariant::Type::Enum type, const wdVariant& defaultValue)
        : m_uiIndex(uiIndex)
        , m_Type(type)
        , m_DefaultValue(defaultValue)
      {
      }

      wdUInt16 m_uiIndex;
      wdEnum<wdVariant::Type> m_Type;
      wdVariant m_DefaultValue;
    };

    /// \brief Flattens all POD type properties of the given wdRTTI into m_PathToStorageInfoTable.
    ///
    /// The functions first adds all parent class properties and then adds its own properties.
    /// POD type properties are added under the current path.
    void AddProperties(const wdRTTI* pType);
    void AddPropertiesRecursive(const wdRTTI* pType, wdSet<const wdDocumentObject*>& ref_requiresPatchingEmbeddedClass);

    void UpdateInstances(wdUInt32 uiIndex, const wdAbstractProperty* pProperty, wdSet<const wdDocumentObject*>& ref_requiresPatchingEmbeddedClass);
    void AddPropertyToInstances(wdUInt32 uiIndex, const wdAbstractProperty* pProperty, wdSet<const wdDocumentObject*>& ref_requiresPatchingEmbeddedClass);

    wdVariantType::Enum GetStorageType(const wdAbstractProperty* pProperty);

    wdSet<wdReflectedTypeStorageAccessor*> m_Instances;
    wdHashTable<wdString, StorageInfo> m_PathToStorageInfoTable;
  };

  WD_MAKE_SUBSYSTEM_STARTUP_FRIEND(ToolsFoundation, ReflectedTypeStorageManager);
  friend class wdReflectedTypeStorageAccessor;

  static void Startup();
  static void Shutdown();

  static const ReflectedTypeStorageMapping* AddStorageAccessor(wdReflectedTypeStorageAccessor* pInstance);
  static void RemoveStorageAccessor(wdReflectedTypeStorageAccessor* pInstance);

  static ReflectedTypeStorageMapping* GetTypeStorageMapping(const wdRTTI* pType);
  static void TypeEventHandler(const wdPhantomRttiManagerEvent& e);

private:
  static wdMap<const wdRTTI*, ReflectedTypeStorageMapping*> s_ReflectedTypeToStorageMapping;
};
