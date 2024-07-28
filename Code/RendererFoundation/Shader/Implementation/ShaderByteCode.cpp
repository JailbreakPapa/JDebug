#include <RendererFoundation/RendererFoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <RendererFoundation/Shader/ShaderByteCode.h>
#include <RendererFoundation/Shader/Types.h>

nsUInt32 nsShaderConstant::s_TypeSize[(nsUInt32)Type::ENUM_COUNT] = {0, sizeof(float) * 1, sizeof(float) * 2, sizeof(float) * 3, sizeof(float) * 4, sizeof(int) * 1, sizeof(int) * 2, sizeof(int) * 3, sizeof(int) * 4, sizeof(nsUInt32) * 1, sizeof(nsUInt32) * 2,
  sizeof(nsUInt32) * 3, sizeof(nsUInt32) * 4, sizeof(nsShaderMat3), sizeof(nsMat4), sizeof(nsShaderTransform), sizeof(nsShaderBool)};

void nsShaderConstant::CopyDataFormVariant(nsUInt8* pDest, nsVariant* pValue) const
{
  NS_ASSERT_DEV(m_uiArrayElements == 1, "Array constants are not supported");

  nsResult conversionResult = NS_FAILURE;

  if (pValue != nullptr)
  {
    switch (m_Type)
    {
      case Type::Float1:
        *reinterpret_cast<float*>(pDest) = pValue->ConvertTo<float>(&conversionResult);
        break;
      case Type::Float2:
        *reinterpret_cast<nsVec2*>(pDest) = pValue->Get<nsVec2>();
        return;
      case Type::Float3:
        *reinterpret_cast<nsVec3*>(pDest) = pValue->Get<nsVec3>();
        return;
      case Type::Float4:
        if (pValue->GetType() == nsVariant::Type::Color || pValue->GetType() == nsVariant::Type::ColorGamma)
        {
          const nsColor tmp = pValue->ConvertTo<nsColor>();
          *reinterpret_cast<nsVec4*>(pDest) = *reinterpret_cast<const nsVec4*>(&tmp);
        }
        else
        {
          *reinterpret_cast<nsVec4*>(pDest) = pValue->Get<nsVec4>();
        }
        return;

      case Type::Int1:
        *reinterpret_cast<nsInt32*>(pDest) = pValue->ConvertTo<nsInt32>(&conversionResult);
        break;
      case Type::Int2:
        *reinterpret_cast<nsVec2I32*>(pDest) = pValue->Get<nsVec2I32>();
        return;
      case Type::Int3:
        *reinterpret_cast<nsVec3I32*>(pDest) = pValue->Get<nsVec3I32>();
        return;
      case Type::Int4:
        *reinterpret_cast<nsVec4I32*>(pDest) = pValue->Get<nsVec4I32>();
        return;

      case Type::UInt1:
        *reinterpret_cast<nsUInt32*>(pDest) = pValue->ConvertTo<nsUInt32>(&conversionResult);
        break;
      case Type::UInt2:
        *reinterpret_cast<nsVec2U32*>(pDest) = pValue->Get<nsVec2U32>();
        return;
      case Type::UInt3:
        *reinterpret_cast<nsVec3U32*>(pDest) = pValue->Get<nsVec3U32>();
        return;
      case Type::UInt4:
        *reinterpret_cast<nsVec4U32*>(pDest) = pValue->Get<nsVec4U32>();
        return;

      case Type::Mat3x3:
        *reinterpret_cast<nsShaderMat3*>(pDest) = pValue->Get<nsMat3>();
        return;
      case Type::Mat4x4:
        *reinterpret_cast<nsMat4*>(pDest) = pValue->Get<nsMat4>();
        return;
      case Type::Transform:
        *reinterpret_cast<nsShaderTransform*>(pDest) = pValue->Get<nsTransform>();
        return;

      case Type::Bool:
        *reinterpret_cast<nsShaderBool*>(pDest) = pValue->ConvertTo<bool>(&conversionResult);
        break;

      default:
        NS_ASSERT_NOT_IMPLEMENTED;
    }
  }

  if (conversionResult.Succeeded())
  {
    return;
  }

  // nsLog::Error("Constant '{0}' is not set, invalid or couldn't be converted to target type and will be set to zero.", m_sName);
  const nsUInt32 uiSize = s_TypeSize[m_Type];
  nsMemoryUtils::ZeroFill(pDest, uiSize);
}

