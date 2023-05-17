#pragma once

#include <Foundation/Math/Vec2.h>
#include <Foundation/Math/Vec3.h>

// *** Vec2 and Vec3 Code ***
// Cannot put this into the Vec3_inl.h file, that would result in circular dependencies

template <typename Type>
WD_FORCE_INLINE const wdVec3Template<Type> wdVec2Template<Type>::GetAsVec3(Type z) const
{
  WD_NAN_ASSERT(this);

  return wdVec3Template<Type>(x, y, z);
}

template <typename Type>
WD_FORCE_INLINE const wdVec4Template<Type> wdVec2Template<Type>::GetAsVec4(Type z, Type w) const
{
  WD_NAN_ASSERT(this);

  return wdVec4Template<Type>(x, y, z, w);
}

template <typename Type>
WD_FORCE_INLINE const wdVec2Template<Type> wdVec3Template<Type>::GetAsVec2() const
{
  // don't assert here, as the 3rd and 4th component may be NaN when this is fine, e.g. during interop with the SIMD classes
  // WD_NAN_ASSERT(this);

  return wdVec2Template<Type>(x, y);
}

template <typename Type>
WD_FORCE_INLINE const wdVec4Template<Type> wdVec3Template<Type>::GetAsVec4(Type w) const
{
  WD_NAN_ASSERT(this);

  return wdVec4Template<Type>(x, y, z, w);
}

template <typename Type>
WD_FORCE_INLINE const wdVec4Template<Type> wdVec3Template<Type>::GetAsPositionVec4() const
{
  // don't assert here, as the 4th component may be NaN when this is fine, e.g. during interop with the SIMD classes
  // WD_NAN_ASSERT(this);

  return wdVec4Template<Type>(x, y, z, 1);
}

template <typename Type>
WD_FORCE_INLINE const wdVec4Template<Type> wdVec3Template<Type>::GetAsDirectionVec4() const
{
  // don't assert here, as the 4th component may be NaN when this is fine, e.g. during interop with the SIMD classes
  // WD_NAN_ASSERT(this);

  return wdVec4Template<Type>(x, y, z, 0);
}

// *****************

template <typename Type>
WD_ALWAYS_INLINE wdVec4Template<Type>::wdVec4Template()
{
#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  const Type TypeNaN = wdMath::NaN<Type>();
  x = TypeNaN;
  y = TypeNaN;
  z = 0;
  w = 0;
#endif
}

template <typename Type>
WD_ALWAYS_INLINE wdVec4Template<Type>::wdVec4Template(Type x, Type y, Type z, Type w)
  : x(x)
  , y(y)
  , z(z)
  , w(w)
{
}

template <typename Type>
WD_ALWAYS_INLINE wdVec4Template<Type>::wdVec4Template(Type v)
  : x(v)
  , y(v)
  , z(v)
  , w(v)
{
}

template <typename Type>
WD_FORCE_INLINE const wdVec2Template<Type> wdVec4Template<Type>::GetAsVec2() const
{
  // don't assert here, as the 4th component may be NaN when this is fine, e.g. during interop with the SIMD classes
  // WD_NAN_ASSERT(this);

  return wdVec2Template<Type>(x, y);
}

template <typename Type>
WD_FORCE_INLINE const wdVec3Template<Type> wdVec4Template<Type>::GetAsVec3() const
{
  // don't assert here, as the 4th component may be NaN when this is fine, e.g. during interop with the SIMD classes
  // WD_NAN_ASSERT(this);

  return wdVec3Template<Type>(x, y, z);
}

template <typename Type>
WD_ALWAYS_INLINE void wdVec4Template<Type>::Set(Type xyzw)
{
  x = xyzw;
  y = xyzw;
  z = xyzw;
  w = xyzw;
}

template <typename Type>
WD_ALWAYS_INLINE void wdVec4Template<Type>::Set(Type inX, Type inY, Type inZ, Type inW)
{
  x = inX;
  y = inY;
  z = inZ;
  w = inW;
}

template <typename Type>
inline void wdVec4Template<Type>::SetZero()
{
  x = y = z = w = 0;
}

template <typename Type>
WD_ALWAYS_INLINE Type wdVec4Template<Type>::GetLength() const
{
  return (wdMath::Sqrt(GetLengthSquared()));
}

template <typename Type>
WD_FORCE_INLINE Type wdVec4Template<Type>::GetLengthSquared() const
{
  WD_NAN_ASSERT(this);

  return (x * x + y * y + z * z + w * w);
}

template <typename Type>
WD_FORCE_INLINE Type wdVec4Template<Type>::GetLengthAndNormalize()
{
  const Type fLength = GetLength();
  *this /= fLength;
  return fLength;
}

