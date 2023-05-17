#pragma once

#include <Foundation/Math/BoundingBox.h>
#include <Foundation/Math/BoundingSphere.h>
#include <Foundation/Math/Mat3.h>
#include <Foundation/Math/Mat4.h>
#include <Foundation/Math/Plane.h>

template <typename Type>
WD_ALWAYS_INLINE bool wdBoundingBoxTemplate<Type>::Contains(const wdBoundingSphereTemplate<Type>& sphere) const
{
  return Contains(sphere.GetBoundingBox());
}

template <typename Type>
WD_ALWAYS_INLINE bool wdBoundingBoxTemplate<Type>::Overlaps(const wdBoundingSphereTemplate<Type>& sphere) const
{
  // check whether the closest point between box and sphere is inside the sphere (it is definitely inside the box)
  return sphere.Contains(GetClampedPoint(sphere.m_vCenter));
}

template <typename Type>
inline Type wdBoundingBoxTemplate<Type>::GetDistanceTo(const wdBoundingSphereTemplate<Type>& sphere) const
{
  return (GetClampedPoint(sphere.m_vCenter) - sphere.m_vCenter).GetLength() - sphere.m_fRadius;
}

template <typename Type>
inline const wdBoundingSphereTemplate<Type> wdBoundingBoxTemplate<Type>::GetBoundingSphere() const
{
  return wdBoundingSphereTemplate<Type>(GetCenter(), (m_vMax - m_vMin).GetLength() * (Type)0.5);
}

template <typename Type>
void wdBoundingSphereTemplate<Type>::ExpandToInclude(const wdBoundingBoxTemplate<Type>& rhs)
{
  // compute the min and max extends of the AABB relative to the sphere (sphere center is the new origin)
  const wdVec3 vDiffMax = rhs.m_vMax - m_vCenter;
  const wdVec3 vDiffMin = rhs.m_vMin - m_vCenter;

  // compute the absolute distance to each AABB extremum, per axis
  const wdVec3 vDiffMaxAbs(wdMath::Abs(vDiffMax.x), wdMath::Abs(vDiffMax.y), wdMath::Abs(vDiffMax.z));
  const wdVec3 vDiffMinAbs(wdMath::Abs(vDiffMin.x), wdMath::Abs(vDiffMin.y), wdMath::Abs(vDiffMin.z));

  // take the maximum distance for each axis, to compute the point that is the farthest away from the sphere
  const wdVec3 vMostDistantPoint = vDiffMinAbs.CompMax(vDiffMaxAbs);

  const Type fDistSQR = vMostDistantPoint.GetLengthSquared();

  if (wdMath::Square(m_fRadius) < fDistSQR)
    m_fRadius = wdMath::Sqrt(fDistSQR);
}

template <typename Type>
Type wdBoundingSphereTemplate<Type>::GetDistanceTo(const wdBoundingBoxTemplate<Type>& rhs) const
{
  const wdVec3Template<Type> vPointOnBox = rhs.GetClampedPoint(m_vCenter);

  return GetDistanceTo(vPointOnBox);
}

template <typename Type>
bool wdBoundingSphereTemplate<Type>::Contains(const wdBoundingBoxTemplate<Type>& rhs) const
{
  // compute the min and max extends of the AABB relative to the sphere (sphere center is the new origin)
  const wdVec3 vDiffMax = rhs.m_vMax - m_vCenter;
  const wdVec3 vDiffMin = rhs.m_vMin - m_vCenter;

  // compute the absolute distance to each AABB extremum, per axis
  const wdVec3 vDiffMaxAbs(wdMath::Abs(vDiffMax.x), wdMath::Abs(vDiffMax.y), wdMath::Abs(vDiffMax.z));
  const wdVec3 vDiffMinAbs(wdMath::Abs(vDiffMin.x), wdMath::Abs(vDiffMin.y), wdMath::Abs(vDiffMin.z));

  // take the maximum distance for each axis, to compute the point that is the farthest away from the sphere
  const wdVec3 vMostDistantPoint = vDiffMinAbs.CompMax(vDiffMaxAbs);

  // if the squared length of that point is still smaller than the sphere radius, it is inside the sphere
  // and thus the whole AABB is inside the sphere
  return vMostDistantPoint.GetLengthSquared() <= m_fRadius * m_fRadius;
}

template <typename Type>
bool wdBoundingSphereTemplate<Type>::Overlaps(const wdBoundingBoxTemplate<Type>& rhs) const
{
  return Contains(rhs.GetClampedPoint(m_vCenter));
}

template <typename Type>
const wdBoundingBoxTemplate<Type> wdBoundingSphereTemplate<Type>::GetBoundingBox() const
{
  return wdBoundingBoxTemplate<Type>(m_vCenter - wdVec3Template<Type>(m_fRadius), m_vCenter + wdVec3Template<Type>(m_fRadius));
}


