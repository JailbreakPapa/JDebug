#pragma once

WD_ALWAYS_INLINE wdSimdVec4f::wdSimdVec4f() {}

WD_ALWAYS_INLINE wdSimdVec4f::wdSimdVec4f(float xyzw)
{
  m_v.Set(xyzw);
}

WD_ALWAYS_INLINE wdSimdVec4f::wdSimdVec4f(const wdSimdFloat& xyzw)
{
  m_v = xyzw.m_v;
}

WD_ALWAYS_INLINE wdSimdVec4f::wdSimdVec4f(float x, float y, float z, float w)
{
  m_v.Set(x, y, z, w);
}

WD_ALWAYS_INLINE void wdSimdVec4f::Set(float xyzw)
{
  m_v.Set(xyzw);
}

WD_ALWAYS_INLINE void wdSimdVec4f::Set(float x, float y, float z, float w)
{
  m_v.Set(x, y, z, w);
}

WD_ALWAYS_INLINE void wdSimdVec4f::SetX(const wdSimdFloat& f)
{
  m_v.x = f.m_v.x;
}

WD_ALWAYS_INLINE void wdSimdVec4f::SetY(const wdSimdFloat& f)
{
  m_v.y = f.m_v.x;
}

WD_ALWAYS_INLINE void wdSimdVec4f::SetZ(const wdSimdFloat& f)
{
  m_v.z = f.m_v.x;
}

WD_ALWAYS_INLINE void wdSimdVec4f::SetW(const wdSimdFloat& f)
{
  m_v.w = f.m_v.x;
}

WD_ALWAYS_INLINE void wdSimdVec4f::SetZero()
{
  m_v.SetZero();
}

template <int N>
WD_ALWAYS_INLINE void wdSimdVec4f::Load(const float* pFloats)
{
  m_v.SetZero();
  for (int i = 0; i < N; ++i)
  {
    (&m_v.x)[i] = pFloats[i];
  }
}

template <int N>
WD_ALWAYS_INLINE void wdSimdVec4f::Store(float* pFloats) const
{
  for (int i = 0; i < N; ++i)
  {
    pFloats[i] = (&m_v.x)[i];
  }
}

template <wdMathAcc::Enum acc>
WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::GetReciprocal() const
{
  return wdVec4(1.0f).CompDiv(m_v);
}

template <wdMathAcc::Enum acc>
WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::GetSqrt() const
{
  wdSimdVec4f result;
  result.m_v.x = wdMath::Sqrt(m_v.x);
  result.m_v.y = wdMath::Sqrt(m_v.y);
  result.m_v.z = wdMath::Sqrt(m_v.z);
  result.m_v.w = wdMath::Sqrt(m_v.w);

  return result;
}

template <wdMathAcc::Enum acc>
wdSimdVec4f wdSimdVec4f::GetInvSqrt() const
{
  wdSimdVec4f result;
  result.m_v.x = 1.0f / wdMath::Sqrt(m_v.x);
  result.m_v.y = 1.0f / wdMath::Sqrt(m_v.y);
  result.m_v.z = 1.0f / wdMath::Sqrt(m_v.z);
  result.m_v.w = 1.0f / wdMath::Sqrt(m_v.w);

  return result;
}

template <int N, wdMathAcc::Enum acc>
void wdSimdVec4f::NormalizeIfNotZero(const wdSimdFloat& fEpsilon)
{
  wdSimdFloat sqLength = GetLengthSquared<N>();
  m_v *= sqLength.GetInvSqrt<acc>();
  m_v = sqLength > fEpsilon.m_v ? m_v : wdVec4::ZeroVector();
}

template <int N>
WD_ALWAYS_INLINE bool wdSimdVec4f::IsZero() const
{
  for (int i = 0; i < N; ++i)
  {
    if ((&m_v.x)[i] != 0.0f)
      return false;
  }

  return true;
}

template <int N>
WD_ALWAYS_INLINE bool wdSimdVec4f::IsZero(const wdSimdFloat& fEpsilon) const
{
  for (int i = 0; i < N; ++i)
  {
    if (!wdMath::IsZero((&m_v.x)[i], (float)fEpsilon))
      return false;
  }

  return true;
}

template <int N>
WD_ALWAYS_INLINE bool wdSimdVec4f::IsNaN() const
{
  for (int i = 0; i < N; ++i)
  {
    if (wdMath::IsNaN((&m_v.x)[i]))
      return true;
  }

  return false;
}

template <int N>
WD_ALWAYS_INLINE bool wdSimdVec4f::IsValid() const
{
  for (int i = 0; i < N; ++i)
  {
    if (!wdMath::IsFinite((&m_v.x)[i]))
      return false;
  }

  return true;
}

