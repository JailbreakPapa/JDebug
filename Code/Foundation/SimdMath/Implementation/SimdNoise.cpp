#include <Foundation/FoundationPCH.h>

#include <Foundation/Math/Random.h>
#include <Foundation/SimdMath/SimdNoise.h>

nsSimdPerlinNoise::nsSimdPerlinNoise(nsUInt32 uiSeed)
{
  for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(m_Permutations); ++i)
  {
    m_Permutations[i] = static_cast<nsUInt8>(i);
  }

  nsRandom rnd;
  rnd.Initialize(uiSeed);

  for (nsUInt32 i = NS_ARRAY_SIZE(m_Permutations) - 1; i > 0; --i)
  {
    nsUInt32 uiRandomIndex = rnd.UIntInRange(NS_ARRAY_SIZE(m_Permutations));
    nsMath::Swap(m_Permutations[i], m_Permutations[uiRandomIndex]);
  }
}

nsSimdVec4f nsSimdPerlinNoise::NoiseZeroToOne(const nsSimdVec4f& vX, const nsSimdVec4f& vY, const nsSimdVec4f& vZ, nsUInt32 uiNumOctaves /*= 1*/)
{
  nsSimdVec4f result = nsSimdVec4f::MakeZero();
  nsSimdFloat amplitude = 1.0f;
  nsUInt32 uiOffset = 0;

  uiNumOctaves = nsMath::Max(uiNumOctaves, 1u);
  for (nsUInt32 i = 0; i < uiNumOctaves; ++i)
  {
    nsSimdFloat scale = static_cast<float>(NS_BIT(i));
    nsSimdVec4f offset = Permute(nsSimdVec4i(uiOffset) + nsSimdVec4i(0, 1, 2, 3)).ToFloat();
    nsSimdVec4f x = vX * scale + offset.Get<nsSwizzle::XXXX>();
    nsSimdVec4f y = vY * scale + offset.Get<nsSwizzle::YYYY>();
    nsSimdVec4f z = vZ * scale + offset.Get<nsSwizzle::ZZZZ>();

    result += Noise(x, y, z) * amplitude;

    amplitude *= 0.5f;
    uiOffset += 23;
  }

  return result * 0.5f + nsSimdVec4f(0.5f);
}

namespace
{
  NS_FORCE_INLINE nsSimdVec4f Fade(const nsSimdVec4f& t)
  {
    return t.CompMul(t).CompMul(t).CompMul(t.CompMul(t * 6.0f - nsSimdVec4f(15.0f)) + nsSimdVec4f(10.0f));
  }

  NS_FORCE_INLINE nsSimdVec4f Grad(nsSimdVec4i vHash, const nsSimdVec4f& x, const nsSimdVec4f& y, const nsSimdVec4f& z)
  {
    // convert low 4 bits of hash code into 12 gradient directions.
    const nsSimdVec4i h = vHash & nsSimdVec4i(15);
    const nsSimdVec4f u = nsSimdVec4f::Select(h < nsSimdVec4i(8), x, y);
    const nsSimdVec4f v = nsSimdVec4f::Select(h < nsSimdVec4i(4), y, nsSimdVec4f::Select(h == nsSimdVec4i(12) || h == nsSimdVec4i(14), x, z));
    return nsSimdVec4f::Select((h & nsSimdVec4i(1)) == nsSimdVec4i::MakeZero(), u, -u) +
           nsSimdVec4f::Select((h & nsSimdVec4i(2)) == nsSimdVec4i::MakeZero(), v, -v);
  }

  NS_ALWAYS_INLINE nsSimdVec4f Lerp(const nsSimdVec4f& t, const nsSimdVec4f& a, const nsSimdVec4f& b)
  {
    return nsSimdVec4f::Lerp(a, b, t);
  }

} // namespace

// reference: https://mrl.nyu.edu/~perlin/noise/
nsSimdVec4f nsSimdPerlinNoise::Noise(const nsSimdVec4f& inX, const nsSimdVec4f& inY, const nsSimdVec4f& inZ)
{
  nsSimdVec4f x = inX;
  nsSimdVec4f y = inY;
  nsSimdVec4f z = inZ;

  // find unit cube that contains point.
  const nsSimdVec4f xFloored = x.Floor();
  const nsSimdVec4f yFloored = y.Floor();
  const nsSimdVec4f zFloored = z.Floor();

  const nsSimdVec4i maxIndex = nsSimdVec4i(255);
  const nsSimdVec4i X = nsSimdVec4i::Truncate(xFloored) & maxIndex;
  const nsSimdVec4i Y = nsSimdVec4i::Truncate(yFloored) & maxIndex;
  const nsSimdVec4i Z = nsSimdVec4i::Truncate(zFloored) & maxIndex;

  // find relative x,y,z of point in cube.
  x -= xFloored;
  y -= yFloored;
  z -= zFloored;

  // compute fade curves for each of x,y,z.
  const nsSimdVec4f u = Fade(x);
  const nsSimdVec4f v = Fade(y);
  const nsSimdVec4f w = Fade(z);

  // hash coordinates of the 8 cube corners
  const nsSimdVec4i i1 = nsSimdVec4i(1);
  const nsSimdVec4i A = Permute(X) + Y;
  const nsSimdVec4i AA = Permute(A) + Z;
  const nsSimdVec4i AB = Permute(A + i1) + Z;
  const nsSimdVec4i B = Permute(X + i1) + Y;
  const nsSimdVec4i BA = Permute(B) + Z;
  const nsSimdVec4i BB = Permute(B + i1) + Z;

  const nsSimdVec4f f1 = nsSimdVec4f(1.0f);

  // and add blended results from 8 corners of cube.
  const nsSimdVec4f c000 = Grad(Permute(AA), x, y, z);
  const nsSimdVec4f c100 = Grad(Permute(BA), x - f1, y, z);
  const nsSimdVec4f c010 = Grad(Permute(AB), x, y - f1, z);
  const nsSimdVec4f c110 = Grad(Permute(BB), x - f1, y - f1, z);
  const nsSimdVec4f c001 = Grad(Permute(AA + i1), x, y, z - f1);
  const nsSimdVec4f c101 = Grad(Permute(BA + i1), x - f1, y, z - f1);
  const nsSimdVec4f c011 = Grad(Permute(AB + i1), x, y - f1, z - f1);
  const nsSimdVec4f c111 = Grad(Permute(BB + i1), x - f1, y - f1, z - f1);

  const nsSimdVec4f c000_c100 = Lerp(u, c000, c100);
  const nsSimdVec4f c010_c110 = Lerp(u, c010, c110);
  const nsSimdVec4f c001_c101 = Lerp(u, c001, c101);
  const nsSimdVec4f c011_c111 = Lerp(u, c011, c111);

  return Lerp(w, Lerp(v, c000_c100, c010_c110), Lerp(v, c001_c101, c011_c111));
}
