NS_ALWAYS_INLINE vk::Device nsGALDeviceVulkan::GetVulkanDevice() const
{
  return m_device;
}

NS_ALWAYS_INLINE const nsGALDeviceVulkan::Queue& nsGALDeviceVulkan::GetGraphicsQueue() const
{
  return m_graphicsQueue;
}

NS_ALWAYS_INLINE const nsGALDeviceVulkan::Queue& nsGALDeviceVulkan::GetTransferQueue() const
{
  return m_transferQueue;
}

NS_ALWAYS_INLINE vk::PhysicalDevice nsGALDeviceVulkan::GetVulkanPhysicalDevice() const
{
  return m_physicalDevice;
}

NS_ALWAYS_INLINE vk::Instance nsGALDeviceVulkan::GetVulkanInstance() const
{
  return m_instance;
}


NS_ALWAYS_INLINE const nsGALFormatLookupTableVulkan& nsGALDeviceVulkan::GetFormatLookupTable() const
{
  return m_FormatLookupTable;
}

/*
inline ID3D11Query* nsGALDeviceVulkan::GetTimestamp(nsGALTimestampHandle hTimestamp)
{
  if (hTimestamp.m_uiIndex < m_Timestamps.GetCount())
  {
    return m_Timestamps[static_cast<nsUInt32>(hTimestamp.m_uiIndex)];
  }

  return nullptr;
}

*/
