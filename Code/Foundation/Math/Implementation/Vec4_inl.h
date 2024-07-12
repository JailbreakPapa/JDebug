#pragma once

#include <Foundation/Math/Vec2.h>
#include <Foundation/Math/Vec3.h>

// *** Vec2 and Vec3 Code ***
// Cannot put this into the Vec3_inl.h file, that would result in circular dependencies

template <typename Type>
NS_FORCE_INLINE const nsVec3Template<Type> nsVec2Template<Type>::GetAsVec3(Type z) const
{
  NS_NAN_ASSERT(this);

  return nsVec3Template<Type>(x, y, z);
}

template <typename Type>
NS_FORCE_INLINE const nsVec4Template<Type> nsVec2Template<Type>::GetAsVec4(Type z, Type w) const
{
  NS_NAN_ASSERT(this);

  return nsVec4Template<Type>(x, y, z, w);
}

template <typename Type>
NS_FORCE_INLINE const nsVec2Template<Type> nsVec3Template<Type>::GetAsVec2() const
{
  // don't assert here, as the 3rd and 4th component may be NaN when this is fine, e.g. during interop with the SIMD classes
  // NS_NAN_ASSERT(this);

  return nsVec2Template<Type>(x, y);
}

template <typename Type>
NS_FORCE_INLINE const nsVec4Template<Type> nsVec3Template<Type>::GetAsVec4(Type w) const
{
  NS_NAN_ASSERT(this);

  return nsVec4Template<Type>(x, y, z, w);
}

template <typename Type>
NS_FORCE_INLINE const nsVec4Template<Type> nsVec3Template<Type>::GetAsPositionVec4() const
{
  // don't assert here, as the 4th component may be NaN when this is fine, e.g. during interop with the SIMD classes
  // NS_NAN_ASSERT(this);

  return nsVec4Template<Type>(x, y, z, 1);
}

template <typename Type>
NS_FORCE_INLINE const nsVec4Template<Type> nsVec3Template<Type>::GetAsDirectionVec4() const
{
  // don't assert here, as the 4th component may be NaN when this is fine, e.g. during interop with the SIMD classes
  // NS_NAN_ASSERT(this);

  return nsVec4Template<Type>(x, y, z, 0);
}

// *****************

template <typename Type>
NS_ALWAYS_INLINE nsVec4Template<Type>::nsVec4Template()
{
#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  const Type TypeNaN = nsMath::NaN<Type>();
  x = TypeNaN;
  y = TypeNaN;
  z = 0;
  w = 0;
#endif
}

template <typename Type>
NS_ALWAYS_INLINE nsVec4Template<Type>::nsVec4Template(Type x, Type y, Type z, Type w)
  : x(x)
  , y(y)
  , z(z)
  , w(w)
{
}

template <typename Type>
NS_ALWAYS_INLINE nsVec4Template<Type>::nsVec4Template(nsVec3Template<Type> vXyz, Type w)
  : x(vXyz.x)
  , y(vXyz.y)
  , z(vXyz.z)
  , w(w)
{
}

template <typename Type>
NS_ALWAYS_INLINE nsVec4Template<Type>::nsVec4Template(Type v)
  : x(v)
  , y(v)
  , z(v)
  , w(v)
{
}

template <typename Type>
NS_FORCE_INLINE const nsVec2Template<Type> nsVec4Template<Type>::GetAsVec2() const
{
  // don't assert here, as the 4th component may be NaN when this is fine, e.g. during interop with the SIMD classes
  // NS_NAN_ASSERT(this);

  return nsVec2Template<Type>(x, y);
}

template <typename Type>
NS_FORCE_INLINE const nsVec3Template<Type> nsVec4Template<Type>::GetAsVec3() const
{
  // don't assert here, as the 4th component may be NaN when this is fine, e.g. during interop with the SIMD classes
  // NS_NAN_ASSERT(this);

  return nsVec3Template<Type>(x, y, z);
}

template <typename Type>
NS_ALWAYS_INLINE void nsVec4Template<Type>::Set(Type xyzw)
{
  x = xyzw;
  y = xyzw;
  z = xyzw;
  w = xyzw;
}

template <typename Type>
NS_ALWAYS_INLINE void nsVec4Template<Type>::Set(Type inX, Type inY, Type inZ, Type inW)
{
  x = inX;
  y = inY;
  z = inZ;
  w = inW;
}

