#pragma once

#include <RendererVulkan/RendererVulkanDLL.h>

#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/RendererFoundationDLL.h>

#include <vulkan/vulkan.hpp>

NS_DEFINE_AS_POD_TYPE(vk::PresentModeKHR);

/// \brief Helper functions to convert and extract Vulkan objects from NS objects.
class NS_RENDERERVULKAN_DLL nsConversionUtilsVulkan
{
public:
  /// \brief Helper function to hash vk enums.
  template <typename T, typename R = typename std::underlying_type<T>::type>
  static R GetUnderlyingValue(T value)
  {
    return static_cast<typename std::underlying_type<T>::type>(value);
  }

  /// \brief Helper function to hash vk flags.
  template <typename T>
  static auto GetUnderlyingFlagsValue(T value)
  {
    return static_cast<typename T::MaskType>(value);
  }

  static vk::SampleCountFlagBits GetSamples(nsEnum<nsGALMSAASampleCount> samples);
  static vk::PresentModeKHR GetPresentMode(nsEnum<nsGALPresentMode> presentMode, const nsDynamicArray<vk::PresentModeKHR>& supportedModes);
  static vk::ImageSubresourceRange GetSubresourceRange(const nsGALTextureCreationDescription& texDesc, const nsGALRenderTargetViewCreationDescription& desc);
  static vk::ImageSubresourceRange GetSubresourceRange(const nsGALTextureCreationDescription& texDesc, const nsGALTextureResourceViewCreationDescription& viewDesc);
  static vk::ImageSubresourceRange GetSubresourceRange(const nsGALTextureCreationDescription& texDesc, const nsGALTextureUnorderedAccessViewCreationDescription& viewDesc);
  static vk::ImageSubresourceRange GetSubresourceRange(const vk::ImageSubresourceLayers& layers);
  static vk::ImageViewType GetImageViewType(nsEnum<nsGALTextureType> texType, bool bIsArray);

  static bool IsDepthFormat(vk::Format format);
  static bool IsStencilFormat(vk::Format format);
  static vk::ImageLayout GetDefaultLayout(vk::Format format);
  static vk::PrimitiveTopology GetPrimitiveTopology(nsEnum<nsGALPrimitiveTopology> topology);
  static vk::ShaderStageFlagBits GetShaderStage(nsGALShaderStage::Enum stage);
  static vk::ShaderStageFlagBits GetShaderStages(nsBitflags<nsGALShaderStageFlags> stages);
  static vk::PipelineStageFlags GetPipelineStage(nsGALShaderStage::Enum stage);
  static vk::PipelineStageFlags GetPipelineStage(vk::ShaderStageFlags flags);
  static vk::PipelineStageFlags GetPipelineStages(nsBitflags<nsGALShaderStageFlags> stages);
  static vk::DescriptorType GetDescriptorType(nsGALShaderResourceType::Enum type);
};

#include <RendererVulkan/Utils/Implementation/ConversionUtilsVulkan.inl.h>
