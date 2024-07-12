#pragma once

template <typename Type>
NS_FORCE_INLINE nsVec3Template<Type>::nsVec3Template()
{
#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  const Type TypeNaN = nsMath::NaN<Type>();
  x = TypeNaN;
  y = TypeNaN;
  z = TypeNaN;
#endif
}

template <typename Type>
NS_ALWAYS_INLINE nsVec3Template<Type>::nsVec3Template(Type x, Type y, Type z)
  : x(x)
  , y(y)
  , z(z)
{
}

template <typename Type>
NS_ALWAYS_INLINE nsVec3Template<Type>::nsVec3Template(Type v)
  : x(v)
  , y(v)
  , z(v)
{
}

template <typename Type>
NS_ALWAYS_INLINE void nsVec3Template<Type>::Set(Type xyz)
{
  x = xyz;
  y = xyz;
  z = xyz;
}

template <typename Type>
NS_ALWAYS_INLINE void nsVec3Template<Type>::Set(Type inX, Type inY, Type inZ)
{
  x = inX;
  y = inY;
  z = inZ;
}

template <typename Type>
NS_ALWAYS_INLINE void nsVec3Template<Type>::SetZero()
{
  x = y = z = 0;
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE NS_ALWAYS_INLINE Type nsVec3Template<Type>::GetLength() const
{
  return (nsMath::Sqrt(GetLengthSquared()));
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE nsResult nsVec3Template<Type>::SetLength(Type fNewLength, Type fEpsilon /* = nsMath::DefaultEpsilon<Type>() */)
{
  if (NormalizeIfNotZero(nsVec3Template<Type>::MakeZero(), fEpsilon) == NS_FAILURE)
    return NS_FAILURE;

  *this *= fNewLength;
  return NS_SUCCESS;
}

template <typename Type>
NS_FORCE_INLINE Type nsVec3Template<Type>::GetLengthSquared() const
{
  NS_NAN_ASSERT(this);

  return (x * x + y * y + z * z);
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE NS_FORCE_INLINE Type nsVec3Template<Type>::GetLengthAndNormalize()
{
  const Type fLength = GetLength();
  *this /= fLength;
  return fLength;
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE NS_FORCE_INLINE const nsVec3Template<Type> nsVec3Template<Type>::GetNormalized() const
{
  const Type fLen = GetLength();

  const Type fLengthInv = nsMath::Invert(fLen);
  return nsVec3Template<Type>(x * fLengthInv, y * fLengthInv, z * fLengthInv);
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE NS_ALWAYS_INLINE void nsVec3Template<Type>::Normalize()
{
  *this /= GetLength();
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE nsResult nsVec3Template<Type>::NormalizeIfNotZero(const nsVec3Template<Type>& vFallback, Type fEpsilon)
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
NS_IMPLEMENT_IF_FLOAT_TYPE NS_FORCE_INLINE bool nsVec3Template<Type>::IsNormalized(Type fEpsilon /* = nsMath::HugeEpsilon<Type>() */) const
{
  const Type t = GetLengthSquared();
  return nsMath::IsEqual(t, (Type)1, fEpsilon);
}

template <typename Type>
NS_FORCE_INLINE bool nsVec3Template<Type>::IsZero() const
{
  NS_NAN_ASSERT(this);

  return ((x == 0.0f) && (y == 0.0f) && (z == 0.0f));
}

template <typename Type>
bool nsVec3Template<Type>::IsZero(Type fEpsilon) const
{
  NS_NAN_ASSERT(this);

  return (nsMath::IsZero(x, fEpsilon) && nsMath::IsZero(y, fEpsilon) && nsMath::IsZero(z, fEpsilon));
}

template <typename Type>
bool nsVec3Template<Type>::IsNaN() const
{
  if (nsMath::IsNaN(x))
    return true;
  if (nsMath::IsNaN(y))
    return true;
  if (nsMath::IsNaN(z))
    return true;

  return false;
}

template <typename Type>
bool nsVec3Template<Type>::IsValid() const
{
  if (!nsMath::IsFinite(x))
    return false;
  if (!nsMath::IsFinite(y))
    return false;
  if (!nsMath::IsFinite(z))
    return false;

  return true;
}

template <typename Type>
NS_FORCE_INLINE const nsVec3Template<Type> nsVec3Template<Type>::operator-() const
{
  NS_NAN_ASSERT(this);

  return nsVec3Template<Type>(-x, -y, -z);
}

template <typename Type>
NS_FORCE_INLINE void nsVec3Template<Type>::operator+=(const nsVec3Template<Type>& rhs)
{
  x += rhs.x;
  y += rhs.y;
  z += rhs.z;

  NS_NAN_ASSERT(this);
}

template <typename Type>
NS_FORCE_INLINE void nsVec3Template<Type>::operator-=(const nsVec3Template<Type>& rhs)
{
  x -= rhs.x;
  y -= rhs.y;
  z -= rhs.z;

  NS_NAN_ASSERT(this);
}

template <typename Type>
NS_FORCE_INLINE void nsVec3Template<Type>::operator*=(const nsVec3Template& rhs)
{
  /// \test this is new

  x *= rhs.x;
  y *= rhs.y;
  z *= rhs.z;

  NS_NAN_ASSERT(this);
}

template <typename Type>
NS_FORCE_INLINE void nsVec3Template<Type>::operator/=(const nsVec3Template& rhs)
{
  /// \test this is new

  x /= rhs.x;
  y /= rhs.y;
  z /= rhs.z;

  NS_NAN_ASSERT(this);
}

template <typename Type>
NS_FORCE_INLINE void nsVec3Template<Type>::operator*=(Type f)
{
  x *= f;
  y *= f;
  z *= f;

  NS_NAN_ASSERT(this);
}

template <typename Type>
NS_FORCE_INLINE void nsVec3Template<Type>::operator/=(Type f)
{
  if constexpr (std::is_floating_point_v<Type>)
  {
    const Type f_inv = nsMath::Invert(f);
    x *= f_inv;
    y *= f_inv;
    z *= f_inv;
  }
  else
  {
    x /= f;
    y /= f;
    z /= f;
  }

  // if this assert fires, you might have tried to normalize a zero-length vector
  NS_NAN_ASSERT(this);
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE nsResult nsVec3Template<Type>::CalculateNormal(const nsVec3Template<Type>& v1, const nsVec3Template<Type>& v2, const nsVec3Template<Type>& v3)
{
  *this = (v3 - v2).CrossRH(v1 - v2);
  return NormalizeIfNotZero();
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE void nsVec3Template<Type>::MakeOrthogonalTo(const nsVec3Template<Type>& vNormal)
{
  NS_ASSERT_DEBUG(
    vNormal.IsNormalized(), "The vector to make this vector orthogonal to, must be normalized. It's length is {0}", nsArgF(vNormal.GetLength(), 3));

  nsVec3Template<Type> vOrtho = vNormal.CrossRH(*this);
  *this = vOrtho.CrossRH(vNormal);
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE const nsVec3Template<Type> nsVec3Template<Type>::GetOrthogonalVector() const
{
  NS_ASSERT_DEBUG(!IsZero(nsMath::SmallEpsilon<Type>()), "The vector must not be zero to be able to compute an orthogonal vector.");

  Type fDot = nsMath::Abs(this->Dot(nsVec3Template<Type>(0, 1, 0)));
  if (fDot < 0.999f)
    return this->CrossRH(nsVec3Template<Type>(0, 1, 0));

  return this->CrossRH(nsVec3Template<Type>(1, 0, 0));
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE const nsVec3Template<Type> nsVec3Template<Type>::GetReflectedVector(const nsVec3Template<Type>& vNormal) const
{
  NS_ASSERT_DEBUG(vNormal.IsNormalized(), "vNormal must be normalized.");

  return ((*this) - ((Type)2 * this->Dot(vNormal) * vNormal));
}

template <typename Type>
NS_FORCE_INLINE Type nsVec3Template<Type>::Dot(const nsVec3Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return ((x * rhs.x) + (y * rhs.y) + (z * rhs.z));
}

template <typename Type>
const nsVec3Template<Type> nsVec3Template<Type>::CrossRH(const nsVec3Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return nsVec3Template<Type>(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
}

template <typename Type>
nsAngle nsVec3Template<Type>::GetAngleBetween(const nsVec3Template<Type>& rhs) const
{
  NS_ASSERT_DEBUG(this->IsNormalized(), "This vector must be normalized.");
  NS_ASSERT_DEBUG(rhs.IsNormalized(), "The other vector must be normalized.");

  return nsMath::ACos(static_cast<float>(nsMath::Clamp(this->Dot(rhs), (Type)-1, (Type)1)));
}

template <typename Type>
NS_FORCE_INLINE const nsVec3Template<Type> nsVec3Template<Type>::CompMin(const nsVec3Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return nsVec3Template<Type>(nsMath::Min(x, rhs.x), nsMath::Min(y, rhs.y), nsMath::Min(z, rhs.z));
}

template <typename Type>
NS_FORCE_INLINE const nsVec3Template<Type> nsVec3Template<Type>::CompMax(const nsVec3Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return nsVec3Template<Type>(nsMath::Max(x, rhs.x), nsMath::Max(y, rhs.y), nsMath::Max(z, rhs.z));
}

template <typename Type>
NS_FORCE_INLINE const nsVec3Template<Type> nsVec3Template<Type>::CompClamp(const nsVec3Template& vLow, const nsVec3Template& vHigh) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&vLow);
  NS_NAN_ASSERT(&vHigh);

  return nsVec3Template<Type>(nsMath::Clamp(x, vLow.x, vHigh.x), nsMath::Clamp(y, vLow.y, vHigh.y), nsMath::Clamp(z, vLow.z, vHigh.z));
}

template <typename Type>
NS_FORCE_INLINE const nsVec3Template<Type> nsVec3Template<Type>::CompMul(const nsVec3Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return nsVec3Template<Type>(x * rhs.x, y * rhs.y, z * rhs.z);
}

template <typename Type>
NS_FORCE_INLINE const nsVec3Template<Type> nsVec3Template<Type>::CompDiv(const nsVec3Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return nsVec3Template<Type>(x / rhs.x, y / rhs.y, z / rhs.z);
}

template <typename Type>
inline const nsVec3Template<Type> nsVec3Template<Type>::Abs() const
{
  NS_NAN_ASSERT(this);

  return nsVec3Template<Type>(nsMath::Abs(x), nsMath::Abs(y), nsMath::Abs(z));
}

template <typename Type>
NS_FORCE_INLINE const nsVec3Template<Type> operator+(const nsVec3Template<Type>& v1, const nsVec3Template<Type>& v2)
{
  NS_NAN_ASSERT(&v1);
  NS_NAN_ASSERT(&v2);

  return nsVec3Template<Type>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

template <typename Type>
NS_FORCE_INLINE const nsVec3Template<Type> operator-(const nsVec3Template<Type>& v1, const nsVec3Template<Type>& v2)
{
  NS_NAN_ASSERT(&v1);
  NS_NAN_ASSERT(&v2);

  return nsVec3Template<Type>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

template <typename Type>
NS_FORCE_INLINE const nsVec3Template<Type> operator*(Type f, const nsVec3Template<Type>& v)
{
  NS_NAN_ASSERT(&v);

  return nsVec3Template<Type>(v.x * f, v.y * f, v.z * f);
}

template <typename Type>
NS_FORCE_INLINE const nsVec3Template<Type> operator*(const nsVec3Template<Type>& v, Type f)
{
  NS_NAN_ASSERT(&v);

  return nsVec3Template<Type>(v.x * f, v.y * f, v.z * f);
}

template <typename Type>
NS_FORCE_INLINE const nsVec3Template<Type> operator/(const nsVec3Template<Type>& v, Type f)
{
  NS_NAN_ASSERT(&v);

  if constexpr (std::is_floating_point_v<Type>)
  {
    // multiplication is much faster than division
    const Type f_inv = nsMath::Invert(f);
    return nsVec3Template<Type>(v.x * f_inv, v.y * f_inv, v.z * f_inv);
  }
  else
  {
    return nsVec3Template<Type>(v.x / f, v.y / f, v.z / f);
  }
}

template <typename Type>
bool nsVec3Template<Type>::IsIdentical(const nsVec3Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return ((x == rhs.x) && (y == rhs.y) && (z == rhs.z));
}

template <typename Type>
bool nsVec3Template<Type>::IsEqual(const nsVec3Template<Type>& rhs, Type fEpsilon) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return (nsMath::IsEqual(x, rhs.x, fEpsilon) && nsMath::IsEqual(y, rhs.y, fEpsilon) && nsMath::IsEqual(z, rhs.z, fEpsilon));
}

template <typename Type>
NS_ALWAYS_INLINE bool operator==(const nsVec3Template<Type>& v1, const nsVec3Template<Type>& v2)
{
  return v1.IsIdentical(v2);
}

template <typename Type>
NS_ALWAYS_INLINE bool operator!=(const nsVec3Template<Type>& v1, const nsVec3Template<Type>& v2)
{
  return !v1.IsIdentical(v2);
}

template <typename Type>
NS_FORCE_INLINE bool operator<(const nsVec3Template<Type>& v1, const nsVec3Template<Type>& v2)
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

  return (v1.z < v2.z);
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE const nsVec3Template<Type> nsVec3Template<Type>::GetRefractedVector(const nsVec3Template<Type>& vNormal, Type fRefIndex1, Type fRefIndex2) const
{
  NS_ASSERT_DEBUG(vNormal.IsNormalized(), "vNormal must be normalized.");

  const Type n = fRefIndex1 / fRefIndex2;
  const Type cosI = this->Dot(vNormal);
  const Type sinT2 = n * n * (1.0f - (cosI * cosI));

  // invalid refraction
  if (sinT2 > 1.0f)
    return (*this);

  return ((n * (*this)) - (n + nsMath::Sqrt(1.0f - sinT2)) * vNormal);
}
