#pragma once

#include <Foundation/Math/BoundingBox.h>
#include <Foundation/Math/BoundingSphere.h>
#include <Foundation/Math/Color.h>
#include <Foundation/Math/Color8UNorm.h>
#include <Foundation/Math/Mat3.h>
#include <Foundation/Math/Mat4.h>
#include <Foundation/Math/Plane.h>
#include <Foundation/Math/Quat.h>
#include <Foundation/Math/Transform.h>
#include <Foundation/Math/Vec2.h>
#include <Foundation/Math/Vec3.h>
#include <Foundation/Math/Vec4.h>

// wdVec2Template

template <typename Type>
inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdVec2Template<Type>& vValue)
{
  inout_stream.WriteBytes(&vValue, sizeof(wdVec2Template<Type>)).AssertSuccess();
  return inout_stream;
}

template <typename Type>
inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdVec2Template<Type>& ref_vValue)
{
  WD_VERIFY(inout_stream.ReadBytes(&ref_vValue, sizeof(wdVec2Template<Type>)) == sizeof(wdVec2Template<Type>), "End of stream reached.");
  return inout_stream;
}

template <typename Type>
wdResult SerializeArray(wdStreamWriter& inout_stream, const wdVec2Template<Type>* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(wdVec2Template<Type>) * uiCount);
}

template <typename Type>
wdResult DeserializeArray(wdStreamReader& inout_stream, wdVec2Template<Type>* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(wdVec2Template<Type>) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}


// wdVec3Template

template <typename Type>
inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdVec3Template<Type>& vValue)
{
  inout_stream.WriteBytes(&vValue, sizeof(wdVec3Template<Type>)).AssertSuccess();
  return inout_stream;
}

template <typename Type>
inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdVec3Template<Type>& ref_vValue)
{
  WD_VERIFY(inout_stream.ReadBytes(&ref_vValue, sizeof(wdVec3Template<Type>)) == sizeof(wdVec3Template<Type>), "End of stream reached.");
  return inout_stream;
}

template <typename Type>
wdResult SerializeArray(wdStreamWriter& inout_stream, const wdVec3Template<Type>* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(wdVec3Template<Type>) * uiCount);
}

template <typename Type>
wdResult DeserializeArray(wdStreamReader& inout_stream, wdVec3Template<Type>* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(wdVec3Template<Type>) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}


// wdVec4Template

template <typename Type>
inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdVec4Template<Type>& vValue)
{
  inout_stream.WriteBytes(&vValue, sizeof(wdVec4Template<Type>)).AssertSuccess();
  return inout_stream;
}

template <typename Type>
inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdVec4Template<Type>& ref_vValue)
{
  WD_VERIFY(inout_stream.ReadBytes(&ref_vValue, sizeof(wdVec4Template<Type>)) == sizeof(wdVec4Template<Type>), "End of stream reached.");
  return inout_stream;
}

template <typename Type>
wdResult SerializeArray(wdStreamWriter& inout_stream, const wdVec4Template<Type>* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(wdVec4Template<Type>) * uiCount);
}

template <typename Type>
wdResult DeserializeArray(wdStreamReader& inout_stream, wdVec4Template<Type>* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(wdVec4Template<Type>) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}


// wdMat3Template

template <typename Type>
inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdMat3Template<Type>& mValue)
{
  inout_stream.WriteBytes(mValue.m_fElementsCM, sizeof(Type) * 9).AssertSuccess();
  return inout_stream;
}

template <typename Type>
inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdMat3Template<Type>& ref_mValue)
{
  WD_VERIFY(inout_stream.ReadBytes(ref_mValue.m_fElementsCM, sizeof(Type) * 9) == sizeof(Type) * 9, "End of stream reached.");
  return inout_stream;
}

template <typename Type>
wdResult SerializeArray(wdStreamWriter& inout_stream, const wdMat3Template<Type>* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(wdMat3Template<Type>) * uiCount);
}

template <typename Type>
wdResult DeserializeArray(wdStreamReader& inout_stream, wdMat3Template<Type>* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(wdMat3Template<Type>) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}


// wdMat4Template

template <typename Type>
inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdMat4Template<Type>& mValue)
{
  inout_stream.WriteBytes(mValue.m_fElementsCM, sizeof(Type) * 16).AssertSuccess();
  return inout_stream;
}

template <typename Type>
inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdMat4Template<Type>& ref_mValue)
{
  WD_VERIFY(inout_stream.ReadBytes(ref_mValue.m_fElementsCM, sizeof(Type) * 16) == sizeof(Type) * 16, "End of stream reached.");
  return inout_stream;
}

template <typename Type>
wdResult SerializeArray(wdStreamWriter& inout_stream, const wdMat4Template<Type>* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(wdMat4Template<Type>) * uiCount);
}

template <typename Type>
wdResult DeserializeArray(wdStreamReader& inout_stream, wdMat4Template<Type>* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(wdMat4Template<Type>) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}


// wdTransformTemplate

template <typename Type>
inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdTransformTemplate<Type>& value)
{
  inout_stream << value.m_qRotation;
  inout_stream << value.m_vPosition;
  inout_stream << value.m_vScale;

  return inout_stream;
}

template <typename Type>
inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdTransformTemplate<Type>& out_value)
{
  inout_stream >> out_value.m_qRotation;
  inout_stream >> out_value.m_vPosition;
  inout_stream >> out_value.m_vScale;

  return inout_stream;
}

// wdPlaneTemplate

template <typename Type>
inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdPlaneTemplate<Type>& value)
{
  inout_stream.WriteBytes(&value, sizeof(wdPlaneTemplate<Type>)).AssertSuccess();
  return inout_stream;
}

