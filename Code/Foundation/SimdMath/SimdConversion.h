#pragma once

#include <Foundation/Math/BoundingBox.h>
#include <Foundation/Math/BoundingBoxSphere.h>
#include <Foundation/Math/BoundingSphere.h>
#include <Foundation/Math/Transform.h>
#include <Foundation/SimdMath/SimdBBox.h>
#include <Foundation/SimdMath/SimdBBoxSphere.h>

namespace wdSimdConversion
{
  WD_ALWAYS_INLINE wdVec3 ToVec3(const wdSimdVec4f& v)
  {
    wdVec4 tmp;
    v.Store<4>(&tmp.x);
    return *reinterpret_cast<wdVec3*>(&tmp.x);
  }

  WD_ALWAYS_INLINE wdSimdVec4f ToVec3(const wdVec3& v)
  {
    wdSimdVec4f tmp;
    tmp.Load<3>(&v.x);
    return tmp;
  }

  WD_ALWAYS_INLINE wdVec4 ToVec4(const wdSimdVec4f& v)
  {
    wdVec4 tmp;
    v.Store<4>(&tmp.x);
    return tmp;
  }

  WD_ALWAYS_INLINE wdSimdVec4f ToVec4(const wdVec4& v)
  {
    wdSimdVec4f tmp;
    tmp.Load<4>(&v.x);
    return tmp;
  }

  WD_ALWAYS_INLINE wdQuat ToQuat(const wdSimdQuat& q)
  {
    wdQuat tmp;
    q.m_v.Store<4>(&tmp.v.x);
    return tmp;
  }

  WD_ALWAYS_INLINE wdSimdQuat ToQuat(const wdQuat& q)
  {
    wdSimdVec4f tmp;
    tmp.Load<4>(&q.v.x);
    return wdSimdQuat(tmp);
  }

  WD_ALWAYS_INLINE wdTransform ToTransform(const wdSimdTransform& t)
  {
    return wdTransform(ToVec3(t.m_Position), ToQuat(t.m_Rotation), ToVec3(t.m_Scale));
  }

  inline wdSimdTransform ToTransform(const wdTransform& t)
  {
    return wdSimdTransform(ToVec3(t.m_vPosition), ToQuat(t.m_qRotation), ToVec3(t.m_vScale));
  }

  WD_ALWAYS_INLINE wdMat4 ToMat4(const wdSimdMat4f& m)
  {
    wdMat4 tmp;
    m.GetAsArray(tmp.m_fElementsCM, wdMatrixLayout::ColumnMajor);
    return tmp;
  }

  WD_ALWAYS_INLINE wdSimdMat4f ToMat4(const wdMat4& m)
  {
    wdSimdMat4f tmp;
    tmp.SetFromArray(m.m_fElementsCM, wdMatrixLayout::ColumnMajor);
    return tmp;
  }

  WD_ALWAYS_INLINE wdBoundingBoxSphere ToBBoxSphere(const wdSimdBBoxSphere& b)
  {
    wdVec4 centerAndRadius = ToVec4(b.m_CenterAndRadius);
    return wdBoundingBoxSphere(centerAndRadius.GetAsVec3(), ToVec3(b.m_BoxHalfExtents), centerAndRadius.w);
  }

  WD_ALWAYS_INLINE wdSimdBBoxSphere ToBBoxSphere(const wdBoundingBoxSphere& b)
  {
    return wdSimdBBoxSphere(ToVec3(b.m_vCenter), ToVec3(b.m_vBoxHalfExtends), b.m_fSphereRadius);
  }

  WD_ALWAYS_INLINE wdBoundingSphere ToBSphere(const wdSimdBSphere& s)
  {
    wdVec4 centerAndRadius = ToVec4(s.m_CenterAndRadius);
    return wdBoundingSphere(centerAndRadius.GetAsVec3(), centerAndRadius.w);
  }

  WD_ALWAYS_INLINE wdSimdBSphere ToBSphere(const wdBoundingSphere& s) { return wdSimdBSphere(ToVec3(s.m_vCenter), s.m_fRadius); }

  WD_ALWAYS_INLINE wdSimdBBox ToBBox(const wdBoundingBox& b) { return wdSimdBBox(ToVec3(b.m_vMin), ToVec3(b.m_vMax)); }

  WD_ALWAYS_INLINE wdBoundingBox ToBBox(const wdSimdBBox& b) { return wdBoundingBox(ToVec3(b.m_Min), ToVec3(b.m_Max)); }

}; // namespace wdSimdConversion
