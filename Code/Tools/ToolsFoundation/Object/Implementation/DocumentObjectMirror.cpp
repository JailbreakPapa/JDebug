#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Serialization/BinarySerializer.h>
#include <ToolsFoundation/Object/DocumentObjectMirror.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

// clang-format off
WD_BEGIN_STATIC_REFLECTED_TYPE(wdObjectChange, wdNoBase, 1, wdRTTIDefaultAllocator<wdObjectChange>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Change", m_Change),
    WD_MEMBER_PROPERTY("Root", m_Root),
    WD_ARRAY_MEMBER_PROPERTY("Steps", m_Steps),
    WD_MEMBER_PROPERTY("Graph", m_GraphData),
  }
  WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;
// clang-format on

wdObjectChange::wdObjectChange(const wdObjectChange&)
{
  WD_REPORT_FAILURE("Not supported!");
}

void wdObjectChange::GetGraph(wdAbstractObjectGraph& ref_graph) const
{
  ref_graph.Clear();

  wdRawMemoryStreamReader reader(m_GraphData);
  wdAbstractGraphBinarySerializer::Read(reader, &ref_graph);
}

void wdObjectChange::SetGraph(wdAbstractObjectGraph& ref_graph)
{
  wdContiguousMemoryStreamStorage storage;
  wdMemoryStreamWriter writer(&storage);
  wdAbstractGraphBinarySerializer::Write(writer, &ref_graph);

  m_GraphData = {storage.GetData(), storage.GetStorageSize32()};
}

wdObjectChange::wdObjectChange(wdObjectChange&& rhs)
{
  m_Change = std::move(rhs.m_Change);
  m_Root = rhs.m_Root;
  m_Steps = std::move(rhs.m_Steps);
  m_GraphData = std::move(rhs.m_GraphData);
}

void wdObjectChange::operator=(wdObjectChange&& rhs)
{
  m_Change = std::move(rhs.m_Change);
  m_Root = rhs.m_Root;
  m_Steps = std::move(rhs.m_Steps);
  m_GraphData = std::move(rhs.m_GraphData);
}

void wdObjectChange::operator=(wdObjectChange& rhs)
{
  WD_REPORT_FAILURE("Not supported!");
}


wdDocumentObjectMirror::wdDocumentObjectMirror()
{
  m_pContext = nullptr;
  m_pManager = nullptr;
}

wdDocumentObjectMirror::~wdDocumentObjectMirror()
{
  WD_ASSERT_DEV(m_pManager == nullptr && m_pContext == nullptr, "Need to call DeInit before d-tor!");
}

void wdDocumentObjectMirror::InitSender(const wdDocumentObjectManager* pManager)
{
  m_pManager = pManager;
  m_pManager->m_StructureEvents.AddEventHandler(wdMakeDelegate(&wdDocumentObjectMirror::TreeStructureEventHandler, this));
  m_pManager->m_PropertyEvents.AddEventHandler(wdMakeDelegate(&wdDocumentObjectMirror::TreePropertyEventHandler, this));
}

void wdDocumentObjectMirror::InitReceiver(wdRttiConverterContext* pContext)
{
  m_pContext = pContext;
}

void wdDocumentObjectMirror::DeInit()
{
  if (m_pManager)
  {
    m_pManager->m_StructureEvents.RemoveEventHandler(wdMakeDelegate(&wdDocumentObjectMirror::TreeStructureEventHandler, this));
    m_pManager->m_PropertyEvents.RemoveEventHandler(wdMakeDelegate(&wdDocumentObjectMirror::TreePropertyEventHandler, this));
    m_pManager = nullptr;
  }

  if (m_pContext)
  {
    m_pContext = nullptr;
  }
}

void wdDocumentObjectMirror::SetFilterFunction(FilterFunction filter)
{
  m_Filter = filter;
}

void wdDocumentObjectMirror::SendDocument()
{
  const auto* pRoot = m_pManager->GetRootObject();
  for (auto* pChild : pRoot->GetChildren())
  {
    if (IsDiscardedByFilter(pRoot, pChild->GetParentProperty()))
      continue;

    wdObjectChange change;
    change.m_Change.m_Operation = wdObjectChangeType::NodeAdded;
    change.m_Change.m_Value = pChild->GetGuid();

    wdAbstractObjectGraph graph;
    wdDocumentObjectConverterWriter objectConverter(&graph, m_pManager);
    objectConverter.AddObjectToGraph(pChild, "Object");
    change.SetGraph(graph);

    ApplyOp(change);
  }
}

