#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/ChunkStream.h>
#include <RendererCore/Utils/CoreRenderProfile.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCoreRenderProfileConfig, 1, nsRTTIDefaultAllocator<nsCoreRenderProfileConfig>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ShadowAtlasTextureSize", m_uiShadowAtlasTextureSize)->AddAttributes(new nsDefaultValueAttribute(4096), new nsClampValueAttribute(512, 8192)),
    NS_MEMBER_PROPERTY("MaxShadowMapSize", m_uiMaxShadowMapSize)->AddAttributes(new nsDefaultValueAttribute(1024), new nsClampValueAttribute(64, 4096)),
    NS_MEMBER_PROPERTY("MinShadowMapSize", m_uiMinShadowMapSize)->AddAttributes(new nsDefaultValueAttribute(64), new nsClampValueAttribute(8, 512)),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsCoreRenderProfileConfig::SaveRuntimeData(nsChunkStreamWriter& inout_stream) const
{
  inout_stream.BeginChunk("nsCoreRenderProfileConfig", 1);

  inout_stream << m_uiShadowAtlasTextureSize;
  inout_stream << m_uiMaxShadowMapSize;
  inout_stream << m_uiMinShadowMapSize;

  inout_stream.EndChunk();
}

void nsCoreRenderProfileConfig::LoadRuntimeData(nsChunkStreamReader& inout_stream)
{
  const auto& chunk = inout_stream.GetCurrentChunk();

  if (chunk.m_sChunkName == "nsCoreRenderProfileConfig" && chunk.m_uiChunkVersion >= 1)
  {
    inout_stream >> m_uiShadowAtlasTextureSize;
    inout_stream >> m_uiMaxShadowMapSize;
    inout_stream >> m_uiMinShadowMapSize;
  }
}


NS_STATICLINK_FILE(RendererCore, RendererCore_Utils_Implementation_CoreRenderProfile);
