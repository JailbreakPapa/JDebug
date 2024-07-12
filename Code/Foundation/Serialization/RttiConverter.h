#pragma once

/// \file

#include <Foundation/Basics.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>

class nsAbstractObjectGraph;
class nsAbstractObjectNode;

struct NS_FOUNDATION_DLL nsRttiConverterObject
{
  nsRttiConverterObject()
    : m_pType(nullptr)
    , m_pObject(nullptr)
  {
  }
  nsRttiConverterObject(const nsRTTI* pType, void* pObject)
    : m_pType(pType)
    , m_pObject(pObject)
  {
  }

  NS_DECLARE_POD_TYPE();

  const nsRTTI* m_pType;
  void* m_pObject;
};


class NS_FOUNDATION_DLL nsRttiConverterContext
{
public:
  virtual void Clear();

  /// \brief Generates a guid for a new object. Default implementation generates stable guids derived from
  /// parentGuid + property name + index and ignores the address of pObject.
  virtual nsUuid GenerateObjectGuid(const nsUuid& parentGuid, const nsAbstractProperty* pProp, nsVariant index, void* pObject) const;

  virtual nsInternal::NewInstance<void> CreateObject(const nsUuid& guid, const nsRTTI* pRtti);
  virtual void DeleteObject(const nsUuid& guid);

  virtual void RegisterObject(const nsUuid& guid, const nsRTTI* pRtti, void* pObject);
  virtual void UnregisterObject(const nsUuid& guid);

  virtual nsRttiConverterObject GetObjectByGUID(const nsUuid& guid) const;
  virtual nsUuid GetObjectGUID(const nsRTTI* pRtti, const void* pObject) const;

  virtual const nsRTTI* FindTypeByName(nsStringView sName) const;

  template <typename T>
  void GetObjectsByType(nsDynamicArray<T*>& out_objects, nsDynamicArray<nsUuid>* out_pUuids = nullptr)
  {
    for (auto it : m_GuidToObject)
    {
      if (it.Value().m_pType->IsDerivedFrom(nsGetStaticRTTI<T>()))
      {
        out_objects.PushBack(static_cast<T*>(it.Value().m_pObject));
        if (out_pUuids)
        {
          out_pUuids->PushBack(it.Key());
        }
      }
    }
  }

  virtual nsUuid EnqueObject(const nsUuid& guid, const nsRTTI* pRtti, void* pObject);
  virtual nsRttiConverterObject DequeueObject();

  virtual void OnUnknownTypeError(nsStringView sTypeName);

protected:
  nsHashTable<nsUuid, nsRttiConverterObject> m_GuidToObject;
  mutable nsHashTable<const void*, nsUuid> m_ObjectToGuid;
  nsSet<nsUuid> m_QueuedObjects;
};


class NS_FOUNDATION_DLL nsRttiConverterWriter
{
public:
  using FilterFunction = nsDelegate<bool(const void* pObject, const nsAbstractProperty* pProp)>;

  nsRttiConverterWriter(nsAbstractObjectGraph* pGraph, nsRttiConverterContext* pContext, bool bSerializeReadOnly, bool bSerializeOwnerPtrs);
  nsRttiConverterWriter(nsAbstractObjectGraph* pGraph, nsRttiConverterContext* pContext, FilterFunction filter);

  nsAbstractObjectNode* AddObjectToGraph(nsReflectedClass* pObject, const char* szNodeName = nullptr)
  {
    return AddObjectToGraph(pObject->GetDynamicRTTI(), pObject, szNodeName);
  }
  nsAbstractObjectNode* AddObjectToGraph(const nsRTTI* pRtti, const void* pObject, const char* szNodeName = nullptr);

  void AddProperty(nsAbstractObjectNode* pNode, const nsAbstractProperty* pProp, const void* pObject);
  void AddProperties(nsAbstractObjectNode* pNode, const nsRTTI* pRtti, const void* pObject);

  nsAbstractObjectNode* AddSubObjectToGraph(const nsRTTI* pRtti, const void* pObject, const nsUuid& guid, const char* szNodeName);

private:
  nsRttiConverterContext* m_pContext = nullptr;
  nsAbstractObjectGraph* m_pGraph = nullptr;
  FilterFunction m_Filter;
};

class NS_FOUNDATION_DLL nsRttiConverterReader
{
public:
  nsRttiConverterReader(const nsAbstractObjectGraph* pGraph, nsRttiConverterContext* pContext);

  nsInternal::NewInstance<void> CreateObjectFromNode(const nsAbstractObjectNode* pNode);
  void ApplyPropertiesToObject(const nsAbstractObjectNode* pNode, const nsRTTI* pRtti, void* pObject);

private:
  void ApplyProperty(void* pObject, const nsAbstractProperty* pProperty, const nsAbstractObjectNode::Property* pSource);
  void CallOnObjectCreated(const nsAbstractObjectNode* pNode, const nsRTTI* pRtti, void* pObject);

  nsRttiConverterContext* m_pContext = nullptr;
  const nsAbstractObjectGraph* m_pGraph = nullptr;
};
