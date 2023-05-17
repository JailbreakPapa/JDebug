#pragma once

template <typename Type>
WD_FORCE_INLINE wdVec3Template<Type>::wdVec3Template()
{
#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  const Type TypeNaN = wdMath::NaN<Type>();
  x = TypeNaN;
  y = TypeNaN;
  z = TypeNaN;
#endif
}

template <typename Type>
WD_ALWAYS_INLINE wdVec3Template<Type>::wdVec3Template(Type x, Type y, Type z)
  : x(x)
  , y(y)
  , z(z)
{
}

template <typename Type>
WD_ALWAYS_INLINE wdVec3Template<Type>::wdVec3Template(Type v)
  : x(v)
  , y(v)
  , z(v)
{
}

template <typename Type>
WD_ALWAYS_INLINE void wdVec3Template<Type>::Set(Type xyz)
{
  x = xyz;
  y = xyz;
  z = xyz;
}

template <typename Type>
WD_ALWAYS_INLINE void wdVec3Template<Type>::Set(Type inX, Type inY, Type inZ)
{
  x = inX;
  y = inY;
  z = inZ;
}

template <typename Type>
WD_ALWAYS_INLINE void wdVec3Template<Type>::SetZero()
{
  x = y = z = 0;
}

template <typename Type>
WD_ALWAYS_INLINE Type wdVec3Template<Type>::GetLength() const
{
  return (wdMath::Sqrt(GetLengthSquared()));
}

template <typename Type>
wdResult wdVec3Template<Type>::SetLength(Type fNewLength, Type fEpsilon /* = wdMath::DefaultEpsilon<Type>() */)
{
  if (NormalizeIfNotZero(wdVec3Template<Type>::ZeroVector(), fEpsilon) == WD_FAILURE)
    return WD_FAILURE;

  *this *= fNewLength;
  return WD_SUCCESS;
}

template <typename Type>
WD_FORCE_INLINE Type wdVec3Template<Type>::GetLengthSquared() const
{
  WD_NAN_ASSERT(this);

  return (x * x + y * y + z * z);
}

template <typename Type>
WD_FORCE_INLINE Type wdVec3Template<Type>::GetLengthAndNormalize()
{
  const Type fLength = GetLength();
  *this /= fLength;
  return fLength;
}

template <typename Type>
WD_FORCE_INLINE const wdVec3Template<Type> wdVec3Template<Type>::GetNormalized() const
{
  const Type fLen = GetLength();

  const Type fLengthInv = wdMath::Invert(fLen);
  return wdVec3Template<Type>(x * fLengthInv, y * fLengthInv, z * fLengthInv);
}

template <typename Type>
WD_ALWAYS_INLINE void wdVec3Template<Type>::Normalize()
{
  *this /= GetLength();
}

template <typename Type>
wdResult wdVec3Template<Type>::NormalizeIfNotZero(const wdVec3Template<Type>& vFallback, Type fEpsilon)
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
WD_FORCE_INLINE bool wdVec3Template<Type>::IsNormalized(Type fEpsilon /* = wdMath::HugeEpsilon<Type>() */) const
{
  const Type t = GetLengthSquared();
  return wdMath::IsEqual(t, (Type)1, fEpsilon);
}

template <typename Type>
WD_FORCE_INLINE bool wdVec3Template<Type>::IsZero() const
{
  WD_NAN_ASSERT(this);

  return ((x == 0.0f) && (y == 0.0f) && (z == 0.0f));
}

template <typename Type>
bool wdVec3Template<Type>::IsZero(Type fEpsilon) const
{
  WD_NAN_ASSERT(this);

  return (wdMath::IsZero(x, fEpsilon) && wdMath::IsZero(y, fEpsilon) && wdMath::IsZero(z, fEpsilon));
}

template <typename Type>
bool wdVec3Template<Type>::IsNaN() const
{
  if (wdMath::IsNaN(x))
    return true;
  if (wdMath::IsNaN(y))
    return true;
  if (wdMath::IsNaN(z))
    return true;

  return false;
}

template <typename Type>
bool wdVec3Template<Type>::IsValid() const
{
  if (!wdMath::IsFinite(x))
    return false;
  if (!wdMath::IsFinite(y))
    return false;
  if (!wdMath::IsFinite(z))
    return false;

  return true;
}

template <typename Type>
WD_FORCE_INLINE const wdVec3Template<Type> wdVec3Template<Type>::operator-() const
{
  WD_NAN_ASSERT(this);

  return wdVec3Template<Type>(-x, -y, -z);
}

template <typename Type>
WD_FORCE_INLINE void wdVec3Template<Type>::operator+=(const wdVec3Template<Type>& rhs)
{
  x += rhs.x;
  y += rhs.y;
  z += rhs.z;

  WD_NAN_ASSERT(this);
}

