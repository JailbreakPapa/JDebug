#pragma once

#include <Core/Configuration/PlatformProfile.h>
#include <Foundation/Configuration/CVar.h>
#include <RendererCore/RendererCoreDLL.h>

class NS_RENDERERCORE_DLL nsCoreRenderProfileConfig : public nsProfileConfigData
{
  NS_ADD_DYNAMIC_REFLECTION(nsCoreRenderProfileConfig, nsProfileConfigData);

public:
  virtual void SaveRuntimeData(nsChunkStreamWriter& inout_stream) const override;
  virtual void LoadRuntimeData(nsChunkStreamReader& inout_stream) override;

  nsUInt32 m_uiShadowAtlasTextureSize = 4096;
  nsUInt32 m_uiMaxShadowMapSize = 1024;
  nsUInt32 m_uiMinShadowMapSize = 64;
};

NS_RENDERERCORE_DLL extern nsCVarInt cvar_RenderingShadowsAtlasSize;
NS_RENDERERCORE_DLL extern nsCVarInt cvar_RenderingShadowsMaxShadowMapSize;
NS_RENDERERCORE_DLL extern nsCVarInt cvar_RenderingShadowsMinShadowMapSize;
