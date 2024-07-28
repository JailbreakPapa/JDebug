#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/TypeVersionContext.h>
#include <RendererCore/Lights/Implementation/ClusteredDataUtils.h>
#include <RendererCore/Lights/SimplifiedDataExtractor.h>
#include <RendererCore/Pipeline/View.h>

//////////////////////////////////////////////////////////////////////////

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSimplifiedDataCPU, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

nsSimplifiedDataCPU::nsSimplifiedDataCPU() = default;
nsSimplifiedDataCPU::~nsSimplifiedDataCPU() = default;

//////////////////////////////////////////////////////////////////////////

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSimplifiedDataExtractor, 1, nsRTTIDefaultAllocator<nsSimplifiedDataExtractor>)
NS_END_DYNAMIC_REFLECTED_TYPE;

nsSimplifiedDataExtractor::nsSimplifiedDataExtractor(const char* szName)
  : nsExtractor(szName)
{
  m_DependsOn.PushBack(nsMakeHashedString("nsVisibleObjectsExtractor"));
}

nsSimplifiedDataExtractor::~nsSimplifiedDataExtractor() = default;

void nsSimplifiedDataExtractor::PostSortAndBatch(
  const nsView& view, const nsDynamicArray<const nsGameObject*>& visibleObjects, nsExtractedRenderData& ref_extractedRenderData)
{
  nsSimplifiedDataCPU* pData = NS_NEW(nsFrameAllocator::GetCurrentAllocator(), nsSimplifiedDataCPU);

  pData->m_uiSkyIrradianceIndex = view.GetWorld()->GetIndex();
  pData->m_cameraUsageHint = view.GetCameraUsageHint();

  ref_extractedRenderData.AddFrameData(pData);
}

nsResult nsSimplifiedDataExtractor::Serialize(nsStreamWriter& inout_stream) const
{
  NS_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  return NS_SUCCESS;
}

nsResult nsSimplifiedDataExtractor::Deserialize(nsStreamReader& inout_stream)
{
  NS_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  const nsUInt32 uiVersion = nsTypeVersionReadContext::GetContext()->GetTypeVersion(GetStaticRTTI());
  NS_IGNORE_UNUSED(uiVersion);
  return NS_SUCCESS;
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_SimplifiedDataExtractor);
