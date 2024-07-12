#pragma once

NS_ALWAYS_INLINE nsColorBaseUB::nsColorBaseUB(nsUInt8 r, nsUInt8 g, nsUInt8 b, nsUInt8 a /* = 255*/)
{
  this->r = r;
  this->g = g;
  this->b = b;
  this->a = a;
}

NS_ALWAYS_INLINE nsColorLinearUB::nsColorLinearUB(nsUInt8 r, nsUInt8 g, nsUInt8 b, nsUInt8 a /* = 255*/)
  : nsColorBaseUB(r, g, b, a)
{
}

inline nsColorLinearUB::nsColorLinearUB(const nsColor& color)
{
  *this = color;
}

inline void nsColorLinearUB::operator=(const nsColor& color)
{
  r = nsMath::ColorFloatToByte(color.r);
  g = nsMath::ColorFloatToByte(color.g);
  b = nsMath::ColorFloatToByte(color.b);
  a = nsMath::ColorFloatToByte(color.a);
}

inline nsColor nsColorLinearUB::ToLinearFloat() const
{
  return nsColor(nsMath::ColorByteToFloat(r), nsMath::ColorByteToFloat(g), nsMath::ColorByteToFloat(b), nsMath::ColorByteToFloat(a));
}

// *****************

NS_ALWAYS_INLINE nsColorGammaUB::nsColorGammaUB(nsUInt8 r, nsUInt8 g, nsUInt8 b, nsUInt8 a)
  : nsColorBaseUB(r, g, b, a)
{
}

inline nsColorGammaUB::nsColorGammaUB(const nsColor& color)
{
  *this = color;
}

inline void nsColorGammaUB::operator=(const nsColor& color)
{
  const nsVec3 gamma = nsColor::LinearToGamma(nsVec3(color.r, color.g, color.b));

  r = nsMath::ColorFloatToByte(gamma.x);
  g = nsMath::ColorFloatToByte(gamma.y);
  b = nsMath::ColorFloatToByte(gamma.z);
  a = nsMath::ColorFloatToByte(color.a);
}

inline nsColor nsColorGammaUB::ToLinearFloat() const
{
  nsVec3 gamma;
  gamma.x = nsMath::ColorByteToFloat(r);
  gamma.y = nsMath::ColorByteToFloat(g);
  gamma.z = nsMath::ColorByteToFloat(b);

  const nsVec3 linear = nsColor::GammaToLinear(gamma);

  return nsColor(linear.x, linear.y, linear.z, nsMath::ColorByteToFloat(a));
}
