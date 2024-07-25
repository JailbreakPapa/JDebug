#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Serialization/BinarySerializer.h>
#include <ToolsFoundation/Object/DocumentObjectMirror.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsObjectChange, nsNoBase, 1, nsRTTIDefaultAllocator<nsObjectChange>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Change", m_Change),
    NS_MEMBER_PROPERTY("Root", m_Root),
    NS_ARRAY_MEMBER_PROPERTY("Steps", m_Steps),
    NS_MEMBER_PROPERTY("Graph", m_GraphData),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

nsObjectChange::nsObjectChange(const nsObjectChange&)
{
  NS_REPORT_FAILURE("Not supported!");
}

void nsObjectChange::GetGraph(nsAbstractObjectGraph& ref_graph) const
{
  ref_graph.Clear();

  nsRawMemoryStreamReader reader(m_GraphData);
  nsAbstractGraphBinarySerializer::Read(reader, &ref_graph);
}

void nsObjectChange::SetGraph(nsAbstractObjectGraph& ref_graph)
{
  nsContiguousMemoryStreamStorage storage;
  nsMemoryStreamWriter writer(&storage);
  nsAbstractGraphBinarySerializer::Write(writer, &ref_graph);

  m_GraphData = {storage.GetData(), storage.GetStorageSize32()};
}

nsObjectChange::nsObjectChange(nsObjectChange&& rhs)
{
  m_Change = std::move(rhs.m_Change);
  m_Root = rhs.m_Root;
  m_Steps = std::move(rhs.m_Steps);
  m_GraphData = std::move(rhs.m_GraphData);
}

void nsObjectChange::operator=(nsObjectChange&& rhs)
{
  m_Change = std::move(rhs.m_Change);
  m_Root = rhs.m_Root;
  m_Steps = std::move(rhs.m_Steps);
  m_GraphData = std::move(rhs.m_GraphData);
}

void nsObjectChange::operator=(nsObjectChange& rhs)
{
  NS_REPORT_FAILURE("Not supported!");
}


nsDocumentObjectMirror::nsDocumentObjectMirror()
{
  m_pContext = nullptr;
  m_pManager = nullptr;
}

nsDocumentObjectMirror::~nsDocumentObjectMirror()
{
  NS_ASSERT_DEV(m_pManager == nullptr && m_pContext == nullptr, "Need to call DeInit before d-tor!");
}

void nsDocumentObjectMirror::InitSender(const nsDocumentObjectManager* pManager)
{
  m_pManager = pManager;
  m_pManager->m_StructureEvents.AddEventHandler(nsMakeDelegate(&nsDocumentObjectMirror::TreeStructureEventHandler, this));
  m_pManager->m_PropertyEvents.AddEventHandler(nsMakeDelegate(&nsDocumentObjectMirror::TreePropertyEventHandler, this));
}

void nsDocumentObjectMirror::InitReceiver(nsRttiConverterContext* pContext)
{
  m_pContext = pContext;
}

void nsDocumentObjectMirror::DeInit()
{
  if (m_pManager)
  {
    m_pManager->m_StructureEvents.RemoveEventHandler(nsMakeDelegate(&nsDocumentObjectMirror::TreeStructureEventHandler, this));
    m_pManager->m_PropertyEvents.RemoveEventHandler(nsMakeDelegate(&nsDocumentObjectMirror::TreePropertyEventHandler, this));
    m_pManager = nullptr;
  }

  if (m_pContext)
  {
    m_pContext = nullptr;
  }
}

void nsDocumentObjectMirror::SetFilterFunction(FilterFunction filter)
{
  m_Filter = filter;
}

void nsDocumentObjectMirror::SendDocument()
{
  const auto* pRoot = m_pManager->GetRootObject();
  for (auto* pChild : pRoot->GetChildren())
  {
    if (IsDiscardedByFilter(pRoot, pChild->GetParentProperty()))
      continue;

    nsObjectChange change;
    change.m_Change.m_Operation = nsObjectChangeType::NodeAdded;
    change.m_Change.m_Value = pChild->GetGuid();

    nsAbstractObjectGraph graph;
    nsDocumentObjectConverterWriter objectConverter(&graph, m_pManager);
    objectConverter.AddObjectToGraph(pChild, "Object");
    change.SetGraph(graph);

    ApplyOp(change);
  }
}

