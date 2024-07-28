#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptAttributes.h>
#include <Core/World/World.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsComponent, 1, nsRTTINoAllocator)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Active", GetActiveFlag, SetActiveFlag)->AddAttributes(new nsDefaultValueAttribute(true)),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_SCRIPT_FUNCTION_PROPERTY(IsActive),
    NS_SCRIPT_FUNCTION_PROPERTY(IsActiveAndInitialized),
    NS_SCRIPT_FUNCTION_PROPERTY(IsActiveAndSimulating),
    NS_SCRIPT_FUNCTION_PROPERTY(Reflection_GetOwner),
    NS_SCRIPT_FUNCTION_PROPERTY(Reflection_GetWorld),
    NS_SCRIPT_FUNCTION_PROPERTY(GetUniqueID),
    NS_SCRIPT_FUNCTION_PROPERTY(Initialize)->AddAttributes(new nsScriptBaseClassFunctionAttribute(nsComponent_ScriptBaseClassFunctions::Initialize)),
    NS_SCRIPT_FUNCTION_PROPERTY(Deinitialize)->AddAttributes(new nsScriptBaseClassFunctionAttribute(nsComponent_ScriptBaseClassFunctions::Deinitialize)),
    NS_SCRIPT_FUNCTION_PROPERTY(OnActivated)->AddAttributes(new nsScriptBaseClassFunctionAttribute(nsComponent_ScriptBaseClassFunctions::OnActivated)),
    NS_SCRIPT_FUNCTION_PROPERTY(OnDeactivated)->AddAttributes(new nsScriptBaseClassFunctionAttribute(nsComponent_ScriptBaseClassFunctions::OnDeactivated)),
    NS_SCRIPT_FUNCTION_PROPERTY(OnSimulationStarted)->AddAttributes(new nsScriptBaseClassFunctionAttribute(nsComponent_ScriptBaseClassFunctions::OnSimulationStarted)),
    NS_SCRIPT_FUNCTION_PROPERTY(Reflection_Update, In, "DeltaTime")->AddAttributes(new nsScriptBaseClassFunctionAttribute(nsComponent_ScriptBaseClassFunctions::Update)),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsComponent::SetActiveFlag(bool bEnabled)
{
  if (m_ComponentFlags.IsSet(nsObjectFlags::ActiveFlag) != bEnabled)
  {
    m_ComponentFlags.AddOrRemove(nsObjectFlags::ActiveFlag, bEnabled);

    UpdateActiveState(GetOwner() == nullptr ? true : GetOwner()->IsActive());
  }
}

nsWorld* nsComponent::GetWorld()
{
  return m_pManager->GetWorld();
}

const nsWorld* nsComponent::GetWorld() const
{
  return m_pManager->GetWorld();
}

void nsComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
}

void nsComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
}

void nsComponent::EnsureInitialized()
{
  NS_ASSERT_DEV(m_pOwner != nullptr, "Owner must not be null");

  if (IsInitializing())
  {
    nsLog::Error("Recursive initialize call is ignored.");
    return;
  }

  if (!IsInitialized())
  {
    m_pMessageDispatchType = GetDynamicRTTI();

    m_ComponentFlags.Add(nsObjectFlags::Initializing);

    Initialize();

    m_ComponentFlags.Remove(nsObjectFlags::Initializing);
    m_ComponentFlags.Add(nsObjectFlags::Initialized);
  }
}

void nsComponent::EnsureSimulationStarted()
{
  NS_ASSERT_DEV(IsActiveAndInitialized(), "Must not be called on uninitialized or inactive components.");
  NS_ASSERT_DEV(GetWorld()->GetWorldSimulationEnabled(), "Must not be called when the world is not simulated.");

  if (m_ComponentFlags.IsSet(nsObjectFlags::SimulationStarting))
  {
    nsLog::Error("Recursive simulation started call is ignored.");
    return;
  }

  if (!IsSimulationStarted())
  {
    m_ComponentFlags.Add(nsObjectFlags::SimulationStarting);

    OnSimulationStarted();

    m_ComponentFlags.Remove(nsObjectFlags::SimulationStarting);
    m_ComponentFlags.Add(nsObjectFlags::SimulationStarted);
  }
}

void nsComponent::PostMessage(const nsMessage& msg, nsTime delay, nsObjectMsgQueueType::Enum queueType) const
{
  GetWorld()->PostMessage(GetHandle(), msg, delay, queueType);
}

bool nsComponent::HandlesMessage(const nsMessage& msg) const
{
  return m_pMessageDispatchType->CanHandleMessage(msg.GetId());
}

