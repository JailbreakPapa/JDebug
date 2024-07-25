#pragma once

template <typename KEY, typename VALUE>
nsObjectMetaData<KEY, VALUE>::nsObjectMetaData()
{
  m_DefaultValue = VALUE();

  auto pStorage = NS_DEFAULT_NEW(Storage);
  pStorage->m_AcessingKey = KEY();
  pStorage->m_AccessMode = Storage::AccessMode::Nothing;
  SwapStorage(pStorage);
}

template <typename KEY, typename VALUE>
const VALUE* nsObjectMetaData<KEY, VALUE>::BeginReadMetaData(const KEY objectKey) const
{
  m_pMetaStorage->m_Mutex.Lock();
  NS_ASSERT_DEV(m_pMetaStorage->m_AccessMode == Storage::AccessMode::Nothing, "Already accessing some data");
  m_pMetaStorage->m_AccessMode = Storage::AccessMode::Read;
  m_pMetaStorage->m_AcessingKey = objectKey;

  const VALUE* pRes = nullptr;
  if (m_pMetaStorage->m_MetaData.TryGetValue(objectKey, pRes)) // TryGetValue is not const correct with the second parameter
    return pRes;

  return &m_DefaultValue;
}

template <typename KEY, typename VALUE>
void nsObjectMetaData<KEY, VALUE>::ClearMetaData(const KEY objectKey)
{
  NS_LOCK(m_pMetaStorage->m_Mutex);
  NS_ASSERT_DEV(m_pMetaStorage->m_AccessMode == Storage::AccessMode::Nothing, "Already accessing some data");

  if (HasMetaData(objectKey))
  {
    m_pMetaStorage->m_MetaData.Remove(objectKey);

    EventData e;
    e.m_ObjectKey = objectKey;
    e.m_pValue = &m_DefaultValue;

    m_pMetaStorage->m_DataModifiedEvent.Broadcast(e);
  }
}

template <typename KEY, typename VALUE>
bool nsObjectMetaData<KEY, VALUE>::HasMetaData(const KEY objectKey) const
{
  NS_LOCK(m_pMetaStorage->m_Mutex);
  const VALUE* pValue = nullptr;
  return m_pMetaStorage->m_MetaData.TryGetValue(objectKey, pValue);
}

template <typename KEY, typename VALUE>
VALUE* nsObjectMetaData<KEY, VALUE>::BeginModifyMetaData(const KEY objectKey)
{
  m_pMetaStorage->m_Mutex.Lock();
  NS_ASSERT_DEV(m_pMetaStorage->m_AccessMode == Storage::AccessMode::Nothing, "Already accessing some data");
  m_pMetaStorage->m_AccessMode = Storage::AccessMode::Write;
  m_pMetaStorage->m_AcessingKey = objectKey;

  return &m_pMetaStorage->m_MetaData[objectKey];
}

template <typename KEY, typename VALUE>
void nsObjectMetaData<KEY, VALUE>::EndReadMetaData() const
{
  NS_ASSERT_DEV(m_pMetaStorage->m_AccessMode == Storage::AccessMode::Read, "Not accessing data at the moment");

  m_pMetaStorage->m_AccessMode = Storage::AccessMode::Nothing;
  m_pMetaStorage->m_Mutex.Unlock();
}


template <typename KEY, typename VALUE>
void nsObjectMetaData<KEY, VALUE>::EndModifyMetaData(nsUInt32 uiModifiedFlags /*= 0xFFFFFFFF*/)
{
  NS_ASSERT_DEV(m_pMetaStorage->m_AccessMode == Storage::AccessMode::Write, "Not accessing data at the moment");
  m_pMetaStorage->m_AccessMode = Storage::AccessMode::Nothing;

  if (uiModifiedFlags != 0)
  {
    EventData e;
    e.m_ObjectKey = m_pMetaStorage->m_AcessingKey;
    e.m_pValue = &m_pMetaStorage->m_MetaData[m_pMetaStorage->m_AcessingKey];
    e.m_uiModifiedFlags = uiModifiedFlags;

    m_pMetaStorage->m_DataModifiedEvent.Broadcast(e);
  }

  m_pMetaStorage->m_Mutex.Unlock();
}


