#include <RendererCore/RendererCorePCH.h>

#include <Core/Interfaces/RemoteToolingInterface.h>
#include <Foundation/Communication/RemoteInterface.h>
#include <Foundation/Configuration/Singleton.h>
#include <Foundation/IO/FileSystem/DeferredFileWriter.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Types/UniquePtr.h>
#include <RendererCore/ShaderCompiler/ShaderCompiler.h>
#include <RendererCore/ShaderCompiler/ShaderManager.h>
#include <RendererCore/ShaderCompiler/ShaderParser.h>

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsShaderProgramCompiler, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

namespace
{
  static bool PlatformEnabled(const nsString& sPlatforms, const char* szPlatform)
  {
    nsStringBuilder sTemp;
    sTemp = szPlatform;

    sTemp.Prepend("!");

    // if it contains '!platform'
    if (sPlatforms.FindWholeWord_NoCase(sTemp, nsStringUtils::IsIdentifierDelimiter_C_Code) != nullptr)
      return false;

    sTemp = szPlatform;

    // if it contains 'platform'
    if (sPlatforms.FindWholeWord_NoCase(sTemp, nsStringUtils::IsIdentifierDelimiter_C_Code) != nullptr)
      return true;

    // do not enable this when ALL is specified
    if (nsStringUtils::IsEqual(szPlatform, "DEBUG"))
      return false;

    // if it contains 'ALL'
    if (sPlatforms.FindWholeWord_NoCase("ALL", nsStringUtils::IsIdentifierDelimiter_C_Code) != nullptr)
      return true;

    return false;
  }

  static void GenerateDefines(const char* szPlatform, const nsArrayPtr<nsPermutationVar>& permutationVars, nsHybridArray<nsString, 32>& out_defines)
  {
    nsStringBuilder sTemp;

    if (out_defines.IsEmpty())
    {
      out_defines.PushBack("TRUE 1");
      out_defines.PushBack("FALSE 0");

      sTemp = szPlatform;
      sTemp.ToUpper();

      out_defines.PushBack(sTemp);
    }

    for (const nsPermutationVar& var : permutationVars)
    {
      const char* szValue = var.m_sValue;
      const bool isBoolVar = nsStringUtils::IsEqual(szValue, "TRUE") || nsStringUtils::IsEqual(szValue, "FALSE");

      if (isBoolVar)
      {
        sTemp.Set(var.m_sName, " ", var.m_sValue);
        out_defines.PushBack(sTemp);
      }
      else
      {
        const char* szName = var.m_sName;
        auto enumValues = nsShaderManager::GetPermutationEnumValues(var.m_sName);

        for (const auto& ev : enumValues)
        {
          sTemp.SetFormat("{1} {2}", szName, ev.m_sValueName, ev.m_iValueValue);
          out_defines.PushBack(sTemp);
        }

        if (nsStringUtils::StartsWith(szValue, szName))
        {
          sTemp.Set(szName, " ", szValue);
        }
        else
        {
          sTemp.Set(szName, " ", szName, "_", szValue);
        }
        out_defines.PushBack(sTemp);
      }
    }
  }

  static const char* s_szStageDefines[nsGALShaderStage::ENUM_COUNT] = {"VERTEX_SHADER", "HULL_SHADER", "DOMAIN_SHADER", "GEOMETRY_SHADER", "PIXEL_SHADER", "COMPUTE_SHADER"};
} // namespace