template <int N>
WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::GetComponent() const
{
  return (&m_v.x)[N];
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::x() const
{
  return m_v.x;
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::y() const
{
  return m_v.y;
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::z() const
{
  return m_v.z;
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::w() const
{
  return m_v.w;
}

template <wdSwizzle::Enum s>
WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::Get() const
{
  wdSimdVec4f result;

  const float* v = &m_v.x;
  result.m_v.x = v[(s & 0x3000) >> 12];
  result.m_v.y = v[(s & 0x0300) >> 8];
  result.m_v.z = v[(s & 0x0030) >> 4];
  result.m_v.w = v[(s & 0x0003)];

  return result;
}

template <wdSwizzle::Enum s>
WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::GetCombined(const wdSimdVec4f& other) const
{
  wdSimdVec4f result;

  const float* v = &m_v.x;
  const float* o = &other.m_v.x;
  result.m_v.x = v[(s & 0x3000) >> 12];
  result.m_v.y = v[(s & 0x0300) >> 8];
  result.m_v.z = o[(s & 0x0030) >> 4];
  result.m_v.w = o[(s & 0x0003)];

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::operator-() const
{
  return -m_v;
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::operator+(const wdSimdVec4f& v) const
{
  return m_v + v.m_v;
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::operator-(const wdSimdVec4f& v) const
{
  return m_v - v.m_v;
}


WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::operator*(const wdSimdFloat& f) const
{
  return m_v * f.m_v.x;
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::operator/(const wdSimdFloat& f) const
{
  return m_v / f.m_v.x;
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::CompMul(const wdSimdVec4f& v) const
{
  return m_v.CompMul(v.m_v);
}

template <wdMathAcc::Enum acc>
WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::CompDiv(const wdSimdVec4f& v) const
{
  return m_v.CompDiv(v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::CompMin(const wdSimdVec4f& v) const
{
  return m_v.CompMin(v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::CompMax(const wdSimdVec4f& v) const
{
  return m_v.CompMax(v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::Abs() const
{
  return m_v.Abs();
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::Round() const
{
  wdSimdVec4f result;
  result.m_v.x = wdMath::Round(m_v.x);
  result.m_v.y = wdMath::Round(m_v.y);
  result.m_v.z = wdMath::Round(m_v.z);
  result.m_v.w = wdMath::Round(m_v.w);

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::Floor() const
{
  wdSimdVec4f result;
  result.m_v.x = wdMath::Floor(m_v.x);
  result.m_v.y = wdMath::Floor(m_v.y);
  result.m_v.z = wdMath::Floor(m_v.z);
  result.m_v.w = wdMath::Floor(m_v.w);

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::Ceil() const
{
  wdSimdVec4f result;
  result.m_v.x = wdMath::Ceil(m_v.x);
  result.m_v.y = wdMath::Ceil(m_v.y);
  result.m_v.z = wdMath::Ceil(m_v.z);
  result.m_v.w = wdMath::Ceil(m_v.w);

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::Trunc() const
{
  wdSimdVec4f result;
  result.m_v.x = wdMath::Trunc(m_v.x);
  result.m_v.y = wdMath::Trunc(m_v.y);
  result.m_v.z = wdMath::Trunc(m_v.z);
  result.m_v.w = wdMath::Trunc(m_v.w);

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::FlipSign(const wdSimdVec4b& cmp) const
{
  wdSimdVec4f result;
  result.m_v.x = cmp.m_v.x ? -m_v.x : m_v.x;
  result.m_v.y = cmp.m_v.y ? -m_v.y : m_v.y;
  result.m_v.z = cmp.m_v.z ? -m_v.z : m_v.z;
  result.m_v.w = cmp.m_v.w ? -m_v.w : m_v.w;

  return result;
}

// static
WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::Select(const wdSimdVec4b& cmp, const wdSimdVec4f& ifTrue, const wdSimdVec4f& ifFalse)
{
  wdSimdVec4f result;
  result.m_v.x = cmp.m_v.x ? ifTrue.m_v.x : ifFalse.m_v.x;
  result.m_v.y = cmp.m_v.y ? ifTrue.m_v.y : ifFalse.m_v.y;
  result.m_v.z = cmp.m_v.z ? ifTrue.m_v.z : ifFalse.m_v.z;
  result.m_v.w = cmp.m_v.w ? ifTrue.m_v.w : ifFalse.m_v.w;

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4f& wdSimdVec4f::operator+=(const wdSimdVec4f& v)
{
  m_v += v.m_v;
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4f& wdSimdVec4f::operator-=(const wdSimdVec4f& v)
{
  m_v -= v.m_v;
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4f& wdSimdVec4f::operator*=(const wdSimdFloat& f)
{
  m_v *= f.m_v.x;
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4f& wdSimdVec4f::operator/=(const wdSimdFloat& f)
{
  m_v /= f.m_v.x;
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4f::operator==(const wdSimdVec4f& v) const
{
  bool result[4];
  result[0] = m_v.x == v.m_v.x;
  result[1] = m_v.y == v.m_v.y;
  result[2] = m_v.z == v.m_v.z;
  result[3] = m_v.w == v.m_v.w;

  return wdSimdVec4b(result[0], result[1], result[2], result[3]);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4f::operator!=(const wdSimdVec4f& v) const
{
  return !(*this == v);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4f::operator<=(const wdSimdVec4f& v) const
{
  return !(*this > v);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4f::operator<(const wdSimdVec4f& v) const
{
  bool result[4];
  result[0] = m_v.x < v.m_v.x;
  result[1] = m_v.y < v.m_v.y;
  result[2] = m_v.z < v.m_v.z;
  result[3] = m_v.w < v.m_v.w;

  return wdSimdVec4b(result[0], result[1], result[2], result[3]);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4f::operator>=(const wdSimdVec4f& v) const
{
  return !(*this < v);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4f::operator>(const wdSimdVec4f& v) const
{
  bool result[4];
  result[0] = m_v.x > v.m_v.x;
  result[1] = m_v.y > v.m_v.y;
  result[2] = m_v.z > v.m_v.z;
  result[3] = m_v.w > v.m_v.w;

  return wdSimdVec4b(result[0], result[1], result[2], result[3]);
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::HorizontalSum<2>() const
{
  return m_v.x + m_v.y;
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::HorizontalSum<3>() const
{
  return (float)HorizontalSum<2>() + m_v.z;
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::HorizontalSum<4>() const
{
  return (float)HorizontalSum<3>() + m_v.w;
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::HorizontalMin<2>() const
{
  return wdMath::Min(m_v.x, m_v.y);
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::HorizontalMin<3>() const
{
  return wdMath::Min((float)HorizontalMin<2>(), m_v.z);
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::HorizontalMin<4>() const
{
  return wdMath::Min((float)HorizontalMin<3>(), m_v.w);
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::HorizontalMax<2>() const
{
  return wdMath::Max(m_v.x, m_v.y);
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::HorizontalMax<3>() const
{
  return wdMath::Max((float)HorizontalMax<2>(), m_v.z);
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::HorizontalMax<4>() const
{
  return wdMath::Max((float)HorizontalMax<3>(), m_v.w);
}

template <int N>
WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::Dot(const wdSimdVec4f& v) const
{
  float result = 0.0f;

  for (int i = 0; i < N; ++i)
  {
    result += (&m_v.x)[i] * (&v.m_v.x)[i];
  }

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::CrossRH(const wdSimdVec4f& v) const
{
  return m_v.GetAsVec3().CrossRH(v.m_v.GetAsVec3()).GetAsVec4(0.0f);
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::GetOrthogonalVector() const
{
  if (wdMath::Abs(m_v.y) < 0.99f)
  {
    return wdVec4(-m_v.z, 0.0f, m_v.x, 0.0f);
  }
  else
  {
    return wdVec4(0.0f, m_v.z, -m_v.y, 0.0f);
  }
}

// static
WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::ZeroVector()
{
  return wdVec4::ZeroVector();
}

// static
WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::MulAdd(const wdSimdVec4f& a, const wdSimdVec4f& b, const wdSimdVec4f& c)
{
  return a.CompMul(b) + c;
}

// static
WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::MulAdd(const wdSimdVec4f& a, const wdSimdFloat& b, const wdSimdVec4f& c)
{
  return a * b + c;
}

// static
WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::MulSub(const wdSimdVec4f& a, const wdSimdVec4f& b, const wdSimdVec4f& c)
{
  return a.CompMul(b) - c;
}

// static
WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::MulSub(const wdSimdVec4f& a, const wdSimdFloat& b, const wdSimdVec4f& c)
{
  return a * b - c;
}

// static
WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::CopySign(const wdSimdVec4f& magnitude, const wdSimdVec4f& sign)
{
  wdSimdVec4f result;
  result.m_v.x = sign.m_v.x < 0.0f ? -magnitude.m_v.x : magnitude.m_v.x;
  result.m_v.y = sign.m_v.y < 0.0f ? -magnitude.m_v.y : magnitude.m_v.y;
  result.m_v.z = sign.m_v.z < 0.0f ? -magnitude.m_v.z : magnitude.m_v.z;
  result.m_v.w = sign.m_v.w < 0.0f ? -magnitude.m_v.w : magnitude.m_v.w;

  return result;
}
