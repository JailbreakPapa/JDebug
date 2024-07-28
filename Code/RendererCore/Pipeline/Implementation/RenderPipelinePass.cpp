#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/TypeVersionContext.h>
#include <RendererCore/Pipeline/RenderPipeline.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Pipeline/Renderer.h>
#include <RendererCore/RenderContext/RenderContext.h>

#include <Foundation/IO/TypeVersionContext.h>
#include <RendererFoundation/Profiling/Profiling.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsRenderPipelinePass, 1, nsRTTINoAllocator)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Active", m_bActive)->AddAttributes(new nsDefaultValueAttribute(true)),
    NS_ACCESSOR_PROPERTY("Name", GetName, SetName),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Grape))
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsRenderPipelinePass::nsRenderPipelinePass(const char* szName, bool bIsStereoAware)
  : m_bIsStereoAware(bIsStereoAware)

{
  m_sName.Assign(szName);
}

nsRenderPipelinePass::~nsRenderPipelinePass() = default;

void nsRenderPipelinePass::SetName(const char* szName)
{
  if (!nsStringUtils::IsNullOrEmpty(szName))
  {
    m_sName.Assign(szName);
  }
}

const char* nsRenderPipelinePass::GetName() const
{
  return m_sName.GetData();
}

void nsRenderPipelinePass::InitRenderPipelinePass(const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) {}

void nsRenderPipelinePass::ExecuteInactive(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) {}

void nsRenderPipelinePass::ReadBackProperties(nsView* pView) {}

nsResult nsRenderPipelinePass::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream << m_bActive;
  inout_stream << m_sName;
  return NS_SUCCESS;
}

nsResult nsRenderPipelinePass::Deserialize(nsStreamReader& inout_stream)
{
  const nsUInt32 uiVersion = nsTypeVersionReadContext::GetContext()->GetTypeVersion(GetStaticRTTI());
  NS_ASSERT_DEBUG(uiVersion == 1, "Unknown version encountered");

  inout_stream >> m_bActive;
  inout_stream >> m_sName;
  return NS_SUCCESS;
}

void nsRenderPipelinePass::RenderDataWithCategory(const nsRenderViewContext& renderViewContext, nsRenderData::Category category, nsRenderDataBatch::Filter filter)
{
  NS_PROFILE_AND_MARKER(renderViewContext.m_pRenderContext->GetCommandEncoder(), nsRenderData::GetCategoryName(category));

  auto batchList = m_pPipeline->GetRenderDataBatchesWithCategory(category, filter);
  const nsUInt32 uiBatchCount = batchList.GetBatchCount();
  for (nsUInt32 i = 0; i < uiBatchCount; ++i)
  {
    const nsRenderDataBatch& batch = batchList.GetBatch(i);

    if (const nsRenderData* pRenderData = batch.GetFirstData<nsRenderData>())
    {
      const nsRTTI* pType = pRenderData->GetDynamicRTTI();

      if (const nsRenderer* pRenderer = nsRenderData::GetCategoryRenderer(category, pType))
      {
        pRenderer->RenderBatch(renderViewContext, this, batch);
      }
    }
  }
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_RenderPipelinePass);
