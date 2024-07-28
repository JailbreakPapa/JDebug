#include <RendererFoundation/RendererFoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Device/ImmutableSamplers.h>

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(RendererFoundation, ImmutableSamplers)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
    "Core"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    nsGALImmutableSamplers::OnEngineStartup();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsGALImmutableSamplers::OnEngineShutdown();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

bool nsGALImmutableSamplers::s_bInitialized = false;
nsGALImmutableSamplers::ImmutableSamplers nsGALImmutableSamplers::s_ImmutableSamplers;
nsHashTable<nsHashedString, nsGALSamplerStateCreationDescription> nsGALImmutableSamplers::s_ImmutableSamplerDesc;

void nsGALImmutableSamplers::OnEngineStartup()
{
  nsGALDevice::s_Events.AddEventHandler(nsMakeDelegate(&nsGALImmutableSamplers::GALDeviceEventHandler));
}

void nsGALImmutableSamplers::OnEngineShutdown()
{
  nsGALDevice::s_Events.RemoveEventHandler(nsMakeDelegate(&nsGALImmutableSamplers::GALDeviceEventHandler));

  NS_ASSERT_DEBUG(s_ImmutableSamplers.IsEmpty(), "nsGALDeviceEvent::BeforeShutdown should have been fired before engine shutdown");
  s_ImmutableSamplers.Clear();
  s_ImmutableSamplerDesc.Clear();
}

nsResult nsGALImmutableSamplers::RegisterImmutableSampler(nsHashedString sSamplerName, const nsGALSamplerStateCreationDescription& desc)
{
  NS_ASSERT_DEBUG(!s_bInitialized, "Registering immutable samplers is only allowed at sub-system startup");
  if (s_ImmutableSamplerDesc.Contains(sSamplerName))
    return NS_FAILURE;

  s_ImmutableSamplerDesc.Insert(sSamplerName, desc);
  return NS_SUCCESS;
}

void nsGALImmutableSamplers::GALDeviceEventHandler(const nsGALDeviceEvent& e)
{
  switch (e.m_Type)
  {
    case nsGALDeviceEvent::AfterInit:
      CreateSamplers(e.m_pDevice);
      break;
    case nsGALDeviceEvent::BeforeShutdown:
      DestroySamplers(e.m_pDevice);
      break;
    default:
      break;
  }
}

void nsGALImmutableSamplers::CreateSamplers(nsGALDevice* pDevice)
{
  NS_ASSERT_DEBUG(s_ImmutableSamplers.IsEmpty(), "Creating more than one GAL device is not supported");
  for (auto it : s_ImmutableSamplerDesc)
  {
    nsGALSamplerStateHandle hSampler = pDevice->CreateSamplerState(it.Value());
    s_ImmutableSamplers.Insert(it.Key(), hSampler);
  }
  s_bInitialized = true;
}

void nsGALImmutableSamplers::DestroySamplers(nsGALDevice* pDevice)
{
  for (auto it : s_ImmutableSamplers)
  {
    pDevice->DestroySamplerState(it.Value());
  }
  s_ImmutableSamplers.Clear();
  s_bInitialized = false;
}

const nsGALImmutableSamplers::ImmutableSamplers& nsGALImmutableSamplers::GetImmutableSamplers()
{
  return s_ImmutableSamplers;
}


NS_STATICLINK_FILE(RendererFoundation, RendererFoundation_Device_Implementation_ImmutableSamplers);
