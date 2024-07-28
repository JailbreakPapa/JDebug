#include <ShaderCompilerDXC/ShaderCompilerDXC.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Memory/MemoryUtils.h>
#include <Foundation/Strings/StringConversion.h>

#include <spirv_reflect.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
#  include <d3dcompiler.h>
#endif

#include <dxc/dxcapi.h>

template <typename T>
struct nsComPtr
{
public:
  nsComPtr() {}
  ~nsComPtr()
  {
    if (m_ptr != nullptr)
    {
      m_ptr->Release();
      m_ptr = nullptr;
    }
  }

  nsComPtr(const nsComPtr& other)
    : m_ptr(other.m_ptr)
  {
    if (m_ptr)
    {
      m_ptr->AddRef();
    }
  }

  T* operator->() { return m_ptr; }
  T* const operator->() const { return m_ptr; }

  T** put()
  {
    NS_ASSERT_DEV(m_ptr == nullptr, "Can only put into an empty nsComPtr");
    return &m_ptr;
  }

  bool operator==(nullptr_t)
  {
    return m_ptr == nullptr;
  }

  bool operator!=(nullptr_t)
  {
    return m_ptr != nullptr;
  }

private:
  T* m_ptr = nullptr;
};

nsComPtr<IDxcUtils> s_pDxcUtils;
nsComPtr<IDxcCompiler3> s_pDxcCompiler;

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(ShaderCompilerDXC, ShaderCompilerDXCPlugin)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(s_pDxcUtils.put()));
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(s_pDxcCompiler.put()));
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    s_pDxcUtils = {};
    s_pDxcCompiler = {};
  }

NS_END_SUBSYSTEM_DECLARATION;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsShaderCompilerDXC, 1, nsRTTIDefaultAllocator<nsShaderCompilerDXC>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

static nsResult CompileVulkanShader(const char* szFile, const char* szSource, bool bDebug, const char* szProfile, const char* szEntryPoint, nsDynamicArray<nsUInt8>& out_ByteCode);

static const char* GetProfileName(nsStringView sPlatform, nsGALShaderStage::Enum Stage)
{
  if (sPlatform == "VULKAN")
  {
    switch (Stage)
    {
      case nsGALShaderStage::VertexShader:
        return "vs_6_0";
      case nsGALShaderStage::HullShader:
        return "hs_6_0";
      case nsGALShaderStage::DomainShader:
        return "ds_6_0";
      case nsGALShaderStage::GeometryShader:
        return "gs_6_0";
      case nsGALShaderStage::PixelShader:
        return "ps_6_0";
      case nsGALShaderStage::ComputeShader:
        return "cs_6_0";
      default:
        break;
    }
  }

  NS_REPORT_FAILURE("Unknown Platform '{}' or Stage {}", sPlatform, Stage);
  return "";
}

nsResult nsShaderCompilerDXC::Initialize()
{
  if (m_VertexInputMapping.IsEmpty())
  {
    m_VertexInputMapping["in.var.POSITION"] = nsGALVertexAttributeSemantic::Position;
    m_VertexInputMapping["in.var.NORMAL"] = nsGALVertexAttributeSemantic::Normal;
    m_VertexInputMapping["in.var.TANGENT"] = nsGALVertexAttributeSemantic::Tangent;

    m_VertexInputMapping["in.var.COLOR0"] = nsGALVertexAttributeSemantic::Color0;
    m_VertexInputMapping["in.var.COLOR1"] = nsGALVertexAttributeSemantic::Color1;
    m_VertexInputMapping["in.var.COLOR2"] = nsGALVertexAttributeSemantic::Color2;
    m_VertexInputMapping["in.var.COLOR3"] = nsGALVertexAttributeSemantic::Color3;
    m_VertexInputMapping["in.var.COLOR4"] = nsGALVertexAttributeSemantic::Color4;
    m_VertexInputMapping["in.var.COLOR5"] = nsGALVertexAttributeSemantic::Color5;
    m_VertexInputMapping["in.var.COLOR6"] = nsGALVertexAttributeSemantic::Color6;
    m_VertexInputMapping["in.var.COLOR7"] = nsGALVertexAttributeSemantic::Color7;

    m_VertexInputMapping["in.var.TEXCOORD0"] = nsGALVertexAttributeSemantic::TexCoord0;
    m_VertexInputMapping["in.var.TEXCOORD1"] = nsGALVertexAttributeSemantic::TexCoord1;
    m_VertexInputMapping["in.var.TEXCOORD2"] = nsGALVertexAttributeSemantic::TexCoord2;
    m_VertexInputMapping["in.var.TEXCOORD3"] = nsGALVertexAttributeSemantic::TexCoord3;
    m_VertexInputMapping["in.var.TEXCOORD4"] = nsGALVertexAttributeSemantic::TexCoord4;
    m_VertexInputMapping["in.var.TEXCOORD5"] = nsGALVertexAttributeSemantic::TexCoord5;
    m_VertexInputMapping["in.var.TEXCOORD6"] = nsGALVertexAttributeSemantic::TexCoord6;
    m_VertexInputMapping["in.var.TEXCOORD7"] = nsGALVertexAttributeSemantic::TexCoord7;
    m_VertexInputMapping["in.var.TEXCOORD8"] = nsGALVertexAttributeSemantic::TexCoord8;
    m_VertexInputMapping["in.var.TEXCOORD9"] = nsGALVertexAttributeSemantic::TexCoord9;

    m_VertexInputMapping["in.var.BITANGENT"] = nsGALVertexAttributeSemantic::BiTangent;
    m_VertexInputMapping["in.var.BONEINDICES0"] = nsGALVertexAttributeSemantic::BoneIndices0;
    m_VertexInputMapping["in.var.BONEINDICES1"] = nsGALVertexAttributeSemantic::BoneIndices1;
    m_VertexInputMapping["in.var.BONEWEIGHTS0"] = nsGALVertexAttributeSemantic::BoneWeights0;
    m_VertexInputMapping["in.var.BONEWEIGHTS1"] = nsGALVertexAttributeSemantic::BoneWeights1;
  }

  NS_ASSERT_DEV(s_pDxcUtils != nullptr && s_pDxcCompiler != nullptr, "ShaderCompiler SubSystem init should have initialized library pointers.");
  return NS_SUCCESS;
}

