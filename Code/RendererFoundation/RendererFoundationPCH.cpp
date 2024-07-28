#include <RendererFoundation/RendererFoundationPCH.h>

NS_STATICLINK_LIBRARY(RendererFoundation)
{
  if (bReturn)
    return;

  NS_STATICLINK_REFERENCE(RendererFoundation_Device_Implementation_ImmutableSamplers);
  NS_STATICLINK_REFERENCE(RendererFoundation_Device_Implementation_SharedTextureSwapChain);
  NS_STATICLINK_REFERENCE(RendererFoundation_Device_Implementation_SwapChain);
  NS_STATICLINK_REFERENCE(RendererFoundation_Profiling_Implementation_Profiling);
  NS_STATICLINK_REFERENCE(RendererFoundation_RendererReflection);
}