void nsDocumentObjectMirror::Clear()
{
  if (m_pManager)
  {
    const auto* pRoot = m_pManager->GetRootObject();
    for (auto* pChild : pRoot->GetChildren())
    {
      nsObjectChange change;
      change.m_Change.m_Operation = nsObjectChangeType::NodeRemoved;
      change.m_Change.m_Value = pChild->GetGuid();

      /*nsAbstractObjectGraph graph;
      nsDocumentObjectConverterWriter objectConverter(&graph, m_pManager);
      nsAbstractObjectNode* pNode = objectConverter.AddObjectToGraph(pChild, "Object");
      change.SetGraph(graph);*/

      ApplyOp(change);
    }
  }

  if (m_pContext)
  {
    m_pContext->Clear();
  }
}

void nsDocumentObjectMirror::TreeStructureEventHandler(const nsDocumentObjectStructureEvent& e)
{
  if (e.m_pNewParent && IsDiscardedByFilter(e.m_pNewParent, e.m_sParentProperty))
    return;
  if (e.m_pPreviousParent && IsDiscardedByFilter(e.m_pPreviousParent, e.m_sParentProperty))
    return;

  switch (e.m_EventType)
  {
    case nsDocumentObjectStructureEvent::Type::AfterObjectMoved:
    {
      if (IsHeapAllocated(e.m_pNewParent, e.m_sParentProperty))
      {
        if (e.m_pNewParent == nullptr || e.m_pNewParent == m_pManager->GetRootObject())
        {
          // Object is now a root object, nothing to do to attach it to its new parent.
          break;
        }

        if (e.GetProperty()->GetCategory() == nsPropertyCategory::Set && e.m_pPreviousParent == e.m_pNewParent)
        {
          // Sets only have ordering in the editor. We can ignore set order changes in the mirror.
          break;
        }
        nsObjectChange change;
        CreatePath(change, e.m_pNewParent, e.m_sParentProperty);

        change.m_Change.m_Operation = nsObjectChangeType::PropertyInserted;
        change.m_Change.m_Index = e.getInsertIndex();
        change.m_Change.m_Value = e.m_pObject->GetGuid();

        ApplyOp(change);
        break;
      }
      // Intended falltrough as non ptr object might as well be destroyed and rebuild.
    }
      // case nsDocumentObjectStructureEvent::Type::BeforeObjectAdded:
    case nsDocumentObjectStructureEvent::Type::AfterObjectAdded:
    {
      nsObjectChange change;
      CreatePath(change, e.m_pNewParent, e.m_sParentProperty);

      change.m_Change.m_Operation = nsObjectChangeType::NodeAdded;
      change.m_Change.m_Index = e.getInsertIndex();
      change.m_Change.m_Value = e.m_pObject->GetGuid();

      nsAbstractObjectGraph graph;
      nsDocumentObjectConverterWriter objectConverter(&graph, m_pManager);
      objectConverter.AddObjectToGraph(e.m_pObject, "Object");
      change.SetGraph(graph);

      ApplyOp(change);
    }
    break;
    case nsDocumentObjectStructureEvent::Type::BeforeObjectMoved:
    {
      if (IsHeapAllocated(e.m_pPreviousParent, e.m_sParentProperty))
      {
        NS_ASSERT_DEBUG(IsHeapAllocated(e.m_pNewParent, e.m_sParentProperty), "Old and new parent must have the same heap allocation state!");
        if (e.m_pPreviousParent == nullptr || e.m_pPreviousParent == m_pManager->GetRootObject())
        {
          // Object is currently a root object, nothing to do to detach it from its parent.
          break;
        }

        if (e.GetProperty()->GetCategory() == nsPropertyCategory::Set && e.m_pPreviousParent == e.m_pNewParent)
        {
          // Sets only have ordering in the editor. We can ignore set order changes in the mirror.
          break;
        }

        nsObjectChange change;
        CreatePath(change, e.m_pPreviousParent, e.m_sParentProperty);

        // Do not delete heap object, just remove it from its owner.
        change.m_Change.m_Operation = nsObjectChangeType::PropertyRemoved;
        change.m_Change.m_Index = e.m_OldPropertyIndex;
        change.m_Change.m_Value = e.m_pObject->GetGuid();

        ApplyOp(change);
        break;
      }
      else
      {
        nsObjectChange change;
        CreatePath(change, e.m_pPreviousParent, e.m_sParentProperty);

        change.m_Change.m_Operation = nsObjectChangeType::PropertyRemoved;
        change.m_Change.m_Index = e.m_OldPropertyIndex;
        change.m_Change.m_Value = e.m_pObject->GetGuid();

        ApplyOp(change);
        break;
      }
    }
      // case nsDocumentObjectStructureEvent::Type::AfterObjectRemoved:
    case nsDocumentObjectStructureEvent::Type::BeforeObjectRemoved:
    {
      nsObjectChange change;
      CreatePath(change, e.m_pPreviousParent, e.m_sParentProperty);

      change.m_Change.m_Operation = nsObjectChangeType::NodeRemoved;
      change.m_Change.m_Index = e.m_OldPropertyIndex;
      change.m_Change.m_Value = e.m_pObject->GetGuid();

      ApplyOp(change);
    }
    break;

    default:
      break;
  }
}

