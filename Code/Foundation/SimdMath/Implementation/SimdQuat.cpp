#include <Foundation/FoundationPCH.h>

#include <Foundation/SimdMath/SimdQuat.h>

///\todo optimize these methods if needed

void wdSimdQuat::SetShortestRotation(const wdSimdVec4f& vDirFrom, const wdSimdVec4f& vDirTo)
{
  const wdSimdVec4f v0 = vDirFrom.GetNormalized<3>();
  const wdSimdVec4f v1 = vDirTo.GetNormalized<3>();

  const wdSimdFloat fDot = v0.Dot<3>(v1);

  // if both vectors are identical -> no rotation needed
  if (fDot.IsEqual(1.0f, 0.0001f))
  {
    SetIdentity();
    return;
  }
  else if (fDot.IsEqual(-1.0f, 0.0001f)) // if both vectors are opposing
  {
    SetFromAxisAndAngle(v0.GetOrthogonalVector().GetNormalized<3>(), wdAngle::Radian(wdMath::Pi<float>()));
    return;
  }

  const wdSimdVec4f c = v0.CrossRH(v1);
  const wdSimdFloat s = ((fDot + wdSimdFloat(1.0f)) * wdSimdFloat(2.0f)).GetSqrt();

  m_v = c / s;
  m_v.SetW(s * wdSimdFloat(0.5f));

  Normalize();
}

void wdSimdQuat::SetSlerp(const wdSimdQuat& qFrom, const wdSimdQuat& qTo, const wdSimdFloat& t)
{
  WD_ASSERT_DEBUG((t >= 0.0f) && (t <= 1.0f), "Invalid lerp factor.");

  const wdSimdFloat one = 1.0f;
  const wdSimdFloat qdelta = 1.0f - 0.001f;

  const wdSimdFloat fDot = qFrom.m_v.Dot<4>(qTo.m_v);

  wdSimdFloat cosTheta = fDot;

  bool bFlipSign = false;
  if (cosTheta < 0.0f)
  {
    bFlipSign = true;
    cosTheta = -cosTheta;
  }

  wdSimdFloat t0, t1;

  if (cosTheta < qdelta)
  {
    wdAngle theta = wdMath::ACos(cosTheta);

    // use sqrtInv(1+c^2) instead of 1.0/sin(theta)
    const wdSimdFloat iSinTheta = (one - (cosTheta * cosTheta)).GetInvSqrt();
    const wdAngle tTheta = (float)t * theta;

    wdSimdFloat s0 = wdMath::Sin(theta - tTheta);
    wdSimdFloat s1 = wdMath::Sin(tTheta);

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

  m_v = qFrom.m_v * t0 + qTo.m_v * t1;

  Normalize();
}

bool wdSimdQuat::IsEqualRotation(const wdSimdQuat& qOther, const wdSimdFloat& fEpsilon) const
{
  wdSimdVec4f vA1, vA2;
  wdSimdFloat fA1, fA2;

  if (GetRotationAxisAndAngle(vA1, fA1) == WD_FAILURE)
    return false;
  if (qOther.GetRotationAxisAndAngle(vA2, fA2) == WD_FAILURE)
    return false;

  wdAngle A1 = wdAngle::Radian(fA1);
  wdAngle A2 = wdAngle::Radian(fA2);

  if ((A1.IsEqualSimple(A2, wdAngle::Degree(fEpsilon))) && (vA1.IsEqual(vA2, fEpsilon).AllSet<3>()))
    return true;

  if ((A1.IsEqualSimple(-A2, wdAngle::Degree(fEpsilon))) && (vA1.IsEqual(-vA2, fEpsilon).AllSet<3>()))
    return true;

  return false;
}



WD_STATICLINK_FILE(Foundation, Foundation_SimdMath_Implementation_SimdQuat);
