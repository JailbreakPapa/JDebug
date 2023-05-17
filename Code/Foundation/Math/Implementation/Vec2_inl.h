#pragma once

template <typename Type>
WD_ALWAYS_INLINE wdVec2Template<Type>::wdVec2Template()
{
#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  const Type TypeNaN = wdMath::NaN<Type>();
  x = TypeNaN;
  y = TypeNaN;
#endif
}

template <typename Type>
WD_ALWAYS_INLINE wdVec2Template<Type>::wdVec2Template(Type x, Type y)
  : x(x)
  , y(y)
{
}

template <typename Type>
WD_ALWAYS_INLINE wdVec2Template<Type>::wdVec2Template(Type v)
  : x(v)
  , y(v)
{
}

template <typename Type>
WD_ALWAYS_INLINE void wdVec2Template<Type>::Set(Type xy)
{
  x = xy;
  y = xy;
}

template <typename Type>
WD_ALWAYS_INLINE void wdVec2Template<Type>::Set(Type inX, Type inY)
{
  x = inX;
  y = inY;
}

template <typename Type>
WD_ALWAYS_INLINE void wdVec2Template<Type>::SetZero()
{
  x = y = 0;
}

template <typename Type>
WD_ALWAYS_INLINE Type wdVec2Template<Type>::GetLength() const
{
  return (wdMath::Sqrt(GetLengthSquared()));
}

template <typename Type>
wdResult wdVec2Template<Type>::SetLength(Type fNewLength, Type fEpsilon /* = wdMath::DefaultEpsilon<Type>() */)
{
  if (NormalizeIfNotZero(wdVec2Template<Type>::ZeroVector(), fEpsilon) == WD_FAILURE)
    return WD_FAILURE;

  *this *= fNewLength;
  return WD_SUCCESS;
}

template <typename Type>
WD_ALWAYS_INLINE Type wdVec2Template<Type>::GetLengthSquared() const
{
  return (x * x + y * y);
}

template <typename Type>
WD_FORCE_INLINE Type wdVec2Template<Type>::GetLengthAndNormalize()
{
  const Type fLength = GetLength();
  *this /= fLength;
  return fLength;
}

template <typename Type>
WD_FORCE_INLINE const wdVec2Template<Type> wdVec2Template<Type>::GetNormalized() const
{
  const Type fLen = GetLength();

  const Type fLengthInv = wdMath::Invert(fLen);
  return wdVec2Template<Type>(x * fLengthInv, y * fLengthInv);
}

template <typename Type>
WD_ALWAYS_INLINE void wdVec2Template<Type>::Normalize()
{
  *this /= GetLength();
}

template <typename Type>
inline wdResult wdVec2Template<Type>::NormalizeIfNotZero(const wdVec2Template<Type>& vFallback, Type fEpsilon)
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
inline bool wdVec2Template<Type>::IsNormalized(Type fEpsilon /* = wdMath::HugeEpsilon<Type>() */) const
{
  const Type t = GetLengthSquared();
  return wdMath::IsEqual(t, (Type)(1), fEpsilon);
}

template <typename Type>
inline bool wdVec2Template<Type>::IsZero() const
{
  return (x == 0 && y == 0);
}

template <typename Type>
inline bool wdVec2Template<Type>::IsZero(Type fEpsilon) const
{
  WD_NAN_ASSERT(this);

  return (wdMath::IsZero(x, fEpsilon) && wdMath::IsZero(y, fEpsilon));
}

template <typename Type>
inline bool wdVec2Template<Type>::IsNaN() const
{
  if (wdMath::IsNaN(x))
    return true;
  if (wdMath::IsNaN(y))
    return true;

  return false;
}

template <typename Type>
inline bool wdVec2Template<Type>::IsValid() const
{
  if (!wdMath::IsFinite(x))
    return false;
  if (!wdMath::IsFinite(y))
    return false;

  return true;
}

template <typename Type>
WD_FORCE_INLINE const wdVec2Template<Type> wdVec2Template<Type>::operator-() const
{
  WD_NAN_ASSERT(this);

  return wdVec2Template<Type>(-x, -y);
}

template <typename Type>
WD_FORCE_INLINE void wdVec2Template<Type>::operator+=(const wdVec2Template<Type>& rhs)
{
  x += rhs.x;
  y += rhs.y;

  WD_NAN_ASSERT(this);
}

template <typename Type>
WD_FORCE_INLINE void wdVec2Template<Type>::operator-=(const wdVec2Template<Type>& rhs)
{
  x -= rhs.x;
  y -= rhs.y;

  WD_NAN_ASSERT(this);
}

template <typename Type>
WD_FORCE_INLINE void wdVec2Template<Type>::operator*=(Type f)
{
  x *= f;
  y *= f;

  WD_NAN_ASSERT(this);
}

template <typename Type>
WD_FORCE_INLINE void wdVec2Template<Type>::operator/=(Type f)
{
  const Type f_inv = wdMath::Invert(f);

  x *= f_inv;
  y *= f_inv;

  WD_NAN_ASSERT(this);
}

template <typename Type>
inline void wdVec2Template<Type>::MakeOrthogonalTo(const wdVec2Template<Type>& vNormal)
{
  WD_ASSERT_DEBUG(vNormal.IsNormalized(), "The normal must be normalized.");

  const Type fDot = this->Dot(vNormal);
  *this -= fDot * vNormal;
}

template <typename Type>
WD_FORCE_INLINE const wdVec2Template<Type> wdVec2Template<Type>::GetOrthogonalVector() const
{
  WD_NAN_ASSERT(this);
  WD_ASSERT_DEBUG(!IsZero(wdMath::SmallEpsilon<Type>()), "The vector must not be zero to be able to compute an orthogonal vector.");

  return wdVec2Template<Type>(-y, x);
}

