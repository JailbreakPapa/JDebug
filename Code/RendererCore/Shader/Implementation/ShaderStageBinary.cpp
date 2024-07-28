#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <RendererCore/Shader/ShaderStageBinary.h>
#include <RendererCore/ShaderCompiler/ShaderManager.h>



//////////////////////////////////////////////////////////////////////////

nsMap<nsUInt32, nsShaderStageBinary> nsShaderStageBinary::s_ShaderStageBinaries[nsGALShaderStage::ENUM_COUNT];

nsShaderStageBinary::nsShaderStageBinary() = default;

nsShaderStageBinary::~nsShaderStageBinary()
{
  m_pGALByteCode = nullptr;
}

nsResult nsShaderStageBinary::Write(nsStreamWriter& inout_stream) const
{
  const nsUInt8 uiVersion = nsShaderStageBinary::VersionCurrent;

  // nsShaderStageBinary
  inout_stream << uiVersion;
  inout_stream << m_uiSourceHash;

  // nsGALShaderByteCode
  inout_stream << m_pGALByteCode->m_uiTessellationPatchControlPoints;
  inout_stream << m_pGALByteCode->m_Stage;
  inout_stream << m_pGALByteCode->m_bWasCompiledWithDebug;

  // m_ByteCode
  const nsUInt32 uiByteCodeSize = m_pGALByteCode->m_ByteCode.GetCount();
  inout_stream << uiByteCodeSize;
  if (!m_pGALByteCode->m_ByteCode.IsEmpty() && inout_stream.WriteBytes(&m_pGALByteCode->m_ByteCode[0], uiByteCodeSize).Failed())
    return NS_FAILURE;

  // m_ShaderResourceBindings
  const nsUInt16 uiResources = static_cast<nsUInt16>(m_pGALByteCode->m_ShaderResourceBindings.GetCount());
  inout_stream << uiResources;
  for (const auto& r : m_pGALByteCode->m_ShaderResourceBindings)
  {
    inout_stream << r.m_ResourceType;
    inout_stream << r.m_TextureType;
    inout_stream << r.m_Stages;
    inout_stream << r.m_iSet;
    inout_stream << r.m_iSlot;
    inout_stream << r.m_uiArraySize;
    inout_stream << r.m_sName.GetData();
    const bool bHasLayout = r.m_pLayout != nullptr;
    inout_stream << bHasLayout;
    if (bHasLayout)
    {
      NS_SUCCEED_OR_RETURN(Write(inout_stream, *r.m_pLayout));
    }
  }

  // m_ShaderVertexInput
  const nsUInt16 uiVertexInputs = static_cast<nsUInt16>(m_pGALByteCode->m_ShaderVertexInput.GetCount());
  inout_stream << uiVertexInputs;
  for (const auto& v : m_pGALByteCode->m_ShaderVertexInput)
  {
    inout_stream << v.m_eSemantic;
    inout_stream << v.m_eFormat;
    inout_stream << v.m_uiLocation;
  }

  return NS_SUCCESS;
}


nsResult nsShaderStageBinary::Write(nsStreamWriter& inout_stream, const nsShaderConstantBufferLayout& layout) const
{
  inout_stream << layout.m_uiTotalSize;

  nsUInt16 uiConstants = static_cast<nsUInt16>(layout.m_Constants.GetCount());
  inout_stream << uiConstants;

  for (auto& constant : layout.m_Constants)
  {
    inout_stream << constant.m_sName;
    inout_stream << constant.m_Type;
    inout_stream << constant.m_uiArrayElements;
    inout_stream << constant.m_uiOffset;
  }

  return NS_SUCCESS;
}

