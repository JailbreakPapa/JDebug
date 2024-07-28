#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/FrameDataProvider.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>

struct nsPerInstanceData;
class nsInstanceDataProvider;
class nsInstancedMeshComponent;

struct NS_RENDERERCORE_DLL nsInstanceData
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsInstanceData);

public:
  nsInstanceData(nsUInt32 uiMaxInstanceCount = 1024);
  ~nsInstanceData();

  nsGALBufferHandle m_hInstanceDataBuffer;

  nsConstantBufferStorageHandle m_hConstantBuffer;

  void BindResources(nsRenderContext* pRenderContext);

  nsArrayPtr<nsPerInstanceData> GetInstanceData(nsUInt32 uiCount, nsUInt32& out_uiOffset);
  void UpdateInstanceData(nsRenderContext* pRenderContext, nsUInt32 uiCount);

private:
  friend nsInstanceDataProvider;
  friend nsInstancedMeshComponent;

  void CreateBuffer(nsUInt32 uiSize);
  void Reset();

  nsUInt32 m_uiBufferSize = 0;
  nsUInt32 m_uiBufferOffset = 0;
  nsDynamicArray<nsPerInstanceData, nsAlignedAllocatorWrapper> m_PerInstanceData;
};

class NS_RENDERERCORE_DLL nsInstanceDataProvider : public nsFrameDataProvider<nsInstanceData>
{
  NS_ADD_DYNAMIC_REFLECTION(nsInstanceDataProvider, nsFrameDataProviderBase);

public:
  nsInstanceDataProvider();
  ~nsInstanceDataProvider();

private:
  virtual void* UpdateData(const nsRenderViewContext& renderViewContext, const nsExtractedRenderData& extractedData) override;

  nsInstanceData m_Data;
};