void nsComponent::SetUserFlag(nsUInt8 uiFlagIndex, bool bSet)
{
  NS_ASSERT_DEBUG(uiFlagIndex < 8, "Flag index {0} is out of the valid range [0 - 7]", uiFlagIndex);

  m_ComponentFlags.AddOrRemove(static_cast<nsObjectFlags::Enum>(nsObjectFlags::UserFlag0 << uiFlagIndex), bSet);
}

bool nsComponent::GetUserFlag(nsUInt8 uiFlagIndex) const
{
  NS_ASSERT_DEBUG(uiFlagIndex < 8, "Flag index {0} is out of the valid range [0 - 7]", uiFlagIndex);

  return m_ComponentFlags.IsSet(static_cast<nsObjectFlags::Enum>(nsObjectFlags::UserFlag0 << uiFlagIndex));
}

void nsComponent::Initialize() {}

void nsComponent::Deinitialize()
{
  NS_ASSERT_DEV(m_pOwner != nullptr, "Owner must still be valid");

  SetActiveFlag(false);
}

void nsComponent::OnActivated() {}

void nsComponent::OnDeactivated() {}

void nsComponent::OnSimulationStarted() {}

void nsComponent::EnableUnhandledMessageHandler(bool enable)
{
  m_ComponentFlags.AddOrRemove(nsObjectFlags::UnhandledMessageHandler, enable);
}

bool nsComponent::OnUnhandledMessage(nsMessage& msg, bool bWasPostedMsg)
{
  return false;
}

bool nsComponent::OnUnhandledMessage(nsMessage& msg, bool bWasPostedMsg) const
{
  return false;
}

void nsComponent::UpdateActiveState(bool bOwnerActive)
{
  const bool bSelfActive = bOwnerActive && m_ComponentFlags.IsSet(nsObjectFlags::ActiveFlag);

  if (m_ComponentFlags.IsSet(nsObjectFlags::ActiveState) != bSelfActive)
  {
    m_ComponentFlags.AddOrRemove(nsObjectFlags::ActiveState, bSelfActive);

    if (IsInitialized())
    {
      if (bSelfActive)
      {
        // Don't call OnActivated & EnsureSimulationStarted here since there might be other components
        // that are needed in the OnSimulation callback but are activated right after this component.
        // Instead add the component to the initialization batch again.
        // There initialization will be skipped since the component is already initialized.
        GetWorld()->AddComponentToInitialize(GetHandle());
      }
      else
      {
        OnDeactivated();

        m_ComponentFlags.Remove(nsObjectFlags::SimulationStarted);
      }
    }
  }
}

nsGameObject* nsComponent::Reflection_GetOwner() const
{
  return m_pOwner;
}

nsWorld* nsComponent::Reflection_GetWorld() const
{
  return m_pManager->GetWorld();
}

void nsComponent::Reflection_Update(nsTime deltaTime)
{
  // This is just a dummy function for the scripting reflection
}

bool nsComponent::SendMessageInternal(nsMessage& msg, bool bWasPostedMsg)
{
  if (!IsActiveAndInitialized() && !IsInitializing())
  {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    if (msg.GetDebugMessageRouting())
      nsLog::Warning("Discarded message with ID {0} because component of type '{1}' is neither initialized nor active at the moment", msg.GetId(),
        GetDynamicRTTI()->GetTypeName());
#endif

    return false;
  }

  if (m_pMessageDispatchType->DispatchMessage(this, msg))
    return true;

  if (m_ComponentFlags.IsSet(nsObjectFlags::UnhandledMessageHandler) && OnUnhandledMessage(msg, bWasPostedMsg))
    return true;

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  if (msg.GetDebugMessageRouting())
    nsLog::Warning("Component type '{0}' does not have a message handler for messages of type {1}", GetDynamicRTTI()->GetTypeName(), msg.GetId());
#endif

  return false;
}

bool nsComponent::SendMessageInternal(nsMessage& msg, bool bWasPostedMsg) const
{
  if (!IsActiveAndInitialized() && !IsInitializing())
  {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    if (msg.GetDebugMessageRouting())
      nsLog::Warning("Discarded message with ID {0} because component of type '{1}' is neither initialized nor active at the moment", msg.GetId(),
        GetDynamicRTTI()->GetTypeName());
#endif

    return false;
  }

  if (m_pMessageDispatchType->DispatchMessage(this, msg))
    return true;

  if (m_ComponentFlags.IsSet(nsObjectFlags::UnhandledMessageHandler) && OnUnhandledMessage(msg, bWasPostedMsg))
    return true;

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  if (msg.GetDebugMessageRouting())
    nsLog::Warning(
      "(const) Component type '{0}' does not have a CONST message handler for messages of type {1}", GetDynamicRTTI()->GetTypeName(), msg.GetId());
#endif

  return false;
}


NS_STATICLINK_FILE(Core, Core_World_Implementation_Component);
