#include <RendererCore/RendererCorePCH.h>

#include <Foundation/CodeUtils/Preprocessor.h>
#include <RendererCore/Shader/ShaderHelper.h>
#include <RendererCore/Shader/ShaderPermutationResource.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererCore/ShaderCompiler/ShaderManager.h>
#include <RendererCore/ShaderCompiler/ShaderParser.h>

bool nsShaderManager::s_bEnableRuntimeCompilation = false;
nsString nsShaderManager::s_sPlatform;
nsString nsShaderManager::s_sPermVarSubDir;
nsString nsShaderManager::s_sShaderCacheDirectory;

namespace
{
  struct PermutationVarConfig
  {
    nsHashedString m_sName;
    nsVariant m_DefaultValue;
    nsDynamicArray<nsShaderParser::EnumValue, nsStaticsAllocatorWrapper> m_EnumValues;
  };

  static nsDeque<PermutationVarConfig, nsStaticsAllocatorWrapper> s_PermutationVarConfigsStorage;
  static nsHashTable<nsHashedString, PermutationVarConfig*> s_PermutationVarConfigs;
  static nsMutex s_PermutationVarConfigsMutex;

  const PermutationVarConfig* FindConfig(const char* szName, const nsTempHashedString& sHashedName)
  {
    NS_LOCK(s_PermutationVarConfigsMutex);

    PermutationVarConfig* pConfig = nullptr;
    if (!s_PermutationVarConfigs.TryGetValue(sHashedName, pConfig))
    {
      nsShaderManager::ReloadPermutationVarConfig(szName, sHashedName);
      s_PermutationVarConfigs.TryGetValue(sHashedName, pConfig);
    }

    return pConfig;
  }

  const PermutationVarConfig* FindConfig(const nsHashedString& sName)
  {
    NS_LOCK(s_PermutationVarConfigsMutex);

    PermutationVarConfig* pConfig = nullptr;
    if (!s_PermutationVarConfigs.TryGetValue(sName, pConfig))
    {
      nsShaderManager::ReloadPermutationVarConfig(sName.GetData(), sName);
      s_PermutationVarConfigs.TryGetValue(sName, pConfig);
    }

    return pConfig;
  }

  static nsHashedString s_sTrue = nsMakeHashedString("TRUE");
  static nsHashedString s_sFalse = nsMakeHashedString("FALSE");

  bool IsValueAllowed(const PermutationVarConfig& config, const nsTempHashedString& sValue, nsHashedString& out_sValue)
  {
    if (config.m_DefaultValue.IsA<bool>())
    {
      if (sValue == s_sTrue)
      {
        out_sValue = s_sTrue;
        return true;
      }

      if (sValue == s_sFalse)
      {
        out_sValue = s_sFalse;
        return true;
      }
    }
    else
    {
      for (auto& enumValue : config.m_EnumValues)
      {
        if (enumValue.m_sValueName == sValue)
        {
          out_sValue = enumValue.m_sValueName;
          return true;
        }
      }
    }

    return false;
  }

  bool IsValueAllowed(const PermutationVarConfig& config, const nsTempHashedString& sValue)
  {
    if (config.m_DefaultValue.IsA<bool>())
    {
      return sValue == s_sTrue || sValue == s_sFalse;
    }
    else
    {
      for (auto& enumValue : config.m_EnumValues)
      {
        if (enumValue.m_sValueName == sValue)
          return true;
      }
    }

    return false;
  }

  static nsHashTable<nsUInt64, nsString> s_PermutationPaths;
} // namespace

//////////////////////////////////////////////////////////////////////////

void nsShaderManager::Configure(const char* szActivePlatform, bool bEnableRuntimeCompilation, const char* szShaderCacheDirectory, const char* szPermVarSubDirectory)
{
  nsStringBuilder s = szActivePlatform;
  s.ToUpper();
  s_sPlatform = s;
  s_bEnableRuntimeCompilation = bEnableRuntimeCompilation;
  s_sShaderCacheDirectory = szShaderCacheDirectory;
  s_sPermVarSubDir = szPermVarSubDirectory;
}

