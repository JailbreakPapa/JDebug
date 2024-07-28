#include <RendererCore/AdvancedShadows/VirtualShadowMapPageIndexer.h>

nsUInt32 nsVirtualShadowMapPageIndexer::operator[](nsVirtualShadowMapPage i_Page)
{
  return POffsets[i_Page.Page_MIP] + i_Page.Page_Y * PSizes[i_Page.Page_MIP] + i_Page.Page_X;
}

nsVirtualShadowMapPageIndexer::nsVirtualShadowMapPageIndexer(nsVirtualTextureInfo i_VTInfo)
{
  VTInfo = i_VTInfo;
  MIPCount = nsMath::Log2((float)VTInfo.PageSize) + 1;
  PSizes.Reserve(MIPCount);
  for (int i = 0; i < MIPCount; ++i)
    PSizes[i] = (i_VTInfo.VTSize / i_VTInfo.TileSize) >> i;

  POffsets.Reserve(MIPCount);
  PCount = 0;
  for (int i = 0; i < MIPCount; ++i)
  {
    POffsets[i] = PCount;
    PCount += PSizes[i] * PSizes[i];
  }

  // Calculate reverse mapping
  ReverseFallback.Reserve(PCount);
  for (int i = 0; i < MIPCount; ++i)
  {
    int size = PSizes[i];
    for (int y = 0; y < size; ++y)
      for (int x = 0; x < size; ++x)
      {
        nsVirtualShadowMapPage page = nsVirtualShadowMapPage(x, y, i);
        ReverseFallback[(*this)[page]] = page;
      }
  }
}

nsVirtualShadowMapPage nsVirtualShadowMapPageIndexer::GetPageFromIndex(int index) const
{
  return ReverseFallback[index];
}

bool nsVirtualShadowMapPageIndexer::IsValid(const nsVirtualShadowMapPage& page)
{
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  if (page.Page_MIP < 0)
  {
    nsLog::Warning("Mip level smaller than zero {0}.", page.Page_MIP);
    return false;
  }
  else if (page.Page_MIP >= MIPCount)
  {
    nsLog::Warning("Mip level is larger than the max! {0} {1}.", page.Page_MIP, page.Page_MIP);
    return false;
  }

  if (page.Page_X < 0)
  {
    nsLog::Warning("X smaller than zero: {0}", page.Page_X);
    return false;
  }
  else if (page.Page_X >= PSizes[page.Page_MIP])
  {
    nsLog::Warning("X larger than max {0},{1}", (PSizes[page.Page_MIP]), page.Page_X);
    return false;
  }

  if (page.Page_Y < 0)
  {
    nsLog::Warning("Y smaller than zero: {0}", page.Page_Y);
    return false;
  }
  else if (page.Page_Y >= PSizes[page.Page_MIP])
  {
    nsLog::Warning("Y larger than max {0},{1}", (PSizes[page.Page_MIP]), page.Page_Y);
    return false;
  }
#endif
  return true;
}
