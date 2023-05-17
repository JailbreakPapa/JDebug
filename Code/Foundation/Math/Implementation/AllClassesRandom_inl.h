#pragma once

#include <Foundation/Math/Angle.h>
#include <Foundation/Math/Random.h>
#include <Foundation/Math/Vec3.h>

template <typename Type>
wdVec3Template<Type> wdVec3Template<Type>::CreateRandomPointInSphere(wdRandom& inout_rng)
{
  double px, py, pz;
  double len = 0.0;

  do
  {
    px = inout_rng.DoubleMinMax(-1, 1);
    py = inout_rng.DoubleMinMax(-1, 1);
    pz = inout_rng.DoubleMinMax(-1, 1);

    len = (px * px) + (py * py) + (pz * pz);
  } while (len > 1.0 || len <= 0.000001); // prevent the exact center

  return wdVec3Template<Type>((Type)px, (Type)py, (Type)pz);
}

template <typename Type>
wdVec3Template<Type> wdVec3Template<Type>::CreateRandomDirection(wdRandom& inout_rng)
{
  wdVec3Template<Type> vec = CreateRandomPointInSphere(inout_rng);
  vec.Normalize();
  return vec;
}

template <typename Type>
wdVec3Template<Type> wdVec3Template<Type>::CreateRandomDeviationX(wdRandom& inout_rng, const wdAngle& maxDeviation)
{
  const double twoPi = 2.0 * wdMath::Pi<double>();

  const double cosAngle = wdMath::Cos(maxDeviation);

  const double x = inout_rng.DoubleZeroToOneInclusive() * (1 - cosAngle) + cosAngle;
  const wdAngle phi = wdAngle::Radian((float)(inout_rng.DoubleZeroToOneInclusive() * twoPi));
  const double invSqrt = wdMath::Sqrt(1 - (x * x));
  const double y = invSqrt * wdMath::Cos(phi);
  const double z = invSqrt * wdMath::Sin(phi);

  return wdVec3Template<Type>((Type)x, (Type)y, (Type)z);
}

template <typename Type>
wdVec3Template<Type> wdVec3Template<Type>::CreateRandomDeviationY(wdRandom& inout_rng, const wdAngle& maxDeviation)
{
  wdVec3Template<Type> vec = CreateRandomDeviationX(inout_rng, maxDeviation);
  wdMath::Swap(vec.x, vec.y);
  return vec;
}

template <typename Type>
wdVec3Template<Type> wdVec3Template<Type>::CreateRandomDeviationZ(wdRandom& inout_rng, const wdAngle& maxDeviation)
{
  wdVec3Template<Type> vec = CreateRandomDeviationX(inout_rng, maxDeviation);
  wdMath::Swap(vec.x, vec.z);
  return vec;
}

template <typename Type>
wdVec3Template<Type> wdVec3Template<Type>::CreateRandomDeviation(wdRandom& inout_rng, const wdAngle& maxDeviation, const wdVec3Template<Type>& vNormal)
{
  // If you need to do this very often:
  // *** Pre-compute this once: ***

  // how to get from the X axis to our desired basis
  wdQuatTemplate<Type> qRotXtoDir;
  qRotXtoDir.SetShortestRotation(wdVec3Template<Type>(1, 0, 0), vNormal);

  // *** Then call this with the precomputed value as often as needed: ***

  // create a random vector along X
  wdVec3Template<Type> vec = CreateRandomDeviationX(inout_rng, maxDeviation);
  // rotate from X to our basis
  return qRotXtoDir * vec;
}