nsResult nsShaderCompilerDXC::Compile(nsShaderProgramData& inout_Data, nsLogInterface* pLog)
{
  NS_SUCCEED_OR_RETURN(Initialize());

  for (nsUInt32 stage = 0; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
  {
    if (inout_Data.m_uiSourceHash[stage] == 0)
      continue;
    if (inout_Data.m_bWriteToDisk[stage] == false)
    {
      nsLog::Debug("Shader for stage '{}' is already compiled.", nsGALShaderStage::Names[stage]);
      continue;
    }

    const nsStringBuilder sShaderSource = inout_Data.m_sShaderSource[stage];

    if (!sShaderSource.IsEmpty() && sShaderSource.FindSubString("main") != nullptr)
    {
      const nsStringBuilder sSourceFile = inout_Data.m_sSourceFile;

      if (CompileVulkanShader(sSourceFile, sShaderSource, inout_Data.m_Flags.IsSet(nsShaderCompilerFlags::Debug), GetProfileName(inout_Data.m_sPlatform, (nsGALShaderStage::Enum)stage), "main", inout_Data.m_ByteCode[stage]->m_ByteCode).Succeeded())
      {
        NS_SUCCEED_OR_RETURN(ReflectShaderStage(inout_Data, (nsGALShaderStage::Enum)stage));
      }
      else
      {
        return NS_FAILURE;
      }
    }
  }

  return NS_SUCCESS;
}

nsResult CompileVulkanShader(const char* szFile, const char* szSource, bool bDebug, const char* szProfile, const char* szEntryPoint, nsDynamicArray<nsUInt8>& out_ByteCode)
{
  out_ByteCode.Clear();

  const char* szCompileSource = szSource;
  nsStringBuilder sDebugSource;

  nsDynamicArray<nsStringWChar> args;
  args.PushBack(nsStringWChar(szFile));
  args.PushBack(L"-E");
  args.PushBack(nsStringWChar(szEntryPoint));
  args.PushBack(L"-T");
  args.PushBack(nsStringWChar(szProfile));
  args.PushBack(L"-spirv");
  args.PushBack(L"-fvk-use-dx-position-w");
  args.PushBack(L"-fspv-target-env=vulkan1.1");

  if (bDebug)
  {
    // In debug mode we need to remove '#line' as any shader debugger won't work with them.
    sDebugSource = szSource;
    sDebugSource.ReplaceAll("#line ", "//ine ");
    szCompileSource = sDebugSource;

    // nsLog::Warning("Vulkan DEBUG shader support not really implemented.");

    args.PushBack(L"-Zi"); // Enable debug information.
    // args.PushBack(L"-Fo"); // Optional. Stored in the pdb.
    // args.PushBack(L"myshader.bin");
    // args.PushBack(L"-Fd"); // The file name of the pdb.
    // args.PushBack(L"myshader.pdb");
  }

  nsComPtr<IDxcBlobEncoding> pSource;
  s_pDxcUtils->CreateBlob(szCompileSource, (UINT32)strlen(szCompileSource), DXC_CP_UTF8, pSource.put());

  DxcBuffer Source;
  Source.Ptr = pSource->GetBufferPointer();
  Source.Size = pSource->GetBufferSize();
  Source.Encoding = DXC_CP_UTF8;

  nsHybridArray<LPCWSTR, 16> pszArgs;
  pszArgs.SetCount(args.GetCount());
  for (nsUInt32 i = 0; i < args.GetCount(); ++i)
  {
    pszArgs[i] = args[i].GetData();
  }

  nsComPtr<IDxcResult> pResults;
  s_pDxcCompiler->Compile(&Source, pszArgs.GetData(), pszArgs.GetCount(), nullptr, IID_PPV_ARGS(pResults.put()));

  nsComPtr<IDxcBlobUtf8> pErrors;
  pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(pErrors.put()), nullptr);

  HRESULT hrStatus;
  pResults->GetStatus(&hrStatus);
  if (FAILED(hrStatus))
  {
    nsLog::Error("Vulkan shader compilation failed.");

    if (pErrors != nullptr && pErrors->GetStringLength() != 0)
    {
      nsLog::Error("{}", nsStringUtf8(pErrors->GetStringPointer()).GetData());
    }

    return NS_FAILURE;
  }
  else
  {
    if (pErrors != nullptr && pErrors->GetStringLength() != 0)
    {
      nsLog::Warning("{}", nsStringUtf8(pErrors->GetStringPointer()).GetData());
    }
  }

  nsComPtr<IDxcBlob> pShader;
  nsComPtr<IDxcBlobWide> pShaderName;
  pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(pShader.put()), pShaderName.put());

  if (pShader == nullptr)
  {
    nsLog::Error("No Vulkan bytecode was generated.");
    return NS_FAILURE;
  }

  out_ByteCode.SetCountUninitialized(static_cast<nsUInt32>(pShader->GetBufferSize()));

  nsMemoryUtils::Copy(out_ByteCode.GetData(), reinterpret_cast<nsUInt8*>(pShader->GetBufferPointer()), out_ByteCode.GetCount());

  return NS_SUCCESS;
}