template <typename Type>
inline void nsVec4Template<Type>::SetZero()
{
  x = y = z = w = 0;
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE NS_ALWAYS_INLINE Type nsVec4Template<Type>::GetLength() const
{
  return (nsMath::Sqrt(GetLengthSquared()));
}

template <typename Type>
NS_FORCE_INLINE Type nsVec4Template<Type>::GetLengthSquared() const
{
  NS_NAN_ASSERT(this);

  return (x * x + y * y + z * z + w * w);
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE NS_FORCE_INLINE Type nsVec4Template<Type>::GetLengthAndNormalize()
{
  const Type fLength = GetLength();
  *this /= fLength;
  return fLength;
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE NS_FORCE_INLINE const nsVec4Template<Type> nsVec4Template<Type>::GetNormalized() const
{
  const Type fLen = GetLength();

  const Type fLengthInv = nsMath::Invert(fLen);
  return nsVec4Template<Type>(x * fLengthInv, y * fLengthInv, z * fLengthInv, w * fLengthInv);
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE NS_ALWAYS_INLINE void nsVec4Template<Type>::Normalize()
{
  *this /= GetLength();
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE inline nsResult nsVec4Template<Type>::NormalizeIfNotZero(const nsVec4Template<Type>& vFallback, Type fEpsilon)
{
  NS_NAN_ASSERT(&vFallback);

  const Type fLength = GetLength();

  if (!nsMath::IsFinite(fLength) || nsMath::IsZero(fLength, fEpsilon))
  {
    *this = vFallback;
    return NS_FAILURE;
  }

  *this /= fLength;
  return NS_SUCCESS;
}

/*! \note Normalization, especially with SSE is not very precise. So this function checks whether the (squared)
  length is between a lower and upper limit.
*/
template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE inline bool nsVec4Template<Type>::IsNormalized(Type fEpsilon /* = nsMath::HugeEpsilon<Type>() */) const
{
  const Type t = GetLengthSquared();
  return nsMath::IsEqual(t, (Type)1, fEpsilon);
}

template <typename Type>
inline bool nsVec4Template<Type>::IsZero() const
{
  NS_NAN_ASSERT(this);

  return ((x == 0.0f) && (y == 0.0f) && (z == 0.0f) && (w == 0.0f));
}

template <typename Type>
inline bool nsVec4Template<Type>::IsZero(Type fEpsilon) const
{
  NS_NAN_ASSERT(this);

  return (nsMath::IsZero(x, fEpsilon) && nsMath::IsZero(y, fEpsilon) && nsMath::IsZero(z, fEpsilon) && nsMath::IsZero(w, fEpsilon));
}

template <typename Type>
inline bool nsVec4Template<Type>::IsNaN() const
{
  if (nsMath::IsNaN(x))
    return true;
  if (nsMath::IsNaN(y))
    return true;
  if (nsMath::IsNaN(z))
    return true;
  if (nsMath::IsNaN(w))
    return true;

  return false;
}

template <typename Type>
inline bool nsVec4Template<Type>::IsValid() const
{
  if (!nsMath::IsFinite(x))
    return false;
  if (!nsMath::IsFinite(y))
    return false;
  if (!nsMath::IsFinite(z))
    return false;
  if (!nsMath::IsFinite(w))
    return false;

  return true;
}

template <typename Type>
NS_FORCE_INLINE const nsVec4Template<Type> nsVec4Template<Type>::operator-() const
{
  NS_NAN_ASSERT(this);

  return nsVec4Template<Type>(-x, -y, -z, -w);
}

template <typename Type>
NS_FORCE_INLINE void nsVec4Template<Type>::operator+=(const nsVec4Template<Type>& vCc)
{
  x += vCc.x;
  y += vCc.y;
  z += vCc.z;
  w += vCc.w;

  NS_NAN_ASSERT(this);
}

template <typename Type>
NS_FORCE_INLINE void nsVec4Template<Type>::operator-=(const nsVec4Template<Type>& vCc)
{
  x -= vCc.x;
  y -= vCc.y;
  z -= vCc.z;
  w -= vCc.w;

  NS_NAN_ASSERT(this);
}

template <typename Type>
NS_FORCE_INLINE void nsVec4Template<Type>::operator*=(Type f)
{
  x *= f;
  y *= f;
  z *= f;
  w *= f;

  NS_NAN_ASSERT(this);
}

template <typename Type>
NS_FORCE_INLINE void nsVec4Template<Type>::operator/=(Type f)
{
  if constexpr (std::is_floating_point_v<Type>)
  {
    const Type f_inv = nsMath::Invert(f);
    x *= f_inv;
    y *= f_inv;
    z *= f_inv;
    w *= f_inv;
  }
  else
  {
    x /= f;
    y /= f;
    z /= f;
    w /= f;
  }

  NS_NAN_ASSERT(this);
}

template <typename Type>
NS_FORCE_INLINE Type nsVec4Template<Type>::Dot(const nsVec4Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return ((x * rhs.x) + (y * rhs.y) + (z * rhs.z) + (w * rhs.w));
}

template <typename Type>
inline const nsVec4Template<Type> nsVec4Template<Type>::CompMin(const nsVec4Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return nsVec4Template<Type>(nsMath::Min(x, rhs.x), nsMath::Min(y, rhs.y), nsMath::Min(z, rhs.z), nsMath::Min(w, rhs.w));
}

template <typename Type>
inline const nsVec4Template<Type> nsVec4Template<Type>::CompMax(const nsVec4Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return nsVec4Template<Type>(nsMath::Max(x, rhs.x), nsMath::Max(y, rhs.y), nsMath::Max(z, rhs.z), nsMath::Max(w, rhs.w));
}

template <typename Type>
inline const nsVec4Template<Type> nsVec4Template<Type>::CompClamp(const nsVec4Template& vLow, const nsVec4Template& vHigh) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&vLow);
  NS_NAN_ASSERT(&vHigh);

  return nsVec4Template<Type>(nsMath::Clamp(x, vLow.x, vHigh.x), nsMath::Clamp(y, vLow.y, vHigh.y), nsMath::Clamp(z, vLow.z, vHigh.z), nsMath::Clamp(w, vLow.w, vHigh.w));
}

template <typename Type>
inline const nsVec4Template<Type> nsVec4Template<Type>::CompMul(const nsVec4Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return nsVec4Template<Type>(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w);
}

NS_MSVC_ANALYSIS_WARNING_PUSH
NS_MSVC_ANALYSIS_WARNING_DISABLE(4723)
template <typename Type>
inline const nsVec4Template<Type> nsVec4Template<Type>::CompDiv(const nsVec4Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return nsVec4Template<Type>(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w);
}
NS_MSVC_ANALYSIS_WARNING_POP

template <typename Type>
inline const nsVec4Template<Type> nsVec4Template<Type>::Abs() const
{
  NS_NAN_ASSERT(this);

  return nsVec4Template<Type>(nsMath::Abs(x), nsMath::Abs(y), nsMath::Abs(z), nsMath::Abs(w));
}

template <typename Type>
NS_FORCE_INLINE const nsVec4Template<Type> operator+(const nsVec4Template<Type>& v1, const nsVec4Template<Type>& v2)
{
  NS_NAN_ASSERT(&v1);
  NS_NAN_ASSERT(&v2);

  return nsVec4Template<Type>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

template <typename Type>
NS_FORCE_INLINE const nsVec4Template<Type> operator-(const nsVec4Template<Type>& v1, const nsVec4Template<Type>& v2)
{
  NS_NAN_ASSERT(&v1);
  NS_NAN_ASSERT(&v2);

  return nsVec4Template<Type>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}

template <typename Type>
NS_FORCE_INLINE const nsVec4Template<Type> operator*(Type f, const nsVec4Template<Type>& v)
{
  NS_NAN_ASSERT(&v);

  return nsVec4Template<Type>(v.x * f, v.y * f, v.z * f, v.w * f);
}

template <typename Type>
NS_FORCE_INLINE const nsVec4Template<Type> operator*(const nsVec4Template<Type>& v, Type f)
{
  NS_NAN_ASSERT(&v);

  return nsVec4Template<Type>(v.x * f, v.y * f, v.z * f, v.w * f);
}

template <typename Type>
NS_FORCE_INLINE const nsVec4Template<Type> operator/(const nsVec4Template<Type>& v, Type f)
{
  NS_NAN_ASSERT(&v);

  if constexpr (std::is_floating_point_v<Type>)
  {
    // multiplication is much faster than division
    const Type f_inv = nsMath::Invert(f);
    return nsVec4Template<Type>(v.x * f_inv, v.y * f_inv, v.z * f_inv, v.w * f_inv);
  }
  else
  {
    return nsVec4Template<Type>(v.x / f, v.y / f, v.z / f, v.w / f);
  }
}

template <typename Type>
inline bool nsVec4Template<Type>::IsIdentical(const nsVec4Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return ((x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w));
}

template <typename Type>
inline bool nsVec4Template<Type>::IsEqual(const nsVec4Template<Type>& rhs, Type fEpsilon) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return (nsMath::IsEqual(x, rhs.x, fEpsilon) && nsMath::IsEqual(y, rhs.y, fEpsilon) && nsMath::IsEqual(z, rhs.z, fEpsilon) && nsMath::IsEqual(w, rhs.w, fEpsilon));
}

template <typename Type>
NS_ALWAYS_INLINE bool operator==(const nsVec4Template<Type>& v1, const nsVec4Template<Type>& v2)
{
  return v1.IsIdentical(v2);
}

template <typename Type>
NS_ALWAYS_INLINE bool operator!=(const nsVec4Template<Type>& v1, const nsVec4Template<Type>& v2)
{
  return !v1.IsIdentical(v2);
}

template <typename Type>
NS_FORCE_INLINE bool operator<(const nsVec4Template<Type>& v1, const nsVec4Template<Type>& v2)
{
  NS_NAN_ASSERT(&v1);
  NS_NAN_ASSERT(&v2);

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
