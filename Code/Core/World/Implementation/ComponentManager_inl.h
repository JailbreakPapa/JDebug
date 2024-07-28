
NS_FORCE_INLINE bool nsComponentManagerBase::IsValidComponent(const nsComponentHandle& hComponent) const
{
  return m_Components.Contains(hComponent);
}

NS_FORCE_INLINE bool nsComponentManagerBase::TryGetComponent(const nsComponentHandle& hComponent, nsComponent*& out_pComponent)
{
  return m_Components.TryGetValue(hComponent, out_pComponent);
}

NS_FORCE_INLINE bool nsComponentManagerBase::TryGetComponent(const nsComponentHandle& hComponent, const nsComponent*& out_pComponent) const
{
  nsComponent* pComponent = nullptr;
  bool res = m_Components.TryGetValue(hComponent, pComponent);
  out_pComponent = pComponent;
  return res;
}

NS_ALWAYS_INLINE nsUInt32 nsComponentManagerBase::GetComponentCount() const
{
  return static_cast<nsUInt32>(m_Components.GetCount());
}

template <typename ComponentType>
NS_ALWAYS_INLINE nsTypedComponentHandle<ComponentType> nsComponentManagerBase::CreateComponent(nsGameObject* pOwnerObject, ComponentType*& out_pComponent)
{
  nsComponent* pComponent = nullptr;
  nsComponentHandle hComponent = CreateComponentNoInit(pOwnerObject, pComponent);

  if (pComponent != nullptr)
  {
    InitializeComponent(pComponent);
  }

  out_pComponent = nsStaticCast<ComponentType*>(pComponent);
  return nsTypedComponentHandle<ComponentType>(hComponent);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T, nsBlockStorageType::Enum StorageType>
nsComponentManager<T, StorageType>::nsComponentManager(nsWorld* pWorld)
  : nsComponentManagerBase(pWorld)
  , m_ComponentStorage(GetBlockAllocator(), GetAllocator())
{
  NS_CHECK_AT_COMPILETIME_MSG(NS_IS_DERIVED_FROM_STATIC(nsComponent, ComponentType), "Not a valid component type");
}

template <typename T, nsBlockStorageType::Enum StorageType>
nsComponentManager<T, StorageType>::~nsComponentManager() = default;

template <typename T, nsBlockStorageType::Enum StorageType>
NS_FORCE_INLINE bool nsComponentManager<T, StorageType>::TryGetComponent(const nsComponentHandle& hComponent, ComponentType*& out_pComponent)
{
  NS_ASSERT_DEV(ComponentType::TypeId() == hComponent.GetInternalID().m_TypeId,
    "The given component handle is not of the expected type. Expected type id {0}, got type id {1}", ComponentType::TypeId(),
    hComponent.GetInternalID().m_TypeId);
  NS_ASSERT_DEV(hComponent.GetInternalID().m_WorldIndex == GetWorldIndex(),
    "Component does not belong to this world. Expected world id {0} got id {1}", GetWorldIndex(), hComponent.GetInternalID().m_WorldIndex);

  nsComponent* pComponent = nullptr;
  bool bResult = nsComponentManagerBase::TryGetComponent(hComponent, pComponent);
  out_pComponent = static_cast<ComponentType*>(pComponent);
  return bResult;
}

template <typename T, nsBlockStorageType::Enum StorageType>
NS_FORCE_INLINE bool nsComponentManager<T, StorageType>::TryGetComponent(
  const nsComponentHandle& hComponent, const ComponentType*& out_pComponent) const
{
  NS_ASSERT_DEV(ComponentType::TypeId() == hComponent.GetInternalID().m_TypeId,
    "The given component handle is not of the expected type. Expected type id {0}, got type id {1}", ComponentType::TypeId(),
    hComponent.GetInternalID().m_TypeId);
  NS_ASSERT_DEV(hComponent.GetInternalID().m_WorldIndex == GetWorldIndex(),
    "Component does not belong to this world. Expected world id {0} got id {1}", GetWorldIndex(), hComponent.GetInternalID().m_WorldIndex);

  const nsComponent* pComponent = nullptr;
  bool bResult = nsComponentManagerBase::TryGetComponent(hComponent, pComponent);
  out_pComponent = static_cast<const ComponentType*>(pComponent);
  return bResult;
}

template <typename T, nsBlockStorageType::Enum StorageType>
NS_ALWAYS_INLINE typename nsBlockStorage<T, nsInternal::DEFAULT_BLOCK_SIZE, StorageType>::Iterator nsComponentManager<T, StorageType>::GetComponents(nsUInt32 uiStartIndex /*= 0*/)
{
  return m_ComponentStorage.GetIterator(uiStartIndex);
}

template <typename T, nsBlockStorageType::Enum StorageType>
NS_ALWAYS_INLINE typename nsBlockStorage<T, nsInternal::DEFAULT_BLOCK_SIZE, StorageType>::ConstIterator
nsComponentManager<T, StorageType>::GetComponents(nsUInt32 uiStartIndex /*= 0*/) const
{
  return m_ComponentStorage.GetIterator(uiStartIndex);
}

// static
template <typename T, nsBlockStorageType::Enum StorageType>
NS_ALWAYS_INLINE nsWorldModuleTypeId nsComponentManager<T, StorageType>::TypeId()
{
  return T::TypeId();
}

template <typename T, nsBlockStorageType::Enum StorageType>
void nsComponentManager<T, StorageType>::CollectAllComponents(nsDynamicArray<nsComponentHandle>& out_allComponents, bool bOnlyActive)
{
  out_allComponents.Reserve(out_allComponents.GetCount() + m_ComponentStorage.GetCount());

  for (auto it = GetComponents(); it.IsValid(); it.Next())
  {
    if (!bOnlyActive || it->IsActive())
    {
      out_allComponents.PushBack(it->GetHandle());
    }
  }
}

template <typename T, nsBlockStorageType::Enum StorageType>
void nsComponentManager<T, StorageType>::CollectAllComponents(nsDynamicArray<nsComponent*>& out_allComponents, bool bOnlyActive)
{
  out_allComponents.Reserve(out_allComponents.GetCount() + m_ComponentStorage.GetCount());

  for (auto it = GetComponents(); it.IsValid(); it.Next())
  {
    if (!bOnlyActive || it->IsActive())
    {
      out_allComponents.PushBack(it);
    }
  }
}

template <typename T, nsBlockStorageType::Enum StorageType>
NS_ALWAYS_INLINE nsComponent* nsComponentManager<T, StorageType>::CreateComponentStorage()
{
  return m_ComponentStorage.Create();
}

template <typename T, nsBlockStorageType::Enum StorageType>
NS_FORCE_INLINE void nsComponentManager<T, StorageType>::DeleteComponentStorage(nsComponent* pComponent, nsComponent*& out_pMovedComponent)
{
  T* pMovedComponent = nullptr;
  m_ComponentStorage.Delete(static_cast<T*>(pComponent), pMovedComponent);
  out_pMovedComponent = pMovedComponent;
}

template <typename T, nsBlockStorageType::Enum StorageType>
NS_FORCE_INLINE void nsComponentManager<T, StorageType>::RegisterUpdateFunction(UpdateFunctionDesc& desc)
{
  // round up to multiple of data block capacity so tasks only have to deal with complete data blocks
  if (desc.m_uiGranularity != 0)
    desc.m_uiGranularity = static_cast<nsUInt16>(
      nsMath::RoundUp(static_cast<nsInt32>(desc.m_uiGranularity), nsDataBlock<ComponentType, nsInternal::DEFAULT_BLOCK_SIZE>::CAPACITY));

  nsComponentManagerBase::RegisterUpdateFunction(desc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename ComponentType, nsComponentUpdateType::Enum UpdateType, nsBlockStorageType::Enum StorageType>
nsComponentManagerSimple<ComponentType, UpdateType, StorageType>::nsComponentManagerSimple(nsWorld* pWorld)
  : nsComponentManager<ComponentType, StorageType>(pWorld)
{
}

template <typename ComponentType, nsComponentUpdateType::Enum UpdateType, nsBlockStorageType::Enum StorageType>
void nsComponentManagerSimple<ComponentType, UpdateType, StorageType>::Initialize()
{
  using OwnType = nsComponentManagerSimple<ComponentType, UpdateType, StorageType>;

  nsStringBuilder functionName;
  SimpleUpdateName(functionName);

  auto desc = nsWorldModule::UpdateFunctionDesc(nsWorldModule::UpdateFunction(&OwnType::SimpleUpdate, this), functionName);
  desc.m_bOnlyUpdateWhenSimulating = (UpdateType == nsComponentUpdateType::WhenSimulating);

  this->RegisterUpdateFunction(desc);
}

template <typename ComponentType, nsComponentUpdateType::Enum UpdateType, nsBlockStorageType::Enum StorageType>
void nsComponentManagerSimple<ComponentType, UpdateType, StorageType>::SimpleUpdate(const nsWorldModule::UpdateContext& context)
{
  for (auto it = this->m_ComponentStorage.GetIterator(context.m_uiFirstComponentIndex, context.m_uiComponentCount); it.IsValid(); ++it)
  {
    ComponentType* pComponent = it;
    if (pComponent->IsActiveAndInitialized())
    {
      pComponent->Update();
    }
  }
}

// static
template <typename ComponentType, nsComponentUpdateType::Enum UpdateType, nsBlockStorageType::Enum StorageType>
void nsComponentManagerSimple<ComponentType, UpdateType, StorageType>::SimpleUpdateName(nsStringBuilder& out_sName)
{
  nsStringView sName(NS_SOURCE_FUNCTION);
  const char* szEnd = sName.FindSubString(",");

  if (szEnd != nullptr && sName.StartsWith("nsComponentManagerSimple<class "))
  {
    nsStringView sChoppedName(sName.GetStartPointer() + nsStringUtils::GetStringElementCount("nsComponentManagerSimple<class "), szEnd);

    NS_ASSERT_DEV(!sChoppedName.IsEmpty(), "Chopped name is empty: '{0}'", sName);

    out_sName = sChoppedName;
    out_sName.Append("::SimpleUpdate");
  }
  else
  {
    out_sName = sName;
  }
}
