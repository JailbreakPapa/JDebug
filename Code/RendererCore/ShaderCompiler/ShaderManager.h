#pragma once

#include <Foundation/Containers/HashTable.h>
#include <RendererCore/Declarations.h>
#include <RendererCore/ShaderCompiler/PermutationGenerator.h>
#include <RendererCore/ShaderCompiler/ShaderParser.h>

class NS_RENDERERCORE_DLL nsShaderManager
{
public:
  static void Configure(const char* szActivePlatform, bool bEnableRuntimeCompilation, const char* szShaderCacheDirectory = ":shadercache/ShaderCache",
    const char* szPermVarSubDirectory = "Shaders/PermutationVars");
  static const nsString& GetPermutationVarSubDirectory() { return s_sPermVarSubDir; }
  static const nsString& GetActivePlatform() { return s_sPlatform; }
  static const nsString& GetCacheDirectory() { return s_sShaderCacheDirectory; }
  static bool IsRuntimeCompilationEnabled() { return s_bEnableRuntimeCompilation; }

  static void ReloadPermutationVarConfig(const char* szName, const nsTempHashedString& sHashedName);
  static bool IsPermutationValueAllowed(const char* szName, const nsTempHashedString& sHashedName, const nsTempHashedString& sValue,
    nsHashedString& out_sName, nsHashedString& out_sValue);
  static bool IsPermutationValueAllowed(const nsHashedString& sName, const nsHashedString& sValue);

  /// \brief If the given permutation variable is an enum variable, this returns the possible values.
  /// Returns an empty array for other types of permutation variables.
  static nsArrayPtr<const nsShaderParser::EnumValue> GetPermutationEnumValues(const nsHashedString& sName);

  /// \brief Same as GetPermutationEnumValues() but also returns values for other types of variables.
  /// E.g. returns TRUE and FALSE for boolean variables.
  static void GetPermutationValues(const nsHashedString& sName, nsDynamicArray<nsHashedString>& out_values);

  static void PreloadPermutations(
    nsShaderResourceHandle hShader, const nsHashTable<nsHashedString, nsHashedString>& permVars, nsTime shouldBeAvailableIn);
  static nsShaderPermutationResourceHandle PreloadSinglePermutation(
    nsShaderResourceHandle hShader, const nsHashTable<nsHashedString, nsHashedString>& permVars, bool bAllowFallback);

private:
  static nsUInt32 FilterPermutationVars(nsArrayPtr<const nsHashedString> usedVars, const nsHashTable<nsHashedString, nsHashedString>& permVars,
    nsDynamicArray<nsPermutationVar>& out_FilteredPermutationVariables);
  static nsShaderPermutationResourceHandle PreloadSinglePermutationInternal(
    const char* szResourceId, nsUInt64 uiResourceIdHash, nsUInt32 uiPermutationHash, nsArrayPtr<nsPermutationVar> filteredPermutationVariables);

  static bool s_bEnableRuntimeCompilation;
  static nsString s_sPlatform;
  static nsString s_sPermVarSubDir;
  static nsString s_sShaderCacheDirectory;
};