void wdDocumentObjectMirror::Clear()
{
  if (m_pManager)
  {
    const auto* pRoot = m_pManager->GetRootObject();
    for (auto* pChild : pRoot->GetChildren())
    {
      wdObjectChange change;
      change.m_Change.m_Operation = wdObjectChangeType::NodeRemoved;
      change.m_Change.m_Value = pChild->GetGuid();

      /*wdAbstractObjectGraph graph;
      wdDocumentObjectConverterWriter objectConverter(&graph, m_pManager);
      wdAbstractObjectNode* pNode = objectConverter.AddObjectToGraph(pChild, "Object");
      change.SetGraph(graph);*/

      ApplyOp(change);
    }
  }

  if (m_pContext)
  {
    m_pContext->Clear();
  }
}

void wdDocumentObjectMirror::TreeStructureEventHandler(const wdDocumentObjectStructureEvent& e)
{
  if (e.m_pNewParent && IsDiscardedByFilter(e.m_pNewParent, e.m_sParentProperty))
    return;
  if (e.m_pPreviousParent && IsDiscardedByFilter(e.m_pPreviousParent, e.m_sParentProperty))
    return;

  switch (e.m_EventType)
  {
    case wdDocumentObjectStructureEvent::Type::AfterObjectMoved:
    {
      if (IsHeapAllocated(e.m_pNewParent, e.m_sParentProperty))
      {
        if (e.m_pNewParent == nullptr || e.m_pNewParent == m_pManager->GetRootObject())
        {
          // Object is now a root object, nothing to do to attach it to its new parent.
          break;
        }

        if (e.GetProperty()->GetCategory() == wdPropertyCategory::Set && e.m_pPreviousParent == e.m_pNewParent)
        {
          // Sets only have ordering in the editor. We can ignore set order changes in the mirror.
          break;
        }
        wdObjectChange change;
        CreatePath(change, e.m_pNewParent, e.m_sParentProperty);

        change.m_Change.m_Operation = wdObjectChangeType::PropertyInserted;
        change.m_Change.m_Index = e.getInsertIndex();
        change.m_Change.m_Value = e.m_pObject->GetGuid();

        ApplyOp(change);
        break;
      }
      // Intended falltrough as non ptr object might as well be destroyed and rebuild.
    }
      // case wdDocumentObjectStructureEvent::Type::BeforeObjectAdded:
    case wdDocumentObjectStructureEvent::Type::AfterObjectAdded:
    {
      wdObjectChange change;
      CreatePath(change, e.m_pNewParent, e.m_sParentProperty);

      change.m_Change.m_Operation = wdObjectChangeType::NodeAdded;
      change.m_Change.m_Index = e.getInsertIndex();
      change.m_Change.m_Value = e.m_pObject->GetGuid();

      wdAbstractObjectGraph graph;
      wdDocumentObjectConverterWriter objectConverter(&graph, m_pManager);
      objectConverter.AddObjectToGraph(e.m_pObject, "Object");
      change.SetGraph(graph);

      ApplyOp(change);
    }
    break;
    case wdDocumentObjectStructureEvent::Type::BeforeObjectMoved:
    {
      if (IsHeapAllocated(e.m_pPreviousParent, e.m_sParentProperty))
      {
        WD_ASSERT_DEBUG(IsHeapAllocated(e.m_pNewParent, e.m_sParentProperty), "Old and new parent must have the same heap allocation state!");
        if (e.m_pPreviousParent == nullptr || e.m_pPreviousParent == m_pManager->GetRootObject())
        {
          // Object is currently a root object, nothing to do to detach it from its parent.
          break;
        }

        if (e.GetProperty()->GetCategory() == wdPropertyCategory::Set && e.m_pPreviousParent == e.m_pNewParent)
        {
          // Sets only have ordering in the editor. We can ignore set order changes in the mirror.
          break;
        }

        wdObjectChange change;
        CreatePath(change, e.m_pPreviousParent, e.m_sParentProperty);

        // Do not delete heap object, just remove it from its owner.
        change.m_Change.m_Operation = wdObjectChangeType::PropertyRemoved;
        change.m_Change.m_Index = e.m_OldPropertyIndex;
        change.m_Change.m_Value = e.m_pObject->GetGuid();

        ApplyOp(change);
        break;
      }
      else
      {
        wdObjectChange change;
        CreatePath(change, e.m_pPreviousParent, e.m_sParentProperty);

        change.m_Change.m_Operation = wdObjectChangeType::PropertyRemoved;
        change.m_Change.m_Index = e.m_OldPropertyIndex;
        change.m_Change.m_Value = e.m_pObject->GetGuid();

        ApplyOp(change);
        break;
      }
    }
      // case wdDocumentObjectStructureEvent::Type::AfterObjectRemoved:
    case wdDocumentObjectStructureEvent::Type::BeforeObjectRemoved:
    {
      wdObjectChange change;
      CreatePath(change, e.m_pPreviousParent, e.m_sParentProperty);

      change.m_Change.m_Operation = wdObjectChangeType::NodeRemoved;
      change.m_Change.m_Index = e.m_OldPropertyIndex;
      change.m_Change.m_Value = e.m_pObject->GetGuid();

      ApplyOp(change);
    }
    break;

    default:
      break;
  }
}

