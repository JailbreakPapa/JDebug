#include <Core/CorePCH.h>

#include <Core/WorldSerializer/WorldWriter.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/StringDeduplicationContext.h>

void nsWorldWriter::Clear()
{
  m_AllRootObjects.Clear();
  m_AllChildObjects.Clear();
  m_AllComponents.Clear();

  m_pStream = nullptr;
  m_pExclude = nullptr;

  // invalid handles
  {
    m_WrittenGameObjectHandles.Clear();
    m_WrittenGameObjectHandles[nsGameObjectHandle()] = 0;
  }
}

void nsWorldWriter::WriteWorld(nsStreamWriter& inout_stream, nsWorld& ref_world, const nsTagSet* pExclude)
{
  Clear();

  m_pStream = &inout_stream;
  m_pExclude = pExclude;

  NS_LOCK(ref_world.GetReadMarker());

  ref_world.Traverse(nsMakeDelegate(&nsWorldWriter::ObjectTraverser, this), nsWorld::TraversalMethod::DepthFirst);

  WriteToStream().IgnoreResult();
}

void nsWorldWriter::WriteObjects(nsStreamWriter& inout_stream, const nsDeque<const nsGameObject*>& rootObjects)
{
  Clear();

  m_pStream = &inout_stream;

  for (const nsGameObject* pObject : rootObjects)
  {
    // traversal function takes a non-const object, but we only read it anyway
    Traverse(const_cast<nsGameObject*>(pObject));
  }

  WriteToStream().IgnoreResult();
}

void nsWorldWriter::WriteObjects(nsStreamWriter& inout_stream, nsArrayPtr<const nsGameObject*> rootObjects)
{
  Clear();

  m_pStream = &inout_stream;

  for (const nsGameObject* pObject : rootObjects)
  {
    // traversal function takes a non-const object, but we only read it anyway
    Traverse(const_cast<nsGameObject*>(pObject));
  }

  WriteToStream().IgnoreResult();
}

nsResult nsWorldWriter::WriteToStream()
{
  const nsUInt8 uiVersion = 10;
  *m_pStream << uiVersion;

  // version 8: use string dedup instead of handle writer
  nsStringDeduplicationWriteContext stringDedupWriteContext(*m_pStream);
  m_pStream = &stringDedupWriteContext.Begin();

  IncludeAllComponentBaseTypes();

  nsUInt32 uiNumRootObjects = m_AllRootObjects.GetCount();
  nsUInt32 uiNumChildObjects = m_AllChildObjects.GetCount();
  nsUInt32 uiNumComponentTypes = m_AllComponents.GetCount();

  *m_pStream << uiNumRootObjects;
  *m_pStream << uiNumChildObjects;
  *m_pStream << uiNumComponentTypes;

  // this is used to sort all component types by name, to make the file serialization deterministic
  nsMap<nsString, const nsRTTI*> sortedTypes;

  for (auto it = m_AllComponents.GetIterator(); it.IsValid(); ++it)
  {
    sortedTypes[it.Key()->GetTypeName()] = it.Key();
  }

  AssignGameObjectIndices();
  AssignComponentHandleIndices(sortedTypes);

  for (const auto* pObject : m_AllRootObjects)
  {
    WriteGameObject(pObject);
  }

  for (const auto* pObject : m_AllChildObjects)
  {
    WriteGameObject(pObject);
  }

  for (auto it = sortedTypes.GetIterator(); it.IsValid(); ++it)
  {
    WriteComponentTypeInfo(it.Value());
  }

  for (auto it = sortedTypes.GetIterator(); it.IsValid(); ++it)
  {
    WriteComponentCreationData(m_AllComponents[it.Value()].m_Components);
  }

  for (auto it = sortedTypes.GetIterator(); it.IsValid(); ++it)
  {
    WriteComponentSerializationData(m_AllComponents[it.Value()].m_Components);
  }

  NS_SUCCEED_OR_RETURN(stringDedupWriteContext.End());
  m_pStream = &stringDedupWriteContext.GetOriginalStream();

  return NS_SUCCESS;
}