nsResult nsShaderStageBinary::Read(nsStreamReader& inout_stream)
{
  NS_ASSERT_DEBUG(m_pGALByteCode == nullptr, "");
  m_pGALByteCode = NS_DEFAULT_NEW(nsGALShaderByteCode);

  nsUInt8 uiVersion = 0;

  if (inout_stream.ReadBytes(&uiVersion, sizeof(nsUInt8)) != sizeof(nsUInt8))
    return NS_FAILURE;

  if (uiVersion < nsShaderStageBinary::Version::Version6)
  {
    nsLog::Error("Old shader binaries are not supported anymore and need to be recompiled, please delete shader cache.");
    return NS_FAILURE;
  }

  NS_ASSERT_DEV(uiVersion <= nsShaderStageBinary::VersionCurrent, "Wrong Version {0}", uiVersion);

  inout_stream >> m_uiSourceHash;

  // nsGALShaderByteCode
  if (uiVersion >= nsShaderStageBinary::Version::Version7)
  {
    inout_stream >> m_pGALByteCode->m_uiTessellationPatchControlPoints;
  }
  inout_stream >> m_pGALByteCode->m_Stage;
  inout_stream >> m_pGALByteCode->m_bWasCompiledWithDebug;

  // m_ByteCode
  {
    nsUInt32 uiByteCodeSize = 0;
    inout_stream >> uiByteCodeSize;
    m_pGALByteCode->m_ByteCode.SetCountUninitialized(uiByteCodeSize);
    if (!m_pGALByteCode->m_ByteCode.IsEmpty() && inout_stream.ReadBytes(&m_pGALByteCode->m_ByteCode[0], uiByteCodeSize) != uiByteCodeSize)
      return NS_FAILURE;
  }

  // m_ShaderResourceBindings
  {
    nsUInt16 uiResources = 0;
    inout_stream >> uiResources;

    m_pGALByteCode->m_ShaderResourceBindings.SetCount(uiResources);

    nsString sTemp;

    for (auto& r : m_pGALByteCode->m_ShaderResourceBindings)
    {
      inout_stream >> r.m_ResourceType;
      inout_stream >> r.m_TextureType;
      inout_stream >> r.m_Stages;
      inout_stream >> r.m_iSet;
      inout_stream >> r.m_iSlot;
      inout_stream >> r.m_uiArraySize;
      inout_stream >> sTemp;
      r.m_sName.Assign(sTemp.GetData());

      bool bHasLayout = false;
      inout_stream >> bHasLayout;

      if (bHasLayout)
      {
        r.m_pLayout = NS_DEFAULT_NEW(nsShaderConstantBufferLayout);
        NS_SUCCEED_OR_RETURN(Read(inout_stream, *r.m_pLayout));
      }
    }
  }

  // m_ShaderVertexInput
  {
    nsUInt16 uiVertexInputs = 0;
    inout_stream >> uiVertexInputs;
    m_pGALByteCode->m_ShaderVertexInput.SetCount(uiVertexInputs);

    for (auto& v : m_pGALByteCode->m_ShaderVertexInput)
    {
      inout_stream >> v.m_eSemantic;
      inout_stream >> v.m_eFormat;
      inout_stream >> v.m_uiLocation;
    }
  }

  return NS_SUCCESS;
}



nsResult nsShaderStageBinary::Read(nsStreamReader& inout_stream, nsShaderConstantBufferLayout& out_layout)
{
  inout_stream >> out_layout.m_uiTotalSize;

  nsUInt16 uiConstants = 0;
  inout_stream >> uiConstants;

  out_layout.m_Constants.SetCount(uiConstants);

  for (auto& constant : out_layout.m_Constants)
  {
    inout_stream >> constant.m_sName;
    inout_stream >> constant.m_Type;
    inout_stream >> constant.m_uiArrayElements;
    inout_stream >> constant.m_uiOffset;
  }

  return NS_SUCCESS;
}

nsSharedPtr<const nsGALShaderByteCode> nsShaderStageBinary::GetByteCode() const
{
  return m_pGALByteCode;
}

nsResult nsShaderStageBinary::WriteStageBinary(nsLogInterface* pLog, nsStringView sPlatform) const
{
  nsStringBuilder sShaderStageFile = nsShaderManager::GetCacheDirectory();

  sShaderStageFile.AppendPath(sPlatform);
  sShaderStageFile.AppendFormat("/{0}_{1}.nsShaderStage", nsGALShaderStage::Names[m_pGALByteCode->m_Stage], nsArgU(m_uiSourceHash, 8, true, 16, true));

  nsFileWriter StageFileOut;
  if (StageFileOut.Open(sShaderStageFile).Failed())
  {
    nsLog::Error(pLog, "Could not open shader stage file '{0}' for writing", sShaderStageFile);
    return NS_FAILURE;
  }

  if (Write(StageFileOut).Failed())
  {
    nsLog::Error(pLog, "Could not write shader stage file '{0}'", sShaderStageFile);
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

// static
nsShaderStageBinary* nsShaderStageBinary::LoadStageBinary(nsGALShaderStage::Enum Stage, nsUInt32 uiHash, nsStringView sPlatform)
{
  auto itStage = s_ShaderStageBinaries[Stage].Find(uiHash);

  if (!itStage.IsValid())
  {
    nsStringBuilder sShaderStageFile = nsShaderManager::GetCacheDirectory();

    sShaderStageFile.AppendPath(sPlatform);
    sShaderStageFile.AppendFormat("/{0}_{1}.nsShaderStage", nsGALShaderStage::Names[Stage], nsArgU(uiHash, 8, true, 16, true));

    nsFileReader StageFileIn;
    if (StageFileIn.Open(sShaderStageFile.GetData()).Failed())
    {
      nsLog::Debug("Could not open shader stage file '{0}' for reading", sShaderStageFile);
      return nullptr;
    }

    nsShaderStageBinary shaderStageBinary;
    if (shaderStageBinary.Read(StageFileIn).Failed())
    {
      nsLog::Error("Could not read shader stage file '{0}'", sShaderStageFile);
      return nullptr;
    }

    itStage = nsShaderStageBinary::s_ShaderStageBinaries[Stage].Insert(uiHash, shaderStageBinary);
  }

  nsShaderStageBinary* pShaderStageBinary = &itStage.Value();
  return pShaderStageBinary;
}

// static
void nsShaderStageBinary::OnEngineShutdown()
{
  for (nsUInt32 stage = 0; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
  {
    s_ShaderStageBinaries[stage].Clear();
  }
}