void wdDocumentObjectMirror::TreePropertyEventHandler(const wdDocumentObjectPropertyEvent& e)
{
  if (IsDiscardedByFilter(e.m_pObject, e.m_sProperty))
    return;

  switch (e.m_EventType)
  {
    case wdDocumentObjectPropertyEvent::Type::PropertySet:
    {
      wdObjectChange change;
      CreatePath(change, e.m_pObject, e.m_sProperty);

      change.m_Change.m_Operation = wdObjectChangeType::PropertySet;
      change.m_Change.m_Index = e.m_NewIndex;
      change.m_Change.m_Value = e.m_NewValue;
      ApplyOp(change);
    }
    break;
    case wdDocumentObjectPropertyEvent::Type::PropertyInserted:
    {
      wdObjectChange change;
      CreatePath(change, e.m_pObject, e.m_sProperty);

      change.m_Change.m_Operation = wdObjectChangeType::PropertyInserted;
      change.m_Change.m_Index = e.m_NewIndex;
      change.m_Change.m_Value = e.m_NewValue;
      ApplyOp(change);
    }
    break;
    case wdDocumentObjectPropertyEvent::Type::PropertyRemoved:
    {
      wdObjectChange change;
      CreatePath(change, e.m_pObject, e.m_sProperty);

      change.m_Change.m_Operation = wdObjectChangeType::PropertyRemoved;
      change.m_Change.m_Index = e.m_OldIndex;
      change.m_Change.m_Value = e.m_OldValue;
      ApplyOp(change);
    }
    break;
    case wdDocumentObjectPropertyEvent::Type::PropertyMoved:
    {
      wdUInt32 uiOldIndex = e.m_OldIndex.ConvertTo<wdUInt32>();
      wdUInt32 uiNewIndex = e.m_NewIndex.ConvertTo<wdUInt32>();
      WD_ASSERT_DEBUG(e.m_NewValue.IsValid(), "Value must be valid");

      {
        wdObjectChange change;
        CreatePath(change, e.m_pObject, e.m_sProperty);

        change.m_Change.m_Operation = wdObjectChangeType::PropertyRemoved;
        change.m_Change.m_Index = uiOldIndex;
        change.m_Change.m_Value = e.m_NewValue;
        ApplyOp(change);
      }

      if (uiNewIndex > uiOldIndex)
      {
        uiNewIndex -= 1;
      }

      {
        wdObjectChange change;
        CreatePath(change, e.m_pObject, e.m_sProperty);

        change.m_Change.m_Operation = wdObjectChangeType::PropertyInserted;
        change.m_Change.m_Index = uiNewIndex;
        change.m_Change.m_Value = e.m_NewValue;
        ApplyOp(change);
      }

      return;
    }
    break;
  }
}

void* wdDocumentObjectMirror::GetNativeObjectPointer(const wdDocumentObject* pObject)
{
  auto object = m_pContext->GetObjectByGUID(pObject->GetGuid());
  return object.m_pObject;
}

const void* wdDocumentObjectMirror::GetNativeObjectPointer(const wdDocumentObject* pObject) const
{
  auto object = m_pContext->GetObjectByGUID(pObject->GetGuid());
  return object.m_pObject;
}

