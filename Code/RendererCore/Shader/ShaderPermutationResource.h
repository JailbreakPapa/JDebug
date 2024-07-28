#pragma once

#include <Core/ResourceManager/Resource.h>
#include <Core/ResourceManager/ResourceTypeLoader.h>
#include <Foundation/Time/Timestamp.h>
#include <RendererCore/RendererCoreDLL.h>
#include <RendererCore/Shader/ShaderPermutationBinary.h>
#include <RendererCore/ShaderCompiler/PermutationGenerator.h>

using nsShaderPermutationResourceHandle = nsTypedResourceHandle<class nsShaderPermutationResource>;
using nsShaderStateResourceHandle = nsTypedResourceHandle<class nsShaderStateResource>;

struct nsShaderPermutationResourceDescriptor
{
};

class NS_RENDERERCORE_DLL nsShaderPermutationResource : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsShaderPermutationResource, nsResource);
  NS_RESOURCE_DECLARE_COMMON_CODE(nsShaderPermutationResource);
  NS_RESOURCE_DECLARE_CREATEABLE(nsShaderPermutationResource, nsShaderPermutationResourceDescriptor);

public:
  nsShaderPermutationResource();

  nsGALShaderHandle GetGALShader() const { return m_hShader; }
  const nsGALShaderByteCode* GetShaderByteCode(nsGALShaderStage::Enum stage) const { return m_ByteCodes[stage]; }

  nsGALBlendStateHandle GetBlendState() const { return m_hBlendState; }
  nsGALDepthStencilStateHandle GetDepthStencilState() const { return m_hDepthStencilState; }
  nsGALRasterizerStateHandle GetRasterizerState() const { return m_hRasterizerState; }

  bool IsShaderValid() const { return m_bShaderPermutationValid; }

  nsArrayPtr<const nsPermutationVar> GetPermutationVars() const { return m_PermutationVars; }

private:
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;
  virtual nsResourceTypeLoader* GetDefaultResourceTypeLoader() const override;

private:
  friend class nsShaderManager;

  nsSharedPtr<const nsGALShaderByteCode> m_ByteCodes[nsGALShaderStage::ENUM_COUNT];

  bool m_bShaderPermutationValid;
  nsGALShaderHandle m_hShader;

  nsGALBlendStateHandle m_hBlendState;
  nsGALDepthStencilStateHandle m_hDepthStencilState;
  nsGALRasterizerStateHandle m_hRasterizerState;

  nsHybridArray<nsPermutationVar, 16> m_PermutationVars;
};


class nsShaderPermutationResourceLoader : public nsResourceTypeLoader
{
public:
  virtual nsResourceLoadData OpenDataStream(const nsResource* pResource) override;
  virtual void CloseDataStream(const nsResource* pResource, const nsResourceLoadData& loaderData) override;

  virtual bool IsResourceOutdated(const nsResource* pResource) const override;

private:
  nsResult RunCompiler(const nsResource* pResource, nsShaderPermutationBinary& BinaryInfo, bool bForce);
};
