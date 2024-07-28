#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/BakedProbes/BakingUtils.h>

nsVec3 nsBakingUtils::FibonacciSphere(nsUInt32 uiSampleIndex, nsUInt32 uiNumSamples)
{
  float offset = 2.0f / uiNumSamples;
  float increment = nsMath::Pi<float>() * (3.0f - nsMath::Sqrt(5.0f));

  float y = ((uiSampleIndex * offset) - 1) + (offset / 2);
  float r = nsMath::Sqrt(1 - y * y);

  nsAngle phi = nsAngle::MakeFromRadian(((uiSampleIndex + 1) % uiNumSamples) * increment);

  float x = nsMath::Cos(phi) * r;
  float z = nsMath::Sin(phi) * r;

  return nsVec3(x, y, z);
}

static nsUInt32 s_BitsPerDir[nsAmbientCubeBasis::NumDirs] = {5, 5, 5, 5, 6, 6};

nsCompressedSkyVisibility nsBakingUtils::CompressSkyVisibility(const nsAmbientCube<float>& skyVisibility)
{
  nsCompressedSkyVisibility result = 0;
  nsUInt32 uiOffset = 0;
  for (nsUInt32 i = 0; i < nsAmbientCubeBasis::NumDirs; ++i)
  {
    float maxValue = static_cast<float>((1u << s_BitsPerDir[i]) - 1u);
    nsUInt32 compressedDir = static_cast<nsUInt8>(nsMath::Saturate(skyVisibility.m_Values[i]) * maxValue + 0.5f);
    result |= (compressedDir << uiOffset);
    uiOffset += s_BitsPerDir[i];
  }

  return result;
}

void nsBakingUtils::DecompressSkyVisibility(nsCompressedSkyVisibility compressedSkyVisibility, nsAmbientCube<float>& out_skyVisibility)
{
  nsUInt32 uiOffset = 0;
  for (nsUInt32 i = 0; i < nsAmbientCubeBasis::NumDirs; ++i)
  {
    nsUInt32 maxValue = (1u << s_BitsPerDir[i]) - 1u;
    out_skyVisibility.m_Values[i] = static_cast<float>((compressedSkyVisibility >> uiOffset) & maxValue) * (1.0f / maxValue);
    uiOffset += s_BitsPerDir[i];
  }
}
