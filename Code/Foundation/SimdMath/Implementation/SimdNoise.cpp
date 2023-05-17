#include <Foundation/FoundationPCH.h>

#include <Foundation/Math/Random.h>
#include <Foundation/SimdMath/SimdNoise.h>

wdSimdPerlinNoise::wdSimdPerlinNoise(wdUInt32 uiSeed)
{
  for (wdUInt32 i = 0; i < WD_ARRAY_SIZE(m_Permutations); ++i)
  {
    m_Permutations[i] = static_cast<wdUInt8>(i);
  }

  wdRandom rnd;
  rnd.Initialize(uiSeed);

  for (wdUInt32 i = WD_ARRAY_SIZE(m_Permutations) - 1; i > 0; --i)
  {
    wdUInt32 uiRandomIndex = rnd.UIntInRange(WD_ARRAY_SIZE(m_Permutations));
    wdMath::Swap(m_Permutations[i], m_Permutations[uiRandomIndex]);
  }
}

wdSimdVec4f wdSimdPerlinNoise::NoiseZeroToOne(const wdSimdVec4f& vX, const wdSimdVec4f& vY, const wdSimdVec4f& vZ, wdUInt32 uiNumOctaves /*= 1*/)
{
  wdSimdVec4f result = wdSimdVec4f::ZeroVector();
  wdSimdFloat amplitude = 1.0f;
  wdUInt32 uiOffset = 0;

  uiNumOctaves = wdMath::Max(uiNumOctaves, 1u);
  for (wdUInt32 i = 0; i < uiNumOctaves; ++i)
  {
    wdSimdFloat scale = static_cast<float>(WD_BIT(i));
    wdSimdVec4f offset = Permute(wdSimdVec4i(uiOffset) + wdSimdVec4i(0, 1, 2, 3)).ToFloat();
    wdSimdVec4f x = vX * scale + offset.Get<wdSwizzle::XXXX>();
    wdSimdVec4f y = vY * scale + offset.Get<wdSwizzle::YYYY>();
    wdSimdVec4f z = vZ * scale + offset.Get<wdSwizzle::ZZZZ>();

    result += Noise(x, y, z) * amplitude;

    amplitude *= 0.5f;
    uiOffset += 23;
  }

  return result * 0.5f + wdSimdVec4f(0.5f);
}

namespace
{
  WD_FORCE_INLINE wdSimdVec4f Fade(const wdSimdVec4f& t)
  {
    return t.CompMul(t).CompMul(t).CompMul(t.CompMul(t * 6.0f - wdSimdVec4f(15.0f)) + wdSimdVec4f(10.0f));
  }

  WD_FORCE_INLINE wdSimdVec4f Grad(wdSimdVec4i vHash, const wdSimdVec4f& x, const wdSimdVec4f& y, const wdSimdVec4f& z)
  {
    // convert low 4 bits of hash code into 12 gradient directions.
    const wdSimdVec4i h = vHash & wdSimdVec4i(15);
    const wdSimdVec4f u = wdSimdVec4f::Select(h < wdSimdVec4i(8), x, y);
    const wdSimdVec4f v = wdSimdVec4f::Select(h < wdSimdVec4i(4), y, wdSimdVec4f::Select(h == wdSimdVec4i(12) || h == wdSimdVec4i(14), x, z));
    return wdSimdVec4f::Select((h & wdSimdVec4i(1)) == wdSimdVec4i::ZeroVector(), u, -u) +
           wdSimdVec4f::Select((h & wdSimdVec4i(2)) == wdSimdVec4i::ZeroVector(), v, -v);
  }

  WD_ALWAYS_INLINE wdSimdVec4f Lerp(const wdSimdVec4f& t, const wdSimdVec4f& a, const wdSimdVec4f& b) { return wdSimdVec4f::Lerp(a, b, t); }

} // namespace

// reference: https://mrl.nyu.edu/~perlin/noise/
wdSimdVec4f wdSimdPerlinNoise::Noise(const wdSimdVec4f& inX, const wdSimdVec4f& inY, const wdSimdVec4f& inZ)
{
  wdSimdVec4f x = inX;
  wdSimdVec4f y = inY;
  wdSimdVec4f z = inZ;

  // find unit cube that contains point.
  const wdSimdVec4f xFloored = x.Floor();
  const wdSimdVec4f yFloored = y.Floor();
  const wdSimdVec4f zFloored = z.Floor();

  const wdSimdVec4i maxIndex = wdSimdVec4i(255);
  const wdSimdVec4i X = wdSimdVec4i::Truncate(xFloored) & maxIndex;
  const wdSimdVec4i Y = wdSimdVec4i::Truncate(yFloored) & maxIndex;
  const wdSimdVec4i Z = wdSimdVec4i::Truncate(zFloored) & maxIndex;

  // find relative x,y,z of point in cube.
  x -= xFloored;
  y -= yFloored;
  z -= zFloored;

  // compute fade curves for each of x,y,z.
  const wdSimdVec4f u = Fade(x);
  const wdSimdVec4f v = Fade(y);
  const wdSimdVec4f w = Fade(z);

  // hash coordinates of the 8 cube corners
  const wdSimdVec4i i1 = wdSimdVec4i(1);
  const wdSimdVec4i A = Permute(X) + Y;
  const wdSimdVec4i AA = Permute(A) + Z;
  const wdSimdVec4i AB = Permute(A + i1) + Z;
  const wdSimdVec4i B = Permute(X + i1) + Y;
  const wdSimdVec4i BA = Permute(B) + Z;
  const wdSimdVec4i BB = Permute(B + i1) + Z;

  const wdSimdVec4f f1 = wdSimdVec4f(1.0f);

  // and add blended results from 8 corners of cube.
  const wdSimdVec4f c000 = Grad(Permute(AA), x, y, z);
  const wdSimdVec4f c100 = Grad(Permute(BA), x - f1, y, z);
  const wdSimdVec4f c010 = Grad(Permute(AB), x, y - f1, z);
  const wdSimdVec4f c110 = Grad(Permute(BB), x - f1, y - f1, z);
  const wdSimdVec4f c001 = Grad(Permute(AA + i1), x, y, z - f1);
  const wdSimdVec4f c101 = Grad(Permute(BA + i1), x - f1, y, z - f1);
  const wdSimdVec4f c011 = Grad(Permute(AB + i1), x, y - f1, z - f1);
  const wdSimdVec4f c111 = Grad(Permute(BB + i1), x - f1, y - f1, z - f1);

  const wdSimdVec4f c000_c100 = Lerp(u, c000, c100);
  const wdSimdVec4f c010_c110 = Lerp(u, c010, c110);
  const wdSimdVec4f c001_c101 = Lerp(u, c001, c101);
  const wdSimdVec4f c011_c111 = Lerp(u, c011, c111);

  return Lerp(w, Lerp(v, c000_c100, c010_c110), Lerp(v, c001_c101, c011_c111));
}


WD_STATICLINK_FILE(Foundation, Foundation_SimdMath_Implementation_SimdNoise);