template <typename KEY, typename VALUE>
void nsObjectMetaData<KEY, VALUE>::AttachMetaDataToAbstractGraph(nsAbstractObjectGraph& inout_graph) const
{
  auto& AllNodes = inout_graph.GetAllNodes();

  NS_LOCK(m_pMetaStorage->m_Mutex);

  nsHashTable<const char*, nsVariant> DefaultValues;

  // store the default values in an easily accessible hash map, to be able to compare against them
  {
    DefaultValues.Reserve(m_DefaultValue.GetDynamicRTTI()->GetProperties().GetCount());

    for (const auto& pProp : m_DefaultValue.GetDynamicRTTI()->GetProperties())
    {
      if (pProp->GetCategory() != nsPropertyCategory::Member)
        continue;

      DefaultValues[pProp->GetPropertyName()] =
        nsReflectionUtils::GetMemberPropertyValue(static_cast<const nsAbstractMemberProperty*>(pProp), &m_DefaultValue);
    }
  }

  // now serialize all properties that differ from the default value
  {
    nsVariant value;

    for (auto it = AllNodes.GetIterator(); it.IsValid(); ++it)
    {
      auto* pNode = it.Value();
      const nsUuid& guid = pNode->GetGuid();

      const VALUE* pMeta = nullptr;
      if (!m_pMetaStorage->m_MetaData.TryGetValue(guid, pMeta)) // TryGetValue is not const correct with the second parameter
        continue;                                               // it is the default object, so all values are default -> skip

      for (const auto& pProp : pMeta->GetDynamicRTTI()->GetProperties())
      {
        if (pProp->GetCategory() != nsPropertyCategory::Member)
          continue;

        value = nsReflectionUtils::GetMemberPropertyValue(static_cast<const nsAbstractMemberProperty*>(pProp), pMeta);

        if (value.IsValid() && DefaultValues[pProp->GetPropertyName()] != value)
        {
          pNode->AddProperty(pProp->GetPropertyName(), value);
        }
      }
    }
  }
}


template <typename KEY, typename VALUE>
void nsObjectMetaData<KEY, VALUE>::RestoreMetaDataFromAbstractGraph(const nsAbstractObjectGraph& graph)
{
  NS_LOCK(m_pMetaStorage->m_Mutex);

  nsHybridArray<nsString, 16> PropertyNames;

  // find all properties (names) that we want to read
  {
    for (const auto& pProp : m_DefaultValue.GetDynamicRTTI()->GetProperties())
    {
      if (pProp->GetCategory() != nsPropertyCategory::Member)
        continue;

      PropertyNames.PushBack(pProp->GetPropertyName());
    }
  }

  auto& AllNodes = graph.GetAllNodes();

  for (auto it = AllNodes.GetIterator(); it.IsValid(); ++it)
  {
    auto* pNode = it.Value();
    const nsUuid& guid = pNode->GetGuid();

    for (const auto& name : PropertyNames)
    {
      if (const auto* pProp = pNode->FindProperty(name))
      {
        VALUE* pValue = &m_pMetaStorage->m_MetaData[guid];

        nsReflectionUtils::SetMemberPropertyValue(
          static_cast<const nsAbstractMemberProperty*>(pValue->GetDynamicRTTI()->FindPropertyByName(name)), pValue, pProp->m_Value);
      }
    }
  }
}

template <typename KEY, typename VALUE>
nsSharedPtr<typename nsObjectMetaData<KEY, VALUE>::Storage> nsObjectMetaData<KEY, VALUE>::SwapStorage(nsSharedPtr<typename nsObjectMetaData<KEY, VALUE>::Storage> pNewStorage)
{
  NS_ASSERT_ALWAYS(pNewStorage != nullptr, "Need a valid history storage object");

  auto retVal = m_pMetaStorage;

  m_EventsUnsubscriber.Unsubscribe();

  m_pMetaStorage = pNewStorage;

  m_pMetaStorage->m_DataModifiedEvent.AddEventHandler([this](const EventData& e)
    { m_DataModifiedEvent.Broadcast(e); }, m_EventsUnsubscriber);

  return retVal;
}
