#pragma once

#include <Foundation/CodeUtils/Preprocessor.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/Reflection.h>
#include <RendererCore/Shader/ShaderPermutationBinary.h>
#include <RendererCore/ShaderCompiler/Declarations.h>
#include <RendererCore/ShaderCompiler/PermutationGenerator.h>
#include <RendererCore/ShaderCompiler/ShaderParser.h>
#include <RendererFoundation/Descriptors/Descriptors.h>

class nsRemoteMessage;

/// \brief Shader compiler interface.
/// Custom shader compiles need to derive from this class and implement the pure virtual interface functions. Instances are created via reflection so each implementation must be properly reflected.
class NS_RENDERERCORE_DLL nsShaderProgramCompiler : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsShaderProgramCompiler, nsReflectedClass);

public:
  /// \brief Returns the platforms that this shader compiler supports.
  /// \param out_platforms Filled with the platforms this compiler supports.
  virtual void GetSupportedPlatforms(nsHybridArray<nsString, 4>& out_platforms) = 0;

  /// Allows the shader compiler to modify the shader source before hashing and compiling. This allows it to implement custom features by injecting code before the compile process. Mostly used to define resource bindings that do not cause conflicts across shader stages.
  /// \param inout_data The state of the shader compiler. Only m_sShaderSource should be modified by the implementation.
  /// \param pLog Logging interface to be used when outputting any errors.
  /// \return Returns whether the shader could be modified. On failure, the shader won't be compiled.
  virtual nsResult ModifyShaderSource(nsShaderProgramData& inout_data, nsLogInterface* pLog) = 0;

  /// Compiles the shader comprised of multiple stages defined in inout_data.
  /// \param inout_data The state of the shader compiler. m_Resources and m_ByteCode should be written to on successful return code.
  /// \param pLog Logging interface to be used when outputting any errors.
  /// \return Returns whether the shader was compiled successfully. On failure, errors should be written to pLog.
  virtual nsResult Compile(nsShaderProgramData& inout_data, nsLogInterface* pLog) = 0;
};

class NS_RENDERERCORE_DLL nsShaderCompiler
{
public:
  nsResult CompileShaderPermutationForPlatforms(nsStringView sFile, const nsArrayPtr<const nsPermutationVar>& permutationVars, nsLogInterface* pLog, nsStringView sPlatform = "ALL");

private:
  nsResult RunShaderCompiler(nsStringView sFile, nsStringView sPlatform, nsShaderProgramCompiler* pCompiler, nsLogInterface* pLog);

  void WriteFailedShaderSource(nsShaderProgramData& spd, nsLogInterface* pLog);

  bool PassThroughUnknownCommandCB(nsStringView sCmd) { return sCmd == "version"; }

  void ShaderCompileMsg(nsRemoteMessage& msg);

  struct nsShaderData
  {
    nsString m_Platforms;
    nsHybridArray<nsPermutationVar, 16> m_Permutations;
    nsHybridArray<nsPermutationVar, 16> m_FixedPermVars;
    nsString m_StateSource;
    nsString m_ShaderStageSource[nsGALShaderStage::ENUM_COUNT];
  };

  nsResult FileOpen(nsStringView sAbsoluteFile, nsDynamicArray<nsUInt8>& FileContent, nsTimestamp& out_FileModification);

  nsStringBuilder m_StageSourceFile[nsGALShaderStage::ENUM_COUNT];

  nsTokenizedFileCache m_FileCache;
  nsShaderData m_ShaderData;

  nsSet<nsString> m_IncludeFiles;
  bool m_bCompilingShaderRemote = false;
  nsResult m_RemoteShaderCompileResult = NS_FAILURE;
};