void nsDocumentObjectMirror::TreePropertyEventHandler(const nsDocumentObjectPropertyEvent& e)
{
  if (IsDiscardedByFilter(e.m_pObject, e.m_sProperty))
    return;

  switch (e.m_EventType)
  {
    case nsDocumentObjectPropertyEvent::Type::PropertySet:
    {
      nsObjectChange change;
      CreatePath(change, e.m_pObject, e.m_sProperty);

      change.m_Change.m_Operation = nsObjectChangeType::PropertySet;
      change.m_Change.m_Index = e.m_NewIndex;
      change.m_Change.m_Value = e.m_NewValue;
      ApplyOp(change);
    }
    break;
    case nsDocumentObjectPropertyEvent::Type::PropertyInserted:
    {
      nsObjectChange change;
      CreatePath(change, e.m_pObject, e.m_sProperty);

      change.m_Change.m_Operation = nsObjectChangeType::PropertyInserted;
      change.m_Change.m_Index = e.m_NewIndex;
      change.m_Change.m_Value = e.m_NewValue;
      ApplyOp(change);
    }
    break;
    case nsDocumentObjectPropertyEvent::Type::PropertyRemoved:
    {
      nsObjectChange change;
      CreatePath(change, e.m_pObject, e.m_sProperty);

      change.m_Change.m_Operation = nsObjectChangeType::PropertyRemoved;
      change.m_Change.m_Index = e.m_OldIndex;
      change.m_Change.m_Value = e.m_OldValue;
      ApplyOp(change);
    }
    break;
    case nsDocumentObjectPropertyEvent::Type::PropertyMoved:
    {
      nsUInt32 uiOldIndex = e.m_OldIndex.ConvertTo<nsUInt32>();
      nsUInt32 uiNewIndex = e.m_NewIndex.ConvertTo<nsUInt32>();
      // NewValue can be invalid if an invalid variant in a variant array is moved
      // NS_ASSERT_DEBUG(e.m_NewValue.IsValid(), "Value must be valid");

      {
        nsObjectChange change;
        CreatePath(change, e.m_pObject, e.m_sProperty);

        change.m_Change.m_Operation = nsObjectChangeType::PropertyRemoved;
        change.m_Change.m_Index = uiOldIndex;
        change.m_Change.m_Value = e.m_NewValue;
        ApplyOp(change);
      }

      if (uiNewIndex > uiOldIndex)
      {
        uiNewIndex -= 1;
      }

      {
        nsObjectChange change;
        CreatePath(change, e.m_pObject, e.m_sProperty);

        change.m_Change.m_Operation = nsObjectChangeType::PropertyInserted;
        change.m_Change.m_Index = uiNewIndex;
        change.m_Change.m_Value = e.m_NewValue;
        ApplyOp(change);
      }

      return;
    }
    break;
  }
}

void* nsDocumentObjectMirror::GetNativeObjectPointer(const nsDocumentObject* pObject)
{
  auto object = m_pContext->GetObjectByGUID(pObject->GetGuid());
  return object.m_pObject;
}

