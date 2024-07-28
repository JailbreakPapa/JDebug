#include <RendererVulkan/RendererVulkanPCH.h>

#include <RendererFoundation/Shader/Shader.h>
#include <RendererVulkan/Device/DeviceVulkan.h>
#include <RendererVulkan/Shader/ShaderVulkan.h>
#include <RendererVulkan/Shader/VertexDeclarationVulkan.h>

nsGALVertexDeclarationVulkan::nsGALVertexDeclarationVulkan(const nsGALVertexDeclarationCreationDescription& Description)
  : nsGALVertexDeclaration(Description)
{
}

nsGALVertexDeclarationVulkan::~nsGALVertexDeclarationVulkan() = default;

nsResult nsGALVertexDeclarationVulkan::InitPlatform(nsGALDevice* pDevice)
{
  nsGALDeviceVulkan* pVulkanDevice = static_cast<nsGALDeviceVulkan*>(pDevice);

  const nsGALShaderVulkan* pShader = static_cast<const nsGALShaderVulkan*>(pDevice->GetShader(m_Description.m_hShader));

  if (pShader == nullptr || !pShader->GetDescription().HasByteCodeForStage(nsGALShaderStage::VertexShader))
  {
    return NS_FAILURE;
  }

  nsHybridArray<nsShaderVertexInputAttribute, 8> vias(pShader->GetVertexInputAttributes());
  auto FindLocation = [&](nsGALVertexAttributeSemantic::Enum sematic, nsGALResourceFormat::Enum format) -> nsUInt32
  {
    for (nsUInt32 i = 0; i < vias.GetCount(); i++)
    {
      if (vias[i].m_eSemantic == sematic)
      {
        // NS_ASSERT_DEBUG(vias[i].m_eFormat == format, "Found matching sematic {} but format differs: {} : {}", sematic, format, vias[i].m_eFormat);
        nsUInt32 uiLocation = vias[i].m_uiLocation;
        vias.RemoveAtAndSwap(i);
        return uiLocation;
      }
    }
    return nsMath::MaxValue<nsUInt32>();
  };

  // Copy attribute descriptions
  nsUInt32 usedBindings = 0;
  for (nsUInt32 i = 0; i < m_Description.m_VertexAttributes.GetCount(); i++)
  {
    const nsGALVertexAttribute& Current = m_Description.m_VertexAttributes[i];

    const nsUInt32 uiLocation = FindLocation(Current.m_eSemantic, Current.m_eFormat);
    if (uiLocation == nsMath::MaxValue<nsUInt32>())
    {
      nsLog::Warning("Vertex buffer semantic {} not used by shader", Current.m_eSemantic);
      continue;
    }
    vk::VertexInputAttributeDescription& attrib = m_attributes.ExpandAndGetRef();
    attrib.binding = Current.m_uiVertexBufferSlot;
    attrib.location = uiLocation;
    attrib.format = pVulkanDevice->GetFormatLookupTable().GetFormatInfo(Current.m_eFormat).m_format;
    attrib.offset = Current.m_uiOffset;

    if (attrib.format == vk::Format::eUndefined)
    {
      nsLog::Error("Vertex attribute format {0} of attribute at index {1} is undefined!", Current.m_eFormat, i);
      return NS_FAILURE;
    }

    usedBindings |= NS_BIT(Current.m_uiVertexBufferSlot);
    if (Current.m_uiVertexBufferSlot >= m_bindings.GetCount())
    {
      m_bindings.SetCount(Current.m_uiVertexBufferSlot + 1);
    }
    vk::VertexInputBindingDescription& binding = m_bindings[Current.m_uiVertexBufferSlot];
    binding.binding = Current.m_uiVertexBufferSlot;
    binding.stride = 0;
    binding.inputRate = Current.m_bInstanceData ? vk::VertexInputRate::eInstance : vk::VertexInputRate::eVertex;
  }
  for (nsInt32 i = (nsInt32)m_bindings.GetCount() - 1; i >= 0; --i)
  {
    if ((usedBindings & NS_BIT(i)) == 0)
    {
      m_bindings.RemoveAtAndCopy(i);
    }
  }

  if (!vias.IsEmpty())
  {
    nsLog::Error("Vertex buffers do not cover all vertex attributes defined in the shader!");
    return NS_FAILURE;
  }
  return NS_SUCCESS;
}

nsResult nsGALVertexDeclarationVulkan::DeInitPlatform(nsGALDevice* pDevice)
{
  return NS_SUCCESS;
}