void nsWorldWriter::AssignGameObjectIndices()
{
  nsUInt32 uiGameObjectIndex = 1;
  for (const auto* pObject : m_AllRootObjects)
  {
    m_WrittenGameObjectHandles[pObject->GetHandle()] = uiGameObjectIndex;
    ++uiGameObjectIndex;
  }

  for (const auto* pObject : m_AllChildObjects)
  {
    m_WrittenGameObjectHandles[pObject->GetHandle()] = uiGameObjectIndex;
    ++uiGameObjectIndex;
  }
}

void nsWorldWriter::AssignComponentHandleIndices(const nsMap<nsString, const nsRTTI*>& sortedTypes)
{
  nsUInt16 uiTypeIndex = 0;

  NS_ASSERT_DEV(m_AllComponents.GetCount() <= nsMath::MaxValue<nsUInt16>(), "Too many types for world writer");

  // assign the component handle indices in the order in which the components are written
  for (auto it = sortedTypes.GetIterator(); it.IsValid(); ++it)
  {
    auto& components = m_AllComponents[it.Value()];

    components.m_uiSerializedTypeIndex = uiTypeIndex;
    ++uiTypeIndex;

    nsUInt32 uiComponentIndex = 1;
    components.m_HandleToIndex[nsComponentHandle()] = 0;

    for (const nsComponent* pComp : components.m_Components)
    {
      components.m_HandleToIndex[pComp->GetHandle()] = uiComponentIndex;
      ++uiComponentIndex;
    }
  }
}


void nsWorldWriter::IncludeAllComponentBaseTypes()
{
  nsDynamicArray<const nsRTTI*> allNow;
  allNow.Reserve(m_AllComponents.GetCount());
  for (auto it = m_AllComponents.GetIterator(); it.IsValid(); ++it)
  {
    allNow.PushBack(it.Key());
  }

  for (auto pRtti : allNow)
  {
    IncludeAllComponentBaseTypes(pRtti->GetParentType());
  }
}


void nsWorldWriter::IncludeAllComponentBaseTypes(const nsRTTI* pRtti)
{
  if (pRtti == nullptr || !pRtti->IsDerivedFrom<nsComponent>() || m_AllComponents.Contains(pRtti))
    return;

  // this is actually used to insert the type, but we have no component of this type
  m_AllComponents[pRtti];

  IncludeAllComponentBaseTypes(pRtti->GetParentType());
}


void nsWorldWriter::Traverse(nsGameObject* pObject)
{
  if (ObjectTraverser(pObject) == nsVisitorExecution::Continue)
  {
    for (auto it = pObject->GetChildren(); it.IsValid(); ++it)
    {
      Traverse(&(*it));
    }
  }
}

void nsWorldWriter::WriteGameObjectHandle(const nsGameObjectHandle& hObject)
{
  auto it = m_WrittenGameObjectHandles.Find(hObject);

  nsUInt32 uiIndex = 0;

  NS_ASSERT_DEV(it.IsValid(), "Referenced object does not exist in the scene. This can happen, if it was optimized away, because it had no name, no children and no essential components.");

  if (it.IsValid())
    uiIndex = it.Value();

  *m_pStream << uiIndex;
}

void nsWorldWriter::WriteComponentHandle(const nsComponentHandle& hComponent)
{
  nsUInt16 uiTypeIndex = 0;
  nsUInt32 uiIndex = 0;

  nsComponent* pComponent = nullptr;
  if (nsWorld::GetWorld(hComponent)->TryGetComponent(hComponent, pComponent))
  {
    if (auto* components = m_AllComponents.GetValue(pComponent->GetDynamicRTTI()))
    {
      auto it = components->m_HandleToIndex.Find(hComponent);
      NS_ASSERT_DEBUG(it.IsValid(), "Handle should always be in the written map at this point");

      if (it.IsValid())
      {
        uiTypeIndex = components->m_uiSerializedTypeIndex;
        uiIndex = it.Value();
      }
    }
  }

  *m_pStream << uiTypeIndex;
  *m_pStream << uiIndex;
}