void nsShaderManager::ReloadPermutationVarConfig(const char* szName, const nsTempHashedString& sHashedName)
{
  // clear earlier data
  {
    NS_LOCK(s_PermutationVarConfigsMutex);

    s_PermutationVarConfigs.Remove(sHashedName);
  }

  nsStringBuilder sPath;
  sPath.SetFormat("{0}/{1}.nsPermVar", s_sPermVarSubDir, szName);

  nsStringBuilder sTemp = s_sPlatform;
  sTemp.Append(" 1");

  nsPreprocessor pp;
  pp.SetLogInterface(nsLog::GetThreadLocalLogSystem());
  pp.SetPassThroughLine(false);
  pp.SetPassThroughPragma(false);
  pp.AddCustomDefine(sTemp.GetData()).IgnoreResult();

  if (pp.Process(sPath, sTemp, false).Failed())
  {
    nsLog::Error("Could not read shader permutation variable '{0}' from file '{1}'", szName, sPath);
  }

  nsVariant defaultValue;
  nsShaderParser::EnumDefinition enumDef;

  nsShaderParser::ParsePermutationVarConfig(sTemp, defaultValue, enumDef);
  if (defaultValue.IsValid())
  {
    NS_LOCK(s_PermutationVarConfigsMutex);

    auto pConfig = &s_PermutationVarConfigsStorage.ExpandAndGetRef();
    pConfig->m_sName.Assign(szName);
    pConfig->m_DefaultValue = defaultValue;
    pConfig->m_EnumValues = enumDef.m_Values;

    s_PermutationVarConfigs.Insert(pConfig->m_sName, pConfig);
  }
}

bool nsShaderManager::IsPermutationValueAllowed(const char* szName, const nsTempHashedString& sHashedName, const nsTempHashedString& sValue, nsHashedString& out_sName, nsHashedString& out_sValue)
{
  const PermutationVarConfig* pConfig = FindConfig(szName, sHashedName);
  if (pConfig == nullptr)
  {
    nsLog::Error("Permutation variable '{0}' does not exist", szName);
    return false;
  }

  out_sName = pConfig->m_sName;

  if (!IsValueAllowed(*pConfig, sValue, out_sValue))
  {
    if (!s_bEnableRuntimeCompilation)
    {
      return false;
    }

    nsLog::Debug("Invalid Shader Permutation: '{0}' cannot be set to value '{1}' -> reloading config for variable", szName, sValue.GetHash());
    ReloadPermutationVarConfig(szName, sHashedName);

    if (!IsValueAllowed(*pConfig, sValue, out_sValue))
    {
      nsLog::Error("Invalid Shader Permutation: '{0}' cannot be set to value '{1}'", szName, sValue.GetHash());
      return false;
    }
  }

  return true;
}

bool nsShaderManager::IsPermutationValueAllowed(const nsHashedString& sName, const nsHashedString& sValue)
{
  const PermutationVarConfig* pConfig = FindConfig(sName);
  if (pConfig == nullptr)
  {
    nsLog::Error("Permutation variable '{0}' does not exist", sName);
    return false;
  }

  if (!IsValueAllowed(*pConfig, sValue))
  {
    if (!s_bEnableRuntimeCompilation)
    {
      return false;
    }

    nsLog::Debug("Invalid Shader Permutation: '{0}' cannot be set to value '{1}' -> reloading config for variable", sName, sValue);
    ReloadPermutationVarConfig(sName, sName);

    if (!IsValueAllowed(*pConfig, sValue))
    {
      nsLog::Error("Invalid Shader Permutation: '{0}' cannot be set to value '{1}'", sName, sValue);
      return false;
    }
  }

  return true;
}

void nsShaderManager::GetPermutationValues(const nsHashedString& sName, nsDynamicArray<nsHashedString>& out_values)
{
  out_values.Clear();

  const PermutationVarConfig* pConfig = FindConfig(sName);
  if (pConfig == nullptr)
    return;

  if (pConfig->m_DefaultValue.IsA<bool>())
  {
    out_values.PushBack(s_sTrue);
    out_values.PushBack(s_sFalse);
  }
  else
  {
    for (const auto& val : pConfig->m_EnumValues)
    {
      out_values.PushBack(val.m_sValueName);
    }
  }
}

nsArrayPtr<const nsShaderParser::EnumValue> nsShaderManager::GetPermutationEnumValues(const nsHashedString& sName)
{
  const PermutationVarConfig* pConfig = FindConfig(sName);
  if (pConfig != nullptr)
  {
    return pConfig->m_EnumValues;
  }

  return {};
}

