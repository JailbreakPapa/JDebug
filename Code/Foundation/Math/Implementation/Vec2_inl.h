#pragma once

template <typename Type>
NS_ALWAYS_INLINE nsVec2Template<Type>::nsVec2Template()
{
#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  const Type TypeNaN = nsMath::NaN<Type>();
  x = TypeNaN;
  y = TypeNaN;
#endif
}

template <typename Type>
NS_ALWAYS_INLINE nsVec2Template<Type>::nsVec2Template(Type x, Type y)
  : x(x)
  , y(y)
{
}

template <typename Type>
NS_ALWAYS_INLINE nsVec2Template<Type>::nsVec2Template(Type v)
  : x(v)
  , y(v)
{
}

template <typename Type>
NS_ALWAYS_INLINE void nsVec2Template<Type>::Set(Type xy)
{
  x = xy;
  y = xy;
}

template <typename Type>
NS_ALWAYS_INLINE void nsVec2Template<Type>::Set(Type inX, Type inY)
{
  x = inX;
  y = inY;
}

template <typename Type>
NS_ALWAYS_INLINE void nsVec2Template<Type>::SetZero()
{
  x = y = 0;
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE NS_ALWAYS_INLINE Type nsVec2Template<Type>::GetLength() const
{
  return (nsMath::Sqrt(GetLengthSquared()));
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE nsResult nsVec2Template<Type>::SetLength(Type fNewLength, Type fEpsilon /* = nsMath::DefaultEpsilon<Type>() */)
{
  if (NormalizeIfNotZero(nsVec2Template<Type>::MakeZero(), fEpsilon) == NS_FAILURE)
    return NS_FAILURE;

  *this *= fNewLength;
  return NS_SUCCESS;
}

template <typename Type>
NS_ALWAYS_INLINE Type nsVec2Template<Type>::GetLengthSquared() const
{
  return (x * x + y * y);
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE NS_FORCE_INLINE Type nsVec2Template<Type>::GetLengthAndNormalize()
{
  const Type fLength = GetLength();
  *this /= fLength;
  return fLength;
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE NS_FORCE_INLINE const nsVec2Template<Type> nsVec2Template<Type>::GetNormalized() const
{
  const Type fLen = GetLength();

  const Type fLengthInv = nsMath::Invert(fLen);
  return nsVec2Template<Type>(x * fLengthInv, y * fLengthInv);
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE NS_ALWAYS_INLINE void nsVec2Template<Type>::Normalize()
{
  *this /= GetLength();
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE inline nsResult nsVec2Template<Type>::NormalizeIfNotZero(const nsVec2Template<Type>& vFallback, Type fEpsilon)
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
NS_IMPLEMENT_IF_FLOAT_TYPE inline bool nsVec2Template<Type>::IsNormalized(Type fEpsilon /* = nsMath::HugeEpsilon<Type>() */) const
{
  const Type t = GetLengthSquared();
  return nsMath::IsEqual(t, (Type)(1), fEpsilon);
}

template <typename Type>
inline bool nsVec2Template<Type>::IsZero() const
{
  return (x == 0 && y == 0);
}

template <typename Type>
inline bool nsVec2Template<Type>::IsZero(Type fEpsilon) const
{
  NS_NAN_ASSERT(this);

  return (nsMath::IsZero(x, fEpsilon) && nsMath::IsZero(y, fEpsilon));
}

template <typename Type>
inline bool nsVec2Template<Type>::IsNaN() const
{
  if (nsMath::IsNaN(x))
    return true;
  if (nsMath::IsNaN(y))
    return true;

  return false;
}

template <typename Type>
inline bool nsVec2Template<Type>::IsValid() const
{
  if (!nsMath::IsFinite(x))
    return false;
  if (!nsMath::IsFinite(y))
    return false;

  return true;
}

template <typename Type>
NS_FORCE_INLINE const nsVec2Template<Type> nsVec2Template<Type>::operator-() const
{
  NS_NAN_ASSERT(this);

  return nsVec2Template<Type>(-x, -y);
}

template <typename Type>
NS_FORCE_INLINE void nsVec2Template<Type>::operator+=(const nsVec2Template<Type>& rhs)
{
  x += rhs.x;
  y += rhs.y;

  NS_NAN_ASSERT(this);
}

template <typename Type>
NS_FORCE_INLINE void nsVec2Template<Type>::operator-=(const nsVec2Template<Type>& rhs)
{
  x -= rhs.x;
  y -= rhs.y;

  NS_NAN_ASSERT(this);
}

template <typename Type>
NS_FORCE_INLINE void nsVec2Template<Type>::operator*=(Type f)
{
  x *= f;
  y *= f;

  NS_NAN_ASSERT(this);
}

template <typename Type>
NS_FORCE_INLINE void nsVec2Template<Type>::operator/=(Type f)
{
  if constexpr (std::is_floating_point_v<Type>)
  {
    const Type f_inv = nsMath::Invert(f);
    x *= f_inv;
    y *= f_inv;
  }
  else
  {
    x /= f;
    y /= f;
  }

  NS_NAN_ASSERT(this);
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE inline void nsVec2Template<Type>::MakeOrthogonalTo(const nsVec2Template<Type>& vNormal)
{
  NS_ASSERT_DEBUG(vNormal.IsNormalized(), "The normal must be normalized.");

  const Type fDot = this->Dot(vNormal);
  *this -= fDot * vNormal;
}

template <typename Type>
NS_FORCE_INLINE const nsVec2Template<Type> nsVec2Template<Type>::GetOrthogonalVector() const
{
  NS_NAN_ASSERT(this);
  NS_ASSERT_DEBUG(!IsZero(nsMath::SmallEpsilon<Type>()), "The vector must not be zero to be able to compute an orthogonal vector.");

  return nsVec2Template<Type>(-y, x);
}

template <typename Type>
NS_IMPLEMENT_IF_FLOAT_TYPE inline const nsVec2Template<Type> nsVec2Template<Type>::GetReflectedVector(const nsVec2Template<Type>& vNormal) const
{
  NS_ASSERT_DEBUG(vNormal.IsNormalized(), "vNormal must be normalized.");

  return ((*this) - (2 * this->Dot(vNormal) * vNormal));
}

template <typename Type>
NS_FORCE_INLINE Type nsVec2Template<Type>::Dot(const nsVec2Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return ((x * rhs.x) + (y * rhs.y));
}

template <typename Type>
inline nsAngle nsVec2Template<Type>::GetAngleBetween(const nsVec2Template<Type>& rhs) const
{
  NS_ASSERT_DEBUG(this->IsNormalized(), "This vector must be normalized.");
  NS_ASSERT_DEBUG(rhs.IsNormalized(), "The other vector must be normalized.");

  return nsMath::ACos(static_cast<float>(nsMath::Clamp<Type>(this->Dot(rhs), (Type)-1, (Type)1)));
}

template <typename Type>
NS_FORCE_INLINE const nsVec2Template<Type> nsVec2Template<Type>::CompMin(const nsVec2Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return nsVec2Template<Type>(nsMath::Min(x, rhs.x), nsMath::Min(y, rhs.y));
}

template <typename Type>
NS_FORCE_INLINE const nsVec2Template<Type> nsVec2Template<Type>::CompMax(const nsVec2Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return nsVec2Template<Type>(nsMath::Max(x, rhs.x), nsMath::Max(y, rhs.y));
}

template <typename Type>
NS_FORCE_INLINE const nsVec2Template<Type> nsVec2Template<Type>::CompClamp(const nsVec2Template<Type>& vLow, const nsVec2Template<Type>& vHigh) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&vLow);
  NS_NAN_ASSERT(&vHigh);

  return nsVec2Template<Type>(nsMath::Clamp(x, vLow.x, vHigh.x), nsMath::Clamp(y, vLow.y, vHigh.y));
}

template <typename Type>
NS_FORCE_INLINE const nsVec2Template<Type> nsVec2Template<Type>::CompMul(const nsVec2Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return nsVec2Template<Type>(x * rhs.x, y * rhs.y);
}

template <typename Type>
NS_FORCE_INLINE const nsVec2Template<Type> nsVec2Template<Type>::CompDiv(const nsVec2Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return nsVec2Template<Type>(x / rhs.x, y / rhs.y);
}

template <typename Type>
inline const nsVec2Template<Type> nsVec2Template<Type>::Abs() const
{
  NS_NAN_ASSERT(this);

  return nsVec2Template<Type>(nsMath::Abs(x), nsMath::Abs(y));
}

template <typename Type>
NS_FORCE_INLINE const nsVec2Template<Type> operator+(const nsVec2Template<Type>& v1, const nsVec2Template<Type>& v2)
{
  NS_NAN_ASSERT(&v1);
  NS_NAN_ASSERT(&v2);

  return nsVec2Template<Type>(v1.x + v2.x, v1.y + v2.y);
}

template <typename Type>
NS_FORCE_INLINE const nsVec2Template<Type> operator-(const nsVec2Template<Type>& v1, const nsVec2Template<Type>& v2)
{
  NS_NAN_ASSERT(&v1);
  NS_NAN_ASSERT(&v2);

  return nsVec2Template<Type>(v1.x - v2.x, v1.y - v2.y);
}

template <typename Type>
NS_FORCE_INLINE const nsVec2Template<Type> operator*(Type f, const nsVec2Template<Type>& v)
{
  NS_NAN_ASSERT(&v);

  return nsVec2Template<Type>(v.x * f, v.y * f);
}

template <typename Type>
NS_FORCE_INLINE const nsVec2Template<Type> operator*(const nsVec2Template<Type>& v, Type f)
{
  NS_NAN_ASSERT(&v);

  return nsVec2Template<Type>(v.x * f, v.y * f);
}

template <typename Type>
NS_FORCE_INLINE const nsVec2Template<Type> operator/(const nsVec2Template<Type>& v, Type f)
{
  NS_NAN_ASSERT(&v);

  if constexpr (std::is_floating_point_v<Type>)
  {
    // multiplication is much faster than division
    const Type f_inv = nsMath::Invert(f);
    return nsVec2Template<Type>(v.x * f_inv, v.y * f_inv);
  }
  else
  {
    return nsVec2Template<Type>(v.x / f, v.y / f);
  }
}

template <typename Type>
inline bool nsVec2Template<Type>::IsIdentical(const nsVec2Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return ((x == rhs.x) && (y == rhs.y));
}

template <typename Type>
inline bool nsVec2Template<Type>::IsEqual(const nsVec2Template<Type>& rhs, Type fEpsilon) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return (nsMath::IsEqual(x, rhs.x, fEpsilon) && nsMath::IsEqual(y, rhs.y, fEpsilon));
}

template <typename Type>
NS_FORCE_INLINE bool operator==(const nsVec2Template<Type>& v1, const nsVec2Template<Type>& v2)
{
  return v1.IsIdentical(v2);
}

template <typename Type>
NS_FORCE_INLINE bool operator!=(const nsVec2Template<Type>& v1, const nsVec2Template<Type>& v2)
{
  return !v1.IsIdentical(v2);
}

template <typename Type>
NS_FORCE_INLINE bool operator<(const nsVec2Template<Type>& v1, const nsVec2Template<Type>& v2)
{
  NS_NAN_ASSERT(&v1);
  NS_NAN_ASSERT(&v2);

  if (v1.x < v2.x)
    return true;
  if (v1.x > v2.x)
    return false;

  return (v1.y < v2.y);
}
