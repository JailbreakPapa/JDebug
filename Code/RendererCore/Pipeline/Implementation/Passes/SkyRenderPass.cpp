#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Pipeline/Passes/SkyRenderPass.h>

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSkyRenderPass, 1, nsRTTIDefaultAllocator<nsSkyRenderPass>)
NS_END_DYNAMIC_REFLECTED_TYPE;

nsSkyRenderPass::nsSkyRenderPass(const char* szName)
  : nsForwardRenderPass(szName)
{
}

nsSkyRenderPass::~nsSkyRenderPass() = default;

void nsSkyRenderPass::RenderObjects(const nsRenderViewContext& renderViewContext)
{
  RenderDataWithCategory(renderViewContext, nsDefaultRenderDataCategories::Sky);
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_SkyRenderPass);
