#pragma once

WD_ALWAYS_INLINE wdColorBaseUB::wdColorBaseUB(wdUInt8 r, wdUInt8 g, wdUInt8 b, wdUInt8 a /* = 255*/)
{
  this->r = r;
  this->g = g;
  this->b = b;
  this->a = a;
}

WD_ALWAYS_INLINE wdColorLinearUB::wdColorLinearUB(wdUInt8 r, wdUInt8 g, wdUInt8 b, wdUInt8 a /* = 255*/)
  : wdColorBaseUB(r, g, b, a)
{
}

inline wdColorLinearUB::wdColorLinearUB(const wdColor& color)
{
  *this = color;
}

inline void wdColorLinearUB::operator=(const wdColor& color)
{
  r = wdMath::ColorFloatToByte(color.r);
  g = wdMath::ColorFloatToByte(color.g);
  b = wdMath::ColorFloatToByte(color.b);
  a = wdMath::ColorFloatToByte(color.a);
}

inline wdColor wdColorLinearUB::ToLinearFloat() const
{
  return wdColor(wdMath::ColorByteToFloat(r), wdMath::ColorByteToFloat(g), wdMath::ColorByteToFloat(b), wdMath::ColorByteToFloat(a));
}

// *****************

WD_ALWAYS_INLINE wdColorGammaUB::wdColorGammaUB(wdUInt8 r, wdUInt8 g, wdUInt8 b, wdUInt8 a)
  : wdColorBaseUB(r, g, b, a)
{
}

inline wdColorGammaUB::wdColorGammaUB(const wdColor& color)
{
  *this = color;
}

inline void wdColorGammaUB::operator=(const wdColor& color)
{
  const wdVec3 gamma = wdColor::LinearToGamma(wdVec3(color.r, color.g, color.b));

  r = wdMath::ColorFloatToByte(gamma.x);
  g = wdMath::ColorFloatToByte(gamma.y);
  b = wdMath::ColorFloatToByte(gamma.z);
  a = wdMath::ColorFloatToByte(color.a);
}

inline wdColor wdColorGammaUB::ToLinearFloat() const
{
  wdVec3 gamma;
  gamma.x = wdMath::ColorByteToFloat(r);
  gamma.y = wdMath::ColorByteToFloat(g);
  gamma.z = wdMath::ColorByteToFloat(b);

  const wdVec3 linear = wdColor::GammaToLinear(gamma);

  return wdColor(linear.x, linear.y, linear.z, wdMath::ColorByteToFloat(a));
}
