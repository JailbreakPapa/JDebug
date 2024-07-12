#pragma once

inline nsColor::nsColor()
{
#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  const float TypeNaN = nsMath::NaN<float>();
  r = TypeNaN;
  g = TypeNaN;
  b = TypeNaN;
  a = TypeNaN;
#endif
}

NS_FORCE_INLINE constexpr nsColor::nsColor(float fLinearRed, float fLinearGreen, float fLinearBlue, float fLinearAlpha /* = 1.0f */)
  : r(fLinearRed)
  , g(fLinearGreen)
  , b(fLinearBlue)
  , a(fLinearAlpha)
{
}

inline nsColor::nsColor(const nsColorLinearUB& cc)
{
  *this = cc;
}

inline nsColor::nsColor(const nsColorGammaUB& cc)
{
  *this = cc;
}

inline void nsColor::SetRGB(float fLinearRed, float fLinearGreen, float fLinearBlue)
{
  r = fLinearRed;
  g = fLinearGreen;
  b = fLinearBlue;
}

inline void nsColor::SetRGBA(float fLinearRed, float fLinearGreen, float fLinearBlue, float fLinearAlpha /* = 1.0f */)
{
  r = fLinearRed;
  g = fLinearGreen;
  b = fLinearBlue;
  a = fLinearAlpha;
}

inline nsColor nsColor::MakeFromKelvin(nsUInt32 uiKelvin)
{
  nsColor finalColor;
  float kelvin = nsMath::Clamp(uiKelvin, 1000u, 40000u) / 1000.0f;
  float kelvin2 = kelvin * kelvin;

  // Red
  finalColor.r = kelvin < 6.570f ? 1.0f : nsMath::Clamp((1.35651f + 0.216422f * kelvin + 0.000633715f * kelvin2) / (-3.24223f + 0.918711f * kelvin), 0.0f, 1.0f);
  // Green
  finalColor.g = kelvin < 6.570f ? nsMath::Clamp((-399.809f + 414.271f * kelvin + 111.543f * kelvin2) / (2779.24f + 164.143f * kelvin + 84.7356f * kelvin2), 0.0f, 1.0f) : nsMath::Clamp((1370.38f + 734.616f * kelvin + 0.689955f * kelvin2) / (-4625.69f + 1699.87f * kelvin), 0.0f, 1.0f);
  // Blue
  finalColor.b = kelvin > 6.570f ? 1.0f : nsMath::Clamp((348.963f - 523.53f * kelvin + 183.62f * kelvin2) / (2848.82f - 214.52f * kelvin + 78.8614f * kelvin2), 0.0f, 1.0f);

  return finalColor;
}

// http://en.wikipedia.org/wiki/Luminance_%28relative%29
NS_FORCE_INLINE float nsColor::GetLuminance() const
{
  return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

inline nsColor nsColor::GetInvertedColor() const
{
  NS_NAN_ASSERT(this);
  NS_ASSERT_DEBUG(IsNormalized(), "Cannot invert a color that has values outside the [0; 1] range");

  return nsColor(1.0f - r, 1.0f - g, 1.0f - b, 1.0f - a);
}

inline bool nsColor::IsNaN() const
{
  if (nsMath::IsNaN(r))
    return true;
  if (nsMath::IsNaN(g))
    return true;
  if (nsMath::IsNaN(b))
    return true;
  if (nsMath::IsNaN(a))
    return true;

  return false;
}

inline void nsColor::operator+=(const nsColor& rhs)
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  r += rhs.r;
  g += rhs.g;
  b += rhs.b;
  a += rhs.a;
}

inline void nsColor::operator-=(const nsColor& rhs)
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  r -= rhs.r;
  g -= rhs.g;
  b -= rhs.b;
  a -= rhs.a;
}

inline void nsColor::operator*=(const nsColor& rhs)
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  r *= rhs.r;
  g *= rhs.g;
  b *= rhs.b;
  a *= rhs.a;
}
inline void nsColor::operator*=(float f)
{
  r *= f;
  g *= f;
  b *= f;
  a *= f;

  NS_NAN_ASSERT(this);
}

inline bool nsColor::IsIdenticalRGB(const nsColor& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return r == rhs.r && g == rhs.g && b == rhs.b;
}

inline bool nsColor::IsIdenticalRGBA(const nsColor& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
}

inline nsColor nsColor::WithAlpha(float fAlpha) const
{
  return nsColor(r, g, b, fAlpha);
}

inline const nsColor operator+(const nsColor& c1, const nsColor& c2)
{
  NS_NAN_ASSERT(&c1);
  NS_NAN_ASSERT(&c2);

  return nsColor(c1.r + c2.r, c1.g + c2.g, c1.b + c2.b, c1.a + c2.a);
}

inline const nsColor operator-(const nsColor& c1, const nsColor& c2)
{
  NS_NAN_ASSERT(&c1);
  NS_NAN_ASSERT(&c2);

  return nsColor(c1.r - c2.r, c1.g - c2.g, c1.b - c2.b, c1.a - c2.a);
}

inline const nsColor operator*(const nsColor& c1, const nsColor& c2)
{
  NS_NAN_ASSERT(&c1);
  NS_NAN_ASSERT(&c2);

  return nsColor(c1.r * c2.r, c1.g * c2.g, c1.b * c2.b, c1.a * c2.a);
}

inline const nsColor operator*(float f, const nsColor& c)
{
  NS_NAN_ASSERT(&c);

  return nsColor(c.r * f, c.g * f, c.b * f, c.a * f);
}

inline const nsColor operator*(const nsColor& c, float f)
{
  NS_NAN_ASSERT(&c);

  return nsColor(c.r * f, c.g * f, c.b * f, c.a * f);
}

inline const nsColor operator*(const nsMat4& lhs, const nsColor& rhs)
{
  nsColor r = rhs;
  r *= lhs;
  return r;
}

inline const nsColor operator/(const nsColor& c, float f)
{
  NS_NAN_ASSERT(&c);

  float f_inv = 1.0f / f;
  return nsColor(c.r * f_inv, c.g * f_inv, c.b * f_inv, c.a * f_inv);
}

NS_ALWAYS_INLINE bool operator==(const nsColor& c1, const nsColor& c2)
{
  return c1.IsIdenticalRGBA(c2);
}

NS_ALWAYS_INLINE bool operator!=(const nsColor& c1, const nsColor& c2)
{
  return !c1.IsIdenticalRGBA(c2);
}

NS_FORCE_INLINE bool operator<(const nsColor& c1, const nsColor& c2)
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
