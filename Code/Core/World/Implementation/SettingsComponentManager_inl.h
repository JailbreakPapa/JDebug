
template <typename ComponentType>
nsSettingsComponentManager<ComponentType>::nsSettingsComponentManager(nsWorld* pWorld)
  : nsComponentManagerBase(pWorld)
{
}

template <typename ComponentType>
nsSettingsComponentManager<ComponentType>::~nsSettingsComponentManager()
{
  for (auto& component : m_Components)
  {
    DeinitializeComponent(component.Borrow());
  }
}

template <typename ComponentType>
NS_ALWAYS_INLINE ComponentType* nsSettingsComponentManager<ComponentType>::GetSingletonComponent()
{
  for (const auto& pComponent : m_Components)
  {
    // retrieve the first component that is active
    if (pComponent->IsActive())
      return pComponent.Borrow();
  }

  return nullptr;
}

template <typename ComponentType>
NS_ALWAYS_INLINE const ComponentType* nsSettingsComponentManager<ComponentType>::GetSingletonComponent() const
{
  for (const auto& pComponent : m_Components)
  {
    // retrieve the first component that is active
    if (pComponent->IsActive())
      return pComponent.Borrow();
  }

  return nullptr;
}

// static
template <typename ComponentType>
NS_ALWAYS_INLINE nsWorldModuleTypeId nsSettingsComponentManager<ComponentType>::TypeId()
{
  return ComponentType::TypeId();
}

template <typename ComponentType>
void nsSettingsComponentManager<ComponentType>::CollectAllComponents(nsDynamicArray<nsComponentHandle>& out_allComponents, bool bOnlyActive)
{
  for (auto& component : m_Components)
  {
    if (!bOnlyActive || component->IsActive())
    {
      out_allComponents.PushBack(component->GetHandle());
    }
  }
}

template <typename ComponentType>
void nsSettingsComponentManager<ComponentType>::CollectAllComponents(nsDynamicArray<nsComponent*>& out_allComponents, bool bOnlyActive)
{
  for (auto& component : m_Components)
  {
    if (!bOnlyActive || component->IsActive())
    {
      out_allComponents.PushBack(component.Borrow());
    }
  }
}

template <typename ComponentType>
nsComponent* nsSettingsComponentManager<ComponentType>::CreateComponentStorage()
{
  if (!m_Components.IsEmpty())
  {
    nsLog::Warning("A component of type '{0}' is already present in this world. Having more than one is not allowed.", nsGetStaticRTTI<ComponentType>()->GetTypeName());
  }

  m_Components.PushBack(NS_NEW(GetAllocator(), ComponentType));
  return m_Components.PeekBack().Borrow();
}

template <typename ComponentType>
void nsSettingsComponentManager<ComponentType>::DeleteComponentStorage(nsComponent* pComponent, nsComponent*& out_pMovedComponent)
{
  out_pMovedComponent = pComponent;

  for (nsUInt32 i = 0; i < m_Components.GetCount(); ++i)
  {
    if (m_Components[i].Borrow() == pComponent)
    {
      m_Components.RemoveAtAndCopy(i);
      break;
    }
  }
}
