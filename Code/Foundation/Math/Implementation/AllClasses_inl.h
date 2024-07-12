#pragma once

#include <Foundation/Math/BoundingBox.h>
#include <Foundation/Math/BoundingSphere.h>
#include <Foundation/Math/Mat3.h>
#include <Foundation/Math/Mat4.h>
#include <Foundation/Math/Plane.h>

template <typename Type>
NS_ALWAYS_INLINE bool nsBoundingBoxTemplate<Type>::Contains(const nsBoundingSphereTemplate<Type>& sphere) const
{
  return Contains(sphere.GetBoundingBox());
}

template <typename Type>
NS_ALWAYS_INLINE bool nsBoundingBoxTemplate<Type>::Overlaps(const nsBoundingSphereTemplate<Type>& sphere) const
{
  // check whether the closest point between box and sphere is inside the sphere (it is definitely inside the box)
  return sphere.Contains(GetClampedPoint(sphere.m_vCenter));
}

template <typename Type>
inline Type nsBoundingBoxTemplate<Type>::GetDistanceTo(const nsBoundingSphereTemplate<Type>& sphere) const
{
  return (GetClampedPoint(sphere.m_vCenter) - sphere.m_vCenter).GetLength() - sphere.m_fRadius;
}

template <typename Type>
inline const nsBoundingSphereTemplate<Type> nsBoundingBoxTemplate<Type>::GetBoundingSphere() const
{
  return nsBoundingSphereTemplate<Type>::MakeFromCenterAndRadius(GetCenter(), (m_vMax - m_vMin).GetLength() * (Type)0.5);
}

template <typename Type>
void nsBoundingSphereTemplate<Type>::ExpandToInclude(const nsBoundingBoxTemplate<Type>& rhs)
{
  // compute the min and max extends of the AABB relative to the sphere (sphere center is the new origin)
  const nsVec3 vDiffMax = rhs.m_vMax - m_vCenter;
  const nsVec3 vDiffMin = rhs.m_vMin - m_vCenter;

  // compute the absolute distance to each AABB extremum, per axis
  const nsVec3 vDiffMaxAbs(nsMath::Abs(vDiffMax.x), nsMath::Abs(vDiffMax.y), nsMath::Abs(vDiffMax.z));
  const nsVec3 vDiffMinAbs(nsMath::Abs(vDiffMin.x), nsMath::Abs(vDiffMin.y), nsMath::Abs(vDiffMin.z));

  // take the maximum distance for each axis, to compute the point that is the farthest away from the sphere
  const nsVec3 vMostDistantPoint = vDiffMinAbs.CompMax(vDiffMaxAbs);

  const Type fDistSQR = vMostDistantPoint.GetLengthSquared();

  if (nsMath::Square(m_fRadius) < fDistSQR)
    m_fRadius = nsMath::Sqrt(fDistSQR);
}

template <typename Type>
Type nsBoundingSphereTemplate<Type>::GetDistanceTo(const nsBoundingBoxTemplate<Type>& rhs) const
{
  const nsVec3Template<Type> vPointOnBox = rhs.GetClampedPoint(m_vCenter);

  return GetDistanceTo(vPointOnBox);
}

template <typename Type>
bool nsBoundingSphereTemplate<Type>::Contains(const nsBoundingBoxTemplate<Type>& rhs) const
{
  // compute the min and max extends of the AABB relative to the sphere (sphere center is the new origin)
  const nsVec3 vDiffMax = rhs.m_vMax - m_vCenter;
  const nsVec3 vDiffMin = rhs.m_vMin - m_vCenter;

  // compute the absolute distance to each AABB extremum, per axis
  const nsVec3 vDiffMaxAbs(nsMath::Abs(vDiffMax.x), nsMath::Abs(vDiffMax.y), nsMath::Abs(vDiffMax.z));
  const nsVec3 vDiffMinAbs(nsMath::Abs(vDiffMin.x), nsMath::Abs(vDiffMin.y), nsMath::Abs(vDiffMin.z));

  // take the maximum distance for each axis, to compute the point that is the farthest away from the sphere
  const nsVec3 vMostDistantPoint = vDiffMinAbs.CompMax(vDiffMaxAbs);

  // if the squared length of that point is still smaller than the sphere radius, it is inside the sphere
  // and thus the whole AABB is inside the sphere
  return vMostDistantPoint.GetLengthSquared() <= m_fRadius * m_fRadius;
}

template <typename Type>
bool nsBoundingSphereTemplate<Type>::Overlaps(const nsBoundingBoxTemplate<Type>& rhs) const
{
  return Contains(rhs.GetClampedPoint(m_vCenter));
}

template <typename Type>
const nsBoundingBoxTemplate<Type> nsBoundingSphereTemplate<Type>::GetBoundingBox() const
{
  return nsBoundingBoxTemplate<Type>::MakeFromMinMax(m_vCenter - nsVec3Template<Type>(m_fRadius), m_vCenter + nsVec3Template<Type>(m_fRadius));
}


template <typename Type>
nsPositionOnPlane::Enum nsPlaneTemplate<Type>::GetObjectPosition(const nsBoundingSphereTemplate<Type>& sphere) const
{
  const Type fDist = GetDistanceTo(sphere.m_vCenter);

  if (fDist >= sphere.m_fRadius)
    return nsPositionOnPlane::Front;

  if (-fDist >= sphere.m_fRadius)
    return nsPositionOnPlane::Back;

  return nsPositionOnPlane::Spanning;
}

