#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/String.h>
#include <RendererCore/AdvancedShadows/VirtualShadowMapCore.h>
#include <RendererCore/AdvancedShadows/VirtualShadowMapPage.h>
#include <RendererCore/RendererCoreDLL.h>

NS_RENDERERCORE_DLL class nsVirtualShadowMapPageIndexer
{
public:
  nsVirtualTextureInfo VTInfo;
  nsUInt32 MIPCount;
  nsDynamicArray<nsUInt32> POffsets;
  nsDynamicArray<nsUInt32> PSizes;
  nsDynamicArray<nsVirtualShadowMapPage> ReverseFallback;

  nsString PageIndexer_Status;

  nsUInt32 PCount;
  nsUInt32 operator[](nsVirtualShadowMapPage i_Page);
  nsVirtualShadowMapPageIndexer(nsVirtualTextureInfo i_VTInfo);
  nsVirtualShadowMapPage GetPageFromIndex(int index) const;

  bool IsValid(const nsVirtualShadowMapPage& page);

  nsString GetPageIndexer_Status() const
  {
    return PageIndexer_Status;
  }

  nsUInt32 GetCount() const
  {
    return PCount;
  }
};
