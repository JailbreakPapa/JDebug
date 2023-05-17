#pragma once

inline wdColor::wdColor()
{
#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  const float TypeNaN = wdMath::NaN<float>();
  r = TypeNaN;
  g = TypeNaN;
  b = TypeNaN;
  a = TypeNaN;
#endif
}

WD_FORCE_INLINE constexpr wdColor::wdColor(float fLinearRed, float fLinearGreen, float fLinearBlue, float fLinearAlpha /* = 1.0f */)
  : r(fLinearRed)
  , g(fLinearGreen)
  , b(fLinearBlue)
  , a(fLinearAlpha)
{
}

inline wdColor::wdColor(const wdColorLinearUB& cc)
{
  *this = cc;
}

inline wdColor::wdColor(const wdColorGammaUB& cc)
{
  *this = cc;
}

inline void wdColor::SetRGB(float fLinearRed, float fLinearGreen, float fLinearBlue)
{
  r = fLinearRed;
  g = fLinearGreen;
  b = fLinearBlue;
}

inline void wdColor::SetRGBA(float fLinearRed, float fLinearGreen, float fLinearBlue, float fLinearAlpha /* = 1.0f */)
{
  r = fLinearRed;
  g = fLinearGreen;
  b = fLinearBlue;
  a = fLinearAlpha;
}

inline void wdColor::SetZero()
{
  *this = ZeroColor();
}

// http://en.wikipedia.org/wiki/Luminance_%28relative%29
WD_FORCE_INLINE float wdColor::GetLuminance() const
{
  return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

inline wdColor wdColor::GetInvertedColor() const
{
  WD_NAN_ASSERT(this);
  WD_ASSERT_DEBUG(IsNormalized(), "Cannot invert a color that has values outside the [0; 1] range");

  return wdColor(1.0f - r, 1.0f - g, 1.0f - b, 1.0f - a);
}

inline bool wdColor::IsNaN() const
{
  if (wdMath::IsNaN(r))
    return true;
  if (wdMath::IsNaN(g))
    return true;
  if (wdMath::IsNaN(b))
    return true;
  if (wdMath::IsNaN(a))
    return true;

  return false;
}

inline void wdColor::operator+=(const wdColor& rhs)
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  r += rhs.r;
  g += rhs.g;
  b += rhs.b;
  a += rhs.a;
}

inline void wdColor::operator-=(const wdColor& rhs)
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  r -= rhs.r;
  g -= rhs.g;
  b -= rhs.b;
  a -= rhs.a;
}

inline void wdColor::operator*=(const wdColor& rhs)
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  r *= rhs.r;
  g *= rhs.g;
  b *= rhs.b;
  a *= rhs.a;
}
inline void wdColor::operator*=(float f)
{
  r *= f;
  g *= f;
  b *= f;
  a *= f;

  WD_NAN_ASSERT(this);
}

inline bool wdColor::IsIdenticalRGB(const wdColor& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return r == rhs.r && g == rhs.g && b == rhs.b;
}

inline bool wdColor::IsIdenticalRGBA(const wdColor& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
}

inline wdColor wdColor::WithAlpha(float fAlpha) const
{
  return wdColor(r, g, b, fAlpha);
}

inline const wdColor operator+(const wdColor& c1, const wdColor& c2)
{
  WD_NAN_ASSERT(&c1);
  WD_NAN_ASSERT(&c2);

  return wdColor(c1.r + c2.r, c1.g + c2.g, c1.b + c2.b, c1.a + c2.a);
}

inline const wdColor operator-(const wdColor& c1, const wdColor& c2)
{
  WD_NAN_ASSERT(&c1);
  WD_NAN_ASSERT(&c2);

  return wdColor(c1.r - c2.r, c1.g - c2.g, c1.b - c2.b, c1.a - c2.a);
}

inline const wdColor operator*(const wdColor& c1, const wdColor& c2)
{
  WD_NAN_ASSERT(&c1);
  WD_NAN_ASSERT(&c2);

  return wdColor(c1.r * c2.r, c1.g * c2.g, c1.b * c2.b, c1.a * c2.a);
}

inline const wdColor operator*(float f, const wdColor& c)
{
  WD_NAN_ASSERT(&c);

  return wdColor(c.r * f, c.g * f, c.b * f, c.a * f);
}

inline const wdColor operator*(const wdColor& c, float f)
{
  WD_NAN_ASSERT(&c);

  return wdColor(c.r * f, c.g * f, c.b * f, c.a * f);
}

inline const wdColor operator*(const wdMat4& lhs, const wdColor& rhs)
{
  wdColor r = rhs;
  r *= lhs;
  return r;
}

inline const wdColor operator/(const wdColor& c, float f)
{
  WD_NAN_ASSERT(&c);

  float f_inv = 1.0f / f;
  return wdColor(c.r * f_inv, c.g * f_inv, c.b * f_inv, c.a * f_inv);
}

WD_ALWAYS_INLINE bool operator==(const wdColor& c1, const wdColor& c2)
{
  return c1.IsIdenticalRGBA(c2);
}

WD_ALWAYS_INLINE bool operator!=(const wdColor& c1, const wdColor& c2)
{
  return !c1.IsIdenticalRGBA(c2);
}

WD_FORCE_INLINE bool operator<(const wdColor& c1, const wdColor& c2)
{
  if (c1.r < c2.r)
    return true;
  if (c1.r > c2.r)
    return false;
  if (c1.g < c2.g)
    return true;
  if (c1.g > c2.g)
    return false;
  if (c1.b < c2.b)
    return true;
  if (c1.b > c2.b)
    return false;

  return (c1.a < c2.a);
}