template <typename Type>
nsPositionOnPlane::Enum nsPlaneTemplate<Type>::GetObjectPosition(const nsBoundingBoxTemplate<Type>& box) const
{
  nsVec3Template<Type> vPos = box.m_vMin;
  nsVec3Template<Type> vNeg = box.m_vMax;

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
    return nsPositionOnPlane::Back;

  if (GetDistanceTo(vNeg) >= (Type)0)
    return nsPositionOnPlane::Front;

  return nsPositionOnPlane::Spanning;
}

template <typename Type>
Type nsPlaneTemplate<Type>::GetMinimumDistanceTo(const nsBoundingBoxTemplate<Type>& box) const
{
  nsVec3Template<Type> vNeg = box.m_vMax;

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
Type nsPlaneTemplate<Type>::GetMaximumDistanceTo(const nsBoundingBoxTemplate<Type>& box) const
{
  nsVec3Template<Type> vPos = box.m_vMin;

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
nsMat3Template<Type> nsMat3Template<Type>::MakeAxisRotation(const nsVec3Template<Type>& vAxis, nsAngle angle)
{
  NS_ASSERT_DEBUG(vAxis.IsNormalized(0.1f), "vAxis must be normalized.");

  const Type cos = nsMath::Cos(angle);
  const Type sin = nsMath::Sin(angle);
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

  nsMat3Template<Type> res;

  // Column 1
  res.Element(0, 0) = cos + (oneminuscos * (vAxis.x * vAxis.x));
  res.Element(0, 1) = onecos_xy + zsin;
  res.Element(0, 2) = onecos_xz - ysin;

  // Column 2  )
  res.Element(1, 0) = onecos_xy - zsin;
  res.Element(1, 1) = cos + (oneminuscos * (vAxis.y * vAxis.y));
  res.Element(1, 2) = onecos_yz + xsin;

  // Column 3  )
  res.Element(2, 0) = onecos_xz + ysin;
  res.Element(2, 1) = onecos_yz - xsin;
  res.Element(2, 2) = cos + (oneminuscos * (vAxis.z * vAxis.z));

  return res;
}

template <typename Type>
nsResult nsMat3Template<Type>::Invert(Type fEpsilon)
{
  const Type fDet = Element(0, 0) * (Element(2, 2) * Element(1, 1) - Element(1, 2) * Element(2, 1)) -
                    Element(0, 1) * (Element(2, 2) * Element(1, 0) - Element(1, 2) * Element(2, 0)) +
                    Element(0, 2) * (Element(2, 1) * Element(1, 0) - Element(1, 1) * Element(2, 0));

  if (nsMath::IsZero(fDet, fEpsilon))
    return NS_FAILURE;

  const Type fOneDivDet = (Type)1 / fDet;

  nsMat3Template<Type> Inverse;

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
  return NS_SUCCESS;
}

template <typename Type>
nsMat4Template<Type> nsMat4Template<Type>::MakeAxisRotation(const nsVec3Template<Type>& vAxis, nsAngle angle)
{
  NS_ASSERT_DEBUG(vAxis.IsNormalized(), "vAxis must be normalized.");

  const Type cos = nsMath::Cos(angle);
  const Type sin = nsMath::Sin(angle);
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

  nsMat4Template<Type> res;

  // Column 1
  res.Element(0, 0) = cos + (oneminuscos * (vAxis.x * vAxis.x));
  res.Element(0, 1) = onecos_xy + zsin;
  res.Element(0, 2) = onecos_xz - ysin;
  res.Element(0, 3) = 0;

  // Column 2
  res.Element(1, 0) = onecos_xy - zsin;
  res.Element(1, 1) = cos + (oneminuscos * (vAxis.y * vAxis.y));
  res.Element(1, 2) = onecos_yz + xsin;
  res.Element(1, 3) = 0;

  // Column 3
  res.Element(2, 0) = onecos_xz + ysin;
  res.Element(2, 1) = onecos_yz - xsin;
  res.Element(2, 2) = cos + (oneminuscos * (vAxis.z * vAxis.z));
  res.Element(2, 3) = 0;

  // Column 4
  res.Element(3, 0) = 0;
  res.Element(3, 1) = 0;
  res.Element(3, 2) = 0;
  res.Element(3, 3) = 1;

  return res;
}

template <typename Type>
nsResult nsMat4Template<Type>::Invert(Type fEpsilon)
{
  nsMat4Template<Type> Inverse;

  const Type fDet = GetDeterminantOf4x4Matrix(*this);

  if (nsMath::IsZero(fDet, fEpsilon))
    return NS_FAILURE;

  Type fOneDivDet = nsMath::Invert(fDet);

  for (nsInt32 i = 0; i < 4; ++i)
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
  return NS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////

// static
template <typename T>
bool nsComparisonOperator::Compare(nsComparisonOperator::Enum cmp, const T& a, const T& b)
{
  switch (cmp)
  {
    case nsComparisonOperator::Equal:
      return a == b;
    case nsComparisonOperator::NotEqual:
      return !(a == b);
    case nsComparisonOperator::Less:
      return a < b;
    case nsComparisonOperator::LessEqual:
      return !(b < a);
    case nsComparisonOperator::Greater:
      return b < a;
    case nsComparisonOperator::GreaterEqual:
      return !(a < b);

      NS_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  return false;
}
