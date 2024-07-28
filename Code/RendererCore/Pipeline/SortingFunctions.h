#pragma once

#include <RendererCore/Pipeline/Declarations.h>

class NS_RENDERERCORE_DLL nsRenderSortingFunctions
{
public:
  static nsUInt64 ByRenderDataThenFrontToBack(const nsRenderData* pRenderData, const nsCamera& camera);
  static nsUInt64 BackToFrontThenByRenderData(const nsRenderData* pRenderData, const nsCamera& camera);
};