template <typename Type>
wdPositionOnPlane::Enum wdPlaneTemplate<Type>::GetObjectPosition(const wdBoundingSphereTemplate<Type>& sphere) const
{
  const Type fDist = GetDistanceTo(sphere.m_vCenter);

  if (fDist >= sphere.m_fRadius)
    return wdPositionOnPlane::Front;

  if (-fDist >= sphere.m_fRadius)
    return wdPositionOnPlane::Back;

  return wdPositionOnPlane::Spanning;
}

template <typename Type>
wdPositionOnPlane::Enum wdPlaneTemplate<Type>::GetObjectPosition(const wdBoundingBoxTemplate<Type>& box) const
{
  wdVec3Template<Type> vPos = box.m_vMin;
  wdVec3Template<Type> vNeg = box.m_vMax;

  if (m_vNormal.x >= (Type)0)
  {
    vPos.x = box.m_vMax.x;
    vNeg.x = box.m_vMin.x;
  }

  if (m_vNormal.y >= (Type)0)
  {
    vPos.y = box.m_vMax.y;
    vNeg.y = box.m_vMin.y;
  }

  if (m_vNormal.z >= (Type)0)
  {
    vPos.z = box.m_vMax.z;
    vNeg.z = box.m_vMin.z;
  }

  if (GetDistanceTo(vPos) <= (Type)0)
    return wdPositionOnPlane::Back;

  if (GetDistanceTo(vNeg) >= (Type)0)
    return wdPositionOnPlane::Front;

  return wdPositionOnPlane::Spanning;
}

template <typename Type>
Type wdPlaneTemplate<Type>::GetMinimumDistanceTo(const wdBoundingBoxTemplate<Type>& box) const
{
  wdVec3Template<Type> vNeg = box.m_vMax;

  if (m_vNormal.x >= (Type)0)
  {
    vNeg.x = box.m_vMin.x;
  }

  if (m_vNormal.y >= (Type)0)
  {
    vNeg.y = box.m_vMin.y;
  }

  if (m_vNormal.z >= (Type)0)
  {
    vNeg.z = box.m_vMin.z;
  }

  return GetDistanceTo(vNeg);
}

template <typename Type>
Type wdPlaneTemplate<Type>::GetMaximumDistanceTo(const wdBoundingBoxTemplate<Type>& box) const
{
  wdVec3Template<Type> vPos = box.m_vMin;

  if (m_vNormal.x >= (Type)0)
  {
    vPos.x = box.m_vMax.x;
  }

  if (m_vNormal.y >= (Type)0)
  {
    vPos.y = box.m_vMax.y;
  }

  if (m_vNormal.z >= (Type)0)
  {
    vPos.z = box.m_vMax.z;
  }

  return GetDistanceTo(vPos);
}

template <typename Type>
void wdMat3Template<Type>::SetRotationMatrix(const wdVec3Template<Type>& vAxis, wdAngle angle)
{
  WD_ASSERT_DEBUG(vAxis.IsNormalized(0.1f), "vAxis must be normalized.");

  const Type cos = wdMath::Cos(angle);
  const Type sin = wdMath::Sin(angle);
  const Type oneminuscos = (Type)1 - cos;

  const Type xy = vAxis.x * vAxis.y;
  const Type xz = vAxis.x * vAxis.z;
  const Type yz = vAxis.y * vAxis.z;

  const Type xsin = vAxis.x * sin;
  const Type ysin = vAxis.y * sin;
  const Type zsin = vAxis.z * sin;

  const Type onecos_xy = oneminuscos * xy;
  const Type onecos_xz = oneminuscos * xz;
  const Type onecos_yz = oneminuscos * yz;

  // Column 1
  Element(0, 0) = cos + (oneminuscos * (vAxis.x * vAxis.x));
  Element(0, 1) = onecos_xy + zsin;
  Element(0, 2) = onecos_xz - ysin;

  // Column 2  )
  Element(1, 0) = onecos_xy - zsin;
  Element(1, 1) = cos + (oneminuscos * (vAxis.y * vAxis.y));
  Element(1, 2) = onecos_yz + xsin;

  // Column 3  )
  Element(2, 0) = onecos_xz + ysin;
  Element(2, 1) = onecos_yz - xsin;
  Element(2, 2) = cos + (oneminuscos * (vAxis.z * vAxis.z));
}