nsResult nsShaderCompilerDXC::ModifyShaderSource(nsShaderProgramData& inout_data, nsLogInterface* pLog)
{
  for (nsUInt32 stage = nsGALShaderStage::VertexShader; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
  {
    nsShaderParser::ParseShaderResources(inout_data.m_sShaderSource[stage], inout_data.m_Resources[stage]);
  }

  nsHashTable<nsHashedString, nsShaderResourceBinding> bindings;
  NS_SUCCEED_OR_RETURN(nsShaderParser::MergeShaderResourceBindings(inout_data, bindings, pLog));
  NS_SUCCEED_OR_RETURN(DefineShaderResourceBindings(inout_data, bindings, pLog));
  NS_SUCCEED_OR_RETURN(nsShaderParser::SanityCheckShaderResourceBindings(bindings, pLog));

  // Apply shader resource bindings
  nsStringBuilder sNewShaderCode;
  for (nsUInt32 stage = nsGALShaderStage::VertexShader; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
  {
    if (inout_data.m_sShaderSource[stage].IsEmpty())
      continue;
    nsShaderParser::ApplyShaderResourceBindings(inout_data.m_sPlatform, inout_data.m_sShaderSource[stage], inout_data.m_Resources[stage], bindings, nsMakeDelegate(&nsShaderCompilerDXC::CreateNewShaderResourceDeclaration, this), sNewShaderCode);
    inout_data.m_sShaderSource[stage] = sNewShaderCode;
    inout_data.m_Resources[stage].Clear();
  }
  return NS_SUCCESS;
}

nsResult nsShaderCompilerDXC::DefineShaderResourceBindings(const nsShaderProgramData& data, nsHashTable<nsHashedString, nsShaderResourceBinding>& inout_resourceBinding, nsLogInterface* pLog)
{
  // Determine which indices are hard-coded in the shader already.
  nsHybridArray<nsHybridBitfield<64>, 4> slotInUseInSet;
  for (auto it : inout_resourceBinding)
  {
    nsInt16& iSet = it.Value().m_iSet;
    if (iSet == -1)
      iSet = 0;

    slotInUseInSet.EnsureCount(iSet + 1);

    if (it.Value().m_iSlot != -1)
    {
      slotInUseInSet[iSet].SetCount(nsMath::Max(slotInUseInSet[iSet].GetCount(), static_cast<nsUInt32>(it.Value().m_iSlot + 1)));
      slotInUseInSet[iSet].SetBit(it.Value().m_iSlot);
    }
  }

  // Create stable oder of resources in each set.
  nsHybridArray<nsHybridArray<nsHashedString, 16>, 4> orderInSet;
  orderInSet.SetCount(slotInUseInSet.GetCount());
  for (nsUInt32 stage = nsGALShaderStage::VertexShader; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
  {
    if (data.m_sShaderSource[stage].IsEmpty())
      continue;

    for (const auto& res : data.m_Resources[stage])
    {
      const nsInt16 iSet = res.m_Binding.m_iSet < 0 ? (nsInt16)0 : res.m_Binding.m_iSet;
      if (!orderInSet[iSet].Contains(res.m_Binding.m_sName))
      {
        orderInSet[iSet].PushBack(res.m_Binding.m_sName);
      }
    }
  }

  // Do we have _AutoSampler in use? Combine them!
  struct TextureAndSamplerTuple
  {
    nsHashTable<nsHashedString, nsShaderResourceBinding>::Iterator itSampler;
    nsHashTable<nsHashedString, nsShaderResourceBinding>::Iterator itTexture;
  };
  nsHybridArray<TextureAndSamplerTuple, 2> autoSamplers;
  for (auto itSampler : inout_resourceBinding)
  {
    if (itSampler.Value().m_ResourceType != nsGALShaderResourceType::Sampler || !itSampler.Key().GetView().EndsWith("_AutoSampler"))
      continue;

    nsStringBuilder sb = itSampler.Key().GetString();
    sb.TrimWordEnd("_AutoSampler");
    auto itTexture = inout_resourceBinding.Find(nsTempHashedString(sb));
    if (!itTexture.IsValid())
      continue;

    if (itSampler.Value().m_iSet != itTexture.Value().m_iSet || itSampler.Value().m_iSlot != itTexture.Value().m_iSlot)
      continue;

    itSampler.Value().m_ResourceType = nsGALShaderResourceType::TextureAndSampler;
    itTexture.Value().m_ResourceType = nsGALShaderResourceType::TextureAndSampler;
    // Sampler will match the slot of the texture at the end
    orderInSet[itSampler.Value().m_iSet].RemoveAndCopy(itSampler.Key());
    autoSamplers.PushBack({itSampler, itTexture});
  }

  // Assign slot to each resource in each set.
  for (nsInt16 iSet = 0; iSet < (nsInt16)slotInUseInSet.GetCount(); ++iSet)
  {
    nsUInt32 uiCurrentSlot = 0;
    for (const auto& sName : orderInSet[iSet])
    {
      nsInt16& iSlot = inout_resourceBinding[sName].m_iSlot;
      if (iSlot != -1)
        continue;
      while (uiCurrentSlot < slotInUseInSet[iSet].GetCount() && slotInUseInSet[iSet].IsBitSet(uiCurrentSlot))
      {
        uiCurrentSlot++;
      }
      iSlot = static_cast<nsInt16>(uiCurrentSlot);
      slotInUseInSet[iSet].SetCount(nsMath::Max(slotInUseInSet[iSet].GetCount(), uiCurrentSlot + 1));
      slotInUseInSet[iSet].SetBit(uiCurrentSlot);
    }
  }

  // Copy texture assignments to the samplers.
  for (TextureAndSamplerTuple& tas : autoSamplers)
  {
    tas.itSampler.Value().m_iSlot = tas.itTexture.Value().m_iSlot;
  }
  return NS_SUCCESS;
}

void nsShaderCompilerDXC::CreateNewShaderResourceDeclaration(nsStringView sPlatform, nsStringView sDeclaration, const nsShaderResourceBinding& binding, nsStringBuilder& out_sDeclaration)
{
  nsBitflags<nsGALShaderResourceCategory> type = nsGALShaderResourceCategory::MakeFromShaderDescriptorType(binding.m_ResourceType);
  nsStringView sResourcePrefix;

  // The only descriptor that can have more than one shader resource type is TextureAndSampler.
  // There will be two declarations in the HLSL code, the sampler and the texture.
  if (binding.m_ResourceType == nsGALShaderResourceType::TextureAndSampler)
  {
    type = binding.m_TextureType == nsGALShaderTextureType::Unknown ? nsGALShaderResourceCategory::Sampler : nsGALShaderResourceCategory::TextureSRV;
  }

  switch (type.GetValue())
  {
    case nsGALShaderResourceCategory::Sampler:
      sResourcePrefix = "s"_nssv;
      break;
    case nsGALShaderResourceCategory::ConstantBuffer:
      sResourcePrefix = "b"_nssv;
      break;
    case nsGALShaderResourceCategory::TextureSRV:
    case nsGALShaderResourceCategory::BufferSRV:
      sResourcePrefix = "t"_nssv;
      break;
    case nsGALShaderResourceCategory::TextureUAV:
    case nsGALShaderResourceCategory::BufferUAV:
      sResourcePrefix = "u"_nssv;
      break;
    default:
      NS_ASSERT_NOT_IMPLEMENTED;
      break;
  }

  if (binding.m_ResourceType == nsGALShaderResourceType::TextureAndSampler)
  {
    out_sDeclaration.SetFormat("[[vk::combinedImageSampler]] {} : register({}{}, space{})", sDeclaration, sResourcePrefix, binding.m_iSlot, binding.m_iSet);
  }
  else
  {
    out_sDeclaration.SetFormat("{} : register({}{}, space{})", sDeclaration, sResourcePrefix, binding.m_iSlot, binding.m_iSet);
  }
}

nsResult nsShaderCompilerDXC::FillResourceBinding(nsGALShaderByteCode& shaderBinary, nsShaderResourceBinding& binding, const SpvReflectDescriptorBinding& info)
{
  if ((info.resource_type & SpvReflectResourceType::SPV_REFLECT_RESOURCE_FLAG_SRV) != 0)
  {
    return FillSRVResourceBinding(shaderBinary, binding, info);
  }

  else if (info.resource_type == SpvReflectResourceType::SPV_REFLECT_RESOURCE_FLAG_UAV)
  {
    return FillUAVResourceBinding(shaderBinary, binding, info);
  }

  else if (info.resource_type == SpvReflectResourceType::SPV_REFLECT_RESOURCE_FLAG_CBV)
  {
    binding.m_ResourceType = nsGALShaderResourceType::ConstantBuffer;
    binding.m_pLayout = ReflectConstantBufferLayout(shaderBinary, info.name, info.block);

    return NS_SUCCESS;
  }

  else if (info.resource_type == SpvReflectResourceType::SPV_REFLECT_RESOURCE_FLAG_SAMPLER)
  {
    binding.m_ResourceType = nsGALShaderResourceType::Sampler;

    if (binding.m_sName.GetString().EndsWith("_AutoSampler"))
    {
      nsStringBuilder sb = binding.m_sName.GetString();
      sb.TrimWordEnd("_AutoSampler");
      binding.m_sName.Assign(sb);
    }

    return NS_SUCCESS;
  }

  nsLog::Error("Resource '{}': Unsupported resource type.", info.name);
  return NS_FAILURE;
}

nsGALShaderTextureType::Enum nsShaderCompilerDXC::GetTextureType(const SpvReflectDescriptorBinding& info)
{
  switch (info.image.dim)
  {
    case SpvDim::SpvDim1D:
    {
      if (info.image.ms == 0)
      {
        if (info.image.arrayed > 0)
        {
          return nsGALShaderTextureType::Texture1DArray;
        }
        else
        {
          return nsGALShaderTextureType::Texture1D;
        }
      }

      break;
    }

    case SpvDim::SpvDim2D:
    {
      if (info.image.ms == 0)
      {
        if (info.image.arrayed > 0)
        {
          return nsGALShaderTextureType::Texture2DArray;
        }
        else
        {
          return nsGALShaderTextureType::Texture2D;
        }
      }
      else
      {
        if (info.image.arrayed > 0)
        {
          return nsGALShaderTextureType::Texture2DMSArray;
        }
        else
        {
          return nsGALShaderTextureType::Texture2DMS;
        }
      }

      break;
    }

    case SpvDim::SpvDim3D:
    {
      if (info.image.ms == 0 && info.image.arrayed == 0)
      {
        return nsGALShaderTextureType::Texture3D;
      }

      break;
    }

    case SpvDim::SpvDimCube:
    {
      if (info.image.ms == 0)
      {
        if (info.image.arrayed == 0)
        {
          return nsGALShaderTextureType::TextureCube;
        }
        else
        {
          return nsGALShaderTextureType::TextureCubeArray;
        }
      }

      break;
    }

    case SpvDim::SpvDimBuffer:
      NS_ASSERT_NOT_IMPLEMENTED;
      // binding.m_TextureType = nsGALShaderTextureType::GenericBuffer;
      return nsGALShaderTextureType::Unknown;

    case SpvDim::SpvDimRect:
      NS_ASSERT_NOT_IMPLEMENTED;
      return nsGALShaderTextureType::Unknown;

    case SpvDim::SpvDimSubpassData:
      NS_ASSERT_NOT_IMPLEMENTED;
      return nsGALShaderTextureType::Unknown;

    case SpvDim::SpvDimMax:
      NS_ASSERT_DEV(false, "Invalid enum value");
      break;

    default:
      break;
  }
  return nsGALShaderTextureType::Unknown;
}

nsResult nsShaderCompilerDXC::FillSRVResourceBinding(nsGALShaderByteCode& shaderBinary, nsShaderResourceBinding& binding, const SpvReflectDescriptorBinding& info)
{
  if (info.descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER)
  {
    if (info.type_description->op == SpvOp::SpvOpTypeStruct)
    {
      binding.m_ResourceType = nsGALShaderResourceType::StructuredBuffer;
      return NS_SUCCESS;
    }
  }

  if (info.descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
  {
    binding.m_ResourceType = nsGALShaderResourceType::Texture;
    binding.m_TextureType = GetTextureType(info);
    return NS_SUCCESS;
  }

  if (info.descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
  {
    NS_ASSERT_DEV(!binding.m_sName.GetString().EndsWith("_AutoSampler"), "Combined image sampler should have taken the name from the image part");
    binding.m_ResourceType = nsGALShaderResourceType::TextureAndSampler;
    binding.m_TextureType = GetTextureType(info);
    return NS_SUCCESS;
  }

  if (info.descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER)
  {
    if (info.image.dim == SpvDim::SpvDimBuffer)
    {
      binding.m_ResourceType = nsGALShaderResourceType::TexelBuffer;
      return NS_SUCCESS;
    }

    nsLog::Error("Resource '{}': Unsupported texel buffer SRV type.", info.name);
    return NS_FAILURE;
  }

  nsLog::Error("Resource '{}': Unsupported SRV type.", info.name);
  return NS_FAILURE;
}

nsResult nsShaderCompilerDXC::FillUAVResourceBinding(nsGALShaderByteCode& shaderBinary, nsShaderResourceBinding& binding, const SpvReflectDescriptorBinding& info)
{
  if (info.descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE)
  {
    binding.m_ResourceType = nsGALShaderResourceType::TextureRW;
    binding.m_TextureType = GetTextureType(info);
    return NS_SUCCESS;
  }

  if (info.descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)
  {
    if (info.image.dim == SpvDim::SpvDimBuffer)
    {
      binding.m_ResourceType = nsGALShaderResourceType::TexelBufferRW;
      return NS_SUCCESS;
    }

    nsLog::Error("Resource '{}': Unsupported texel buffer UAV type.", info.name);
    return NS_FAILURE;
  }

  if (info.descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER)
  {
    if (info.image.dim == SpvDim::SpvDimBuffer)
    {
      binding.m_ResourceType = nsGALShaderResourceType::StructuredBufferRW;
      return NS_SUCCESS;
    }
    else if (info.image.dim == SpvDim::SpvDim1D)
    {
      binding.m_ResourceType = nsGALShaderResourceType::StructuredBufferRW;
      return NS_SUCCESS;
    }

    nsLog::Error("Resource '{}': Unsupported storage buffer UAV type.", info.name);
    return NS_FAILURE;
  }
  nsLog::Error("Resource '{}': Unsupported UAV type.", info.name);
  return NS_FAILURE;
}

nsGALResourceFormat::Enum GetNSFormat(SpvReflectFormat format)
{
  switch (format)
  {
    case SpvReflectFormat::SPV_REFLECT_FORMAT_R32_UINT:
      return nsGALResourceFormat::RUInt;
    case SpvReflectFormat::SPV_REFLECT_FORMAT_R32_SINT:
      return nsGALResourceFormat::RInt;
    case SpvReflectFormat::SPV_REFLECT_FORMAT_R32_SFLOAT:
      return nsGALResourceFormat::RFloat;
    case SpvReflectFormat::SPV_REFLECT_FORMAT_R32G32_UINT:
      return nsGALResourceFormat::RGUInt;
    case SpvReflectFormat::SPV_REFLECT_FORMAT_R32G32_SINT:
      return nsGALResourceFormat::RGInt;
    case SpvReflectFormat::SPV_REFLECT_FORMAT_R32G32_SFLOAT:
      return nsGALResourceFormat::RGFloat;
    case SpvReflectFormat::SPV_REFLECT_FORMAT_R32G32B32_UINT:
      return nsGALResourceFormat::RGBUInt;
    case SpvReflectFormat::SPV_REFLECT_FORMAT_R32G32B32_SINT:
      return nsGALResourceFormat::RGBInt;
    case SpvReflectFormat::SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
      return nsGALResourceFormat::RGBFloat;
    case SpvReflectFormat::SPV_REFLECT_FORMAT_R32G32B32A32_UINT:
      return nsGALResourceFormat::RGBAUInt;
    case SpvReflectFormat::SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
      return nsGALResourceFormat::RGBAInt;
    case SpvReflectFormat::SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
      return nsGALResourceFormat::RGBAFloat;
    case SpvReflectFormat::SPV_REFLECT_FORMAT_UNDEFINED:
    default:
      return nsGALResourceFormat::Invalid;
  }
}

nsResult nsShaderCompilerDXC::ReflectShaderStage(nsShaderProgramData& inout_Data, nsGALShaderStage::Enum Stage)
{
  NS_LOG_BLOCK("ReflectShaderStage", inout_Data.m_sSourceFile);

  nsGALShaderByteCode* pShader = inout_Data.m_ByteCode[Stage];
  auto& bytecode = pShader->m_ByteCode;

  SpvReflectShaderModule module;

  if (spvReflectCreateShaderModule(bytecode.GetCount(), bytecode.GetData(), &module) != SPV_REFLECT_RESULT_SUCCESS)
  {
    nsLog::Error("Extracting shader reflection information failed.");
    return NS_FAILURE;
  }

  NS_SCOPE_EXIT(spvReflectDestroyShaderModule(&module));

  //
  auto& vertexInputAttributes = pShader->m_ShaderVertexInput;
  if (Stage == nsGALShaderStage::VertexShader)
  {
    nsUInt32 uiNumVars = 0;
    if (spvReflectEnumerateInputVariables(&module, &uiNumVars, nullptr) != SPV_REFLECT_RESULT_SUCCESS)
    {
      nsLog::Error("Failed to retrieve number of input variables.");
      return NS_FAILURE;
    }
    nsDynamicArray<SpvReflectInterfaceVariable*> vars;
    vars.SetCount(uiNumVars);

    if (spvReflectEnumerateInputVariables(&module, &uiNumVars, vars.GetData()) != SPV_REFLECT_RESULT_SUCCESS)
    {
      nsLog::Error("Failed to retrieve input variables.");
      return NS_FAILURE;
    }

    vertexInputAttributes.Reserve(vars.GetCount());

    for (nsUInt32 i = 0; i < vars.GetCount(); ++i)
    {
      SpvReflectInterfaceVariable* pVar = vars[i];
      if (pVar->name != nullptr)
      {
        nsShaderVertexInputAttribute& attr = vertexInputAttributes.ExpandAndGetRef();
        attr.m_uiLocation = static_cast<nsUInt8>(pVar->location);

        nsGALVertexAttributeSemantic::Enum* pVAS = m_VertexInputMapping.GetValue(pVar->name);
        NS_ASSERT_DEV(pVAS != nullptr, "Unknown vertex input sematic found: {}", pVar->name);
        attr.m_eSemantic = *pVAS;
        attr.m_eFormat = GetNSFormat(pVar->format);
        NS_ASSERT_DEV(pVAS != nullptr, "Unknown vertex input format found: {}", pVar->format);
      }
    }
  }
  else if (Stage == nsGALShaderStage::HullShader)
  {
    pShader->m_uiTessellationPatchControlPoints = module.entry_points[0].output_vertices;
  }

  // descriptor bindings
  {
    nsUInt32 uiNumVars = 0;
    if (spvReflectEnumerateDescriptorBindings(&module, &uiNumVars, nullptr) != SPV_REFLECT_RESULT_SUCCESS)
    {
      nsLog::Error("Failed to retrieve number of descriptor bindings.");
      return NS_FAILURE;
    }

    nsDynamicArray<SpvReflectDescriptorBinding*> vars;
    vars.SetCount(uiNumVars);

    if (spvReflectEnumerateDescriptorBindings(&module, &uiNumVars, vars.GetData()) != SPV_REFLECT_RESULT_SUCCESS)
    {
      nsLog::Error("Failed to retrieve descriptor bindings.");
      return NS_FAILURE;
    }

    for (nsUInt32 i = 0; i < vars.GetCount(); ++i)
    {
      auto& info = *vars[i];

      nsLog::Info("Bound Resource: '{}' at slot {} (Count: {})", info.name, info.binding, info.count);

      nsShaderResourceBinding shaderResourceBinding;
      shaderResourceBinding.m_iSet = static_cast<nsInt16>(info.set);
      shaderResourceBinding.m_iSlot = static_cast<nsInt16>(info.binding);
      shaderResourceBinding.m_uiArraySize = info.count;
      shaderResourceBinding.m_sName.Assign(info.name);
      shaderResourceBinding.m_Stages = nsGALShaderStageFlags::MakeFromShaderStage(Stage);

      if (FillResourceBinding(*inout_Data.m_ByteCode[Stage], shaderResourceBinding, info).Failed())
        continue;

      NS_ASSERT_DEV(shaderResourceBinding.m_ResourceType != nsGALShaderResourceType::Unknown, "FillResourceBinding should have failed.");

      inout_Data.m_ByteCode[Stage]->m_ShaderResourceBindings.PushBack(shaderResourceBinding);
    }
  }

  // Push Constants
  {
    nsUInt32 uiNumVars = 0;
    if (spvReflectEnumeratePushConstantBlocks(&module, &uiNumVars, nullptr) != SPV_REFLECT_RESULT_SUCCESS)
    {
      nsLog::Error("Failed to retrieve number of descriptor bindings.");
      return NS_FAILURE;
    }

    if (uiNumVars > 1)
    {
      nsLog::Error("Only one push constant block is supported right now.");
      return NS_FAILURE;
    }

    nsDynamicArray<SpvReflectBlockVariable*> vars;
    vars.SetCount(uiNumVars);

    if (spvReflectEnumeratePushConstantBlocks(&module, &uiNumVars, vars.GetData()) != SPV_REFLECT_RESULT_SUCCESS)
    {
      nsLog::Error("Failed to retrieve descriptor bindings.");
      return NS_FAILURE;
    }

    for (nsUInt32 i = 0; i < vars.GetCount(); ++i)
    {
      auto& info = *vars[i];

      nsStringBuilder sName = info.name;
      sName.TrimWordStart("type.PushConstant.");
      sName.TrimWordEnd("_PushConstants");

      nsLog::Info("Push Constants: '{}', Offset: {}, Size: {}", sName, info.offset, info.padded_size);

      if (info.offset != 0)
      {
        nsLog::Error("The push constant block '{}' has an offset of '{}', only a zero offset is supported right now. This should be the case if only one block exists", sName, info.offset);
        return NS_FAILURE;
      }

      nsShaderResourceBinding shaderResourceBinding;
      shaderResourceBinding.m_ResourceType = nsGALShaderResourceType::PushConstants;
      shaderResourceBinding.m_iSet = -1;
      shaderResourceBinding.m_iSlot = -1;
      shaderResourceBinding.m_uiArraySize = 1;

      shaderResourceBinding.m_sName.Assign(sName);
      shaderResourceBinding.m_Stages = nsGALShaderStageFlags::MakeFromShaderStage(Stage);
      shaderResourceBinding.m_pLayout = ReflectConstantBufferLayout(*inout_Data.m_ByteCode[Stage], info.name, info);
      inout_Data.m_ByteCode[Stage]->m_ShaderResourceBindings.PushBack(shaderResourceBinding);
    }
  }

  return NS_SUCCESS;
}

nsShaderConstantBufferLayout* nsShaderCompilerDXC::ReflectConstantBufferLayout(nsGALShaderByteCode& pStageBinary, const char* szName, const SpvReflectBlockVariable& block)
{
  NS_LOG_BLOCK("Constant Buffer Layout", szName);
  nsLog::Debug("Constant Buffer has {} variables, Size is {}", block.member_count, block.padded_size);

  nsShaderConstantBufferLayout* pLayout = NS_DEFAULT_NEW(nsShaderConstantBufferLayout);

  pLayout->m_uiTotalSize = block.padded_size;

  for (nsUInt32 var = 0; var < block.member_count; ++var)
  {
    const auto& svd = block.members[var];

    nsShaderConstant constant;
    constant.m_sName.Assign(svd.name);
    constant.m_uiOffset = svd.offset; // TODO: or svd.absolute_offset ??
    constant.m_uiArrayElements = 1;

    nsUInt32 uiFlags = svd.type_description->type_flags;

    if (uiFlags & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_ARRAY)
    {
      uiFlags &= ~SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_ARRAY;

      if (svd.array.dims_count != 1)
      {
        nsLog::Error("Variable '{}': Multi-dimensional arrays are not supported.", constant.m_sName);
        continue;
      }

      constant.m_uiArrayElements = svd.array.dims[0];
    }

    nsUInt32 uiComponents = 0;

    if (uiFlags & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_VECTOR)
    {
      uiFlags &= ~SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_VECTOR;

      uiComponents = svd.numeric.vector.component_count;
    }

    if (uiFlags & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_BOOL)
    {
      uiFlags &= ~SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_BOOL;

      // TODO: unfortunately this never seems to be set, 'bool' types are always exposed as 'int'
      NS_ASSERT_NOT_IMPLEMENTED;

      switch (uiComponents)
      {
        case 0:
        case 1:
          constant.m_Type = nsShaderConstant::Type::Bool;
          break;

        default:
          nsLog::Error("Variable '{}': Multi-component bools are not supported.", constant.m_sName);
          continue;
      }
    }
    else if (uiFlags & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_INT)
    {
      uiFlags &= ~SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_INT;

      // TODO: there doesn't seem to be a way to detect 'unsigned' types

      switch (uiComponents)
      {
        case 0:
        case 1:
          constant.m_Type = nsShaderConstant::Type::Int1;
          break;
        case 2:
          constant.m_Type = nsShaderConstant::Type::Int2;
          break;
        case 3:
          constant.m_Type = nsShaderConstant::Type::Int3;
          break;
        case 4:
          constant.m_Type = nsShaderConstant::Type::Int4;
          break;
      }
    }
    else if (uiFlags & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_FLOAT)
    {
      uiFlags &= ~SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_FLOAT;

      switch (uiComponents)
      {
        case 0:
        case 1:
          constant.m_Type = nsShaderConstant::Type::Float1;
          break;
        case 2:
          constant.m_Type = nsShaderConstant::Type::Float2;
          break;
        case 3:
          constant.m_Type = nsShaderConstant::Type::Float3;
          break;
        case 4:
          constant.m_Type = nsShaderConstant::Type::Float4;
          break;
      }
    }

    if (uiFlags & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_MATRIX)
    {
      uiFlags &= ~SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_MATRIX;

      constant.m_Type = nsShaderConstant::Type::Default;

      const nsUInt32 rows = svd.type_description->traits.numeric.matrix.row_count;
      const nsUInt32 columns = svd.type_description->traits.numeric.matrix.column_count;

      if ((svd.type_description->type_flags & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_FLOAT) == 0)
      {
        nsLog::Error("Variable '{}': Only float matrices are supported", constant.m_sName);
        continue;
      }

      if (columns == 3 && rows == 3)
      {
        constant.m_Type = nsShaderConstant::Type::Mat3x3;
      }
      else if (columns == 4 && rows == 4)
      {
        constant.m_Type = nsShaderConstant::Type::Mat4x4;
      }
      else
      {
        nsLog::Error("Variable '{}': {}x{} matrices are not supported", constant.m_sName, rows, columns);
        continue;
      }
    }

    if (uiFlags & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_STRUCT)
    {
      uiFlags &= ~SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_STRUCT;
      uiFlags &= ~SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_EXTERNAL_BLOCK;

      constant.m_Type = nsShaderConstant::Type::Struct;
    }

    if (uiFlags != 0)
    {
      nsLog::Error("Variable '{}': Unknown additional type flags '{}'", constant.m_sName, uiFlags);
    }

    if (constant.m_Type == nsShaderConstant::Type::Default)
    {
      nsLog::Error("Variable '{}': Variable type is unknown / not supported", constant.m_sName);
      continue;
    }

    const char* typeNames[] = {
      "Default",
      "Float1",
      "Float2",
      "Float3",
      "Float4",
      "Int1",
      "Int2",
      "Int3",
      "Int4",
      "UInt1",
      "UInt2",
      "UInt3",
      "UInt4",
      "Mat3x3",
      "Mat4x4",
      "Transform",
      "Bool",
      "Struct",
    };

    if (constant.m_uiArrayElements > 1)
    {
      nsLog::Info("{1} {3}[{2}] {0}", constant.m_sName, nsArgU(constant.m_uiOffset, 3, true), constant.m_uiArrayElements, typeNames[constant.m_Type]);
    }
    else
    {
      nsLog::Info("{1} {3} {0}", constant.m_sName, nsArgU(constant.m_uiOffset, 3, true), constant.m_uiArrayElements, typeNames[constant.m_Type]);
    }

    pLayout->m_Constants.PushBack(constant);
  }

  return pLayout;
}
