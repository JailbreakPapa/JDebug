#include <RendererVulkan/RendererVulkanPCH.h>

#include <RendererVulkan/Device/DeviceVulkan.h>
#include <RendererVulkan/Resources/QueryVulkan.h>

nsGALQueryVulkan::nsGALQueryVulkan(const nsGALQueryCreationDescription& Description)
  : nsGALQuery(Description)
{
}

nsGALQueryVulkan::~nsGALQueryVulkan() {}

nsResult nsGALQueryVulkan::InitPlatform(nsGALDevice* pDevice)
{
  nsGALDeviceVulkan* pVulkanDevice = static_cast<nsGALDeviceVulkan*>(pDevice);

  if (true)
  {
    return NS_SUCCESS;
  }
  else
  {
    nsLog::Error("Creation of native Vulkan query failed!");
    return NS_FAILURE;
  }
}

nsResult nsGALQueryVulkan::DeInitPlatform(nsGALDevice* pDevice)
{
  // TODO
  return NS_SUCCESS;
}

void nsGALQueryVulkan::SetDebugNamePlatform(const char* szName) const
{
  nsUInt32 uiLength = nsStringUtils::GetStringElementCount(szName);

  // TODO
}
