#pragma once

#include <Foundation/SimdMath/SimdVec4i.h>

class WD_FOUNDATION_DLL wdSimdPerlinNoise
{
public:
  wdSimdPerlinNoise(wdUInt32 uiSeed);

  wdSimdVec4f NoiseZeroToOne(const wdSimdVec4f& x, const wdSimdVec4f& y, const wdSimdVec4f& z, wdUInt32 uiNumOctaves = 1);

private:
  wdSimdVec4f Noise(const wdSimdVec4f& x, const wdSimdVec4f& y, const wdSimdVec4f& z);

  WD_FORCE_INLINE wdSimdVec4i Permute(const wdSimdVec4i& v)
  {
#if 0
    wdArrayPtr<wdUInt8> p = wdMakeArrayPtr(m_Permutations);
#else
    wdUInt8* p = m_Permutations;
#endif

    wdSimdVec4i i = v & wdSimdVec4i(WD_ARRAY_SIZE(m_Permutations) - 1);
    return wdSimdVec4i(p[i.x()], p[i.y()], p[i.z()], p[i.w()]);
  }

  wdUInt8 m_Permutations[256];
};