void nsShaderManager::PreloadPermutations(nsShaderResourceHandle hShader, const nsHashTable<nsHashedString, nsHashedString>& permVars, nsTime shouldBeAvailableIn)
{
  NS_ASSERT_NOT_IMPLEMENTED;
#if 0
  nsResourceLock<nsShaderResource> pShader(hShader, nsResourceAcquireMode::BlockTillLoaded);

  if (!pShader->IsShaderValid())
    return;

  /*nsUInt32 uiPermutationHash = */ FilterPermutationVars(pShader->GetUsedPermutationVars(), permVars);

  generator.RemoveUnusedPermutations(pShader->GetUsedPermutationVars());

  nsHybridArray<nsPermutationVar, 16> usedPermVars;

  const nsUInt32 uiPermutationCount = generator.GetPermutationCount();
  for (nsUInt32 uiPermutation = 0; uiPermutation < uiPermutationCount; ++uiPermutation)
  {
    generator.GetPermutation(uiPermutation, usedPermVars);

    PreloadSingleShaderPermutation(hShader, usedPermVars, tShouldBeAvailableIn);
  }
#endif
}

nsShaderPermutationResourceHandle nsShaderManager::PreloadSinglePermutation(nsShaderResourceHandle hShader, const nsHashTable<nsHashedString, nsHashedString>& permVars, bool bAllowFallback)
{
  nsResourceLock<nsShaderResource> pShader(hShader, bAllowFallback ? nsResourceAcquireMode::AllowLoadingFallback : nsResourceAcquireMode::BlockTillLoaded);

  if (!pShader->IsShaderValid())
    return nsShaderPermutationResourceHandle();

  nsHybridArray<nsPermutationVar, 64> filteredPermutationVariables(nsFrameAllocator::GetCurrentAllocator());
  nsUInt32 uiPermutationHash = FilterPermutationVars(pShader->GetUsedPermutationVars(), permVars, filteredPermutationVariables);

  return PreloadSinglePermutationInternal(pShader->GetResourceID(), pShader->GetResourceIDHash(), uiPermutationHash, filteredPermutationVariables);
}


nsUInt32 nsShaderManager::FilterPermutationVars(nsArrayPtr<const nsHashedString> usedVars, const nsHashTable<nsHashedString, nsHashedString>& permVars, nsDynamicArray<nsPermutationVar>& out_FilteredPermutationVariables)
{
  for (auto& sName : usedVars)
  {
    auto& var = out_FilteredPermutationVariables.ExpandAndGetRef();
    var.m_sName = sName;

    if (!permVars.TryGetValue(sName, var.m_sValue))
    {
      const PermutationVarConfig* pConfig = FindConfig(sName);
      if (pConfig == nullptr)
        continue;

      const nsVariant& defaultValue = pConfig->m_DefaultValue;
      if (defaultValue.IsA<bool>())
      {
        var.m_sValue = defaultValue.Get<bool>() ? s_sTrue : s_sFalse;
      }
      else
      {
        nsUInt32 uiDefaultValue = defaultValue.Get<nsUInt32>();
        var.m_sValue = pConfig->m_EnumValues[uiDefaultValue].m_sValueName;
      }
    }
  }

  return nsShaderHelper::CalculateHash(out_FilteredPermutationVariables);
}



nsShaderPermutationResourceHandle nsShaderManager::PreloadSinglePermutationInternal(const char* szResourceId, nsUInt64 uiResourceIdHash, nsUInt32 uiPermutationHash, nsArrayPtr<nsPermutationVar> filteredPermutationVariables)
{
  const nsUInt64 uiPermutationKey = (nsUInt64)nsHashingUtils::StringHashTo32(uiResourceIdHash) << 32 | uiPermutationHash;

  nsString* pPermutationPath = &s_PermutationPaths[uiPermutationKey];
  if (pPermutationPath->IsEmpty())
  {
    nsStringBuilder sShaderFile = GetCacheDirectory();
    sShaderFile.AppendPath(GetActivePlatform().GetData());
    sShaderFile.AppendPath(szResourceId);
    sShaderFile.ChangeFileExtension("");
    if (sShaderFile.EndsWith("."))
      sShaderFile.Shrink(0, 1);
    sShaderFile.AppendFormat("_{0}.nsPermutation", nsArgU(uiPermutationHash, 8, true, 16, true));

    *pPermutationPath = sShaderFile;
  }

  nsShaderPermutationResourceHandle hShaderPermutation = nsResourceManager::LoadResource<nsShaderPermutationResource>(pPermutationPath->GetData());

  {
    nsResourceLock<nsShaderPermutationResource> pShaderPermutation(hShaderPermutation, nsResourceAcquireMode::PointerOnly);
    if (!pShaderPermutation->IsShaderValid())
    {
      pShaderPermutation->m_PermutationVars = filteredPermutationVariables;
    }
  }

  nsResourceManager::PreloadResource(hShaderPermutation);

  return hShaderPermutation;
}
