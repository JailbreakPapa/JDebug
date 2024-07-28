#include <RendererFoundation/RendererFoundationPCH.h>

#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Device/SwapChain.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsGALSwapChain, nsNoBase, 1, nsRTTINoAllocator)
{
}
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsGALWindowSwapChain, nsGALSwapChain, 1, nsRTTINoAllocator)
{
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

nsGALSwapChainCreationDescription CreateSwapChainCreationDescription(const nsRTTI* pType)
{
  nsGALSwapChainCreationDescription desc;
  desc.m_pSwapChainType = pType;
  return desc;
}

nsGALSwapChain::nsGALSwapChain(const nsRTTI* pSwapChainType)
  : nsGALObject(CreateSwapChainCreationDescription(pSwapChainType))
{
}

nsGALSwapChain::~nsGALSwapChain() = default;

//////////////////////////////////////////////////////////////////////////

nsGALWindowSwapChain::Functor nsGALWindowSwapChain::s_Factory;


nsGALWindowSwapChain::nsGALWindowSwapChain(const nsGALWindowSwapChainCreationDescription& Description)
  : nsGALSwapChain(nsGetStaticRTTI<nsGALWindowSwapChain>())
  , m_WindowDesc(Description)
{
}

void nsGALWindowSwapChain::SetFactoryMethod(Functor factory)
{
  s_Factory = factory;
}

nsGALSwapChainHandle nsGALWindowSwapChain::Create(const nsGALWindowSwapChainCreationDescription& desc)
{
  NS_ASSERT_DEV(s_Factory.IsValid(), "No factory method assigned for nsGALWindowSwapChain.");
  return s_Factory(desc);
}

NS_STATICLINK_FILE(RendererFoundation, RendererFoundation_Device_Implementation_SwapChain);