bool wdDocumentObjectMirror::IsRootObject(const wdDocumentObject* pParent)
{
  return (pParent == nullptr || pParent == m_pManager->GetRootObject());
}

bool wdDocumentObjectMirror::IsHeapAllocated(const wdDocumentObject* pParent, const char* szParentProperty)
{
  if (pParent == nullptr || pParent == m_pManager->GetRootObject())
    return true;

  const wdRTTI* pRtti = pParent->GetTypeAccessor().GetType();

  auto* pProp = pRtti->FindPropertyByName(szParentProperty);
  return pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner);
}


bool wdDocumentObjectMirror::IsDiscardedByFilter(const wdDocumentObject* pObject, const char* szProperty) const
{
  if (m_Filter.IsValid())
  {
    return !m_Filter(pObject, szProperty);
  }
  return false;
}

void wdDocumentObjectMirror::CreatePath(wdObjectChange& out_change, const wdDocumentObject* pRoot, const char* szProperty)
{
  if (pRoot && pRoot->GetDocumentObjectManager()->GetRootObject() != pRoot)
  {
    wdHybridArray<const wdDocumentObject*, 8> path;
    out_change.m_Root = FindRootOpObject(pRoot, path);
    FlattenSteps(path, out_change.m_Steps);
  }

  out_change.m_Change.m_sProperty = szProperty;
}

wdUuid wdDocumentObjectMirror::FindRootOpObject(const wdDocumentObject* pParent, wdHybridArray<const wdDocumentObject*, 8>& path)
{
  path.PushBack(pParent);

  if (!pParent->IsOnHeap())
  {
    return FindRootOpObject(pParent->GetParent(), path);
  }
  else
  {
    return pParent->GetGuid();
  }
}

void wdDocumentObjectMirror::FlattenSteps(const wdArrayPtr<const wdDocumentObject* const> path, wdHybridArray<wdPropertyPathStep, 2>& out_steps)
{
  wdUInt32 uiCount = path.GetCount();
  WD_ASSERT_DEV(uiCount > 0, "Path must not be empty!");
  WD_ASSERT_DEV(path[uiCount - 1]->IsOnHeap(), "Root of steps must be on heap!");

  // Only root object? Then there is no path from it.
  if (uiCount == 1)
    return;

  for (wdInt32 i = (wdInt32)uiCount - 2; i >= 0; --i)
  {
    const wdDocumentObject* pObject = path[i];
    out_steps.PushBack(wdPropertyPathStep({pObject->GetParentProperty(), pObject->GetPropertyIndex()}));
  }
}

void wdDocumentObjectMirror::ApplyOp(wdObjectChange& change)
{
  wdRttiConverterObject object;
  if (change.m_Root.IsValid())
  {
    object = m_pContext->GetObjectByGUID(change.m_Root);
    if (!object.m_pObject)
      return;
    // WD_ASSERT_DEV(object.m_pObject != nullptr, "Root object does not exist in mirrored native object!");
  }

  wdPropertyPath propPath;
  if (propPath.InitializeFromPath(*object.m_pType, change.m_Steps).Failed())
  {
    wdLog::Error("Failed to init property path on object of type '{0}'.", object.m_pType->GetTypeName());
    return;
  }
  propPath.WriteToLeafObject(
            object.m_pObject, *object.m_pType, [this, &change](void* pLeaf, const wdRTTI& type) { ApplyOp(wdRttiConverterObject(&type, pLeaf), change); })
    .IgnoreResult();
}

