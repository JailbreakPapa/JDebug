#pragma once

#include <GameEngine/GameApplication/GameApplication.h>
#include <RendererCore/ShaderCompiler/PermutationGenerator.h>

class nsShaderCompilerApplication : public nsGameApplication
{
public:
  using SUPER = nsGameApplication;

  nsShaderCompilerApplication();

  virtual nsApplication::Execution Run() override;

private:
  void PrintConfig();
  nsResult CompileShader(nsStringView sShaderFile);
  nsResult ExtractPermutationVarValues(nsStringView sShaderFile);

  virtual nsResult BeforeCoreSystemsStartup() override;
  virtual void AfterCoreSystemsStartup() override;
  virtual void Init_LoadProjectPlugins() override {}
  virtual void Init_SetupDefaultResources() override {}
  virtual void Init_ConfigureInput() override {}
  virtual void Init_ConfigureTags() override {}
  virtual bool Run_ProcessApplicationInput() override { return true; }

  nsPermutationGenerator m_PermutationGenerator;
  nsString m_sPlatforms;
  nsString m_sShaderFiles;
  nsMap<nsString, nsHybridArray<nsString, 4>> m_FixedPermVars;
};
