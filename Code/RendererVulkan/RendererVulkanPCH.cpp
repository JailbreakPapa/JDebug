#include <RendererVulkan/RendererVulkanPCH.h>

NS_STATICLINK_LIBRARY(RendererVulkan)
{
  if (bReturn)
    return;

  NS_STATICLINK_REFERENCE(RendererVulkan_Device_Implementation_DeviceVulkan);
  NS_STATICLINK_REFERENCE(RendererVulkan_Resources_Implementation_FallbackResourcesVulkan);
}
