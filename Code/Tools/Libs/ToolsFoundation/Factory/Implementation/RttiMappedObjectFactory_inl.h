

template <typename Object>
nsRttiMappedObjectFactory<Object>::nsRttiMappedObjectFactory() = default;

template <typename Object>
nsRttiMappedObjectFactory<Object>::~nsRttiMappedObjectFactory() = default;

template <typename Object>
void nsRttiMappedObjectFactory<Object>::RegisterCreator(const nsRTTI* pType, CreateObjectFunc creator)
{
  NS_ASSERT_DEV(!m_Creators.Contains(pType), "Type already registered.");

  m_Creators.Insert(pType, creator);
  Event e;
  e.m_Type = Event::Type::CreatorAdded;
  e.m_pRttiType = pType;
  m_Events.Broadcast(e);
}

template <typename Object>
void nsRttiMappedObjectFactory<Object>::UnregisterCreator(const nsRTTI* pType)
{
  NS_ASSERT_DEV(m_Creators.Contains(pType), "Type was never registered.");
  m_Creators.Remove(pType);

  Event e;
  e.m_Type = Event::Type::CreatorRemoved;
  e.m_pRttiType = pType;
  m_Events.Broadcast(e);
}

template <typename Object>
Object* nsRttiMappedObjectFactory<Object>::CreateObject(const nsRTTI* pType)
{
  CreateObjectFunc* creator = nullptr;
  while (pType != nullptr)
  {
    if (m_Creators.TryGetValue(pType, creator))
    {
      return (*creator)(pType);
    }
    pType = pType->GetParentType();
  }
  return nullptr;
}