const void* nsDocumentObjectMirror::GetNativeObjectPointer(const nsDocumentObject* pObject) const
{
  auto object = m_pContext->GetObjectByGUID(pObject->GetGuid());
  return object.m_pObject;
}

bool nsDocumentObjectMirror::IsRootObject(const nsDocumentObject* pParent)
{
  return (pParent == nullptr || pParent == m_pManager->GetRootObject());
}

bool nsDocumentObjectMirror::IsHeapAllocated(const nsDocumentObject* pParent, nsStringView sParentProperty)
{
  if (pParent == nullptr || pParent == m_pManager->GetRootObject())
    return true;

  const nsRTTI* pRtti = pParent->GetTypeAccessor().GetType();

  auto* pProp = pRtti->FindPropertyByName(sParentProperty);
  return pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner);
}


bool nsDocumentObjectMirror::IsDiscardedByFilter(const nsDocumentObject* pObject, nsStringView sProperty) const
{
  if (m_Filter.IsValid())
  {
    return !m_Filter(pObject, sProperty);
  }
  return false;
}

void nsDocumentObjectMirror::CreatePath(nsObjectChange& out_change, const nsDocumentObject* pRoot, nsStringView sProperty)
{
  if (pRoot && pRoot->GetDocumentObjectManager()->GetRootObject() != pRoot)
  {
    nsHybridArray<const nsDocumentObject*, 8> path;
    out_change.m_Root = FindRootOpObject(pRoot, path);
    FlattenSteps(path, out_change.m_Steps);
  }

  out_change.m_Change.m_sProperty = sProperty;
}

nsUuid nsDocumentObjectMirror::FindRootOpObject(const nsDocumentObject* pParent, nsHybridArray<const nsDocumentObject*, 8>& path)
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

void nsDocumentObjectMirror::FlattenSteps(const nsArrayPtr<const nsDocumentObject* const> path, nsHybridArray<nsPropertyPathStep, 2>& out_steps)
{
  nsUInt32 uiCount = path.GetCount();
  NS_ASSERT_DEV(uiCount > 0, "Path must not be empty!");
  NS_ASSERT_DEV(path[uiCount - 1]->IsOnHeap(), "Root of steps must be on heap!");

  // Only root object? Then there is no path from it.
  if (uiCount == 1)
    return;

  for (nsInt32 i = (nsInt32)uiCount - 2; i >= 0; --i)
  {
    const nsDocumentObject* pObject = path[i];
    out_steps.PushBack(nsPropertyPathStep({pObject->GetParentProperty(), pObject->GetPropertyIndex()}));
  }
}

void nsDocumentObjectMirror::ApplyOp(nsObjectChange& change)
{
  nsRttiConverterObject object;
  if (change.m_Root.IsValid())
  {
    object = m_pContext->GetObjectByGUID(change.m_Root);
    if (!object.m_pObject)
      return;
    // NS_ASSERT_DEV(object.m_pObject != nullptr, "Root object does not exist in mirrored native object!");
  }

  nsPropertyPath propPath;
  if (propPath.InitializeFromPath(object.m_pType, change.m_Steps).Failed())
  {
    nsLog::Error("Failed to init property path on object of type '{0}'.", object.m_pType->GetTypeName());
    return;
  }
  propPath.WriteToLeafObject(
            object.m_pObject, *object.m_pType, [this, &change](void* pLeaf, const nsRTTI& type)
            { ApplyOp(nsRttiConverterObject(&type, pLeaf), change); })
    .IgnoreResult();
}

