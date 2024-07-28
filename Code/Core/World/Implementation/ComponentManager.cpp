#include <Core/CorePCH.h>

#include <Core/World/World.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsComponentManagerBase, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsComponentManagerBase::nsComponentManagerBase(nsWorld* pWorld)
  : nsWorldModule(pWorld)
  , m_Components(pWorld->GetAllocator())
{
}

nsComponentManagerBase::~nsComponentManagerBase() = default;

nsComponentHandle nsComponentManagerBase::CreateComponent(nsGameObject* pOwnerObject)
{
  nsComponent* pDummy;
  return CreateComponent(pOwnerObject, pDummy);
}

void nsComponentManagerBase::DeleteComponent(const nsComponentHandle& hComponent)
{
  nsComponent* pComponent = nullptr;
  if (!m_Components.TryGetValue(hComponent, pComponent))
    return;

  DeleteComponent(pComponent);
}

void nsComponentManagerBase::DeleteComponent(nsComponent* pComponent)
{
  if (pComponent == nullptr)
    return;

  DeinitializeComponent(pComponent);

  m_Components.Remove(pComponent->m_InternalId);

  pComponent->m_InternalId.Invalidate();
  pComponent->m_ComponentFlags.Remove(nsObjectFlags::ActiveFlag | nsObjectFlags::ActiveState);

  GetWorld()->m_Data.m_DeadComponents.Insert(pComponent);
}

void nsComponentManagerBase::Deinitialize()
{
  for (auto it = m_Components.GetIterator(); it.IsValid(); ++it)
  {
    DeinitializeComponent(it.Value());
  }

  SUPER::Deinitialize();
}

nsComponentHandle nsComponentManagerBase::CreateComponentNoInit(nsGameObject* pOwnerObject, nsComponent*& out_pComponent)
{
  NS_ASSERT_DEV(m_Components.GetCount() < nsWorld::GetMaxNumComponentsPerType(), "Max number of components per type reached: {}",
    nsWorld::GetMaxNumComponentsPerType());

  nsComponent* pComponent = CreateComponentStorage();
  if (pComponent == nullptr)
  {
    return nsComponentHandle();
  }

  nsComponentId newId = m_Components.Insert(pComponent);
  newId.m_WorldIndex = GetWorldIndex();
  newId.m_TypeId = pComponent->GetTypeId();

  pComponent->m_pManager = this;
  pComponent->m_InternalId = newId;
  pComponent->m_ComponentFlags.AddOrRemove(nsObjectFlags::Dynamic, pComponent->GetMode() == nsComponentMode::Dynamic);

  // In Editor we add components via reflection so it is fine to have a nullptr here.
  // We check for a valid owner before the Initialize() callback.
  if (pOwnerObject != nullptr)
  {
    // AddComponent will update the active state internally
    pOwnerObject->AddComponent(pComponent);
  }
  else
  {
    pComponent->UpdateActiveState(true);
  }

  out_pComponent = pComponent;
  return pComponent->GetHandle();
}

void nsComponentManagerBase::InitializeComponent(nsComponent* pComponent)
{
  GetWorld()->AddComponentToInitialize(pComponent->GetHandle());
}

void nsComponentManagerBase::DeinitializeComponent(nsComponent* pComponent)
{
  if (pComponent->IsInitialized())
  {
    pComponent->Deinitialize();
    pComponent->m_ComponentFlags.Remove(nsObjectFlags::Initialized);
  }

  if (nsGameObject* pOwner = pComponent->GetOwner())
  {
    pOwner->RemoveComponent(pComponent);
  }
}

void nsComponentManagerBase::PatchIdTable(nsComponent* pComponent)
{
  nsComponentId id = pComponent->m_InternalId;
  if (id.m_InstanceIndex != nsComponentId::INVALID_INSTANCE_INDEX)
    m_Components[id] = pComponent;
}

NS_STATICLINK_FILE(Core, Core_World_Implementation_ComponentManager);
