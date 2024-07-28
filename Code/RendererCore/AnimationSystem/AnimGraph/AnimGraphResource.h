#pragma once

#include <RendererCore/RendererCoreDLL.h>

#include <Core/ResourceManager/Resource.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Memory/InstanceDataAllocator.h>
#include <Foundation/Types/UniquePtr.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>

class nsAnimGraphInstance;
class nsAnimGraphNode;

//////////////////////////////////////////////////////////////////////////

using nsAnimGraphResourceHandle = nsTypedResourceHandle<class nsAnimGraphResource>;

struct NS_RENDERERCORE_DLL nsAnimationClipMapping : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimationClipMapping, nsReflectedClass);

  nsHashedString m_sClipName;
  nsAnimationClipResourceHandle m_hClip;

  const char* GetClipName() const { return m_sClipName.GetData(); }
  void SetClipName(const char* szName) { m_sClipName.Assign(szName); }

  const char* GetClip() const;
  void SetClip(const char* szName);
};

class NS_RENDERERCORE_DLL nsAnimGraphResource : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimGraphResource, nsResource);
  NS_RESOURCE_DECLARE_COMMON_CODE(nsAnimGraphResource);

public:
  nsAnimGraphResource();
  ~nsAnimGraphResource();

  const nsAnimGraph& GetAnimationGraph() const { return m_AnimGraph; }

  nsArrayPtr<const nsString> GetIncludeGraphs() const { return m_IncludeGraphs; }
  const nsDynamicArray<nsAnimationClipMapping>& GetAnimationClipMapping() const { return m_AnimationClipMapping; }

private:
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

  nsDynamicArray<nsString> m_IncludeGraphs;
  nsDynamicArray<nsAnimationClipMapping> m_AnimationClipMapping;
  nsAnimGraph m_AnimGraph;
};