void wdDocumentObjectMirror::ApplyOp(wdRttiConverterObject object, const wdObjectChange& change)
{
  wdAbstractProperty* pProp = nullptr;

  if (object.m_pType != nullptr)
  {
    pProp = object.m_pType->FindPropertyByName(change.m_Change.m_sProperty);
    if (pProp == nullptr)
    {
      wdLog::Error("Property '{0}' not found, can't apply mirror op!", change.m_Change.m_sProperty);
      return;
    }
  }

  switch (change.m_Change.m_Operation)
  {
    case wdObjectChangeType::NodeAdded:
    {
      wdAbstractObjectGraph graph;
      change.GetGraph(graph);
      wdRttiConverterReader reader(&graph, m_pContext);
      const wdAbstractObjectNode* pNode = graph.GetNodeByName("Object");
      const wdRTTI* pType = wdRTTI::FindTypeByName(pNode->GetType());
      void* pValue = reader.CreateObjectFromNode(pNode);
      if (!pValue)
      {
        // Can't create object, exiting.
        return;
      }

      if (!change.m_Root.IsValid())
      {
        // Create without parent (root element)
        return;
      }

      if (pProp->GetCategory() == wdPropertyCategory::Member)
      {
        auto pSpecificProp = static_cast<wdAbstractMemberProperty*>(pProp);
        if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
        {
          pSpecificProp->SetValuePtr(object.m_pObject, &pValue);
        }
        else
        {
          pSpecificProp->SetValuePtr(object.m_pObject, pValue);
        }
      }
      else if (pProp->GetCategory() == wdPropertyCategory::Array)
      {
        auto pSpecificProp = static_cast<wdAbstractArrayProperty*>(pProp);
        if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
        {
          pSpecificProp->Insert(object.m_pObject, change.m_Change.m_Index.ConvertTo<wdUInt32>(), &pValue);
        }
        else
        {
          pSpecificProp->Insert(object.m_pObject, change.m_Change.m_Index.ConvertTo<wdUInt32>(), pValue);
        }
      }
      else if (pProp->GetCategory() == wdPropertyCategory::Set)
      {
        WD_ASSERT_DEV(pProp->GetFlags().IsSet(wdPropertyFlags::Pointer), "Set object must always be pointers!");
        auto pSpecificProp = static_cast<wdAbstractSetProperty*>(pProp);
        wdReflectionUtils::InsertSetPropertyValue(pSpecificProp, object.m_pObject, wdVariant(pValue, pType));
      }
      else if (pProp->GetCategory() == wdPropertyCategory::Map)
      {
        auto pSpecificProp = static_cast<wdAbstractMapProperty*>(pProp);
        if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
        {
          pSpecificProp->Insert(object.m_pObject, change.m_Change.m_Index.Get<wdString>(), &pValue);
        }
        else
        {
          pSpecificProp->Insert(object.m_pObject, change.m_Change.m_Index.Get<wdString>(), pValue);
        }
      }

      if (!pProp->GetFlags().AreAllSet(wdPropertyFlags::Pointer | wdPropertyFlags::PointerOwner))
      {
        m_pContext->DeleteObject(pNode->GetGuid());
      }
    }
    break;
    case wdObjectChangeType::NodeRemoved:
    {
      if (!change.m_Root.IsValid())
      {
        // Delete root object
        m_pContext->DeleteObject(change.m_Change.m_Value.Get<wdUuid>());
        return;
      }

      if (pProp->GetCategory() == wdPropertyCategory::Member)
      {
        auto pSpecificProp = static_cast<wdAbstractMemberProperty*>(pProp);
        if (!pProp->GetFlags().AreAllSet(wdPropertyFlags::Pointer | wdPropertyFlags::PointerOwner))
        {
          wdLog::Error("Property '{0}' not a pointer, can't remove object!", change.m_Change.m_sProperty);
          return;
        }

        void* pValue = nullptr;
        pSpecificProp->SetValuePtr(object.m_pObject, &pValue);
      }
      else if (pProp->GetCategory() == wdPropertyCategory::Array)
      {
        auto pSpecificProp = static_cast<wdAbstractArrayProperty*>(pProp);
        wdReflectionUtils::RemoveArrayPropertyValue(pSpecificProp, object.m_pObject, change.m_Change.m_Index.ConvertTo<wdUInt32>());
      }
      else if (pProp->GetCategory() == wdPropertyCategory::Set)
      {
        WD_ASSERT_DEV(pProp->GetFlags().IsSet(wdPropertyFlags::Pointer), "Set object must always be pointers!");
        auto pSpecificProp = static_cast<wdAbstractSetProperty*>(pProp);
        auto valueObject = m_pContext->GetObjectByGUID(change.m_Change.m_Value.Get<wdUuid>());
        wdReflectionUtils::RemoveSetPropertyValue(pSpecificProp, object.m_pObject, wdVariant(valueObject.m_pObject, valueObject.m_pType));
      }
      else if (pProp->GetCategory() == wdPropertyCategory::Map)
      {
        auto pSpecificProp = static_cast<wdAbstractMapProperty*>(pProp);
        pSpecificProp->Remove(object.m_pObject, change.m_Change.m_Index.Get<wdString>());
      }

      if (pProp->GetFlags().AreAllSet(wdPropertyFlags::Pointer | wdPropertyFlags::PointerOwner))
      {
        m_pContext->DeleteObject(change.m_Change.m_Value.Get<wdUuid>());
      }
    }
    break;
    case wdObjectChangeType::PropertySet:
    {
      if (pProp->GetCategory() == wdPropertyCategory::Member)
      {
        auto pSpecificProp = static_cast<wdAbstractMemberProperty*>(pProp);
        wdReflectionUtils::SetMemberPropertyValue(pSpecificProp, object.m_pObject, change.m_Change.m_Value);
      }
      else if (pProp->GetCategory() == wdPropertyCategory::Array)
      {
        auto pSpecificProp = static_cast<wdAbstractArrayProperty*>(pProp);
        wdReflectionUtils::SetArrayPropertyValue(
          pSpecificProp, object.m_pObject, change.m_Change.m_Index.ConvertTo<wdUInt32>(), change.m_Change.m_Value);
      }
      else if (pProp->GetCategory() == wdPropertyCategory::Set)
      {
        auto pSpecificProp = static_cast<wdAbstractSetProperty*>(pProp);
        wdReflectionUtils::InsertSetPropertyValue(pSpecificProp, object.m_pObject, change.m_Change.m_Value);
      }
      else if (pProp->GetCategory() == wdPropertyCategory::Map)
      {
        auto pSpecificProp = static_cast<wdAbstractMapProperty*>(pProp);
        wdReflectionUtils::SetMapPropertyValue(pSpecificProp, object.m_pObject, change.m_Change.m_Index.Get<wdString>(), change.m_Change.m_Value);
      }
    }
    break;
    case wdObjectChangeType::PropertyInserted:
    {
      wdVariant value = change.m_Change.m_Value;
      if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
      {
        auto valueObject = m_pContext->GetObjectByGUID(change.m_Change.m_Value.Get<wdUuid>());
        value = wdTypedPointer(valueObject.m_pObject, valueObject.m_pType);
      }

      if (pProp->GetCategory() == wdPropertyCategory::Array)
      {
        auto pSpecificProp = static_cast<wdAbstractArrayProperty*>(pProp);
        wdReflectionUtils::InsertArrayPropertyValue(pSpecificProp, object.m_pObject, value, change.m_Change.m_Index.ConvertTo<wdUInt32>());
      }
      else if (pProp->GetCategory() == wdPropertyCategory::Set)
      {
        auto pSpecificProp = static_cast<wdAbstractSetProperty*>(pProp);
        wdReflectionUtils::InsertSetPropertyValue(pSpecificProp, object.m_pObject, value);
      }
      else if (pProp->GetCategory() == wdPropertyCategory::Map)
      {
        auto pSpecificProp = static_cast<wdAbstractMapProperty*>(pProp);
        wdReflectionUtils::SetMapPropertyValue(pSpecificProp, object.m_pObject, change.m_Change.m_Index.Get<wdString>(), value);
      }
    }
    break;
    case wdObjectChangeType::PropertyRemoved:
    {
      if (pProp->GetCategory() == wdPropertyCategory::Array)
      {
        auto pSpecificProp = static_cast<wdAbstractArrayProperty*>(pProp);
        wdReflectionUtils::RemoveArrayPropertyValue(pSpecificProp, object.m_pObject, change.m_Change.m_Index.ConvertTo<wdUInt32>());
      }
      else if (pProp->GetCategory() == wdPropertyCategory::Set)
      {
        wdVariant value = change.m_Change.m_Value;
        if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
        {
          auto valueObject = m_pContext->GetObjectByGUID(change.m_Change.m_Value.Get<wdUuid>());
          value = wdTypedPointer(valueObject.m_pObject, valueObject.m_pType);
        }

        auto pSpecificProp = static_cast<wdAbstractSetProperty*>(pProp);
        wdReflectionUtils::RemoveSetPropertyValue(pSpecificProp, object.m_pObject, value);
      }
      else if (pProp->GetCategory() == wdPropertyCategory::Map)
      {
        auto pSpecificProp = static_cast<wdAbstractMapProperty*>(pProp);
        pSpecificProp->Remove(object.m_pObject, change.m_Change.m_Index.Get<wdString>());
      }
    }
    break;
  }
}
