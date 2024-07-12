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

// nsVec2Template

template <typename Type>
inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsVec2Template<Type>& vValue)
{
  inout_stream.WriteBytes(&vValue, sizeof(nsVec2Template<Type>)).AssertSuccess();
  return inout_stream;
}

template <typename Type>
inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsVec2Template<Type>& ref_vValue)
{
  NS_VERIFY(inout_stream.ReadBytes(&ref_vValue, sizeof(nsVec2Template<Type>)) == sizeof(nsVec2Template<Type>), "End of stream reached.");
  return inout_stream;
}

template <typename Type>
nsResult SerializeArray(nsStreamWriter& inout_stream, const nsVec2Template<Type>* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(nsVec2Template<Type>) * uiCount);
}

template <typename Type>
nsResult DeserializeArray(nsStreamReader& inout_stream, nsVec2Template<Type>* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(nsVec2Template<Type>) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}


// nsVec3Template

template <typename Type>
inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsVec3Template<Type>& vValue)
{
  inout_stream.WriteBytes(&vValue, sizeof(nsVec3Template<Type>)).AssertSuccess();
  return inout_stream;
}

template <typename Type>
inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsVec3Template<Type>& ref_vValue)
{
  NS_VERIFY(inout_stream.ReadBytes(&ref_vValue, sizeof(nsVec3Template<Type>)) == sizeof(nsVec3Template<Type>), "End of stream reached.");
  return inout_stream;
}

template <typename Type>
nsResult SerializeArray(nsStreamWriter& inout_stream, const nsVec3Template<Type>* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(nsVec3Template<Type>) * uiCount);
}

template <typename Type>
nsResult DeserializeArray(nsStreamReader& inout_stream, nsVec3Template<Type>* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(nsVec3Template<Type>) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}


// nsVec4Template

template <typename Type>
inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsVec4Template<Type>& vValue)
{
  inout_stream.WriteBytes(&vValue, sizeof(nsVec4Template<Type>)).AssertSuccess();
  return inout_stream;
}

template <typename Type>
inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsVec4Template<Type>& ref_vValue)
{
  NS_VERIFY(inout_stream.ReadBytes(&ref_vValue, sizeof(nsVec4Template<Type>)) == sizeof(nsVec4Template<Type>), "End of stream reached.");
  return inout_stream;
}

template <typename Type>
nsResult SerializeArray(nsStreamWriter& inout_stream, const nsVec4Template<Type>* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(nsVec4Template<Type>) * uiCount);
}

template <typename Type>
nsResult DeserializeArray(nsStreamReader& inout_stream, nsVec4Template<Type>* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(nsVec4Template<Type>) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}


// nsMat3Template

template <typename Type>
inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsMat3Template<Type>& mValue)
{
  inout_stream.WriteBytes(mValue.m_fElementsCM, sizeof(Type) * 9).AssertSuccess();
  return inout_stream;
}

template <typename Type>
inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsMat3Template<Type>& ref_mValue)
{
  NS_VERIFY(inout_stream.ReadBytes(ref_mValue.m_fElementsCM, sizeof(Type) * 9) == sizeof(Type) * 9, "End of stream reached.");
  return inout_stream;
}

template <typename Type>
nsResult SerializeArray(nsStreamWriter& inout_stream, const nsMat3Template<Type>* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(nsMat3Template<Type>) * uiCount);
}

template <typename Type>
nsResult DeserializeArray(nsStreamReader& inout_stream, nsMat3Template<Type>* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(nsMat3Template<Type>) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}


// nsMat4Template

template <typename Type>
inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsMat4Template<Type>& mValue)
{
  inout_stream.WriteBytes(mValue.m_fElementsCM, sizeof(Type) * 16).AssertSuccess();
  return inout_stream;
}

template <typename Type>
inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsMat4Template<Type>& ref_mValue)
{
  NS_VERIFY(inout_stream.ReadBytes(ref_mValue.m_fElementsCM, sizeof(Type) * 16) == sizeof(Type) * 16, "End of stream reached.");
  return inout_stream;
}

template <typename Type>
nsResult SerializeArray(nsStreamWriter& inout_stream, const nsMat4Template<Type>* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(nsMat4Template<Type>) * uiCount);
}

template <typename Type>
nsResult DeserializeArray(nsStreamReader& inout_stream, nsMat4Template<Type>* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(nsMat4Template<Type>) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}


// nsTransformTemplate

template <typename Type>
inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsTransformTemplate<Type>& value)
{
  inout_stream << value.m_qRotation;
  inout_stream << value.m_vPosition;
  inout_stream << value.m_vScale;

  return inout_stream;
}

template <typename Type>
inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsTransformTemplate<Type>& out_value)
{
  inout_stream >> out_value.m_qRotation;
  inout_stream >> out_value.m_vPosition;
  inout_stream >> out_value.m_vScale;

  return inout_stream;
}

// nsPlaneTemplate

template <typename Type>
inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsPlaneTemplate<Type>& value)
{
  inout_stream.WriteBytes(&value, sizeof(nsPlaneTemplate<Type>)).AssertSuccess();
  return inout_stream;
}

template <typename Type>
inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsPlaneTemplate<Type>& out_value)
{
  NS_VERIFY(inout_stream.ReadBytes(&out_value, sizeof(nsPlaneTemplate<Type>)) == sizeof(nsPlaneTemplate<Type>), "End of stream reached.");
  return inout_stream;
}

