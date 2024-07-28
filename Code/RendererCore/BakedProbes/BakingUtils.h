#pragma once

#include <Core/Graphics/AmbientCubeBasis.h>
#include <RendererCore/Declarations.h>

using nsCompressedSkyVisibility = nsUInt32;

namespace nsBakingUtils
{
  NS_RENDERERCORE_DLL nsVec3 FibonacciSphere(nsUInt32 uiSampleIndex, nsUInt32 uiNumSamples);

  NS_RENDERERCORE_DLL nsCompressedSkyVisibility CompressSkyVisibility(const nsAmbientCube<float>& skyVisibility);
  NS_RENDERERCORE_DLL void DecompressSkyVisibility(nsCompressedSkyVisibility compressedSkyVisibility, nsAmbientCube<float>& out_skyVisibility);
} // namespace nsBakingUtils