void nsDocumentObjectMirror::ApplyOp(nsRttiConverterObject object, const nsObjectChange& change)
{
  const nsAbstractProperty* pProp = nullptr;

  if (object.m_pType != nullptr)
  {
    pProp = object.m_pType->FindPropertyByName(change.m_Change.m_sProperty);
    if (pProp == nullptr)
    {
      nsLog::Error("Property '{0}' not found, can't apply mirror op!", change.m_Change.m_sProperty);
      return;
    }
  }

  switch (change.m_Change.m_Operation)
  {
    case nsObjectChangeType::NodeAdded:
    {
      nsAbstractObjectGraph graph;
      change.GetGraph(graph);
      nsRttiConverterReader reader(&graph, m_pContext);
      const nsAbstractObjectNode* pNode = graph.GetNodeByName("Object");
      const nsRTTI* pType = m_pContext->FindTypeByName(pNode->GetType());
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

      if (pProp->GetCategory() == nsPropertyCategory::Member)
      {
        auto pSpecificProp = static_cast<const nsAbstractMemberProperty*>(pProp);
        if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
        {
          pSpecificProp->SetValuePtr(object.m_pObject, &pValue);
        }
        else
        {
          pSpecificProp->SetValuePtr(object.m_pObject, pValue);
        }
      }
      else if (pProp->GetCategory() == nsPropertyCategory::Array)
      {
        auto pSpecificProp = static_cast<const nsAbstractArrayProperty*>(pProp);
        if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
        {
          pSpecificProp->Insert(object.m_pObject, change.m_Change.m_Index.ConvertTo<nsUInt32>(), &pValue);
        }
        else
        {
          pSpecificProp->Insert(object.m_pObject, change.m_Change.m_Index.ConvertTo<nsUInt32>(), pValue);
        }
      }
      else if (pProp->GetCategory() == nsPropertyCategory::Set)
      {
        NS_ASSERT_DEV(pProp->GetFlags().IsSet(nsPropertyFlags::Pointer), "Set object must always be pointers!");
        auto pSpecificProp = static_cast<const nsAbstractSetProperty*>(pProp);
        nsReflectionUtils::InsertSetPropertyValue(pSpecificProp, object.m_pObject, nsVariant(pValue, pType));
      }
      else if (pProp->GetCategory() == nsPropertyCategory::Map)
      {
        auto pSpecificProp = static_cast<const nsAbstractMapProperty*>(pProp);
        if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
        {
          pSpecificProp->Insert(object.m_pObject, change.m_Change.m_Index.Get<nsString>(), &pValue);
        }
        else
        {
          pSpecificProp->Insert(object.m_pObject, change.m_Change.m_Index.Get<nsString>(), pValue);
        }
      }

      if (!pProp->GetFlags().AreAllSet(nsPropertyFlags::Pointer | nsPropertyFlags::PointerOwner))
      {
        m_pContext->DeleteObject(pNode->GetGuid());
      }
    }
    break;
    case nsObjectChangeType::NodeRemoved:
    {
      if (!change.m_Root.IsValid())
      {
        // Delete root object
        m_pContext->DeleteObject(change.m_Change.m_Value.Get<nsUuid>());
        return;
      }

      if (pProp->GetCategory() == nsPropertyCategory::Member)
      {
        auto pSpecificProp = static_cast<const nsAbstractMemberProperty*>(pProp);
        if (!pProp->GetFlags().AreAllSet(nsPropertyFlags::Pointer | nsPropertyFlags::PointerOwner))
        {
          nsLog::Error("Property '{0}' not a pointer, can't remove object!", change.m_Change.m_sProperty);
          return;
        }

        void* pValue = nullptr;
        pSpecificProp->SetValuePtr(object.m_pObject, &pValue);
      }
      else if (pProp->GetCategory() == nsPropertyCategory::Array)
      {
        auto pSpecificProp = static_cast<const nsAbstractArrayProperty*>(pProp);
        nsReflectionUtils::RemoveArrayPropertyValue(pSpecificProp, object.m_pObject, change.m_Change.m_Index.ConvertTo<nsUInt32>());
      }
      else if (pProp->GetCategory() == nsPropertyCategory::Set)
      {
        NS_ASSERT_DEV(pProp->GetFlags().IsSet(nsPropertyFlags::Pointer), "Set object must always be pointers!");
        auto pSpecificProp = static_cast<const nsAbstractSetProperty*>(pProp);
        auto valueObject = m_pContext->GetObjectByGUID(change.m_Change.m_Value.Get<nsUuid>());
        nsReflectionUtils::RemoveSetPropertyValue(pSpecificProp, object.m_pObject, nsVariant(valueObject.m_pObject, valueObject.m_pType));
      }
      else if (pProp->GetCategory() == nsPropertyCategory::Map)
      {
        auto pSpecificProp = static_cast<const nsAbstractMapProperty*>(pProp);
        pSpecificProp->Remove(object.m_pObject, change.m_Change.m_Index.Get<nsString>());
      }

      if (pProp->GetFlags().AreAllSet(nsPropertyFlags::Pointer | nsPropertyFlags::PointerOwner))
      {
        m_pContext->DeleteObject(change.m_Change.m_Value.Get<nsUuid>());
      }
    }
    break;
    case nsObjectChangeType::PropertySet:
    {
      if (pProp->GetCategory() == nsPropertyCategory::Member)
      {
        auto pSpecificProp = static_cast<const nsAbstractMemberProperty*>(pProp);
        nsReflectionUtils::SetMemberPropertyValue(pSpecificProp, object.m_pObject, change.m_Change.m_Value);
      }
      else if (pProp->GetCategory() == nsPropertyCategory::Array)
      {
        auto pSpecificProp = static_cast<const nsAbstractArrayProperty*>(pProp);
        nsReflectionUtils::SetArrayPropertyValue(
          pSpecificProp, object.m_pObject, change.m_Change.m_Index.ConvertTo<nsUInt32>(), change.m_Change.m_Value);
      }
      else if (pProp->GetCategory() == nsPropertyCategory::Set)
      {
        auto pSpecificProp = static_cast<const nsAbstractSetProperty*>(pProp);
        nsReflectionUtils::InsertSetPropertyValue(pSpecificProp, object.m_pObject, change.m_Change.m_Value);
      }
      else if (pProp->GetCategory() == nsPropertyCategory::Map)
      {
        auto pSpecificProp = static_cast<const nsAbstractMapProperty*>(pProp);
        nsReflectionUtils::SetMapPropertyValue(pSpecificProp, object.m_pObject, change.m_Change.m_Index.Get<nsString>(), change.m_Change.m_Value);
      }
    }
    break;
    case nsObjectChangeType::PropertyInserted:
    {
      nsVariant value = change.m_Change.m_Value;
      if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
      {
        auto valueObject = m_pContext->GetObjectByGUID(change.m_Change.m_Value.Get<nsUuid>());
        value = nsTypedPointer(valueObject.m_pObject, valueObject.m_pType);
      }

      if (pProp->GetCategory() == nsPropertyCategory::Array)
      {
        auto pSpecificProp = static_cast<const nsAbstractArrayProperty*>(pProp);
        nsReflectionUtils::InsertArrayPropertyValue(pSpecificProp, object.m_pObject, value, change.m_Change.m_Index.ConvertTo<nsUInt32>());
      }
      else if (pProp->GetCategory() == nsPropertyCategory::Set)
      {
        auto pSpecificProp = static_cast<const nsAbstractSetProperty*>(pProp);
        nsReflectionUtils::InsertSetPropertyValue(pSpecificProp, object.m_pObject, value);
      }
      else if (pProp->GetCategory() == nsPropertyCategory::Map)
      {
        auto pSpecificProp = static_cast<const nsAbstractMapProperty*>(pProp);
        nsReflectionUtils::SetMapPropertyValue(pSpecificProp, object.m_pObject, change.m_Change.m_Index.Get<nsString>(), value);
      }
    }
    break;
    case nsObjectChangeType::PropertyRemoved:
    {
      if (pProp->GetCategory() == nsPropertyCategory::Array)
      {
        auto pSpecificProp = static_cast<const nsAbstractArrayProperty*>(pProp);
        nsReflectionUtils::RemoveArrayPropertyValue(pSpecificProp, object.m_pObject, change.m_Change.m_Index.ConvertTo<nsUInt32>());
      }
      else if (pProp->GetCategory() == nsPropertyCategory::Set)
      {
        nsVariant value = change.m_Change.m_Value;
        if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
        {
          auto valueObject = m_pContext->GetObjectByGUID(change.m_Change.m_Value.Get<nsUuid>());
          value = nsTypedPointer(valueObject.m_pObject, valueObject.m_pType);
        }

        auto pSpecificProp = static_cast<const nsAbstractSetProperty*>(pProp);
        nsReflectionUtils::RemoveSetPropertyValue(pSpecificProp, object.m_pObject, value);
      }
      else if (pProp->GetCategory() == nsPropertyCategory::Map)
      {
        auto pSpecificProp = static_cast<const nsAbstractMapProperty*>(pProp);
        pSpecificProp->Remove(object.m_pObject, change.m_Change.m_Index.Get<nsString>());
      }
    }
    break;
  }
}
