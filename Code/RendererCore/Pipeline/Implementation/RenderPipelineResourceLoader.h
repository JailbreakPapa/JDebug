#pragma once

#include <Foundation/Serialization/RttiConverter.h>
#include <RendererCore/RendererCoreDLL.h>

class nsRenderPipeline;
struct nsRenderPipelineResourceDescriptor;
class nsStreamWriter;
class nsRenderPipelinePass;
class nsExtractor;

struct NS_RENDERERCORE_DLL nsRenderPipelineResourceLoaderConnection
{
  nsUInt32 m_uiSource;
  nsUInt32 m_uiTarget;
  nsString m_sSourcePin;
  nsString m_sTargetPin;

  nsResult Serialize(nsStreamWriter& inout_stream) const;
  nsResult Deserialize(nsStreamReader& inout_stream);
};
NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsRenderPipelineResourceLoaderConnection);

struct NS_RENDERERCORE_DLL nsRenderPipelineResourceLoader
{
  static nsInternal::NewInstance<nsRenderPipeline> CreateRenderPipeline(const nsRenderPipelineResourceDescriptor& desc);
  static void CreateRenderPipelineResourceDescriptor(const nsRenderPipeline* pPipeline, nsRenderPipelineResourceDescriptor& ref_desc);
  static nsResult ExportPipeline(nsArrayPtr<const nsRenderPipelinePass* const> passes, nsArrayPtr<const nsExtractor* const> extractors, nsArrayPtr<const nsRenderPipelineResourceLoaderConnection> connections, nsStreamWriter& ref_streamWriter);
};
