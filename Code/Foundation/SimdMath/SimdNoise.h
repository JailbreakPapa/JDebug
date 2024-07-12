#pragma once

#include <Foundation/SimdMath/SimdVec4i.h>

class NS_FOUNDATION_DLL nsSimdPerlinNoise
{
public:
  nsSimdPerlinNoise(nsUInt32 uiSeed);

  nsSimdVec4f NoiseZeroToOne(const nsSimdVec4f& x, const nsSimdVec4f& y, const nsSimdVec4f& z, nsUInt32 uiNumOctaves = 1);

private:
  nsSimdVec4f Noise(const nsSimdVec4f& x, const nsSimdVec4f& y, const nsSimdVec4f& z);

  NS_FORCE_INLINE nsSimdVec4i Permute(const nsSimdVec4i& v)
  {
#if 0
    nsArrayPtr<nsUInt8> p = nsMakeArrayPtr(m_Permutations);
#else
    nsUInt8* p = m_Permutations;
#endif

    nsSimdVec4i i = v & nsSimdVec4i(NS_ARRAY_SIZE(m_Permutations) - 1);
    return nsSimdVec4i(p[i.x()], p[i.y()], p[i.z()], p[i.w()]);
  }

  nsUInt8 m_Permutations[256];
};