template <typename Type>
WD_FORCE_INLINE const wdVec4Template<Type> wdVec4Template<Type>::GetNormalized() const
{
  const Type fLen = GetLength();

  const Type fLengthInv = wdMath::Invert(fLen);
  return wdVec4Template<Type>(x * fLengthInv, y * fLengthInv, z * fLengthInv, w * fLengthInv);
}

template <typename Type>
WD_ALWAYS_INLINE void wdVec4Template<Type>::Normalize()
{
  *this /= GetLength();
}

template <typename Type>
inline wdResult wdVec4Template<Type>::NormalizeIfNotZero(const wdVec4Template<Type>& vFallback, Type fEpsilon)
{
  WD_NAN_ASSERT(&vFallback);

  const Type fLength = GetLength();

  if (!wdMath::IsFinite(fLength) || wdMath::IsZero(fLength, fEpsilon))
  {
    *this = vFallback;
    return WD_FAILURE;
  }

  *this /= fLength;
  return WD_SUCCESS;
}

/*! \note Normalization, especially with SSE is not very precise. So this function checks whether the (squared)
  length is between a lower and upper limit.
*/
template <typename Type>
inline bool wdVec4Template<Type>::IsNormalized(Type fEpsilon /* = wdMath::HugeEpsilon<Type>() */) const
{
  const Type t = GetLengthSquared();
  return wdMath::IsEqual(t, (Type)1, fEpsilon);
}

template <typename Type>
inline bool wdVec4Template<Type>::IsZero() const
{
  WD_NAN_ASSERT(this);

  return ((x == 0.0f) && (y == 0.0f) && (z == 0.0f) && (w == 0.0f));
}

template <typename Type>
inline bool wdVec4Template<Type>::IsZero(Type fEpsilon) const
{
  WD_NAN_ASSERT(this);

  return (wdMath::IsZero(x, fEpsilon) && wdMath::IsZero(y, fEpsilon) && wdMath::IsZero(z, fEpsilon) && wdMath::IsZero(w, fEpsilon));
}

template <typename Type>
inline bool wdVec4Template<Type>::IsNaN() const
{
  if (wdMath::IsNaN(x))
    return true;
  if (wdMath::IsNaN(y))
    return true;
  if (wdMath::IsNaN(z))
    return true;
  if (wdMath::IsNaN(w))
    return true;

  return false;
}

template <typename Type>
inline bool wdVec4Template<Type>::IsValid() const
{
  if (!wdMath::IsFinite(x))
    return false;
  if (!wdMath::IsFinite(y))
    return false;
  if (!wdMath::IsFinite(z))
    return false;
  if (!wdMath::IsFinite(w))
    return false;

  return true;
}

template <typename Type>
WD_FORCE_INLINE const wdVec4Template<Type> wdVec4Template<Type>::operator-() const
{
  WD_NAN_ASSERT(this);

  return wdVec4Template<Type>(-x, -y, -z, -w);
}

template <typename Type>
WD_FORCE_INLINE void wdVec4Template<Type>::operator+=(const wdVec4Template<Type>& vCc)
{
  x += vCc.x;
  y += vCc.y;
  z += vCc.z;
  w += vCc.w;

  WD_NAN_ASSERT(this);
}

template <typename Type>
WD_FORCE_INLINE void wdVec4Template<Type>::operator-=(const wdVec4Template<Type>& vCc)
{
  x -= vCc.x;
  y -= vCc.y;
  z -= vCc.z;
  w -= vCc.w;

  WD_NAN_ASSERT(this);
}

template <typename Type>
WD_FORCE_INLINE void wdVec4Template<Type>::operator*=(Type f)
{
  x *= f;
  y *= f;
  z *= f;
  w *= f;

  WD_NAN_ASSERT(this);
}

template <typename Type>
WD_FORCE_INLINE void wdVec4Template<Type>::operator/=(Type f)
{
  const Type f_inv = wdMath::Invert(f);

  x *= f_inv;
  y *= f_inv;
  z *= f_inv;
  w *= f_inv;

  WD_NAN_ASSERT(this);
}

template <typename Type>
WD_FORCE_INLINE Type wdVec4Template<Type>::Dot(const wdVec4Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return ((x * rhs.x) + (y * rhs.y) + (z * rhs.z) + (w * rhs.w));
}

template <typename Type>
inline const wdVec4Template<Type> wdVec4Template<Type>::CompMin(const wdVec4Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return wdVec4Template<Type>(wdMath::Min(x, rhs.x), wdMath::Min(y, rhs.y), wdMath::Min(z, rhs.z), wdMath::Min(w, rhs.w));
}

