#pragma once

#include <RendererVulkan/RendererVulkanDLL.h>

#include <vulkan/vulkan.hpp>

NS_DEFINE_AS_POD_TYPE(vk::DescriptorType);

template <>
struct nsHashHelper<vk::DescriptorType>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(vk::DescriptorType value) { return nsHashHelper<nsUInt32>::Hash(nsUInt32(value)); }
  NS_ALWAYS_INLINE static bool Equal(vk::DescriptorType a, vk::DescriptorType b) { return a == b; }
};

class NS_RENDERERVULKAN_DLL nsDescriptorSetPoolVulkan
{
public:
  static void Initialize(vk::Device device);
  static void DeInitialize();
  static nsHashTable<vk::DescriptorType, float>& AccessDescriptorPoolWeights();

  static vk::DescriptorSet CreateDescriptorSet(vk::DescriptorSetLayout layout);
  static void UpdateDescriptorSet(vk::DescriptorSet descriptorSet, nsArrayPtr<vk::WriteDescriptorSet> update);
  static void ReclaimPool(vk::DescriptorPool& descriptorPool);

private:
  static constexpr nsUInt32 s_uiPoolBaseSize = 1024;

  static vk::DescriptorPool GetNewPool();

  static vk::DescriptorPool s_currentPool;
  static nsHybridArray<vk::DescriptorPool, 4> s_freePools;
  static vk::Device s_device;
  static nsHashTable<vk::DescriptorType, float> s_descriptorWeights;
};