nsResult nsShaderResourceBinding::CreateMergedShaderResourceBinding(const nsArrayPtr<nsArrayPtr<const nsShaderResourceBinding>>& resourcesPerStage, nsDynamicArray<nsShaderResourceBinding>& out_bindings, bool bAllowMultipleBindingPerName)
{
  nsUInt32 uiSize = 0;
  for (nsUInt32 stage = nsGALShaderStage::VertexShader; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
  {
    uiSize += resourcesPerStage[stage].GetCount();
  }

  out_bindings.Clear();
  out_bindings.Reserve(uiSize);

  auto EqualBindings = [](const nsShaderResourceBinding& a, const nsShaderResourceBinding& b) -> bool
  {
    return a.m_ResourceType == b.m_ResourceType && a.m_TextureType == b.m_TextureType && a.m_uiArraySize == b.m_uiArraySize && a.m_iSet == b.m_iSet && a.m_iSlot == b.m_iSlot;
  };

  auto AddOrExtendBinding = [&](nsGALShaderStage::Enum stage, nsUInt32 uiStartIndex, const nsShaderResourceBinding& add)
  {
    for (nsUInt32 i = uiStartIndex + 1; i < out_bindings.GetCount(); i++)
    {
      if (EqualBindings(out_bindings[i], add))
      {
        out_bindings[i].m_Stages |= nsGALShaderStageFlags::MakeFromShaderStage(stage);
        return;
      }
    }
    nsShaderResourceBinding& newBinding = out_bindings.ExpandAndGetRef();
    newBinding = add;
    newBinding.m_Stages |= nsGALShaderStageFlags::MakeFromShaderStage(stage);
  };

  nsMap<nsHashedString, nsUInt32> nameToIndex;
  nsMap<nsHashedString, nsUInt32> samplerToIndex;
  for (nsUInt32 stage = nsGALShaderStage::VertexShader; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
  {
    for (const nsShaderResourceBinding& res : resourcesPerStage[stage])
    {
      nsHashedString sName = res.m_sName;

      nsUInt32 uiIndex = nsInvalidIndex;
      if (res.m_ResourceType == nsGALShaderResourceType::Sampler)
      {
        // #TODO_SHADER Samplers are special! Since the shader compiler edits the reflection data and renames "*_AutoSampler" to just "*", we generate a naming collision between the texture and the sampler. See nsRenderContext::BindTexture2D for binding code. For now, we allow this collision, but it will probably bite us later on.
        samplerToIndex.TryGetValue(res.m_sName, uiIndex);
      }
      else
      {
        nameToIndex.TryGetValue(res.m_sName, uiIndex);
      }

      if (uiIndex != nsInvalidIndex)
      {
        nsShaderResourceBinding& current = out_bindings[uiIndex];
        if (!EqualBindings(current, res))
        {
          if (bAllowMultipleBindingPerName)
          {
            AddOrExtendBinding((nsGALShaderStage::Enum)stage, uiIndex, res);
            continue;
          }
          // #TODO_SHADER better error reporting.
          nsLog::Error("A shared shader resource '{}' has a mismatching signatures between stages", sName);
          return NS_FAILURE;
        }

        current.m_Stages |= nsGALShaderStageFlags::MakeFromShaderStage((nsGALShaderStage::Enum)stage);
      }
      else
      {
        nsShaderResourceBinding& newBinding = out_bindings.ExpandAndGetRef();
        newBinding = res;
        newBinding.m_Stages |= nsGALShaderStageFlags::MakeFromShaderStage((nsGALShaderStage::Enum)stage);
        if (res.m_ResourceType == nsGALShaderResourceType::Sampler)
        {
          samplerToIndex[res.m_sName] = out_bindings.GetCount() - 1;
        }
        else
        {
          nameToIndex[res.m_sName] = out_bindings.GetCount() - 1;
        }
      }
    }
  }
  return NS_SUCCESS;
}

nsGALShaderByteCode::nsGALShaderByteCode() = default;

nsGALShaderByteCode::~nsGALShaderByteCode()
{
  for (auto& binding : m_ShaderResourceBindings)
  {
    if (binding.m_pLayout != nullptr)
    {
      nsShaderConstantBufferLayout* pLayout = binding.m_pLayout;
      binding.m_pLayout = nullptr;

      if (pLayout->GetRefCount() == 0)
        NS_DEFAULT_DELETE(pLayout);
    }
  }
}