template <typename Type>
inline const wdVec4Template<Type> wdVec4Template<Type>::CompMax(const wdVec4Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return wdVec4Template<Type>(wdMath::Max(x, rhs.x), wdMath::Max(y, rhs.y), wdMath::Max(z, rhs.z), wdMath::Max(w, rhs.w));
}

template <typename Type>
inline const wdVec4Template<Type> wdVec4Template<Type>::CompClamp(const wdVec4Template& vLow, const wdVec4Template& vHigh) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&vLow);
  WD_NAN_ASSERT(&vHigh);

  return wdVec4Template<Type>(wdMath::Clamp(x, vLow.x, vHigh.x), wdMath::Clamp(y, vLow.y, vHigh.y), wdMath::Clamp(z, vLow.z, vHigh.z), wdMath::Clamp(w, vLow.w, vHigh.w));
}

template <typename Type>
inline const wdVec4Template<Type> wdVec4Template<Type>::CompMul(const wdVec4Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return wdVec4Template<Type>(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w);
}

WD_MSVC_ANALYSIS_WARNING_PUSH
WD_MSVC_ANALYSIS_WARNING_DISABLE(4723)
template <typename Type>
inline const wdVec4Template<Type> wdVec4Template<Type>::CompDiv(const wdVec4Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return wdVec4Template<Type>(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w);
}
WD_MSVC_ANALYSIS_WARNING_POP

template <typename Type>
inline const wdVec4Template<Type> wdVec4Template<Type>::Abs() const
{
  WD_NAN_ASSERT(this);

  return wdVec4Template<Type>(wdMath::Abs(x), wdMath::Abs(y), wdMath::Abs(z), wdMath::Abs(w));
}

template <typename Type>
WD_FORCE_INLINE const wdVec4Template<Type> operator+(const wdVec4Template<Type>& v1, const wdVec4Template<Type>& v2)
{
  WD_NAN_ASSERT(&v1);
  WD_NAN_ASSERT(&v2);

  return wdVec4Template<Type>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

template <typename Type>
WD_FORCE_INLINE const wdVec4Template<Type> operator-(const wdVec4Template<Type>& v1, const wdVec4Template<Type>& v2)
{
  WD_NAN_ASSERT(&v1);
  WD_NAN_ASSERT(&v2);

  return wdVec4Template<Type>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}

template <typename Type>
WD_FORCE_INLINE const wdVec4Template<Type> operator*(Type f, const wdVec4Template<Type>& v)
{
  WD_NAN_ASSERT(&v);

  return wdVec4Template<Type>(v.x * f, v.y * f, v.z * f, v.w * f);
}

template <typename Type>
WD_FORCE_INLINE const wdVec4Template<Type> operator*(const wdVec4Template<Type>& v, Type f)
{
  WD_NAN_ASSERT(&v);

  return wdVec4Template<Type>(v.x * f, v.y * f, v.z * f, v.w * f);
}

template <typename Type>
WD_FORCE_INLINE const wdVec4Template<Type> operator/(const wdVec4Template<Type>& v, Type f)
{
  WD_NAN_ASSERT(&v);

  // multiplication is much faster than division
  const Type f_inv = wdMath::Invert(f);
  return wdVec4Template<Type>(v.x * f_inv, v.y * f_inv, v.z * f_inv, v.w * f_inv);
}

template <typename Type>
inline bool wdVec4Template<Type>::IsIdentical(const wdVec4Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return ((x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w));
}

template <typename Type>
inline bool wdVec4Template<Type>::IsEqual(const wdVec4Template<Type>& rhs, Type fEpsilon) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return (wdMath::IsEqual(x, rhs.x, fEpsilon) && wdMath::IsEqual(y, rhs.y, fEpsilon) && wdMath::IsEqual(z, rhs.z, fEpsilon) && wdMath::IsEqual(w, rhs.w, fEpsilon));
}

template <typename Type>
WD_ALWAYS_INLINE bool operator==(const wdVec4Template<Type>& v1, const wdVec4Template<Type>& v2)
{
  return v1.IsIdentical(v2);
}

template <typename Type>
WD_ALWAYS_INLINE bool operator!=(const wdVec4Template<Type>& v1, const wdVec4Template<Type>& v2)
{
  return !v1.IsIdentical(v2);
}

template <typename Type>
WD_FORCE_INLINE bool operator<(const wdVec4Template<Type>& v1, const wdVec4Template<Type>& v2)
{
  WD_NAN_ASSERT(&v1);
  WD_NAN_ASSERT(&v2);

  if (v1.x < v2.x)
    return true;
  if (v1.x > v2.x)
    return false;
  if (v1.y < v2.y)
    return true;
  if (v1.y > v2.y)
    return false;
  if (v1.z < v2.z)
    return true;
  if (v1.z > v2.z)
    return false;

  return (v1.w < v2.w);
}