template <typename Type>
inline const wdVec2Template<Type> wdVec2Template<Type>::GetReflectedVector(const wdVec2Template<Type>& vNormal) const
{
  WD_ASSERT_DEBUG(vNormal.IsNormalized(), "vNormal must be normalized.");

  return ((*this) - (2 * this->Dot(vNormal) * vNormal));
}

template <typename Type>
WD_FORCE_INLINE Type wdVec2Template<Type>::Dot(const wdVec2Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return ((x * rhs.x) + (y * rhs.y));
}

template <typename Type>
inline wdAngle wdVec2Template<Type>::GetAngleBetween(const wdVec2Template<Type>& rhs) const
{
  WD_ASSERT_DEBUG(this->IsNormalized(), "This vector must be normalized.");
  WD_ASSERT_DEBUG(rhs.IsNormalized(), "The other vector must be normalized.");

  return wdMath::ACos(static_cast<float>(wdMath::Clamp<Type>(this->Dot(rhs), (Type)-1, (Type)1)));
}

template <typename Type>
WD_FORCE_INLINE const wdVec2Template<Type> wdVec2Template<Type>::CompMin(const wdVec2Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return wdVec2Template<Type>(wdMath::Min(x, rhs.x), wdMath::Min(y, rhs.y));
}

template <typename Type>
WD_FORCE_INLINE const wdVec2Template<Type> wdVec2Template<Type>::CompMax(const wdVec2Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return wdVec2Template<Type>(wdMath::Max(x, rhs.x), wdMath::Max(y, rhs.y));
}

template <typename Type>
WD_FORCE_INLINE const wdVec2Template<Type> wdVec2Template<Type>::CompClamp(const wdVec2Template<Type>& vLow, const wdVec2Template<Type>& vHigh) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&vLow);
  WD_NAN_ASSERT(&vHigh);

  return wdVec2Template<Type>(wdMath::Clamp(x, vLow.x, vHigh.x), wdMath::Clamp(y, vLow.y, vHigh.y));
}

template <typename Type>
WD_FORCE_INLINE const wdVec2Template<Type> wdVec2Template<Type>::CompMul(const wdVec2Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return wdVec2Template<Type>(x * rhs.x, y * rhs.y);
}

template <typename Type>
WD_FORCE_INLINE const wdVec2Template<Type> wdVec2Template<Type>::CompDiv(const wdVec2Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return wdVec2Template<Type>(x / rhs.x, y / rhs.y);
}

template <typename Type>
inline const wdVec2Template<Type> wdVec2Template<Type>::Abs() const
{
  WD_NAN_ASSERT(this);

  return wdVec2Template<Type>(wdMath::Abs(x), wdMath::Abs(y));
}

template <typename Type>
WD_FORCE_INLINE const wdVec2Template<Type> operator+(const wdVec2Template<Type>& v1, const wdVec2Template<Type>& v2)
{
  WD_NAN_ASSERT(&v1);
  WD_NAN_ASSERT(&v2);

  return wdVec2Template<Type>(v1.x + v2.x, v1.y + v2.y);
}

template <typename Type>
WD_FORCE_INLINE const wdVec2Template<Type> operator-(const wdVec2Template<Type>& v1, const wdVec2Template<Type>& v2)
{
  WD_NAN_ASSERT(&v1);
  WD_NAN_ASSERT(&v2);

  return wdVec2Template<Type>(v1.x - v2.x, v1.y - v2.y);
}

template <typename Type>
WD_FORCE_INLINE const wdVec2Template<Type> operator*(Type f, const wdVec2Template<Type>& v)
{
  WD_NAN_ASSERT(&v);

  return wdVec2Template<Type>(v.x * f, v.y * f);
}

template <typename Type>
WD_FORCE_INLINE const wdVec2Template<Type> operator*(const wdVec2Template<Type>& v, Type f)
{
  WD_NAN_ASSERT(&v);

  return wdVec2Template<Type>(v.x * f, v.y * f);
}

template <typename Type>
WD_FORCE_INLINE const wdVec2Template<Type> operator/(const wdVec2Template<Type>& v, Type f)
{
  WD_NAN_ASSERT(&v);

  // multiplication is much faster than division
  const Type f_inv = wdMath::Invert(f);
  return wdVec2Template<Type>(v.x * f_inv, v.y * f_inv);
}

template <typename Type>
inline bool wdVec2Template<Type>::IsIdentical(const wdVec2Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return ((x == rhs.x) && (y == rhs.y));
}

template <typename Type>
inline bool wdVec2Template<Type>::IsEqual(const wdVec2Template<Type>& rhs, Type fEpsilon) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return (wdMath::IsEqual(x, rhs.x, fEpsilon) && wdMath::IsEqual(y, rhs.y, fEpsilon));
}

template <typename Type>
WD_FORCE_INLINE bool operator==(const wdVec2Template<Type>& v1, const wdVec2Template<Type>& v2)
{
  return v1.IsIdentical(v2);
}

template <typename Type>
WD_FORCE_INLINE bool operator!=(const wdVec2Template<Type>& v1, const wdVec2Template<Type>& v2)
{
  return !v1.IsIdentical(v2);
}

template <typename Type>
WD_FORCE_INLINE bool operator<(const wdVec2Template<Type>& v1, const wdVec2Template<Type>& v2)
{
  WD_NAN_ASSERT(&v1);
  WD_NAN_ASSERT(&v2);

  if (v1.x < v2.x)
    return true;
  if (v1.x > v2.x)
    return false;

  return (v1.y < v2.y);
}
