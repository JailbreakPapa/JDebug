#pragma once

#include <Foundation/Math/BoundingBox.h>
#include <Foundation/Math/BoundingBoxSphere.h>
#include <Foundation/Math/BoundingSphere.h>
#include <Foundation/Math/Transform.h>
#include <Foundation/SimdMath/SimdBBox.h>
#include <Foundation/SimdMath/SimdBBoxSphere.h>
#include <Foundation/SimdMath/SimdVec4i.h>

namespace nsSimdConversion
{
  NS_ALWAYS_INLINE nsVec3 ToVec3(const nsSimdVec4f& v)
  {
    nsVec4 tmp;
    v.Store<4>(&tmp.x);
    return *reinterpret_cast<nsVec3*>(&tmp.x);
  }

  NS_ALWAYS_INLINE nsSimdVec4f ToVec3(const nsVec3& v)
  {
    nsSimdVec4f tmp;
    tmp.Load<3>(&v.x);
    return tmp;
  }

  NS_ALWAYS_INLINE nsVec3I32 ToVec3i(const nsSimdVec4i& v)
  {
    nsVec4I32 tmp;
    v.Store<4>(&tmp.x);
    return *reinterpret_cast<nsVec3I32*>(&tmp.x);
  }

  NS_ALWAYS_INLINE nsSimdVec4i ToVec3i(const nsVec3I32& v)
  {
    nsSimdVec4i tmp;
    tmp.Load<3>(&v.x);
    return tmp;
  }

  NS_ALWAYS_INLINE nsVec4 ToVec4(const nsSimdVec4f& v)
  {
    nsVec4 tmp;
    v.Store<4>(&tmp.x);
    return tmp;
  }

  NS_ALWAYS_INLINE nsSimdVec4f ToVec4(const nsVec4& v)
  {
    nsSimdVec4f tmp;
    tmp.Load<4>(&v.x);
    return tmp;
  }

  NS_ALWAYS_INLINE nsVec4I32 ToVec4i(const nsSimdVec4i& v)
  {
    nsVec4I32 tmp;
    v.Store<4>(&tmp.x);
    return tmp;
  }

  NS_ALWAYS_INLINE nsSimdVec4i ToVec4i(const nsVec4I32& v)
  {
    nsSimdVec4i tmp;
    tmp.Load<4>(&v.x);
    return tmp;
  }

  NS_ALWAYS_INLINE nsQuat ToQuat(const nsSimdQuat& q)
  {
    nsQuat tmp;
    q.m_v.Store<4>(&tmp.x);
    return tmp;
  }

  NS_ALWAYS_INLINE nsSimdQuat ToQuat(const nsQuat& q)
  {
    nsSimdVec4f tmp;
    tmp.Load<4>(&q.x);
    return nsSimdQuat(tmp);
  }

  NS_ALWAYS_INLINE nsTransform ToTransform(const nsSimdTransform& t)
  {
    return nsTransform(ToVec3(t.m_Position), ToQuat(t.m_Rotation), ToVec3(t.m_Scale));
  }

  inline nsSimdTransform ToTransform(const nsTransform& t)
  {
    return nsSimdTransform(ToVec3(t.m_vPosition), ToQuat(t.m_qRotation), ToVec3(t.m_vScale));
  }

  NS_ALWAYS_INLINE nsMat4 ToMat4(const nsSimdMat4f& m)
  {
    nsMat4 tmp;
    m.GetAsArray(tmp.m_fElementsCM, nsMatrixLayout::ColumnMajor);
    return tmp;
  }

  NS_ALWAYS_INLINE nsSimdMat4f ToMat4(const nsMat4& m)
  {
    return nsSimdMat4f::MakeFromColumnMajorArray(m.m_fElementsCM);
  }

  NS_ALWAYS_INLINE nsBoundingBoxSphere ToBBoxSphere(const nsSimdBBoxSphere& b)
  {
    nsVec4 centerAndRadius = ToVec4(b.m_CenterAndRadius);
    return nsBoundingBoxSphere::MakeFromCenterExtents(centerAndRadius.GetAsVec3(), ToVec3(b.m_BoxHalfExtents), centerAndRadius.w);
  }

  NS_ALWAYS_INLINE nsSimdBBoxSphere ToBBoxSphere(const nsBoundingBoxSphere& b)
  {
    return nsSimdBBoxSphere::MakeFromCenterExtents(ToVec3(b.m_vCenter), ToVec3(b.m_vBoxHalfExtends), b.m_fSphereRadius);
  }

  NS_ALWAYS_INLINE nsBoundingSphere ToBSphere(const nsSimdBSphere& s)
  {
    nsVec4 centerAndRadius = ToVec4(s.m_CenterAndRadius);
    return nsBoundingSphere::MakeFromCenterAndRadius(centerAndRadius.GetAsVec3(), centerAndRadius.w);
  }

  NS_ALWAYS_INLINE nsSimdBSphere ToBSphere(const nsBoundingSphere& s)
  {
    return nsSimdBSphere(ToVec3(s.m_vCenter), s.m_fRadius);
  }

  NS_ALWAYS_INLINE nsSimdBBox ToBBox(const nsBoundingBox& b)
  {
    return nsSimdBBox(ToVec3(b.m_vMin), ToVec3(b.m_vMax));
  }

  NS_ALWAYS_INLINE nsBoundingBox ToBBox(const nsSimdBBox& b)
  {
    return nsBoundingBox::MakeFromMinMax(ToVec3(b.m_Min), ToVec3(b.m_Max));
  }

}; // namespace nsSimdConversion
