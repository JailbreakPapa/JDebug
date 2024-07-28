#pragma once

#include <RendererCore/Pipeline/Declarations.h>

class NS_RENDERERCORE_DLL nsFrameDataProviderBase : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsFrameDataProviderBase, nsReflectedClass);

protected:
  nsFrameDataProviderBase();

  virtual void* UpdateData(const nsRenderViewContext& renderViewContext, const nsExtractedRenderData& extractedData) = 0;

  void* GetData(const nsRenderViewContext& renderViewContext);

private:
  friend class nsRenderPipeline;

  const nsRenderPipeline* m_pOwnerPipeline = nullptr;
  void* m_pData = nullptr;
  nsUInt64 m_uiLastUpdateFrame = 0;
};

template <typename T>
class nsFrameDataProvider : public nsFrameDataProviderBase
{
public:
  T* GetData(const nsRenderViewContext& renderViewContext) { return static_cast<T*>(nsFrameDataProviderBase::GetData(renderViewContext)); }
};
