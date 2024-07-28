#pragma once

#include <RendererCore/Shader/ShaderHelper.h>
#include <RendererCore/ShaderCompiler/Declarations.h>
#include <RendererFoundation/Shader/ShaderByteCode.h>

class nsPropertyAttribute;

class NS_RENDERERCORE_DLL nsShaderParser
{
public:
  struct AttributeDefinition
  {
    nsString m_sType;
    nsHybridArray<nsVariant, 8> m_Values;
  };

  struct ParameterDefinition
  {
    const nsRTTI* m_pType = nullptr;
    nsString m_sType;
    nsString m_sName;

    nsHybridArray<AttributeDefinition, 4> m_Attributes;
  };

  struct EnumValue
  {
    nsHashedString m_sValueName;
    nsInt32 m_iValueValue = 0;
  };

  struct EnumDefinition
  {
    nsString m_sName;
    nsUInt32 m_uiDefaultValue = 0;
    nsHybridArray<EnumValue, 16> m_Values;
  };

  static nsResult PreprocessSection(nsStreamReader& inout_stream, nsShaderHelper::nsShaderSections::Enum section, nsArrayPtr<nsString> customDefines, nsStringBuilder& out_sResult);

  static void ParseMaterialParameterSection(
    nsStreamReader& inout_stream, nsDynamicArray<ParameterDefinition>& out_parameter, nsDynamicArray<EnumDefinition>& out_enumDefinitions);

  static void ParsePermutationSection(
    nsStreamReader& inout_stream, nsDynamicArray<nsHashedString>& out_permVars, nsDynamicArray<nsPermutationVar>& out_fixedPermVars);
  static void ParsePermutationSection(
    nsStringView sPermutationSection, nsDynamicArray<nsHashedString>& out_permVars, nsDynamicArray<nsPermutationVar>& out_fixedPermVars);

  static void ParsePermutationVarConfig(nsStringView sPermutationVarConfig, nsVariant& out_defaultValue, EnumDefinition& out_enumDefinition);



  /// \brief Tries to find shader resource declarations inside the shader source.
  ///
  /// Used by the shader compiler implementations to generate resource mappings to sets/slots without creating conflicts across shader stages. For a list of supported resource declarations and possible pitfalls, please refer to https://nsengine.net/pages/docs/graphics/shaders/shader-resources.html.
  /// \param sShaderStageSource The shader source to parse.
  /// \param out_Resources The shader resources found inside the source.
  static void ParseShaderResources(nsStringView sShaderStageSource, nsDynamicArray<nsShaderResourceDefinition>& out_resources);

  /// \brief Delegate to creates a new declaration and register binding for a specific shader nsShaderResourceDefinition.
  /// \param sPlatform The platform for which the shader is being compiled. Will be one of the values returned by GetSupportedPlatforms.
  /// \param sDeclaration The shader resource declaration without any attributes, e.g. "Texture2D DiffuseTexture"
  /// \param binding The binding that needs to be set on the output out_sDeclaration.
  /// \param out_sDeclaration The new declaration that changes sDeclaration according to the provided 'binding', e.g. "Texture2D DiffuseTexture : register(t0, space5)"
  using CreateResourceDeclaration = nsDelegate<void(nsStringView, nsStringView, const nsShaderResourceBinding&, nsStringBuilder&)>;

  /// \brief Merges the shader resource bindings of all used shader stages.
  ///
  /// The function can fail if a shader resource of the same name has different signatures in two stages. E.g. the type, slot or set is different. Shader resources must be uniquely identified via name.
  /// \param spd The shader currently being processed.
  /// \param out_bindings A hashmap from shader resource name to shader resource binding. If a binding is used in multiple stages, nsShaderResourceBinding::m_Stages will be the combination of all used stages.
  /// \param pLog Log interface to write errors to.
  /// \return Returns failure if the shader stages could not be merged.
  static nsResult MergeShaderResourceBindings(const nsShaderProgramData& spd, nsHashTable<nsHashedString, nsShaderResourceBinding>& out_bindings, nsLogInterface* pLog);

  /// \brief Makes sure that bindings fulfills the basic requirements that nsEngine has for resource bindings in a shader, e.g. that each binding has a set / slot set.
  static nsResult SanityCheckShaderResourceBindings(const nsHashTable<nsHashedString, nsShaderResourceBinding>& bindings, nsLogInterface* pLog);

  /// \brief Creates a new shader source code that patches all shader resources to contain fixed set / slot bindings.
  /// \param sPlatform The platform for which the shader should be patched.
  /// \param sShaderStageSource The original shader source code that should be patched.
  /// \param resources A list of all shader resources that need to be patched within sShaderStageSource.
  /// \param bindings The binding information that each shader resource should have after patching. These bindings must have unique set / slots combinations for each resource.
  /// \param createDeclaration The callback to be called to generate the new shader resource declaration.
  /// \param out_shaderStageSource The new shader source code after patching.
  static void ApplyShaderResourceBindings(nsStringView sPlatform, nsStringView sShaderStageSource, const nsDynamicArray<nsShaderResourceDefinition>& resources, const nsHashTable<nsHashedString, nsShaderResourceBinding>& bindings, const CreateResourceDeclaration& createDeclaration, nsStringBuilder& out_sShaderStageSource);
};
