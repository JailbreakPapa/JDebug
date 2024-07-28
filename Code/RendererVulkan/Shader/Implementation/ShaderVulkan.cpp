#include <RendererVulkan/RendererVulkanPCH.h>

#include <Foundation/Algorithm/HashStream.h>
#include <RendererFoundation/Device/ImmutableSamplers.h>
#include <RendererVulkan/Cache/ResourceCacheVulkan.h>
#include <RendererVulkan/Device/DeviceVulkan.h>
#include <RendererVulkan/Shader/ShaderVulkan.h>
#include <RendererVulkan/State/StateVulkan.h>
#include <RendererVulkan/Utils/ConversionUtilsVulkan.h>

void nsGALShaderVulkan::DescriptorSetLayoutDesc::ComputeHash()
{
  nsHashStreamWriter32 writer;
  const nsUInt32 uiSize = m_bindings.GetCount();
  for (nsUInt32 i = 0; i < uiSize; i++)
  {
    const auto& binding = m_bindings[i];
    writer << binding.binding;
    writer << nsConversionUtilsVulkan::GetUnderlyingValue(binding.descriptorType);
    writer << binding.descriptorCount;
    writer << nsConversionUtilsVulkan::GetUnderlyingFlagsValue(binding.stageFlags);
    writer << binding.pImmutableSamplers;
  }
  m_uiHash = writer.GetHashValue();
}

nsGALShaderVulkan::nsGALShaderVulkan(const nsGALShaderCreationDescription& Description)
  : nsGALShader(Description)
{
}

nsGALShaderVulkan::~nsGALShaderVulkan() {}

void nsGALShaderVulkan::SetDebugName(const char* szName) const
{
  nsGALDeviceVulkan* pVulkanDevice = static_cast<nsGALDeviceVulkan*>(nsGALDevice::GetDefaultDevice());
  for (nsUInt32 i = 0; i < nsGALShaderStage::ENUM_COUNT; i++)
  {
    pVulkanDevice->SetDebugName(szName, m_Shaders[i]);
  }
}

