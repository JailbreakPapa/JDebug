#pragma once

NS_ALWAYS_INLINE nsSimdVec4f::nsSimdVec4f() = default;

NS_ALWAYS_INLINE nsSimdVec4f::nsSimdVec4f(float xyzw)
{
  m_v.Set(xyzw);
}

NS_ALWAYS_INLINE nsSimdVec4f::nsSimdVec4f(const nsSimdFloat& xyzw)
{
  m_v = xyzw.m_v;
}

NS_ALWAYS_INLINE nsSimdVec4f::nsSimdVec4f(float x, float y, float z, float w)
{
  m_v.Set(x, y, z, w);
}

NS_ALWAYS_INLINE void nsSimdVec4f::Set(float xyzw)
{
  m_v.Set(xyzw);
}

NS_ALWAYS_INLINE void nsSimdVec4f::Set(float x, float y, float z, float w)
{
  m_v.Set(x, y, z, w);
}

NS_ALWAYS_INLINE void nsSimdVec4f::SetX(const nsSimdFloat& f)
{
  m_v.x = f.m_v.x;
}

NS_ALWAYS_INLINE void nsSimdVec4f::SetY(const nsSimdFloat& f)
{
  m_v.y = f.m_v.x;
}

NS_ALWAYS_INLINE void nsSimdVec4f::SetZ(const nsSimdFloat& f)
{
  m_v.z = f.m_v.x;
}

NS_ALWAYS_INLINE void nsSimdVec4f::SetW(const nsSimdFloat& f)
{
  m_v.w = f.m_v.x;
}

NS_ALWAYS_INLINE void nsSimdVec4f::SetZero()
{
  m_v.SetZero();
}

template <int N>
NS_ALWAYS_INLINE void nsSimdVec4f::Load(const float* pFloats)
{
  m_v.SetZero();
  for (int i = 0; i < N; ++i)
  {
    (&m_v.x)[i] = pFloats[i];
  }
}

template <int N>
NS_ALWAYS_INLINE void nsSimdVec4f::Store(float* pFloats) const
{
  for (int i = 0; i < N; ++i)
  {
    pFloats[i] = (&m_v.x)[i];
  }
}

template <nsMathAcc::Enum acc>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetReciprocal() const
{
  return nsVec4(1.0f).CompDiv(m_v);
}

template <nsMathAcc::Enum acc>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetSqrt() const
{
  nsSimdVec4f result;
  result.m_v.x = nsMath::Sqrt(m_v.x);
  result.m_v.y = nsMath::Sqrt(m_v.y);
  result.m_v.z = nsMath::Sqrt(m_v.z);
  result.m_v.w = nsMath::Sqrt(m_v.w);

  return result;
}

template <nsMathAcc::Enum acc>
nsSimdVec4f nsSimdVec4f::GetInvSqrt() const
{
  nsSimdVec4f result;
  result.m_v.x = 1.0f / nsMath::Sqrt(m_v.x);
  result.m_v.y = 1.0f / nsMath::Sqrt(m_v.y);
  result.m_v.z = 1.0f / nsMath::Sqrt(m_v.z);
  result.m_v.w = 1.0f / nsMath::Sqrt(m_v.w);

  return result;
}

