#pragma once

#include <Foundation/Math/Angle.h>
#include <Foundation/Math/Random.h>
#include <Foundation/Math/Vec3.h>

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE nsVec3Template<Type> nsVec3Template<Type>::MakeRandomPointInSphere(nsRandom& inout_rng)
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

  return nsVec3Template<Type>((Type)px, (Type)py, (Type)pz);
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE nsVec3Template<Type> nsVec3Template<Type>::MakeRandomDirection(nsRandom& inout_rng)
{
  nsVec3Template<Type> vec = MakeRandomPointInSphere(inout_rng);
  vec.Normalize();
  return vec;
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE nsVec3Template<Type> nsVec3Template<Type>::MakeRandomDeviationX(nsRandom& inout_rng, const nsAngle& maxDeviation)
{
  const double twoPi = 2.0 * nsMath::Pi<double>();

  const double cosAngle = nsMath::Cos(maxDeviation);

  const double x = inout_rng.DoubleZeroToOneInclusive() * (1 - cosAngle) + cosAngle;
  const nsAngle phi = nsAngle::MakeFromRadian((float)(inout_rng.DoubleZeroToOneInclusive() * twoPi));
  const double invSqrt = nsMath::Sqrt(1 - (x * x));
  const double y = invSqrt * nsMath::Cos(phi);
  const double z = invSqrt * nsMath::Sin(phi);

  return nsVec3Template<Type>((Type)x, (Type)y, (Type)z);
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE nsVec3Template<Type> nsVec3Template<Type>::MakeRandomDeviationY(nsRandom& inout_rng, const nsAngle& maxDeviation)
{
  nsVec3Template<Type> vec = MakeRandomDeviationX(inout_rng, maxDeviation);
  nsMath::Swap(vec.x, vec.y);
  return vec;
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE nsVec3Template<Type> nsVec3Template<Type>::MakeRandomDeviationZ(nsRandom& inout_rng, const nsAngle& maxDeviation)
{
  nsVec3Template<Type> vec = MakeRandomDeviationX(inout_rng, maxDeviation);
  nsMath::Swap(vec.x, vec.z);
  return vec;
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE nsVec3Template<Type> nsVec3Template<Type>::MakeRandomDeviation(nsRandom& inout_rng, const nsAngle& maxDeviation, const nsVec3Template<Type>& vNormal)
{
  // If you need to do this very often:
  // *** Pre-compute this once: ***

  // how to get from the X axis to our desired basis
  nsQuatTemplate<Type> qRotXtoDir = nsQuat::MakeShortestRotation(nsVec3Template<Type>(1, 0, 0), vNormal);

  // *** Then call this with the precomputed value as often as needed: ***

  // create a random vector along X
  nsVec3Template<Type> vec = MakeRandomDeviationX(inout_rng, maxDeviation);
  // rotate from X to our basis
  return qRotXtoDir * vec;
}
