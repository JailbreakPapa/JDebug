#pragma once

#include <Core/ResourceManager/Resource.h>
#include <Foundation/Strings/HashedString.h>
#include <RendererCore/RendererCoreDLL.h>

using nsShaderResourceHandle = nsTypedResourceHandle<class nsShaderResource>;

struct nsShaderResourceDescriptor
{
};

class NS_RENDERERCORE_DLL nsShaderResource : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsShaderResource, nsResource);
  NS_RESOURCE_DECLARE_COMMON_CODE(nsShaderResource);
  NS_RESOURCE_DECLARE_CREATEABLE(nsShaderResource, nsShaderResourceDescriptor);

public:
  nsShaderResource();

  bool IsShaderValid() const { return m_bShaderResourceIsValid; }

  nsArrayPtr<const nsHashedString> GetUsedPermutationVars() const { return m_PermutationVarsUsed; }

private:
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

private:
  nsHybridArray<nsHashedString, 16> m_PermutationVarsUsed;
  bool m_bShaderResourceIsValid;
};