template <typename Type>
nsResult SerializeArray(nsStreamWriter& inout_stream, const nsPlaneTemplate<Type>* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(nsPlaneTemplate<Type>) * uiCount);
}

template <typename Type>
nsResult DeserializeArray(nsStreamReader& inout_stream, nsPlaneTemplate<Type>* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(nsPlaneTemplate<Type>) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}


// nsQuatTemplate

template <typename Type>
inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsQuatTemplate<Type>& qValue)
{
  inout_stream.WriteBytes(&qValue, sizeof(nsQuatTemplate<Type>)).AssertSuccess();
  return inout_stream;
}

template <typename Type>
inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsQuatTemplate<Type>& ref_qValue)
{
  NS_VERIFY(inout_stream.ReadBytes(&ref_qValue, sizeof(nsQuatTemplate<Type>)) == sizeof(nsQuatTemplate<Type>), "End of stream reached.");
  return inout_stream;
}

template <typename Type>
nsResult SerializeArray(nsStreamWriter& inout_stream, const nsQuatTemplate<Type>* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(nsQuatTemplate<Type>) * uiCount);
}

template <typename Type>
nsResult DeserializeArray(nsStreamReader& inout_stream, nsQuatTemplate<Type>* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(nsQuatTemplate<Type>) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}


// nsBoundingBoxTemplate

template <typename Type>
inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsBoundingBoxTemplate<Type>& value)
{
  inout_stream << value.m_vMax;
  inout_stream << value.m_vMin;
  return inout_stream;
}

template <typename Type>
inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsBoundingBoxTemplate<Type>& out_value)
{
  inout_stream >> out_value.m_vMax;
  inout_stream >> out_value.m_vMin;
  return inout_stream;
}

// nsBoundingSphereTemplate

template <typename Type>
inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsBoundingSphereTemplate<Type>& value)
{
  inout_stream << value.m_vCenter;
  inout_stream << value.m_fRadius;
  return inout_stream;
}

template <typename Type>
inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsBoundingSphereTemplate<Type>& out_value)
{
  inout_stream >> out_value.m_vCenter;
  inout_stream >> out_value.m_fRadius;
  return inout_stream;
}

// nsBoundingBoxSphereTemplate

template <typename Type>
inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsBoundingBoxSphereTemplate<Type>& value)
{
  inout_stream << value.m_vCenter;
  inout_stream << value.m_fSphereRadius;
  inout_stream << value.m_vBoxHalfExtends;
  return inout_stream;
}

template <typename Type>
inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsBoundingBoxSphereTemplate<Type>& out_value)
{
  inout_stream >> out_value.m_vCenter;
  inout_stream >> out_value.m_fSphereRadius;
  inout_stream >> out_value.m_vBoxHalfExtends;
  return inout_stream;
}

// nsColor
inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsColor& value)
{
  inout_stream.WriteBytes(&value, sizeof(nsColor)).AssertSuccess();
  return inout_stream;
}

inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsColor& ref_value)
{
  NS_VERIFY(inout_stream.ReadBytes(&ref_value, sizeof(nsColor)) == sizeof(nsColor), "End of stream reached.");
  return inout_stream;
}

inline nsResult SerializeArray(nsStreamWriter& inout_stream, const nsColor* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(nsColor) * uiCount);
}

template <typename Type>
nsResult DeserializeArray(nsStreamReader& inout_stream, nsColor* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(nsColor) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}


// nsColorGammaUB
inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsColorGammaUB& value)
{
  inout_stream.WriteBytes(&value, sizeof(nsColorGammaUB)).AssertSuccess();
  return inout_stream;
}

inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsColorGammaUB& ref_value)
{
  NS_VERIFY(inout_stream.ReadBytes(&ref_value, sizeof(nsColorGammaUB)) == sizeof(nsColorGammaUB), "End of stream reached.");
  return inout_stream;
}

template <typename Type>
nsResult SerializeArray(nsStreamWriter& inout_stream, const nsColorGammaUB* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(nsColorGammaUB) * uiCount);
}

template <typename Type>
nsResult DeserializeArray(nsStreamReader& inout_stream, nsColorGammaUB* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(nsColorGammaUB) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}


// nsAngle
inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsAngle& value)
{
  inout_stream << value.GetRadian();
  return inout_stream;
}

inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsAngle& out_value)
{
  float fRadian;
  inout_stream >> fRadian;
  out_value.SetRadian(fRadian);
  return inout_stream;
}

template <typename Type>
nsResult SerializeArray(nsStreamWriter& inout_stream, const nsAngle* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(nsAngle) * uiCount);
}

template <typename Type>
nsResult DeserializeArray(nsStreamReader& inout_stream, nsAngle* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(nsAngle) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}


// nsColor8Unorm
inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsColorLinearUB& value)
{
  inout_stream.WriteBytes(&value, sizeof(nsColorLinearUB)).AssertSuccess();
  return inout_stream;
}

inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsColorLinearUB& ref_value)
{
  NS_VERIFY(inout_stream.ReadBytes(&ref_value, sizeof(nsColorLinearUB)) == sizeof(nsColorLinearUB), "End of stream reached.");
  return inout_stream;
}

template <typename Type>
nsResult SerializeArray(nsStreamWriter& inout_stream, const nsColorLinearUB* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(nsColorLinearUB) * uiCount);
}

template <typename Type>
nsResult DeserializeArray(nsStreamReader& inout_stream, nsColorLinearUB* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(nsColorLinearUB) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}
