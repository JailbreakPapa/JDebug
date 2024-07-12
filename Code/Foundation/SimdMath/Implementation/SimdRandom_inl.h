#pragma once

// static
NS_FORCE_INLINE nsSimdVec4u nsSimdRandom::UInt(const nsSimdVec4i& vPosition, const nsSimdVec4u& vSeed /*= nsSimdVec4u::MakeZero()*/)
{
  // Based on Squirrel3 which was introduced by Squirrel Eiserloh at 'Math for Game Programmers: Noise-Based RNG', GDC17.
  const nsSimdVec4u BIT_NOISE1 = nsSimdVec4u(0xb5297a4d);
  const nsSimdVec4u BIT_NOISE2 = nsSimdVec4u(0x68e31da4);
  const nsSimdVec4u BIT_NOISE3 = nsSimdVec4u(0x1b56c4e9);

  nsSimdVec4u mangled = nsSimdVec4u(vPosition);
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
NS_ALWAYS_INLINE nsSimdVec4f nsSimdRandom::FloatZeroToOne(const nsSimdVec4i& vPosition, const nsSimdVec4u& vSeed /*= nsSimdVec4u::MakeZero()*/)
{
  return UInt(vPosition, vSeed).ToFloat() * (1.0f / 4294967296.0f);
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdRandom::FloatMinMax(const nsSimdVec4i& vPosition, const nsSimdVec4f& vMinValue, const nsSimdVec4f& vMaxValue, const nsSimdVec4u& vSeed /*= nsSimdVec4u::MakeZero()*/)
{
  return nsSimdVec4f::Lerp(vMinValue, vMaxValue, FloatZeroToOne(vPosition, vSeed));
}
