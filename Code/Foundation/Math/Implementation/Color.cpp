#include <Foundation/FoundationPCH.h>

#include <Foundation/Math/Color8UNorm.h>
#include <Foundation/Math/Mat4.h>

// ****** nsColor ******

nsColor nsColor::MakeNaN()
{
  return nsColor(nsMath::NaN<float>(), nsMath::NaN<float>(), nsMath::NaN<float>(), nsMath::NaN<float>());
}

nsColor nsColor::MakeZero()
{
  return nsColor(0.0f, 0.0f, 0.0f, 0.0f);
}

nsColor nsColor::MakeRGBA(float fLinearRed, float fLinearGreen, float fLinearBlue, float fLinearAlpha /*= 1.0f*/)
{
  return nsColor(fLinearRed, fLinearGreen, fLinearBlue, fLinearAlpha);
}

void nsColor::operator=(const nsColorLinearUB& cc)
{
  *this = cc.ToLinearFloat();
}

void nsColor::operator=(const nsColorGammaUB& cc)
{
  *this = cc.ToLinearFloat();
}

bool nsColor::IsNormalized() const
{
  NS_NAN_ASSERT(this);

  return r <= 1.0f && g <= 1.0f && b <= 1.0f && a <= 1.0f && r >= 0.0f && g >= 0.0f && b >= 0.0f && a >= 0.0f;
}


float nsColor::CalcAverageRGB() const
{
  return (1.0f / 3.0f) * (r + g + b);
}

// http://en.literateprograms.org/RGB_to_HSV_color_space_conversion_%28C%29
void nsColor::GetHSV(float& out_fHue, float& out_fSat, float& out_fValue) const
{
  // The formula below assumes values in gamma space
  const float r2 = LinearToGamma(r);
  const float g2 = LinearToGamma(g);
  const float b2 = LinearToGamma(b);

  out_fValue = nsMath::Max(r2, g2, b2); // Value

  if (out_fValue < nsMath::SmallEpsilon<float>())
  {
    out_fHue = 0.0f;
    out_fSat = 0.0f;
    out_fValue = 0.0f;
    return;
  }

  const float invV = 1.0f / out_fValue;
  float norm_r = r2 * invV;
  float norm_g = g2 * invV;
  float norm_b = b2 * invV;
  float rgb_min = nsMath::Min(norm_r, norm_g, norm_b);
  float rgb_max = nsMath::Max(norm_r, norm_g, norm_b);

  out_fSat = rgb_max - rgb_min; // Saturation

  if (out_fSat == 0)
  {
    out_fHue = 0;
    return;
  }

  // Normalize saturation
  const float rgb_delta_inv = 1.0f / (rgb_max - rgb_min);
  norm_r = (norm_r - rgb_min) * rgb_delta_inv;
  norm_g = (norm_g - rgb_min) * rgb_delta_inv;
  norm_b = (norm_b - rgb_min) * rgb_delta_inv;
  rgb_max = nsMath::Max(norm_r, norm_g, norm_b);

  // hue
  if (rgb_max == norm_r)
  {
    out_fHue = 60.0f * (norm_g - norm_b);

    if (out_fHue < 0.0f)
      out_fHue += 360.0f;
  }
  else if (rgb_max == norm_g)
    out_fHue = 120.0f + 60.0f * (norm_b - norm_r);
  else
    out_fHue = 240.0f + 60.0f * (norm_r - norm_g);
}

// http://www.rapidtables.com/convert/color/hsv-to-rgb.htm
nsColor nsColor::MakeHSV(float fHue, float fSat, float fVal)
{
  NS_ASSERT_DEBUG(fHue <= 360 && fHue >= 0, "HSV 'hue' is in invalid range.");
  NS_ASSERT_DEBUG(fSat <= 1 && fVal >= 0, "HSV 'saturation' is in invalid range.");
  NS_ASSERT_DEBUG(fVal >= 0, "HSV 'value' is in invalid range.");

  float c = fSat * fVal;
  float x = c * (1.0f - nsMath::Abs(nsMath::Mod(fHue / 60.0f, 2) - 1.0f));
  float m = fVal - c;

  nsColor res;
  res.a = 1.0f;

  if (fHue < 60)
  {
    res.r = c + m;
    res.g = x + m;
    res.b = 0 + m;
  }
  else if (fHue < 120)
  {
    res.r = x + m;
    res.g = c + m;
    res.b = 0 + m;
  }
  else if (fHue < 180)
  {
    res.r = 0 + m;
    res.g = c + m;
    res.b = x + m;
  }
  else if (fHue < 240)
  {
    res.r = 0 + m;
    res.g = x + m;
    res.b = c + m;
  }
  else if (fHue < 300)
  {
    res.r = x + m;
    res.g = 0 + m;
    res.b = c + m;
  }
  else
  {
    res.r = c + m;
    res.g = 0 + m;
    res.b = x + m;
  }

  // The formula above produces value in gamma space
  res.r = GammaToLinear(res.r);
  res.g = GammaToLinear(res.g);
  res.b = GammaToLinear(res.b);

  return res;
}

