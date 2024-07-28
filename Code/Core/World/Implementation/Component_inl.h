#include <Foundation/Logging/Log.h>

NS_ALWAYS_INLINE nsComponent::nsComponent() = default;

NS_ALWAYS_INLINE nsComponent::~nsComponent()
{
  m_pMessageDispatchType = nullptr;
  m_pManager = nullptr;
  m_pOwner = nullptr;
  m_InternalId.Invalidate();
}

NS_ALWAYS_INLINE bool nsComponent::IsDynamic() const
{
  return m_ComponentFlags.IsSet(nsObjectFlags::Dynamic);
}

NS_ALWAYS_INLINE bool nsComponent::GetActiveFlag() const
{
  return m_ComponentFlags.IsSet(nsObjectFlags::ActiveFlag);
}

NS_ALWAYS_INLINE bool nsComponent::IsActive() const
{
  return m_ComponentFlags.IsSet(nsObjectFlags::ActiveState);
}

NS_ALWAYS_INLINE bool nsComponent::IsActiveAndInitialized() const
{
  return m_ComponentFlags.AreAllSet(nsObjectFlags::ActiveState | nsObjectFlags::Initialized);
}

NS_ALWAYS_INLINE nsComponentManagerBase* nsComponent::GetOwningManager()
{
  return m_pManager;
}

NS_ALWAYS_INLINE const nsComponentManagerBase* nsComponent::GetOwningManager() const
{
  return m_pManager;
}

NS_ALWAYS_INLINE nsGameObject* nsComponent::GetOwner()
{
  return m_pOwner;
}

NS_ALWAYS_INLINE const nsGameObject* nsComponent::GetOwner() const
{
  return m_pOwner;
}

NS_ALWAYS_INLINE nsComponentHandle nsComponent::GetHandle() const
{
  return nsComponentHandle(m_InternalId);
}

NS_ALWAYS_INLINE nsUInt32 nsComponent::GetUniqueID() const
{
  return m_uiUniqueID;
}

NS_ALWAYS_INLINE void nsComponent::SetUniqueID(nsUInt32 uiUniqueID)
{
  m_uiUniqueID = uiUniqueID;
}

NS_ALWAYS_INLINE bool nsComponent::IsInitialized() const
{
  return m_ComponentFlags.IsSet(nsObjectFlags::Initialized);
}

NS_ALWAYS_INLINE bool nsComponent::IsInitializing() const
{
  return m_ComponentFlags.IsSet(nsObjectFlags::Initializing);
}

NS_ALWAYS_INLINE bool nsComponent::IsSimulationStarted() const
{
  return m_ComponentFlags.IsSet(nsObjectFlags::SimulationStarted);
}

NS_ALWAYS_INLINE bool nsComponent::IsActiveAndSimulating() const
{
  return m_ComponentFlags.AreAllSet(nsObjectFlags::Initialized | nsObjectFlags::ActiveState) &&
         m_ComponentFlags.IsAnySet(nsObjectFlags::SimulationStarting | nsObjectFlags::SimulationStarted);
}