nsVisitorExecution::Enum nsWorldWriter::ObjectTraverser(nsGameObject* pObject)
{
  if (m_pExclude && pObject->GetTags().IsAnySet(*m_pExclude))
    return nsVisitorExecution::Skip;
  if (pObject->WasCreatedByPrefab())
    return nsVisitorExecution::Skip;

  if (pObject->GetParent())
    m_AllChildObjects.PushBack(pObject);
  else
    m_AllRootObjects.PushBack(pObject);

  auto components = pObject->GetComponents();

  for (const nsComponent* pComp : components)
  {
    if (pComp->WasCreatedByPrefab())
      continue;

    m_AllComponents[pComp->GetDynamicRTTI()].m_Components.PushBack(pComp);
  }

  return nsVisitorExecution::Continue;
}

void nsWorldWriter::WriteGameObject(const nsGameObject* pObject)
{
  if (pObject->GetParent())
    WriteGameObjectHandle(pObject->GetParent()->GetHandle());
  else
    WriteGameObjectHandle(nsGameObjectHandle());

  nsStreamWriter& s = *m_pStream;

  s << pObject->GetName();
  s << pObject->GetGlobalKey();
  s << pObject->GetLocalPosition();
  s << pObject->GetLocalRotation();
  s << pObject->GetLocalScaling();
  s << pObject->GetLocalUniformScaling();
  s << pObject->GetActiveFlag();
  s << pObject->IsDynamic();
  pObject->GetTags().Save(s);
  s << pObject->GetTeamID();
  s << pObject->GetStableRandomSeed();
}

void nsWorldWriter::WriteComponentTypeInfo(const nsRTTI* pRtti)
{
  nsStreamWriter& s = *m_pStream;

  s << pRtti->GetTypeName();
  s << pRtti->GetTypeVersion();
}

void nsWorldWriter::WriteComponentCreationData(const nsDeque<const nsComponent*>& components)
{
  nsDefaultMemoryStreamStorage storage;
  nsMemoryStreamWriter memWriter(&storage);

  nsStreamWriter* pPrevStream = m_pStream;
  m_pStream = &memWriter;

  // write to memory stream
  {
    nsStreamWriter& s = *m_pStream;
    s << components.GetCount();

    nsUInt32 uiComponentIndex = 1;
    for (auto pComponent : components)
    {
      WriteGameObjectHandle(pComponent->GetOwner()->GetHandle());
      s << uiComponentIndex;
      ++uiComponentIndex;

      s << pComponent->GetActiveFlag();

      // version 7
      {
        nsUInt8 userFlags = 0;
        for (nsUInt8 i = 0; i < 8; ++i)
        {
          userFlags |= pComponent->GetUserFlag(i) ? NS_BIT(i) : 0;
        }

        s << userFlags;
      }
    }
  }

  m_pStream = pPrevStream;

  // write result to actual stream
  {
    nsStreamWriter& s = *m_pStream;
    s << storage.GetStorageSize32();

    NS_ASSERT_ALWAYS(storage.GetStorageSize64() <= nsMath::MaxValue<nsUInt32>(), "Slight file format change and version increase needed to support > 4GB worlds.");

    storage.CopyToStream(s).IgnoreResult();
  }
}

void nsWorldWriter::WriteComponentSerializationData(const nsDeque<const nsComponent*>& components)
{
  nsDefaultMemoryStreamStorage storage;
  nsMemoryStreamWriter memWriter(&storage);

  nsStreamWriter* pPrevStream = m_pStream;
  m_pStream = &memWriter;

  // write to memory stream
  for (auto pComp : components)
  {
    pComp->SerializeComponent(*this);
  }

  m_pStream = pPrevStream;

  // write result to actual stream
  {
    nsStreamWriter& s = *m_pStream;
    s << storage.GetStorageSize32();

    NS_ASSERT_ALWAYS(storage.GetStorageSize64() <= nsMath::MaxValue<nsUInt32>(), "Slight file format change and version increase needed to support > 4GB worlds.");

    storage.CopyToStream(s).IgnoreResult();
  }
}