float nsColor::GetSaturation() const
{
  float hue, sat, val;
  GetHSV(hue, sat, val);

  return sat;
}

bool nsColor::IsValid() const
{
  if (!nsMath::IsFinite(r))
    return false;
  if (!nsMath::IsFinite(g))
    return false;
  if (!nsMath::IsFinite(b))
    return false;
  if (!nsMath::IsFinite(a))
    return false;

  return true;
}

bool nsColor::IsEqualRGB(const nsColor& rhs, float fEpsilon) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return (nsMath::IsEqual(r, rhs.r, fEpsilon) && nsMath::IsEqual(g, rhs.g, fEpsilon) && nsMath::IsEqual(b, rhs.b, fEpsilon));
}

bool nsColor::IsEqualRGBA(const nsColor& rhs, float fEpsilon) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  return (nsMath::IsEqual(r, rhs.r, fEpsilon) && nsMath::IsEqual(g, rhs.g, fEpsilon) && nsMath::IsEqual(b, rhs.b, fEpsilon) &&
          nsMath::IsEqual(a, rhs.a, fEpsilon));
}

void nsColor::operator/=(float f)
{
  float f_inv = 1.0f / f;
  r *= f_inv;
  g *= f_inv;
  b *= f_inv;
  a *= f_inv;

  NS_NAN_ASSERT(this);
}

void nsColor::operator*=(const nsMat4& rhs)
{
  nsVec3 v(r, g, b);
  v = rhs.TransformPosition(v);

  r = v.x;
  g = v.y;
  b = v.z;
}


void nsColor::ScaleRGB(float fFactor)
{
  r *= fFactor;
  g *= fFactor;
  b *= fFactor;
}

void nsColor::ScaleRGBA(float fFactor)
{
  r *= fFactor;
  g *= fFactor;
  b *= fFactor;
  a *= fFactor;
}

float nsColor::ComputeHdrMultiplier() const
{
  return nsMath::Max(1.0f, r, g, b);
}

float nsColor::ComputeHdrExposureValue() const
{
  return nsMath::Log2(ComputeHdrMultiplier());
}

void nsColor::ApplyHdrExposureValue(float fEv)
{
  const float factor = nsMath::Pow2(fEv);
  r *= factor;
  g *= factor;
  b *= factor;
}


void nsColor::NormalizeToLdrRange()
{
  ScaleRGB(1.0f / ComputeHdrMultiplier());
}

nsColor nsColor::GetDarker(float fFactor /*= 2.0f*/) const
{
  float h, s, v;
  GetHSV(h, s, v);

  return nsColor::MakeHSV(h, s, v / fFactor);
}

nsColor nsColor::GetComplementaryColor() const
{
  float hue, sat, val;
  GetHSV(hue, sat, val);

  nsColor Shifted = nsColor::MakeHSV(nsMath::Mod(hue + 180.0f, 360.0f), sat, val);
  Shifted.a = a;

  return Shifted;
}

const nsVec4 nsColor::GetAsVec4() const
{
  return nsVec4(r, g, b, a);
}

float nsColor::GammaToLinear(float fGamma)
{
  return fGamma <= 0.04045f ? (fGamma / 12.92f) : (nsMath::Pow((fGamma + 0.055f) / 1.055f, 2.4f));
}

float nsColor::LinearToGamma(float fLinear)
{
  // assuming we have linear color (not CIE xyY or CIE XYZ)
  return fLinear <= 0.0031308f ? (12.92f * fLinear) : (1.055f * nsMath::Pow(fLinear, 1.0f / 2.4f) - 0.055f);
}