template <typename Type>
WD_FORCE_INLINE void wdVec3Template<Type>::operator-=(const wdVec3Template<Type>& rhs)
{
  x -= rhs.x;
  y -= rhs.y;
  z -= rhs.z;

  WD_NAN_ASSERT(this);
}

template <typename Type>
WD_FORCE_INLINE void wdVec3Template<Type>::operator*=(const wdVec3Template& rhs)
{
  /// \test this is new

  x *= rhs.x;
  y *= rhs.y;
  z *= rhs.z;

  WD_NAN_ASSERT(this);
}

template <typename Type>
WD_FORCE_INLINE void wdVec3Template<Type>::operator/=(const wdVec3Template& rhs)
{
  /// \test this is new

  x /= rhs.x;
  y /= rhs.y;
  z /= rhs.z;

  WD_NAN_ASSERT(this);
}

template <typename Type>
WD_FORCE_INLINE void wdVec3Template<Type>::operator*=(Type f)
{
  x *= f;
  y *= f;
  z *= f;

  WD_NAN_ASSERT(this);
}

template <typename Type>
WD_FORCE_INLINE void wdVec3Template<Type>::operator/=(Type f)
{
  const Type f_inv = wdMath::Invert(f);

  x *= f_inv;
  y *= f_inv;
  z *= f_inv;

  // if this assert fires, you might have tried to normalize a zero-length vector
  WD_NAN_ASSERT(this);
}

template <typename Type>
wdResult wdVec3Template<Type>::CalculateNormal(const wdVec3Template<Type>& v1, const wdVec3Template<Type>& v2, const wdVec3Template<Type>& v3)
{
  *this = (v3 - v2).CrossRH(v1 - v2);
  return NormalizeIfNotZero();
}

template <typename Type>
void wdVec3Template<Type>::MakeOrthogonalTo(const wdVec3Template<Type>& vNormal)
{
  WD_ASSERT_DEBUG(
    vNormal.IsNormalized(), "The vector to make this vector orthogonal to, must be normalized. It's length is {0}", wdArgF(vNormal.GetLength(), 3));

  wdVec3Template<Type> vOrtho = vNormal.CrossRH(*this);
  *this = vOrtho.CrossRH(vNormal);
}

template <typename Type>
const wdVec3Template<Type> wdVec3Template<Type>::GetOrthogonalVector() const
{
  WD_ASSERT_DEBUG(!IsZero(wdMath::SmallEpsilon<Type>()), "The vector must not be zero to be able to compute an orthogonal vector.");

  Type fDot = wdMath::Abs(this->Dot(wdVec3Template<Type>(0, 1, 0)));
  if (fDot < 0.999f)
    return this->CrossRH(wdVec3Template<Type>(0, 1, 0));

  return this->CrossRH(wdVec3Template<Type>(1, 0, 0));
}

template <typename Type>
const wdVec3Template<Type> wdVec3Template<Type>::GetReflectedVector(const wdVec3Template<Type>& vNormal) const
{
  WD_ASSERT_DEBUG(vNormal.IsNormalized(), "vNormal must be normalized.");

  return ((*this) - ((Type)2 * this->Dot(vNormal) * vNormal));
}

template <typename Type>
WD_FORCE_INLINE Type wdVec3Template<Type>::Dot(const wdVec3Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return ((x * rhs.x) + (y * rhs.y) + (z * rhs.z));
}

template <typename Type>
const wdVec3Template<Type> wdVec3Template<Type>::CrossRH(const wdVec3Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return wdVec3Template<Type>(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
}

template <typename Type>
wdAngle wdVec3Template<Type>::GetAngleBetween(const wdVec3Template<Type>& rhs) const
{
  WD_ASSERT_DEBUG(this->IsNormalized(), "This vector must be normalized.");
  WD_ASSERT_DEBUG(rhs.IsNormalized(), "The other vector must be normalized.");

  return wdMath::ACos(static_cast<float>(wdMath::Clamp(this->Dot(rhs), (Type)-1, (Type)1)));
}

template <typename Type>
WD_FORCE_INLINE const wdVec3Template<Type> wdVec3Template<Type>::CompMin(const wdVec3Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return wdVec3Template<Type>(wdMath::Min(x, rhs.x), wdMath::Min(y, rhs.y), wdMath::Min(z, rhs.z));
}

template <typename Type>
WD_FORCE_INLINE const wdVec3Template<Type> wdVec3Template<Type>::CompMax(const wdVec3Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return wdVec3Template<Type>(wdMath::Max(x, rhs.x), wdMath::Max(y, rhs.y), wdMath::Max(z, rhs.z));
}

template <typename Type>
WD_FORCE_INLINE const wdVec3Template<Type> wdVec3Template<Type>::CompClamp(const wdVec3Template& vLow, const wdVec3Template& vHigh) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&vLow);
  WD_NAN_ASSERT(&vHigh);

  return wdVec3Template<Type>(wdMath::Clamp(x, vLow.x, vHigh.x), wdMath::Clamp(y, vLow.y, vHigh.y), wdMath::Clamp(z, vLow.z, vHigh.z));
}

