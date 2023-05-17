#pragma once

// static
WD_FORCE_INLINE wdSimdVec4u wdSimdRandom::UInt(const wdSimdVec4i& vPosition, const wdSimdVec4u& vSeed /*= wdSimdVec4u::ZeroVector()*/)
{
  // Based on Squirrel3 which was introduced by Squirrel Eiserloh at 'Math for Game Programmers: Noise-Based RNG', GDC17.
  const wdSimdVec4u BIT_NOISE1 = wdSimdVec4u(0xb5297a4d);
  const wdSimdVec4u BIT_NOISE2 = wdSimdVec4u(0x68e31da4);
  const wdSimdVec4u BIT_NOISE3 = wdSimdVec4u(0x1b56c4e9);

  wdSimdVec4u mangled = wdSimdVec4u(vPosition);
  mangled = mangled.CompMul(BIT_NOISE1);
  mangled += vSeed;
  mangled ^= (mangled >> 8);
  mangled += BIT_NOISE2;
  mangled ^= (mangled << 8);
  mangled = mangled.CompMul(BIT_NOISE3);
  mangled ^= (mangled >> 8);

  return mangled;
}

// static
WD_ALWAYS_INLINE wdSimdVec4f wdSimdRandom::FloatZeroToOne(const wdSimdVec4i& vPosition, const wdSimdVec4u& vSeed /*= wdSimdVec4u::ZeroVector()*/)
{
  return UInt(vPosition, vSeed).ToFloat() * (1.0f / 4294967296.0f);
}

// static
WD_ALWAYS_INLINE wdSimdVec4f wdSimdRandom::FloatMinMax(const wdSimdVec4i& vPosition, const wdSimdVec4f& vMinValue, const wdSimdVec4f& vMaxValue, const wdSimdVec4u& vSeed /*= wdSimdVec4u::ZeroVector()*/)
{
  return wdSimdVec4f::Lerp(vMinValue, vMaxValue, FloatZeroToOne(vPosition, vSeed));
}