nsVec3 nsColor::GammaToLinear(const nsVec3& vGamma)
{
  return nsVec3(GammaToLinear(vGamma.x), GammaToLinear(vGamma.y), GammaToLinear(vGamma.z));
}

nsVec3 nsColor::LinearToGamma(const nsVec3& vLinear)
{
  // assuming we have linear color (not CIE xyY or CIE XYZ)
  return nsVec3(LinearToGamma(vLinear.x), LinearToGamma(vLinear.y), LinearToGamma(vLinear.z));
}

const nsColor nsColor::AliceBlue(nsColorGammaUB(0xF0, 0xF8, 0xFF));
const nsColor nsColor::AntiqueWhite(nsColorGammaUB(0xFA, 0xEB, 0xD7));
const nsColor nsColor::Aqua(nsColorGammaUB(0x00, 0xFF, 0xFF));
const nsColor nsColor::Aquamarine(nsColorGammaUB(0x7F, 0xFF, 0xD4));
const nsColor nsColor::Azure(nsColorGammaUB(0xF0, 0xFF, 0xFF));
const nsColor nsColor::Beige(nsColorGammaUB(0xF5, 0xF5, 0xDC));
const nsColor nsColor::Bisque(nsColorGammaUB(0xFF, 0xE4, 0xC4));
const nsColor nsColor::Black(nsColorGammaUB(0x00, 0x00, 0x00));
const nsColor nsColor::BlanchedAlmond(nsColorGammaUB(0xFF, 0xEB, 0xCD));
const nsColor nsColor::Blue(nsColorGammaUB(0x00, 0x00, 0xFF));
const nsColor nsColor::BlueViolet(nsColorGammaUB(0x8A, 0x2B, 0xE2));
const nsColor nsColor::Brown(nsColorGammaUB(0xA5, 0x2A, 0x2A));
const nsColor nsColor::BurlyWood(nsColorGammaUB(0xDE, 0xB8, 0x87));
const nsColor nsColor::CadetBlue(nsColorGammaUB(0x5F, 0x9E, 0xA0));
const nsColor nsColor::Chartreuse(nsColorGammaUB(0x7F, 0xFF, 0x00));
const nsColor nsColor::Chocolate(nsColorGammaUB(0xD2, 0x69, 0x1E));
const nsColor nsColor::Coral(nsColorGammaUB(0xFF, 0x7F, 0x50));
const nsColor nsColor::CornflowerBlue(nsColorGammaUB(0x64, 0x95, 0xED)); // The Original!
const nsColor nsColor::Cornsilk(nsColorGammaUB(0xFF, 0xF8, 0xDC));
const nsColor nsColor::Crimson(nsColorGammaUB(0xDC, 0x14, 0x3C));
const nsColor nsColor::Cyan(nsColorGammaUB(0x00, 0xFF, 0xFF));
const nsColor nsColor::DarkBlue(nsColorGammaUB(0x00, 0x00, 0x8B));
const nsColor nsColor::DarkCyan(nsColorGammaUB(0x00, 0x8B, 0x8B));
const nsColor nsColor::DarkGoldenRod(nsColorGammaUB(0xB8, 0x86, 0x0B));
const nsColor nsColor::DarkGray(nsColorGammaUB(0xA9, 0xA9, 0xA9));
const nsColor nsColor::DarkGrey(nsColorGammaUB(0xA9, 0xA9, 0xA9));
const nsColor nsColor::DarkGreen(nsColorGammaUB(0x00, 0x64, 0x00));
const nsColor nsColor::DarkKhaki(nsColorGammaUB(0xBD, 0xB7, 0x6B));
const nsColor nsColor::DarkMagenta(nsColorGammaUB(0x8B, 0x00, 0x8B));
const nsColor nsColor::DarkOliveGreen(nsColorGammaUB(0x55, 0x6B, 0x2F));
const nsColor nsColor::DarkOrange(nsColorGammaUB(0xFF, 0x8C, 0x00));
const nsColor nsColor::DarkOrchid(nsColorGammaUB(0x99, 0x32, 0xCC));
const nsColor nsColor::DarkRed(nsColorGammaUB(0x8B, 0x00, 0x00));
const nsColor nsColor::DarkSalmon(nsColorGammaUB(0xE9, 0x96, 0x7A));
const nsColor nsColor::DarkSeaGreen(nsColorGammaUB(0x8F, 0xBC, 0x8F));
const nsColor nsColor::DarkSlateBlue(nsColorGammaUB(0x48, 0x3D, 0x8B));
const nsColor nsColor::DarkSlateGray(nsColorGammaUB(0x2F, 0x4F, 0x4F));
const nsColor nsColor::DarkSlateGrey(nsColorGammaUB(0x2F, 0x4F, 0x4F));
const nsColor nsColor::DarkTurquoise(nsColorGammaUB(0x00, 0xCE, 0xD1));
const nsColor nsColor::DarkViolet(nsColorGammaUB(0x94, 0x00, 0xD3));
const nsColor nsColor::DeepPink(nsColorGammaUB(0xFF, 0x14, 0x93));
const nsColor nsColor::DeepSkyBlue(nsColorGammaUB(0x00, 0xBF, 0xFF));
const nsColor nsColor::DimGray(nsColorGammaUB(0x69, 0x69, 0x69));
const nsColor nsColor::DimGrey(nsColorGammaUB(0x69, 0x69, 0x69));
const nsColor nsColor::DodgerBlue(nsColorGammaUB(0x1E, 0x90, 0xFF));
const nsColor nsColor::FireBrick(nsColorGammaUB(0xB2, 0x22, 0x22));
const nsColor nsColor::FloralWhite(nsColorGammaUB(0xFF, 0xFA, 0xF0));
const nsColor nsColor::ForestGreen(nsColorGammaUB(0x22, 0x8B, 0x22));
const nsColor nsColor::Fuchsia(nsColorGammaUB(0xFF, 0x00, 0xFF));
const nsColor nsColor::Gainsboro(nsColorGammaUB(0xDC, 0xDC, 0xDC));
const nsColor nsColor::GhostWhite(nsColorGammaUB(0xF8, 0xF8, 0xFF));
const nsColor nsColor::Gold(nsColorGammaUB(0xFF, 0xD7, 0x00));
const nsColor nsColor::GoldenRod(nsColorGammaUB(0xDA, 0xA5, 0x20));
const nsColor nsColor::Gray(nsColorGammaUB(0x80, 0x80, 0x80));
const nsColor nsColor::Grey(nsColorGammaUB(0x80, 0x80, 0x80));
const nsColor nsColor::Green(nsColorGammaUB(0x00, 0x80, 0x00));
const nsColor nsColor::GreenYellow(nsColorGammaUB(0xAD, 0xFF, 0x2F));
const nsColor nsColor::HoneyDew(nsColorGammaUB(0xF0, 0xFF, 0xF0));
const nsColor nsColor::HotPink(nsColorGammaUB(0xFF, 0x69, 0xB4));
const nsColor nsColor::IndianRed(nsColorGammaUB(0xCD, 0x5C, 0x5C));
const nsColor nsColor::Indigo(nsColorGammaUB(0x4B, 0x00, 0x82));
const nsColor nsColor::Ivory(nsColorGammaUB(0xFF, 0xFF, 0xF0));
const nsColor nsColor::Khaki(nsColorGammaUB(0xF0, 0xE6, 0x8C));
const nsColor nsColor::Lavender(nsColorGammaUB(0xE6, 0xE6, 0xFA));
const nsColor nsColor::LavenderBlush(nsColorGammaUB(0xFF, 0xF0, 0xF5));
const nsColor nsColor::LawnGreen(nsColorGammaUB(0x7C, 0xFC, 0x00));
const nsColor nsColor::LemonChiffon(nsColorGammaUB(0xFF, 0xFA, 0xCD));
const nsColor nsColor::LightBlue(nsColorGammaUB(0xAD, 0xD8, 0xE6));
const nsColor nsColor::LightCoral(nsColorGammaUB(0xF0, 0x80, 0x80));
const nsColor nsColor::LightCyan(nsColorGammaUB(0xE0, 0xFF, 0xFF));
const nsColor nsColor::LightGoldenRodYellow(nsColorGammaUB(0xFA, 0xFA, 0xD2));
const nsColor nsColor::LightGray(nsColorGammaUB(0xD3, 0xD3, 0xD3));
const nsColor nsColor::LightGrey(nsColorGammaUB(0xD3, 0xD3, 0xD3));
const nsColor nsColor::LightGreen(nsColorGammaUB(0x90, 0xEE, 0x90));
const nsColor nsColor::LightPink(nsColorGammaUB(0xFF, 0xB6, 0xC1));
const nsColor nsColor::LightSalmon(nsColorGammaUB(0xFF, 0xA0, 0x7A));
const nsColor nsColor::LightSeaGreen(nsColorGammaUB(0x20, 0xB2, 0xAA));
const nsColor nsColor::LightSkyBlue(nsColorGammaUB(0x87, 0xCE, 0xFA));
const nsColor nsColor::LightSlateGray(nsColorGammaUB(0x77, 0x88, 0x99));
const nsColor nsColor::LightSlateGrey(nsColorGammaUB(0x77, 0x88, 0x99));
const nsColor nsColor::LightSteelBlue(nsColorGammaUB(0xB0, 0xC4, 0xDE));
const nsColor nsColor::LightYellow(nsColorGammaUB(0xFF, 0xFF, 0xE0));
const nsColor nsColor::Lime(nsColorGammaUB(0x00, 0xFF, 0x00));
const nsColor nsColor::LimeGreen(nsColorGammaUB(0x32, 0xCD, 0x32));
const nsColor nsColor::Linen(nsColorGammaUB(0xFA, 0xF0, 0xE6));
const nsColor nsColor::Magenta(nsColorGammaUB(0xFF, 0x00, 0xFF));
const nsColor nsColor::Maroon(nsColorGammaUB(0x80, 0x00, 0x00));
const nsColor nsColor::MediumAquaMarine(nsColorGammaUB(0x66, 0xCD, 0xAA));
const nsColor nsColor::MediumBlue(nsColorGammaUB(0x00, 0x00, 0xCD));
const nsColor nsColor::MediumOrchid(nsColorGammaUB(0xBA, 0x55, 0xD3));
const nsColor nsColor::MediumPurple(nsColorGammaUB(0x93, 0x70, 0xDB));
const nsColor nsColor::MediumSeaGreen(nsColorGammaUB(0x3C, 0xB3, 0x71));
const nsColor nsColor::MediumSlateBlue(nsColorGammaUB(0x7B, 0x68, 0xEE));
const nsColor nsColor::MediumSpringGreen(nsColorGammaUB(0x00, 0xFA, 0x9A));
const nsColor nsColor::MediumTurquoise(nsColorGammaUB(0x48, 0xD1, 0xCC));
const nsColor nsColor::MediumVioletRed(nsColorGammaUB(0xC7, 0x15, 0x85));
const nsColor nsColor::MidnightBlue(nsColorGammaUB(0x19, 0x19, 0x70));
const nsColor nsColor::MintCream(nsColorGammaUB(0xF5, 0xFF, 0xFA));
const nsColor nsColor::MistyRose(nsColorGammaUB(0xFF, 0xE4, 0xE1));
const nsColor nsColor::Moccasin(nsColorGammaUB(0xFF, 0xE4, 0xB5));
const nsColor nsColor::NavajoWhite(nsColorGammaUB(0xFF, 0xDE, 0xAD));
const nsColor nsColor::Navy(nsColorGammaUB(0x00, 0x00, 0x80));
const nsColor nsColor::OldLace(nsColorGammaUB(0xFD, 0xF5, 0xE6));
const nsColor nsColor::Olive(nsColorGammaUB(0x80, 0x80, 0x00));
const nsColor nsColor::OliveDrab(nsColorGammaUB(0x6B, 0x8E, 0x23));
const nsColor nsColor::Orange(nsColorGammaUB(0xFF, 0xA5, 0x00));
const nsColor nsColor::OrangeRed(nsColorGammaUB(0xFF, 0x45, 0x00));
const nsColor nsColor::Orchid(nsColorGammaUB(0xDA, 0x70, 0xD6));
const nsColor nsColor::PaleGoldenRod(nsColorGammaUB(0xEE, 0xE8, 0xAA));
const nsColor nsColor::PaleGreen(nsColorGammaUB(0x98, 0xFB, 0x98));
const nsColor nsColor::PaleTurquoise(nsColorGammaUB(0xAF, 0xEE, 0xEE));
const nsColor nsColor::PaleVioletRed(nsColorGammaUB(0xDB, 0x70, 0x93));
const nsColor nsColor::PapayaWhip(nsColorGammaUB(0xFF, 0xEF, 0xD5));
const nsColor nsColor::PeachPuff(nsColorGammaUB(0xFF, 0xDA, 0xB9));
const nsColor nsColor::Peru(nsColorGammaUB(0xCD, 0x85, 0x3F));
const nsColor nsColor::Pink(nsColorGammaUB(0xFF, 0xC0, 0xCB));
const nsColor nsColor::Plum(nsColorGammaUB(0xDD, 0xA0, 0xDD));
const nsColor nsColor::PowderBlue(nsColorGammaUB(0xB0, 0xE0, 0xE6));
const nsColor nsColor::Purple(nsColorGammaUB(0x80, 0x00, 0x80));
const nsColor nsColor::RebeccaPurple(nsColorGammaUB(0x66, 0x33, 0x99));
const nsColor nsColor::Red(nsColorGammaUB(0xFF, 0x00, 0x00));
const nsColor nsColor::RosyBrown(nsColorGammaUB(0xBC, 0x8F, 0x8F));
const nsColor nsColor::RoyalBlue(nsColorGammaUB(0x41, 0x69, 0xE1));
const nsColor nsColor::SaddleBrown(nsColorGammaUB(0x8B, 0x45, 0x13));
const nsColor nsColor::Salmon(nsColorGammaUB(0xFA, 0x80, 0x72));
const nsColor nsColor::SandyBrown(nsColorGammaUB(0xF4, 0xA4, 0x60));
const nsColor nsColor::SeaGreen(nsColorGammaUB(0x2E, 0x8B, 0x57));
const nsColor nsColor::SeaShell(nsColorGammaUB(0xFF, 0xF5, 0xEE));
const nsColor nsColor::Sienna(nsColorGammaUB(0xA0, 0x52, 0x2D));
const nsColor nsColor::Silver(nsColorGammaUB(0xC0, 0xC0, 0xC0));
const nsColor nsColor::SkyBlue(nsColorGammaUB(0x87, 0xCE, 0xEB));
const nsColor nsColor::SlateBlue(nsColorGammaUB(0x6A, 0x5A, 0xCD));
const nsColor nsColor::SlateGray(nsColorGammaUB(0x70, 0x80, 0x90));
const nsColor nsColor::SlateGrey(nsColorGammaUB(0x70, 0x80, 0x90));
const nsColor nsColor::Snow(nsColorGammaUB(0xFF, 0xFA, 0xFA));
const nsColor nsColor::SpringGreen(nsColorGammaUB(0x00, 0xFF, 0x7F));
const nsColor nsColor::SteelBlue(nsColorGammaUB(0x46, 0x82, 0xB4));
const nsColor nsColor::Tan(nsColorGammaUB(0xD2, 0xB4, 0x8C));
const nsColor nsColor::Teal(nsColorGammaUB(0x00, 0x80, 0x80));
const nsColor nsColor::Thistle(nsColorGammaUB(0xD8, 0xBF, 0xD8));
const nsColor nsColor::Tomato(nsColorGammaUB(0xFF, 0x63, 0x47));
const nsColor nsColor::Turquoise(nsColorGammaUB(0x40, 0xE0, 0xD0));
const nsColor nsColor::Violet(nsColorGammaUB(0xEE, 0x82, 0xEE));
const nsColor nsColor::Wheat(nsColorGammaUB(0xF5, 0xDE, 0xB3));
const nsColor nsColor::White(nsColorGammaUB(0xFF, 0xFF, 0xFF));
const nsColor nsColor::WhiteSmoke(nsColorGammaUB(0xF5, 0xF5, 0xF5));
const nsColor nsColor::Yellow(nsColorGammaUB(0xFF, 0xFF, 0x00));
const nsColor nsColor::YellowGreen(nsColorGammaUB(0x9A, 0xCD, 0x32));


nsUInt32 nsColor::ToRGBA8() const
{
  return nsColorLinearUB(*this).ToRGBA8();
}

nsUInt32 nsColor::ToABGR8() const
{
  return nsColorLinearUB(*this).ToABGR8();
}