template <typename Type>
WD_FORCE_INLINE const wdVec3Template<Type> wdVec3Template<Type>::CompMul(const wdVec3Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return wdVec3Template<Type>(x * rhs.x, y * rhs.y, z * rhs.z);
}

template <typename Type>
WD_FORCE_INLINE const wdVec3Template<Type> wdVec3Template<Type>::CompDiv(const wdVec3Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return wdVec3Template<Type>(x / rhs.x, y / rhs.y, z / rhs.z);
}

template <typename Type>
inline const wdVec3Template<Type> wdVec3Template<Type>::Abs() const
{
  WD_NAN_ASSERT(this);

  return wdVec3Template<Type>(wdMath::Abs(x), wdMath::Abs(y), wdMath::Abs(z));
}

template <typename Type>
WD_FORCE_INLINE const wdVec3Template<Type> operator+(const wdVec3Template<Type>& v1, const wdVec3Template<Type>& v2)
{
  WD_NAN_ASSERT(&v1);
  WD_NAN_ASSERT(&v2);

  return wdVec3Template<Type>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

template <typename Type>
WD_FORCE_INLINE const wdVec3Template<Type> operator-(const wdVec3Template<Type>& v1, const wdVec3Template<Type>& v2)
{
  WD_NAN_ASSERT(&v1);
  WD_NAN_ASSERT(&v2);

  return wdVec3Template<Type>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

template <typename Type>
WD_FORCE_INLINE const wdVec3Template<Type> operator*(Type f, const wdVec3Template<Type>& v)
{
  WD_NAN_ASSERT(&v);

  return wdVec3Template<Type>(v.x * f, v.y * f, v.z * f);
}

template <typename Type>
WD_FORCE_INLINE const wdVec3Template<Type> operator*(const wdVec3Template<Type>& v, Type f)
{
  WD_NAN_ASSERT(&v);

  return wdVec3Template<Type>(v.x * f, v.y * f, v.z * f);
}

template <typename Type>
WD_FORCE_INLINE const wdVec3Template<Type> operator/(const wdVec3Template<Type>& v, Type f)
{
  WD_NAN_ASSERT(&v);

  // multiplication is much faster than division
  const Type f_inv = wdMath::Invert(f);
  return wdVec3Template<Type>(v.x * f_inv, v.y * f_inv, v.z * f_inv);
}

template <typename Type>
bool wdVec3Template<Type>::IsIdentical(const wdVec3Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return ((x == rhs.x) && (y == rhs.y) && (z == rhs.z));
}

template <typename Type>
bool wdVec3Template<Type>::IsEqual(const wdVec3Template<Type>& rhs, Type fEpsilon) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return (wdMath::IsEqual(x, rhs.x, fEpsilon) && wdMath::IsEqual(y, rhs.y, fEpsilon) && wdMath::IsEqual(z, rhs.z, fEpsilon));
}

template <typename Type>
WD_ALWAYS_INLINE bool operator==(const wdVec3Template<Type>& v1, const wdVec3Template<Type>& v2)
{
  return v1.IsIdentical(v2);
}

template <typename Type>
WD_ALWAYS_INLINE bool operator!=(const wdVec3Template<Type>& v1, const wdVec3Template<Type>& v2)
{
  return !v1.IsIdentical(v2);
}

template <typename Type>
WD_FORCE_INLINE bool operator<(const wdVec3Template<Type>& v1, const wdVec3Template<Type>& v2)
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

  return (v1.z < v2.z);
}

template <typename Type>
const wdVec3Template<Type> wdVec3Template<Type>::GetRefractedVector(const wdVec3Template<Type>& vNormal, Type fRefIndex1, Type fRefIndex2) const
{
  WD_ASSERT_DEBUG(vNormal.IsNormalized(), "vNormal must be normalized.");

  const Type n = fRefIndex1 / fRefIndex2;
  const Type cosI = this->Dot(vNormal);
  const Type sinT2 = n * n * (1.0f - (cosI * cosI));

  // invalid refraction
  if (sinT2 > 1.0f)
    return (*this);

  return ((n * (*this)) - (n + wdMath::Sqrt(1.0f - sinT2)) * vNormal);
}
