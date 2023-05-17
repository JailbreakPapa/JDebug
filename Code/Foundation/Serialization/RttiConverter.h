#pragma once

/// \file

#include <Foundation/Basics.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>

class wdAbstractObjectGraph;
class wdAbstractObjectNode;

struct WD_FOUNDATION_DLL wdRttiConverterObject
{
  wdRttiConverterObject()
    : m_pType(nullptr)
    , m_pObject(nullptr)
  {
  }
  wdRttiConverterObject(const wdRTTI* pType, void* pObject)
    : m_pType(pType)
    , m_pObject(pObject)
  {
  }

  WD_DECLARE_POD_TYPE();

  const wdRTTI* m_pType;
  void* m_pObject;
};


class WD_FOUNDATION_DLL wdRttiConverterContext
{
public:
  virtual void Clear();

  /// \brief Generates a guid for a new object. Default implementation generates stable guids derived from
  /// parentGuid + property name + index and ignores the address of pObject.
  virtual wdUuid GenerateObjectGuid(const wdUuid& parentGuid, const wdAbstractProperty* pProp, wdVariant index, void* pObject) const;

  virtual wdInternal::NewInstance<void> CreateObject(const wdUuid& guid, const wdRTTI* pRtti);
  virtual void DeleteObject(const wdUuid& guid);

  virtual void RegisterObject(const wdUuid& guid, const wdRTTI* pRtti, void* pObject);
  virtual void UnregisterObject(const wdUuid& guid);

  virtual wdRttiConverterObject GetObjectByGUID(const wdUuid& guid) const;
  virtual wdUuid GetObjectGUID(const wdRTTI* pRtti, const void* pObject) const;

  virtual wdUuid EnqueObject(const wdUuid& guid, const wdRTTI* pRtti, void* pObject);
  virtual wdRttiConverterObject DequeueObject();

  virtual void OnUnknownTypeError(wdStringView sTypeName);

protected:
  wdHashTable<wdUuid, wdRttiConverterObject> m_GuidToObject;
  mutable wdHashTable<const void*, wdUuid> m_ObjectToGuid;
  wdSet<wdUuid> m_QueuedObjects;
};


class WD_FOUNDATION_DLL wdRttiConverterWriter
{
public:
  using FilterFunction = wdDelegate<bool(const void* pObject, const wdAbstractProperty* pProp)>;

  wdRttiConverterWriter(wdAbstractObjectGraph* pGraph, wdRttiConverterContext* pContext, bool bSerializeReadOnly, bool bSerializeOwnerPtrs);
  wdRttiConverterWriter(wdAbstractObjectGraph* pGraph, wdRttiConverterContext* pContext, FilterFunction filter);

  wdAbstractObjectNode* AddObjectToGraph(wdReflectedClass* pObject, const char* szNodeName = nullptr)
  {
    return AddObjectToGraph(pObject->GetDynamicRTTI(), pObject, szNodeName);
  }
  wdAbstractObjectNode* AddObjectToGraph(const wdRTTI* pRtti, const void* pObject, const char* szNodeName = nullptr);

  void AddProperty(wdAbstractObjectNode* pNode, const wdAbstractProperty* pProp, const void* pObject);
  void AddProperties(wdAbstractObjectNode* pNode, const wdRTTI* pRtti, const void* pObject);

  wdAbstractObjectNode* AddSubObjectToGraph(const wdRTTI* pRtti, const void* pObject, const wdUuid& guid, const char* szNodeName);

private:
  wdRttiConverterContext* m_pContext = nullptr;
  wdAbstractObjectGraph* m_pGraph = nullptr;
  FilterFunction m_Filter;
  bool m_bSerializeReadOnly = false;
  bool m_bSerializeOwnerPtrs = false;
};

class WD_FOUNDATION_DLL wdRttiConverterReader
{
public:
  wdRttiConverterReader(const wdAbstractObjectGraph* pGraph, wdRttiConverterContext* pContext);

  wdInternal::NewInstance<void> CreateObjectFromNode(const wdAbstractObjectNode* pNode);
  void ApplyPropertiesToObject(const wdAbstractObjectNode* pNode, const wdRTTI* pRtti, void* pObject);

private:
  void ApplyProperty(void* pObject, wdAbstractProperty* pProperty, const wdAbstractObjectNode::Property* pSource);
  void CallOnObjectCreated(const wdAbstractObjectNode* pNode, const wdRTTI* pRtti, void* pObject);

  wdRttiConverterContext* m_pContext = nullptr;
  const wdAbstractObjectGraph* m_pGraph = nullptr;
};
