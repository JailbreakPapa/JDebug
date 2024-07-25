#pragma once

#include <Foundation/Communication/Event.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Types/RefCounted.h>
#include <Foundation/Types/SharedPtr.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

/// \brief Stores meta data for document objects that is not part of the object itself. E.g. editor-only states like hidden or prefab information.
/// \tparam KEY The key under which data is stored. Usually nsUuid to reference document objects.
/// \tparam VALUE Meta value type to be stored.
template <typename KEY, typename VALUE>
class nsObjectMetaData
{
public:
  struct EventData
  {
    KEY m_ObjectKey;
    const VALUE* m_pValue;
    nsUInt32 m_uiModifiedFlags;
  };

  nsEvent<const EventData&> m_DataModifiedEvent;

  // \brief Storage for the meta data so it can be swapped when using multiple sub documents.
  class Storage : public nsRefCounted
  {
  public:
    mutable enum class AccessMode { Nothing,
      Read,
      Write } m_AccessMode;
    mutable KEY m_AcessingKey;
    mutable nsMutex m_Mutex;
    nsHashTable<KEY, VALUE> m_MetaData;

    nsEvent<const EventData&> m_DataModifiedEvent;
  };

  nsObjectMetaData();

  bool HasMetaData(const KEY objectKey) const;

  void ClearMetaData(const KEY objectKey);

  /// \brief Will always return a non-null result. May be a default object.
  const VALUE* BeginReadMetaData(const KEY objectKey) const;
  void EndReadMetaData() const;

  VALUE* BeginModifyMetaData(const KEY objectKey);
  void EndModifyMetaData(nsUInt32 uiModifiedFlags = 0xFFFFFFFF);


  nsMutex& GetMutex() const { return m_pMetaStorage->m_Mutex; }

  const VALUE& GetDefaultValue() const { return m_DefaultValue; }

  /// \brief Uses reflection information from VALUE to store all properties that differ from the default value as additional properties for the graph
  /// objects.
  void AttachMetaDataToAbstractGraph(nsAbstractObjectGraph& inout_graph) const;

  /// \brief Uses reflection information from VALUE to restore all meta data properties from the graph.
  void RestoreMetaDataFromAbstractGraph(const nsAbstractObjectGraph& graph);

  nsSharedPtr<nsObjectMetaData<KEY, VALUE>::Storage> SwapStorage(nsSharedPtr<nsObjectMetaData<KEY, VALUE>::Storage> pNewStorage);
  nsSharedPtr<nsObjectMetaData<KEY, VALUE>::Storage> GetStorage() { return m_pMetaStorage; }

private:
  VALUE m_DefaultValue;
  nsSharedPtr<nsObjectMetaData<KEY, VALUE>::Storage> m_pMetaStorage;
  typename nsEvent<const EventData&>::Unsubscriber m_EventsUnsubscriber;
};

#include <ToolsFoundation/Object/Implementation/ObjectMetaData_inl.h>