template <typename Type>
wdResult wdMat3Template<Type>::Invert(Type fEpsilon)
{
  const Type fDet = Element(0, 0) * (Element(2, 2) * Element(1, 1) - Element(1, 2) * Element(2, 1)) -
                    Element(0, 1) * (Element(2, 2) * Element(1, 0) - Element(1, 2) * Element(2, 0)) +
                    Element(0, 2) * (Element(2, 1) * Element(1, 0) - Element(1, 1) * Element(2, 0));

  if (wdMath::IsZero(fDet, fEpsilon))
    return WD_FAILURE;

  const Type fOneDivDet = (Type)1 / fDet;

  wdMat3Template<Type> Inverse;

  Inverse.Element(0, 0) = (Element(2, 2) * Element(1, 1) - Element(1, 2) * Element(2, 1));
  Inverse.Element(0, 1) = -(Element(2, 2) * Element(0, 1) - Element(0, 2) * Element(2, 1));
  Inverse.Element(0, 2) = (Element(1, 2) * Element(0, 1) - Element(0, 2) * Element(1, 1));

  Inverse.Element(1, 0) = -(Element(2, 2) * Element(1, 0) - Element(1, 2) * Element(2, 0));
  Inverse.Element(1, 1) = (Element(2, 2) * Element(0, 0) - Element(0, 2) * Element(2, 0));
  Inverse.Element(1, 2) = -(Element(1, 2) * Element(0, 0) - Element(0, 2) * Element(1, 0));

  Inverse.Element(2, 0) = (Element(2, 1) * Element(1, 0) - Element(1, 1) * Element(2, 0));
  Inverse.Element(2, 1) = -(Element(2, 1) * Element(0, 0) - Element(0, 1) * Element(2, 0));
  Inverse.Element(2, 2) = (Element(1, 1) * Element(0, 0) - Element(0, 1) * Element(1, 0));

  *this = Inverse * fOneDivDet;
  return WD_SUCCESS;
}

template <typename Type>
void wdMat4Template<Type>::SetRotationMatrix(const wdVec3Template<Type>& vAxis, wdAngle angle)
{
  WD_ASSERT_DEBUG(vAxis.IsNormalized(), "vAxis must be normalized.");

  const Type cos = wdMath::Cos(angle);
  const Type sin = wdMath::Sin(angle);
  const Type oneminuscos = (Type)1 - cos;

  const Type xy = vAxis.x * vAxis.y;
  const Type xz = vAxis.x * vAxis.z;
  const Type yz = vAxis.y * vAxis.z;

  const Type xsin = vAxis.x * sin;
  const Type ysin = vAxis.y * sin;
  const Type zsin = vAxis.z * sin;

  const Type onecos_xy = oneminuscos * xy;
  const Type onecos_xz = oneminuscos * xz;
  const Type onecos_yz = oneminuscos * yz;

  // Column 1
  Element(0, 0) = cos + (oneminuscos * (vAxis.x * vAxis.x));
  Element(0, 1) = onecos_xy + zsin;
  Element(0, 2) = onecos_xz - ysin;
  Element(0, 3) = 0;

  // Column 2
  Element(1, 0) = onecos_xy - zsin;
  Element(1, 1) = cos + (oneminuscos * (vAxis.y * vAxis.y));
  Element(1, 2) = onecos_yz + xsin;
  Element(1, 3) = 0;

  // Column 3
  Element(2, 0) = onecos_xz + ysin;
  Element(2, 1) = onecos_yz - xsin;
  Element(2, 2) = cos + (oneminuscos * (vAxis.z * vAxis.z));
  Element(2, 3) = 0;

  // Column 4
  Element(3, 0) = 0;
  Element(3, 1) = 0;
  Element(3, 2) = 0;
  Element(3, 3) = 1;
}

template <typename Type>
wdResult wdMat4Template<Type>::Invert(Type fEpsilon)
{
  wdMat4Template<Type> Inverse;

  const Type fDet = GetDeterminantOf4x4Matrix(*this);

  if (wdMath::IsZero(fDet, fEpsilon))
    return WD_FAILURE;

  Type fOneDivDet = wdMath::Invert(fDet);

  for (wdInt32 i = 0; i < 4; ++i)
  {

    Inverse.Element(i, 0) = GetDeterminantOf3x3SubMatrix(*this, i, 0) * fOneDivDet;
    fOneDivDet = -fOneDivDet;
    Inverse.Element(i, 1) = GetDeterminantOf3x3SubMatrix(*this, i, 1) * fOneDivDet;
    fOneDivDet = -fOneDivDet;
    Inverse.Element(i, 2) = GetDeterminantOf3x3SubMatrix(*this, i, 2) * fOneDivDet;
    fOneDivDet = -fOneDivDet;
    Inverse.Element(i, 3) = GetDeterminantOf3x3SubMatrix(*this, i, 3) * fOneDivDet;
  }

  *this = Inverse;
  return WD_SUCCESS;
}