nsResult nsShaderCompiler::FileOpen(nsStringView sAbsoluteFile, nsDynamicArray<nsUInt8>& FileContent, nsTimestamp& out_FileModification)
{
  if (sAbsoluteFile == "ShaderRenderState")
  {
    const nsString& sData = m_ShaderData.m_StateSource;
    const nsUInt32 uiCount = sData.GetElementCount();
    nsStringView sString = sData;

    FileContent.SetCountUninitialized(uiCount);

    if (uiCount > 0)
    {
      nsMemoryUtils::Copy<nsUInt8>(FileContent.GetData(), (const nsUInt8*)sString.GetStartPointer(), uiCount);
    }

    return NS_SUCCESS;
  }

  for (nsUInt32 stage = 0; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
  {
    if (m_StageSourceFile[stage] == sAbsoluteFile)
    {
      const nsString& sData = m_ShaderData.m_ShaderStageSource[stage];
      const nsUInt32 uiCount = sData.GetElementCount();
      const char* szString = sData;

      FileContent.SetCountUninitialized(uiCount);

      if (uiCount > 0)
      {
        nsMemoryUtils::Copy<nsUInt8>(FileContent.GetData(), (const nsUInt8*)szString, uiCount);
      }

      return NS_SUCCESS;
    }
  }

  m_IncludeFiles.Insert(sAbsoluteFile);

  nsFileReader r;
  if (r.Open(sAbsoluteFile).Failed())
  {
    nsLog::Error("Could not find include file '{0}'", sAbsoluteFile);
    return NS_FAILURE;
  }

#if NS_ENABLED(NS_SUPPORTS_FILE_STATS)
  nsFileStats stats;
  if (nsFileSystem::GetFileStats(sAbsoluteFile, stats).Succeeded())
  {
    out_FileModification = stats.m_LastModificationTime;
  }
#endif

  nsUInt8 Temp[4096];

  while (nsUInt64 uiRead = r.ReadBytes(Temp, 4096))
  {
    FileContent.PushBackRange(nsArrayPtr<nsUInt8>(Temp, (nsUInt32)uiRead));
  }

  return NS_SUCCESS;
}

void nsShaderCompiler::ShaderCompileMsg(nsRemoteMessage& msg)
{
  if (msg.GetMessageID() == 'CRES')
  {
    m_bCompilingShaderRemote = false;
    m_RemoteShaderCompileResult = NS_SUCCESS;

    bool success = false;
    msg.GetReader() >> success;
    m_RemoteShaderCompileResult = success ? NS_SUCCESS : NS_FAILURE;

    nsStringBuilder log;
    msg.GetReader() >> log;

    if (!success)
    {
      nsLog::Error("Shader compilation failed:\n{}", log);
    }
  }
}

nsResult nsShaderCompiler::CompileShaderPermutationForPlatforms(nsStringView sFile, const nsArrayPtr<const nsPermutationVar>& permutationVars, nsLogInterface* pLog, nsStringView sPlatform)
{
  if (nsRemoteToolingInterface* pTooling = nsSingletonRegistry::GetSingletonInstance<nsRemoteToolingInterface>())
  {
    auto pNet = pTooling->GetRemoteInterface();

    if (pNet && pNet->IsConnectedToServer())
    {
      m_bCompilingShaderRemote = true;

      pNet->SetMessageHandler('SHDR', nsMakeDelegate(&nsShaderCompiler::ShaderCompileMsg, this));

      nsRemoteMessage msg('SHDR', 'CMPL');
      msg.GetWriter() << sFile;
      msg.GetWriter() << sPlatform;
      msg.GetWriter() << permutationVars.GetCount();
      for (auto& pv : permutationVars)
      {
        msg.GetWriter() << pv.m_sName;
        msg.GetWriter() << pv.m_sValue;
      }

      pNet->Send(nsRemoteTransmitMode::Reliable, msg);

      while (m_bCompilingShaderRemote)
      {
        pNet->UpdateRemoteInterface();
        pNet->ExecuteAllMessageHandlers();
      }

      pNet->SetMessageHandler('SHDR', {});

      return m_RemoteShaderCompileResult;
    }
  }

  nsStringBuilder sFileContent, sTemp;

  {
    nsFileReader File;
    if (File.Open(sFile).Failed())
      return NS_FAILURE;

    sFileContent.ReadAll(File);
  }

  nsShaderHelper::nsTextSectionizer Sections;
  nsShaderHelper::GetShaderSections(sFileContent, Sections);

  nsUInt32 uiFirstLine = 0;
  sTemp = Sections.GetSectionContent(nsShaderHelper::nsShaderSections::PLATFORMS, uiFirstLine);
  sTemp.ToUpper();

  m_ShaderData.m_Platforms = sTemp;

  nsHybridArray<nsHashedString, 16> usedPermutations;
  nsShaderParser::ParsePermutationSection(Sections.GetSectionContent(nsShaderHelper::nsShaderSections::PERMUTATIONS, uiFirstLine), usedPermutations, m_ShaderData.m_FixedPermVars);

  for (const nsHashedString& usedPermutationVar : usedPermutations)
  {
    nsUInt32 uiIndex = nsInvalidIndex;
    for (nsUInt32 i = 0; i < permutationVars.GetCount(); ++i)
    {
      if (permutationVars[i].m_sName == usedPermutationVar)
      {
        uiIndex = i;
        break;
      }
    }

    if (uiIndex != nsInvalidIndex)
    {
      m_ShaderData.m_Permutations.PushBack(permutationVars[uiIndex]);
    }
    else
    {
      nsLog::Error("No value given for permutation var '{0}'. Assuming default value of zero.", usedPermutationVar);

      nsPermutationVar& finalVar = m_ShaderData.m_Permutations.ExpandAndGetRef();
      finalVar.m_sName = usedPermutationVar;
      finalVar.m_sValue.Assign("0");
    }
  }

  m_ShaderData.m_StateSource = Sections.GetSectionContent(nsShaderHelper::nsShaderSections::RENDERSTATE, uiFirstLine);

  nsUInt32 uiFirstShaderLine = 0;
  nsStringView sShaderSource = Sections.GetSectionContent(nsShaderHelper::nsShaderSections::SHADER, uiFirstShaderLine);

  for (nsUInt32 stage = nsGALShaderStage::VertexShader; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
  {
    nsStringView sStageSource = Sections.GetSectionContent(nsShaderHelper::nsShaderSections::VERTEXSHADER + stage, uiFirstLine);

    // later code checks whether the string is empty, to see whether we have any shader source, so this has to be kept empty
    if (!sStageSource.IsEmpty())
    {
      sTemp.Clear();

      // prepend common shader section if there is any
      if (!sShaderSource.IsEmpty())
      {
        sTemp.AppendFormat("#line {0}\n{1}", uiFirstShaderLine, sShaderSource);
      }

      sTemp.AppendFormat("#line {0}\n{1}", uiFirstLine, sStageSource);

      m_ShaderData.m_ShaderStageSource[stage] = sTemp;
    }
    else
    {
      m_ShaderData.m_ShaderStageSource[stage].Clear();
    }
  }

  nsStringBuilder tmp = sFile;
  tmp.MakeCleanPath();

  m_StageSourceFile[nsGALShaderStage::VertexShader] = tmp;
  m_StageSourceFile[nsGALShaderStage::VertexShader].ChangeFileExtension("vs");

  m_StageSourceFile[nsGALShaderStage::HullShader] = tmp;
  m_StageSourceFile[nsGALShaderStage::HullShader].ChangeFileExtension("hs");

  m_StageSourceFile[nsGALShaderStage::DomainShader] = tmp;
  m_StageSourceFile[nsGALShaderStage::DomainShader].ChangeFileExtension("ds");

  m_StageSourceFile[nsGALShaderStage::GeometryShader] = tmp;
  m_StageSourceFile[nsGALShaderStage::GeometryShader].ChangeFileExtension("gs");

  m_StageSourceFile[nsGALShaderStage::PixelShader] = tmp;
  m_StageSourceFile[nsGALShaderStage::PixelShader].ChangeFileExtension("ps");

  m_StageSourceFile[nsGALShaderStage::ComputeShader] = tmp;
  m_StageSourceFile[nsGALShaderStage::ComputeShader].ChangeFileExtension("cs");

  // try out every compiler that we can find
  nsResult result = NS_SUCCESS;
  nsRTTI::ForEachDerivedType<nsShaderProgramCompiler>(
    [&](const nsRTTI* pRtti)
    {
      nsUniquePtr<nsShaderProgramCompiler> pCompiler = pRtti->GetAllocator()->Allocate<nsShaderProgramCompiler>();

      if (RunShaderCompiler(sFile, sPlatform, pCompiler.Borrow(), pLog).Failed())
        result = NS_FAILURE;
    },
    nsRTTI::ForEachOptions::ExcludeNonAllocatable);

  return result;
}

nsResult nsShaderCompiler::RunShaderCompiler(nsStringView sFile, nsStringView sPlatform, nsShaderProgramCompiler* pCompiler, nsLogInterface* pLog)
{
  NS_LOG_BLOCK(pLog, "Compiling Shader", sFile);

  nsStringBuilder sProcessed[nsGALShaderStage::ENUM_COUNT];

  nsHybridArray<nsString, 4> Platforms;
  pCompiler->GetSupportedPlatforms(Platforms);

  for (nsUInt32 p = 0; p < Platforms.GetCount(); ++p)
  {
    if (!PlatformEnabled(sPlatform, Platforms[p]))
      continue;

    // if this shader is not tagged for this platform, ignore it
    if (!PlatformEnabled(m_ShaderData.m_Platforms, Platforms[p]))
      continue;

    NS_LOG_BLOCK(pLog, "Platform", Platforms[p]);

    nsShaderProgramData spd;
    spd.m_sSourceFile = sFile;
    spd.m_sPlatform = Platforms[p];

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
    // 'DEBUG' is a platform tag that enables additional compiler flags
    if (PlatformEnabled(m_ShaderData.m_Platforms, "DEBUG"))
    {
      nsLog::Warning("Shader specifies the 'DEBUG' platform, which enables the debug shader compiler flag.");
      spd.m_Flags.Add(nsShaderCompilerFlags::Debug);
    }
#endif

    m_IncludeFiles.Clear();

    nsHybridArray<nsString, 32> defines;
    GenerateDefines(Platforms[p], m_ShaderData.m_Permutations, defines);
    GenerateDefines(Platforms[p], m_ShaderData.m_FixedPermVars, defines);

    nsShaderPermutationBinary shaderPermutationBinary;

    // Generate Shader State Source
    {
      NS_LOG_BLOCK(pLog, "Preprocessing Shader State Source");

      nsPreprocessor pp;
      pp.SetCustomFileCache(&m_FileCache);
      pp.SetLogInterface(nsLog::GetThreadLocalLogSystem());
      pp.SetFileOpenFunction(nsPreprocessor::FileOpenCB(&nsShaderCompiler::FileOpen, this));
      pp.SetPassThroughPragma(false);
      pp.SetPassThroughLine(false);

      for (auto& define : defines)
      {
        NS_SUCCEED_OR_RETURN(pp.AddCustomDefine(define));
      }

      bool bFoundUndefinedVars = false;
      pp.m_ProcessingEvents.AddEventHandler([&bFoundUndefinedVars](const nsPreprocessor::ProcessingEvent& e)
        {
        if (e.m_Type == nsPreprocessor::ProcessingEvent::EvaluateUnknown)
        {
          bFoundUndefinedVars = true;

          nsLog::Error("Undefined variable is evaluated: '{0}' (File: '{1}', Line: {2}", e.m_pToken->m_DataView, e.m_pToken->m_File, e.m_pToken->m_uiLine);
        } });

      nsStringBuilder sOutput;
      if (pp.Process("ShaderRenderState", sOutput, false).Failed() || bFoundUndefinedVars)
      {
        nsLog::Error(pLog, "Preprocessing the Shader State block failed");
        return NS_FAILURE;
      }
      else
      {
        if (shaderPermutationBinary.m_StateDescriptor.Parse(sOutput).Failed())
        {
          nsLog::Error(pLog, "Failed to interpret the shader state block");
          return NS_FAILURE;
        }
      }
    }

    // Shader Preprocessing
    for (nsUInt32 stage = nsGALShaderStage::VertexShader; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
    {
      spd.m_uiSourceHash[stage] = 0;

      if (m_ShaderData.m_ShaderStageSource[stage].IsEmpty())
        continue;

      bool bFoundUndefinedVars = false;

      nsPreprocessor pp;
      pp.SetCustomFileCache(&m_FileCache);
      pp.SetLogInterface(nsLog::GetThreadLocalLogSystem());
      pp.SetFileOpenFunction(nsPreprocessor::FileOpenCB(&nsShaderCompiler::FileOpen, this));
      pp.SetPassThroughPragma(true);
      pp.SetPassThroughUnknownCmdsCB(nsMakeDelegate(&nsShaderCompiler::PassThroughUnknownCommandCB, this));
      pp.SetPassThroughLine(false);
      pp.m_ProcessingEvents.AddEventHandler([&bFoundUndefinedVars](const nsPreprocessor::ProcessingEvent& e)
        {
        if (e.m_Type == nsPreprocessor::ProcessingEvent::EvaluateUnknown)
        {
          bFoundUndefinedVars = true;

          nsLog::Error("Undefined variable is evaluated: '{0}' (File: '{1}', Line: {2}", e.m_pToken->m_DataView, e.m_pToken->m_File, e.m_pToken->m_uiLine);
        } });

      NS_SUCCEED_OR_RETURN(pp.AddCustomDefine(s_szStageDefines[stage]));
      for (auto& define : defines)
      {
        NS_SUCCEED_OR_RETURN(pp.AddCustomDefine(define));
      }

      if (pp.Process(m_StageSourceFile[stage], sProcessed[stage], true, true, true).Failed() || bFoundUndefinedVars)
      {
        sProcessed[stage].Clear();
        spd.m_sShaderSource[stage] = m_StageSourceFile[stage];

        nsLog::Error(pLog, "Shader preprocessing failed");
        return NS_FAILURE;
      }
      else
      {
        spd.m_sShaderSource[stage] = sProcessed[stage];
      }
    }

    // Let the shader compiler make any modifications to the source code before we hash and compile the shader.
    if (pCompiler->ModifyShaderSource(spd, pLog).Failed())
    {
      WriteFailedShaderSource(spd, pLog);
      return NS_FAILURE;
    }

    // Load shader cache
    for (nsUInt32 stage = nsGALShaderStage::VertexShader; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
    {
      nsUInt32 uiSourceStringLen = spd.m_sShaderSource[stage].GetElementCount();
      spd.m_uiSourceHash[stage] = uiSourceStringLen == 0 ? 0u : nsHashingUtils::xxHash32(spd.m_sShaderSource[stage].GetData(), uiSourceStringLen);

      if (spd.m_uiSourceHash[stage] != 0)
      {
        nsShaderStageBinary* pBinary = nsShaderStageBinary::LoadStageBinary((nsGALShaderStage::Enum)stage, spd.m_uiSourceHash[stage], sPlatform);

        if (pBinary)
        {
          spd.m_ByteCode[stage] = pBinary->m_pGALByteCode;
          spd.m_bWriteToDisk[stage] = false;
        }
        else
        {
          // Can't find shader with given hash on disk, create a new nsGALShaderByteCode and let the compiler build it.
          spd.m_ByteCode[stage] = NS_DEFAULT_NEW(nsGALShaderByteCode);
          spd.m_ByteCode[stage]->m_Stage = (nsGALShaderStage::Enum)stage;
          spd.m_ByteCode[stage]->m_bWasCompiledWithDebug = spd.m_Flags.IsSet(nsShaderCompilerFlags::Debug);
        }
      }
    }

    // copy the source hashes
    for (nsUInt32 stage = nsGALShaderStage::VertexShader; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
    {
      shaderPermutationBinary.m_uiShaderStageHashes[stage] = spd.m_uiSourceHash[stage];
    }

    // if compilation failed, the stage binary for the source hash will simply not exist and therefore cannot be loaded
    // the .nsPermutation file should be updated, however, to store the new source hash to the broken shader
    if (pCompiler->Compile(spd, nsLog::GetThreadLocalLogSystem()).Failed())
    {
      WriteFailedShaderSource(spd, pLog);
      return NS_FAILURE;
    }

    for (nsUInt32 stage = nsGALShaderStage::VertexShader; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
    {
      if (spd.m_uiSourceHash[stage] != 0 && spd.m_bWriteToDisk[stage])
      {
        nsShaderStageBinary bin;
        bin.m_uiSourceHash = spd.m_uiSourceHash[stage];
        bin.m_pGALByteCode = spd.m_ByteCode[stage];

        if (bin.WriteStageBinary(pLog, sPlatform).Failed())
        {
          nsLog::Error(pLog, "Writing stage {0} binary failed", stage);
          return NS_FAILURE;
        }
        nsShaderStageBinary::s_ShaderStageBinaries[stage].Insert(bin.m_uiSourceHash, bin);
      }
    }

    nsStringBuilder sTemp = nsShaderManager::GetCacheDirectory();
    sTemp.AppendPath(Platforms[p]);
    sTemp.AppendPath(sFile);
    sTemp.ChangeFileExtension("");
    if (sTemp.EndsWith("."))
      sTemp.Shrink(0, 1);

    const nsUInt32 uiPermutationHash = nsShaderHelper::CalculateHash(m_ShaderData.m_Permutations);
    sTemp.AppendFormat("_{0}.nsPermutation", nsArgU(uiPermutationHash, 8, true, 16, true));

    shaderPermutationBinary.m_DependencyFile.Clear();
    shaderPermutationBinary.m_DependencyFile.AddFileDependency(sFile);

    for (auto it = m_IncludeFiles.GetIterator(); it.IsValid(); ++it)
    {
      shaderPermutationBinary.m_DependencyFile.AddFileDependency(it.Key());
    }

    shaderPermutationBinary.m_PermutationVars = m_ShaderData.m_Permutations;

    nsDeferredFileWriter PermutationFileOut;
    PermutationFileOut.SetOutput(sTemp);
    NS_SUCCEED_OR_RETURN(shaderPermutationBinary.Write(PermutationFileOut));

    if (PermutationFileOut.Close().Failed())
    {
      nsLog::Error(pLog, "Could not open file for writing: '{0}'", sTemp);
      return NS_FAILURE;
    }
  }

  return NS_SUCCESS;
}


void nsShaderCompiler::WriteFailedShaderSource(nsShaderProgramData& spd, nsLogInterface* pLog)
{
  for (nsUInt32 stage = nsGALShaderStage::VertexShader; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
  {
    if (spd.m_uiSourceHash[stage] != 0 && spd.m_bWriteToDisk[stage])
    {
      nsStringBuilder sShaderStageFile = nsShaderManager::GetCacheDirectory();

      sShaderStageFile.AppendPath(nsShaderManager::GetActivePlatform());
      sShaderStageFile.AppendFormat("/_Failed_{0}_{1}.nsShaderSource", nsGALShaderStage::Names[stage], nsArgU(spd.m_uiSourceHash[stage], 8, true, 16, true));

      nsFileWriter StageFileOut;
      if (StageFileOut.Open(sShaderStageFile).Succeeded())
      {
        StageFileOut.WriteBytes(spd.m_sShaderSource[stage].GetData(), spd.m_sShaderSource[stage].GetElementCount()).AssertSuccess();
        nsLog::Info(pLog, "Failed shader source written to '{0}'", sShaderStageFile);
      }
    }
  }
}

NS_STATICLINK_FILE(RendererCore, RendererCore_ShaderCompiler_Implementation_ShaderCompiler);