nsResult nsGALShaderVulkan::InitPlatform(nsGALDevice* pDevice)
{
  NS_SUCCEED_OR_RETURN(CreateBindingMapping(false));

  nsGALDeviceVulkan* pVulkanDevice = static_cast<nsGALDeviceVulkan*>(pDevice);

  m_SetBindings.Clear();

  for (const nsShaderResourceBinding& binding : GetBindingMapping())
  {
    if (binding.m_ResourceType == nsGALShaderResourceType::PushConstants)
      continue;

    nsUInt32 iMaxSets = nsMath::Max((nsUInt32)m_SetBindings.GetCount(), static_cast<nsUInt32>(binding.m_iSet + 1));
    m_SetBindings.SetCount(iMaxSets);
    m_SetBindings[binding.m_iSet].PushBack(binding);
  }

  const nsGALImmutableSamplers::ImmutableSamplers& immutableSamplers = nsGALImmutableSamplers::GetImmutableSamplers();
  auto GetImmutableSampler = [&](const nsHashedString& sName) -> const vk::Sampler*
  {
    if (const nsGALSamplerStateHandle* hSampler = immutableSamplers.GetValue(sName))
    {
      const auto* pSampler = static_cast<const nsGALSamplerStateVulkan*>(pVulkanDevice->GetSamplerState(*hSampler));
      return &pSampler->GetImageInfo().sampler;
    }
    return nullptr;
  };

  // If no descriptor set is needed, we still need to create an empty one to fulfil the Vulkan spec :-/
  if (m_SetBindings.IsEmpty())
  {
    m_SetBindings.SetCount(1);
  }

  // Sort mappings and build descriptor set layout
  nsHybridArray<DescriptorSetLayoutDesc, 4> descriptorSetLayoutDesc;
  descriptorSetLayoutDesc.SetCount(m_SetBindings.GetCount());
  m_descriptorSetLayout.SetCount(m_SetBindings.GetCount());
  for (nsUInt32 iSet = 0; iSet < m_SetBindings.GetCount(); ++iSet)
  {
    m_SetBindings[iSet].Sort([](const nsShaderResourceBinding& lhs, const nsShaderResourceBinding& rhs)
      { return lhs.m_iSlot < rhs.m_iSlot; });

    // Build Vulkan descriptor set layout
    for (nsUInt32 i = 0; i < m_SetBindings[iSet].GetCount(); i++)
    {
      const nsShaderResourceBinding& nsBinding = m_SetBindings[iSet][i];
      vk::DescriptorSetLayoutBinding& binding = descriptorSetLayoutDesc[nsBinding.m_iSet].m_bindings.ExpandAndGetRef();

      binding.binding = nsBinding.m_iSlot;
      binding.descriptorType = nsConversionUtilsVulkan::GetDescriptorType(nsBinding.m_ResourceType);
      binding.descriptorCount = nsBinding.m_uiArraySize;
      binding.stageFlags = nsConversionUtilsVulkan::GetShaderStages(nsBinding.m_Stages);
      binding.pImmutableSamplers = nsBinding.m_ResourceType == nsGALShaderResourceType::Sampler ? GetImmutableSampler(nsBinding.m_sName) : nullptr;
    }

    descriptorSetLayoutDesc[iSet].ComputeHash();
    m_descriptorSetLayout[iSet] = nsResourceCacheVulkan::RequestDescriptorSetLayout(descriptorSetLayoutDesc[iSet]);
  }

  // Remove immutable samplers and push constants from binding info
  {
    for (nsUInt32 uiSet = 0; uiSet < m_SetBindings.GetCount(); ++uiSet)
    {
      for (nsInt32 iIndex = (nsInt32)m_SetBindings[uiSet].GetCount() - 1; iIndex >= 0; --iIndex)
      {
        const bool bIsImmutableSample = m_SetBindings[uiSet][iIndex].m_ResourceType == nsGALShaderResourceType::Sampler && immutableSamplers.Contains(m_SetBindings[uiSet][iIndex].m_sName);

        if (bIsImmutableSample)
        {
          m_SetBindings[uiSet].RemoveAtAndCopy(iIndex);
        }
      }
    }
    for (nsInt32 iIndex = (nsInt32)m_BindingMapping.GetCount() - 1; iIndex >= 0; --iIndex)
    {
      const bool bIsImmutableSample = m_BindingMapping[iIndex].m_ResourceType == nsGALShaderResourceType::Sampler && immutableSamplers.Contains(m_BindingMapping[iIndex].m_sName);
      const bool bIsPushConstant = m_BindingMapping[iIndex].m_ResourceType == nsGALShaderResourceType::PushConstants;

      if (bIsPushConstant)
      {
        const auto& pushConstant = m_BindingMapping[iIndex];
        m_pushConstants.size = pushConstant.m_pLayout->m_uiTotalSize;
        m_pushConstants.offset = 0;
        m_pushConstants.stageFlags = nsConversionUtilsVulkan::GetShaderStages(pushConstant.m_Stages);
      }

      if (bIsImmutableSample || bIsPushConstant)
      {
        m_BindingMapping.RemoveAtAndCopy(iIndex);
      }
    }
  }

  // Build shaders
  vk::ShaderModuleCreateInfo createInfo;
  for (nsUInt32 i = 0; i < nsGALShaderStage::ENUM_COUNT; i++)
  {
    if (m_Description.HasByteCodeForStage((nsGALShaderStage::Enum)i))
    {
      createInfo.codeSize = m_Description.m_ByteCodes[i]->m_ByteCode.GetCount();
      NS_ASSERT_DEV(createInfo.codeSize % 4 == 0, "Spirv shader code should be a multiple of 4.");
      createInfo.pCode = reinterpret_cast<const nsUInt32*>(m_Description.m_ByteCodes[i]->m_ByteCode.GetData());
      VK_SUCCEED_OR_RETURN_NS_FAILURE(pVulkanDevice->GetVulkanDevice().createShaderModule(&createInfo, nullptr, &m_Shaders[i]));
    }
  }

  return NS_SUCCESS;
}

nsResult nsGALShaderVulkan::DeInitPlatform(nsGALDevice* pDevice)
{
  DestroyBindingMapping();

  // Right now, we do not destroy descriptor set layouts as they are shared among many shaders.
  m_descriptorSetLayout.Clear();
  m_SetBindings.Clear();

  auto* pVulkanDevice = static_cast<nsGALDeviceVulkan*>(pDevice);
  for (auto& m_Shader : m_Shaders)
  {
    pVulkanDevice->DeleteLater(m_Shader);
  }
  return NS_SUCCESS;
}
