#pragma once

#include <RendererCore/Pipeline/RenderData.h>

/// \brief This is the base class for types that handle rendering of different object types.
///
/// E.g. there are different renderers for meshes, particle effects, light sources, etc.
class NS_RENDERERCORE_DLL nsRenderer : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsRenderer, nsReflectedClass);

public:
  virtual void GetSupportedRenderDataTypes(nsHybridArray<const nsRTTI*, 8>& ref_types) const = 0;
  virtual void GetSupportedRenderDataCategories(nsHybridArray<nsRenderData::Category, 8>& ref_categories) const = 0;

  virtual void RenderBatch(const nsRenderViewContext& renderViewContext, const nsRenderPipelinePass* pPass, const nsRenderDataBatch& batch) const = 0;
};