template <typename Type>
inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdPlaneTemplate<Type>& out_value)
{
  WD_VERIFY(inout_stream.ReadBytes(&out_value, sizeof(wdPlaneTemplate<Type>)) == sizeof(wdPlaneTemplate<Type>), "End of stream reached.");
  return inout_stream;
}

template <typename Type>
wdResult SerializeArray(wdStreamWriter& inout_stream, const wdPlaneTemplate<Type>* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(wdPlaneTemplate<Type>) * uiCount);
}

template <typename Type>
wdResult DeserializeArray(wdStreamReader& inout_stream, wdPlaneTemplate<Type>* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(wdPlaneTemplate<Type>) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}


// wdQuatTemplate

template <typename Type>
inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdQuatTemplate<Type>& qValue)
{
  inout_stream.WriteBytes(&qValue, sizeof(wdQuatTemplate<Type>)).AssertSuccess();
  return inout_stream;
}

template <typename Type>
inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdQuatTemplate<Type>& ref_qValue)
{
  WD_VERIFY(inout_stream.ReadBytes(&ref_qValue, sizeof(wdQuatTemplate<Type>)) == sizeof(wdQuatTemplate<Type>), "End of stream reached.");
  return inout_stream;
}

template <typename Type>
wdResult SerializeArray(wdStreamWriter& inout_stream, const wdQuatTemplate<Type>* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(wdQuatTemplate<Type>) * uiCount);
}

template <typename Type>
wdResult DeserializeArray(wdStreamReader& inout_stream, wdQuatTemplate<Type>* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(wdQuatTemplate<Type>) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}


// wdBoundingBoxTemplate

template <typename Type>
inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdBoundingBoxTemplate<Type>& value)
{
  inout_stream << value.m_vMax;
  inout_stream << value.m_vMin;
  return inout_stream;
}

template <typename Type>
inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdBoundingBoxTemplate<Type>& out_value)
{
  inout_stream >> out_value.m_vMax;
  inout_stream >> out_value.m_vMin;
  return inout_stream;
}

// wdBoundingSphereTemplate

template <typename Type>
inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdBoundingSphereTemplate<Type>& value)
{
  inout_stream << value.m_vCenter;
  inout_stream << value.m_fRadius;
  return inout_stream;
}

template <typename Type>
inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdBoundingSphereTemplate<Type>& out_value)
{
  inout_stream >> out_value.m_vCenter;
  inout_stream >> out_value.m_fRadius;
  return inout_stream;
}

// wdBoundingBoxSphereTemplate

template <typename Type>
inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdBoundingBoxSphereTemplate<Type>& value)
{
  inout_stream << value.m_vCenter;
  inout_stream << value.m_fSphereRadius;
  inout_stream << value.m_vBoxHalfExtends;
  return inout_stream;
}

template <typename Type>
inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdBoundingBoxSphereTemplate<Type>& out_value)
{
  inout_stream >> out_value.m_vCenter;
  inout_stream >> out_value.m_fSphereRadius;
  inout_stream >> out_value.m_vBoxHalfExtends;
  return inout_stream;
}

// wdColor
inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdColor& value)
{
  inout_stream.WriteBytes(&value, sizeof(wdColor)).AssertSuccess();
  return inout_stream;
}

inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdColor& ref_value)
{
  WD_VERIFY(inout_stream.ReadBytes(&ref_value, sizeof(wdColor)) == sizeof(wdColor), "End of stream reached.");
  return inout_stream;
}

inline wdResult SerializeArray(wdStreamWriter& inout_stream, const wdColor* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(wdColor) * uiCount);
}

template <typename Type>
wdResult DeserializeArray(wdStreamReader& inout_stream, wdColor* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(wdColor) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}


// wdColorGammaUB
inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdColorGammaUB& value)
{
  inout_stream.WriteBytes(&value, sizeof(wdColorGammaUB)).AssertSuccess();
  return inout_stream;
}

inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdColorGammaUB& ref_value)
{
  WD_VERIFY(inout_stream.ReadBytes(&ref_value, sizeof(wdColorGammaUB)) == sizeof(wdColorGammaUB), "End of stream reached.");
  return inout_stream;
}

template <typename Type>
wdResult SerializeArray(wdStreamWriter& inout_stream, const wdColorGammaUB* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(wdColorGammaUB) * uiCount);
}

template <typename Type>
wdResult DeserializeArray(wdStreamReader& inout_stream, wdColorGammaUB* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(wdColorGammaUB) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}


// wdAngle
inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdAngle& value)
{
  inout_stream << value.GetRadian();
  return inout_stream;
}

inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdAngle& out_value)
{
  float fRadian;
  inout_stream >> fRadian;
  out_value.SetRadian(fRadian);
  return inout_stream;
}

template <typename Type>
wdResult SerializeArray(wdStreamWriter& inout_stream, const wdAngle* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(wdAngle) * uiCount);
}

template <typename Type>
wdResult DeserializeArray(wdStreamReader& inout_stream, wdAngle* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(wdAngle) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}


// wdColor8Unorm
inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdColorLinearUB& value)
{
  inout_stream.WriteBytes(&value, sizeof(wdColorLinearUB)).AssertSuccess();
  return inout_stream;
}

inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdColorLinearUB& ref_value)
{
  WD_VERIFY(inout_stream.ReadBytes(&ref_value, sizeof(wdColorLinearUB)) == sizeof(wdColorLinearUB), "End of stream reached.");
  return inout_stream;
}

template <typename Type>
wdResult SerializeArray(wdStreamWriter& inout_stream, const wdColorLinearUB* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(wdColorLinearUB) * uiCount);
}

template <typename Type>
wdResult DeserializeArray(wdStreamReader& inout_stream, wdColorLinearUB* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(wdColorLinearUB) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}
