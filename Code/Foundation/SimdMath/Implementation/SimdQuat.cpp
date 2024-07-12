#include <Foundation/FoundationPCH.h>

#include <Foundation/SimdMath/SimdQuat.h>

nsSimdQuat nsSimdQuat::MakeShortestRotation(const nsSimdVec4f& vDirFrom, const nsSimdVec4f& vDirTo)
{
  const nsSimdVec4f v0 = vDirFrom.GetNormalized<3>();
  const nsSimdVec4f v1 = vDirTo.GetNormalized<3>();

  const nsSimdFloat fDot = v0.Dot<3>(v1);

  // if both vectors are identical -> no rotation needed
  if (fDot.IsEqual(1.0f, 0.0001f))
  {
    return nsSimdQuat::MakeIdentity();
  }
  else if (fDot.IsEqual(-1.0f, 0.0001f)) // if both vectors are opposing
  {
    return nsSimdQuat::MakeFromAxisAndAngle(v0.GetOrthogonalVector().GetNormalized<3>(), nsAngle::MakeFromRadian(nsMath::Pi<float>()));
  }

  const nsSimdVec4f c = v0.CrossRH(v1);
  const nsSimdFloat s = ((fDot + nsSimdFloat(1.0f)) * nsSimdFloat(2.0f)).GetSqrt();

  nsSimdQuat res;
  res.m_v = c / s;
  res.m_v.SetW(s * nsSimdFloat(0.5f));
  res.Normalize();
  return res;
}

nsSimdQuat nsSimdQuat::MakeSlerp(const nsSimdQuat& qFrom, const nsSimdQuat& qTo, const nsSimdFloat& t)
{
  NS_ASSERT_DEBUG((t >= 0.0f) && (t <= 1.0f), "Invalid lerp factor.");

  const nsSimdFloat one = 1.0f;
  const nsSimdFloat qdelta = 1.0f - 0.001f;

  const nsSimdFloat fDot = qFrom.m_v.Dot<4>(qTo.m_v);

  nsSimdFloat cosTheta = fDot;

  bool bFlipSign = false;
  if (cosTheta < 0.0f)
  {
    bFlipSign = true;
    cosTheta = -cosTheta;
  }

  nsSimdFloat t0, t1;

  if (cosTheta < qdelta)
  {
    nsAngle theta = nsMath::ACos(cosTheta);

    // use sqrtInv(1+c^2) instead of 1.0/sin(theta)
    const nsSimdFloat iSinTheta = (one - (cosTheta * cosTheta)).GetInvSqrt();
    const nsAngle tTheta = (float)t * theta;

    nsSimdFloat s0 = nsMath::Sin(theta - tTheta);
    nsSimdFloat s1 = nsMath::Sin(tTheta);

    t0 = s0 * iSinTheta;
    t1 = s1 * iSinTheta;
  }
  else
  {
    // If q0 is nearly the same as q1 we just linearly interpolate
    t0 = one - t;
    t1 = t;
  }

  if (bFlipSign)
    t1 = -t1;

  nsSimdQuat res;
  res.m_v = qFrom.m_v * t0 + qTo.m_v * t1;
  res.Normalize();
  return res;
}

bool nsSimdQuat::IsEqualRotation(const nsSimdQuat& qOther, const nsSimdFloat& fEpsilon) const
{
  nsSimdVec4f vA1, vA2;
  nsSimdFloat fA1, fA2;

  if (GetRotationAxisAndAngle(vA1, fA1) == NS_FAILURE)
    return false;
  if (qOther.GetRotationAxisAndAngle(vA2, fA2) == NS_FAILURE)
    return false;

  nsAngle A1 = nsAngle::MakeFromRadian(fA1);
  nsAngle A2 = nsAngle::MakeFromRadian(fA2);

  if ((A1.IsEqualSimple(A2, nsAngle::MakeFromDegree(fEpsilon))) && (vA1.IsEqual(vA2, fEpsilon).AllSet<3>()))
    return true;

  if ((A1.IsEqualSimple(-A2, nsAngle::MakeFromDegree(fEpsilon))) && (vA1.IsEqual(-vA2, fEpsilon).AllSet<3>()))
    return true;

  return false;
}