template <int N, nsMathAcc::Enum acc>
void nsSimdVec4f::NormalizeIfNotZero(const nsSimdFloat& fEpsilon)
{
  nsSimdFloat sqLength = GetLengthSquared<N>();
  m_v *= sqLength.GetInvSqrt<acc>();
  m_v = sqLength > fEpsilon.m_v ? m_v : nsVec4::MakeZero();
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4f::IsZero() const
{
  for (int i = 0; i < N; ++i)
  {
    if ((&m_v.x)[i] != 0.0f)
      return false;
  }

  return true;
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4f::IsZero(const nsSimdFloat& fEpsilon) const
{
  for (int i = 0; i < N; ++i)
  {
    if (!nsMath::IsZero((&m_v.x)[i], (float)fEpsilon))
      return false;
  }

  return true;
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4f::IsNaN() const
{
  for (int i = 0; i < N; ++i)
  {
    if (nsMath::IsNaN((&m_v.x)[i]))
      return true;
  }

  return false;
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4f::IsValid() const
{
  for (int i = 0; i < N; ++i)
  {
    if (!nsMath::IsFinite((&m_v.x)[i]))
      return false;
  }

  return true;
}

template <int N>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::GetComponent() const
{
  return (&m_v.x)[N];
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::x() const
{
  return m_v.x;
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::y() const
{
  return m_v.y;
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::z() const
{
  return m_v.z;
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::w() const
{
  return m_v.w;
}

template <nsSwizzle::Enum s>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Get() const
{
  nsSimdVec4f result;

  const float* v = &m_v.x;
  result.m_v.x = v[(s & 0x3000) >> 12];
  result.m_v.y = v[(s & 0x0300) >> 8];
  result.m_v.z = v[(s & 0x0030) >> 4];
  result.m_v.w = v[(s & 0x0003)];

  return result;
}

template <nsSwizzle::Enum s>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetCombined(const nsSimdVec4f& other) const
{
  nsSimdVec4f result;

  const float* v = &m_v.x;
  const float* o = &other.m_v.x;
  result.m_v.x = v[(s & 0x3000) >> 12];
  result.m_v.y = v[(s & 0x0300) >> 8];
  result.m_v.z = o[(s & 0x0030) >> 4];
  result.m_v.w = o[(s & 0x0003)];

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::operator-() const
{
  return -m_v;
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::operator+(const nsSimdVec4f& v) const
{
  return m_v + v.m_v;
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::operator-(const nsSimdVec4f& v) const
{
  return m_v - v.m_v;
}


NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::operator*(const nsSimdFloat& f) const
{
  return m_v * f.m_v.x;
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::operator/(const nsSimdFloat& f) const
{
  return m_v / f.m_v.x;
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CompMul(const nsSimdVec4f& v) const
{
  return m_v.CompMul(v.m_v);
}

template <nsMathAcc::Enum acc>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CompDiv(const nsSimdVec4f& v) const
{
  return m_v.CompDiv(v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CompMin(const nsSimdVec4f& v) const
{
  return m_v.CompMin(v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CompMax(const nsSimdVec4f& v) const
{
  return m_v.CompMax(v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Abs() const
{
  return m_v.Abs();
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Round() const
{
  nsSimdVec4f result;
  result.m_v.x = nsMath::Round(m_v.x);
  result.m_v.y = nsMath::Round(m_v.y);
  result.m_v.z = nsMath::Round(m_v.z);
  result.m_v.w = nsMath::Round(m_v.w);

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Floor() const
{
  nsSimdVec4f result;
  result.m_v.x = nsMath::Floor(m_v.x);
  result.m_v.y = nsMath::Floor(m_v.y);
  result.m_v.z = nsMath::Floor(m_v.z);
  result.m_v.w = nsMath::Floor(m_v.w);

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Ceil() const
{
  nsSimdVec4f result;
  result.m_v.x = nsMath::Ceil(m_v.x);
  result.m_v.y = nsMath::Ceil(m_v.y);
  result.m_v.z = nsMath::Ceil(m_v.z);
  result.m_v.w = nsMath::Ceil(m_v.w);

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Trunc() const
{
  nsSimdVec4f result;
  result.m_v.x = nsMath::Trunc(m_v.x);
  result.m_v.y = nsMath::Trunc(m_v.y);
  result.m_v.z = nsMath::Trunc(m_v.z);
  result.m_v.w = nsMath::Trunc(m_v.w);

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::FlipSign(const nsSimdVec4b& cmp) const
{
  nsSimdVec4f result;
  result.m_v.x = cmp.m_v.x ? -m_v.x : m_v.x;
  result.m_v.y = cmp.m_v.y ? -m_v.y : m_v.y;
  result.m_v.z = cmp.m_v.z ? -m_v.z : m_v.z;
  result.m_v.w = cmp.m_v.w ? -m_v.w : m_v.w;

  return result;
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Select(const nsSimdVec4b& cmp, const nsSimdVec4f& ifTrue, const nsSimdVec4f& ifFalse)
{
  nsSimdVec4f result;
  result.m_v.x = cmp.m_v.x ? ifTrue.m_v.x : ifFalse.m_v.x;
  result.m_v.y = cmp.m_v.y ? ifTrue.m_v.y : ifFalse.m_v.y;
  result.m_v.z = cmp.m_v.z ? ifTrue.m_v.z : ifFalse.m_v.z;
  result.m_v.w = cmp.m_v.w ? ifTrue.m_v.w : ifFalse.m_v.w;

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4f& nsSimdVec4f::operator+=(const nsSimdVec4f& v)
{
  m_v += v.m_v;
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4f& nsSimdVec4f::operator-=(const nsSimdVec4f& v)
{
  m_v -= v.m_v;
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4f& nsSimdVec4f::operator*=(const nsSimdFloat& f)
{
  m_v *= f.m_v.x;
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4f& nsSimdVec4f::operator/=(const nsSimdFloat& f)
{
  m_v /= f.m_v.x;
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4f::operator==(const nsSimdVec4f& v) const
{
  bool result[4];
  result[0] = m_v.x == v.m_v.x;
  result[1] = m_v.y == v.m_v.y;
  result[2] = m_v.z == v.m_v.z;
  result[3] = m_v.w == v.m_v.w;

  return nsSimdVec4b(result[0], result[1], result[2], result[3]);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4f::operator!=(const nsSimdVec4f& v) const
{
  return !(*this == v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4f::operator<=(const nsSimdVec4f& v) const
{
  return !(*this > v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4f::operator<(const nsSimdVec4f& v) const
{
  bool result[4];
  result[0] = m_v.x < v.m_v.x;
  result[1] = m_v.y < v.m_v.y;
  result[2] = m_v.z < v.m_v.z;
  result[3] = m_v.w < v.m_v.w;

  return nsSimdVec4b(result[0], result[1], result[2], result[3]);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4f::operator>=(const nsSimdVec4f& v) const
{
  return !(*this < v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4f::operator>(const nsSimdVec4f& v) const
{
  bool result[4];
  result[0] = m_v.x > v.m_v.x;
  result[1] = m_v.y > v.m_v.y;
  result[2] = m_v.z > v.m_v.z;
  result[3] = m_v.w > v.m_v.w;

  return nsSimdVec4b(result[0], result[1], result[2], result[3]);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalSum<2>() const
{
  return m_v.x + m_v.y;
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalSum<3>() const
{
  return (float)HorizontalSum<2>() + m_v.z;
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalSum<4>() const
{
  return (float)HorizontalSum<3>() + m_v.w;
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMin<2>() const
{
  return nsMath::Min(m_v.x, m_v.y);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMin<3>() const
{
  return nsMath::Min((float)HorizontalMin<2>(), m_v.z);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMin<4>() const
{
  return nsMath::Min((float)HorizontalMin<3>(), m_v.w);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMax<2>() const
{
  return nsMath::Max(m_v.x, m_v.y);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMax<3>() const
{
  return nsMath::Max((float)HorizontalMax<2>(), m_v.z);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMax<4>() const
{
  return nsMath::Max((float)HorizontalMax<3>(), m_v.w);
}

template <int N>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::Dot(const nsSimdVec4f& v) const
{
  float result = 0.0f;

  for (int i = 0; i < N; ++i)
  {
    result += (&m_v.x)[i] * (&v.m_v.x)[i];
  }

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CrossRH(const nsSimdVec4f& v) const
{
  return m_v.GetAsVec3().CrossRH(v.m_v.GetAsVec3()).GetAsVec4(0.0f);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetOrthogonalVector() const
{
  if (nsMath::Abs(m_v.y) < 0.99f)
  {
    return nsVec4(-m_v.z, 0.0f, m_v.x, 0.0f);
  }
  else
  {
    return nsVec4(0.0f, m_v.z, -m_v.y, 0.0f);
  }
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::MulAdd(const nsSimdVec4f& a, const nsSimdVec4f& b, const nsSimdVec4f& c)
{
  return a.CompMul(b) + c;
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::MulAdd(const nsSimdVec4f& a, const nsSimdFloat& b, const nsSimdVec4f& c)
{
  return a * b + c;
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::MulSub(const nsSimdVec4f& a, const nsSimdVec4f& b, const nsSimdVec4f& c)
{
  return a.CompMul(b) - c;
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::MulSub(const nsSimdVec4f& a, const nsSimdFloat& b, const nsSimdVec4f& c)
{
  return a * b - c;
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CopySign(const nsSimdVec4f& magnitude, const nsSimdVec4f& sign)
{
  nsSimdVec4f result;
  result.m_v.x = sign.m_v.x < 0.0f ? -magnitude.m_v.x : magnitude.m_v.x;
  result.m_v.y = sign.m_v.y < 0.0f ? -magnitude.m_v.y : magnitude.m_v.y;
  result.m_v.z = sign.m_v.z < 0.0f ? -magnitude.m_v.z : magnitude.m_v.z;
  result.m_v.w = sign.m_v.w < 0.0f ? -magnitude.m_v.w : magnitude.m_v.w;

  return result;
}
