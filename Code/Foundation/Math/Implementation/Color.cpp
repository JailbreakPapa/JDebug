#include <Foundation/FoundationPCH.h>

#include <Foundation/Math/Color8UNorm.h>
#include <Foundation/Math/Mat4.h>

// ****** wdColor ******

void wdColor::operator=(const wdColorLinearUB& cc)
{
  *this = cc.ToLinearFloat();
}

void wdColor::operator=(const wdColorGammaUB& cc)
{
  *this = cc.ToLinearFloat();
}

bool wdColor::IsNormalized() const
{
  WD_NAN_ASSERT(this);

  return r <= 1.0f && g <= 1.0f && b <= 1.0f && a <= 1.0f && r >= 0.0f && g >= 0.0f && b >= 0.0f && a >= 0.0f;
}


float wdColor::CalcAverageRGB() const
{
  return (1.0f / 3.0f) * (r + g + b);
}

// http://en.literateprograms.org/RGB_to_HSV_color_space_conversion_%28C%29
void wdColor::GetHSV(float& out_fHue, float& out_fSat, float& out_fValue) const
{
  // The formula below assumes values in gamma space
  const float r2 = LinearToGamma(r);
  const float g2 = LinearToGamma(g);
  const float b2 = LinearToGamma(b);

  out_fValue = wdMath::Max(r2, g2, b2); // Value

  if (out_fValue < wdMath::SmallEpsilon<float>())
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
  float rgb_min = wdMath::Min(norm_r, norm_g, norm_b);
  float rgb_max = wdMath::Max(norm_r, norm_g, norm_b);

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
  rgb_min = wdMath::Min(norm_r, norm_g, norm_b);
  rgb_max = wdMath::Max(norm_r, norm_g, norm_b);

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
void wdColor::SetHSV(float fHue, float fSat, float fVal)
{
  WD_ASSERT_DEBUG(fHue <= 360 && fHue >= 0, "HSV 'hue' is in invalid range.");
  WD_ASSERT_DEBUG(fSat <= 1 && fVal >= 0, "HSV 'saturation' is in invalid range.");
  WD_ASSERT_DEBUG(fVal >= 0, "HSV 'value' is in invalid range.");

  float c = fSat * fVal;
  float x = c * (1.0f - wdMath::Abs(wdMath::Mod(fHue / 60.0f, 2) - 1.0f));
  float m = fVal - c;


  a = 1.0f;

  if (fHue < 60)
  {
    r = c + m;
    g = x + m;
    b = 0 + m;
  }
  else if (fHue < 120)
  {
    r = x + m;
    g = c + m;
    b = 0 + m;
  }
  else if (fHue < 180)
  {
    r = 0 + m;
    g = c + m;
    b = x + m;
  }
  else if (fHue < 240)
  {
    r = 0 + m;
    g = x + m;
    b = c + m;
  }
  else if (fHue < 300)
  {
    r = x + m;
    g = 0 + m;
    b = c + m;
  }
  else
  {
    r = c + m;
    g = 0 + m;
    b = x + m;
  }

  // The formula above produces value in gamma space
  r = GammaToLinear(r);
  g = GammaToLinear(g);
  b = GammaToLinear(b);
}

float wdColor::GetSaturation() const
{
  float hue, sat, val;
  GetHSV(hue, sat, val);

  return sat;
}

bool wdColor::IsValid() const
{
  if (!wdMath::IsFinite(r))
    return false;
  if (!wdMath::IsFinite(g))
    return false;
  if (!wdMath::IsFinite(b))
    return false;
  if (!wdMath::IsFinite(a))
    return false;

  return true;
}

bool wdColor::IsEqualRGB(const wdColor& rhs, float fEpsilon) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return (wdMath::IsEqual(r, rhs.r, fEpsilon) && wdMath::IsEqual(g, rhs.g, fEpsilon) && wdMath::IsEqual(b, rhs.b, fEpsilon));
}

bool wdColor::IsEqualRGBA(const wdColor& rhs, float fEpsilon) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  return (wdMath::IsEqual(r, rhs.r, fEpsilon) && wdMath::IsEqual(g, rhs.g, fEpsilon) && wdMath::IsEqual(b, rhs.b, fEpsilon) &&
          wdMath::IsEqual(a, rhs.a, fEpsilon));
}

void wdColor::operator/=(float f)
{
  float f_inv = 1.0f / f;
  r *= f_inv;
  g *= f_inv;
  b *= f_inv;
  a *= f_inv;

  WD_NAN_ASSERT(this);
}

void wdColor::operator*=(const wdMat4& rhs)
{
  wdVec3 v(r, g, b);
  v = rhs.TransformPosition(v);

  r = v.x;
  g = v.y;
  b = v.z;
}


void wdColor::ScaleRGB(float fFactor)
{
  r *= fFactor;
  g *= fFactor;
  b *= fFactor;
}

float wdColor::ComputeHdrMultiplier() const
{
  return wdMath::Max(1.0f, r, g, b);
}

float wdColor::ComputeHdrExposureValue() const
{
  return wdMath::Log2(ComputeHdrMultiplier());
}

void wdColor::ApplyHdrExposureValue(float fEv)
{
  const float factor = wdMath::Pow2(fEv);
  r *= factor;
  g *= factor;
  b *= factor;
}


void wdColor::NormalizeToLdrRange()
{
  ScaleRGB(1.0f / ComputeHdrMultiplier());
}

wdColor wdColor::GetComplementaryColor() const
{
  float hue, sat, val;
  GetHSV(hue, sat, val);

  wdColor Shifted;
  Shifted.SetHSV(wdMath::Mod(hue + 180.0f, 360.0f), sat, val);
  Shifted.a = a;

  return Shifted;
}

const wdVec4 wdColor::GetAsVec4() const
{
  return wdVec4(r, g, b, a);
}

float wdColor::GammaToLinear(float fGamma)
{
  return fGamma <= 0.04045f ? (fGamma / 12.92f) : (wdMath::Pow((fGamma + 0.055f) / 1.055f, 2.4f));
}

float wdColor::LinearToGamma(float fLinear)
{
  // assuming we have linear color (not CIE xyY or CIE XYZ)
  return fLinear <= 0.0031308f ? (12.92f * fLinear) : (1.055f * wdMath::Pow(fLinear, 1.0f / 2.4f) - 0.055f);
}

wdVec3 wdColor::GammaToLinear(const wdVec3& vGamma)
{
  return wdVec3(GammaToLinear(vGamma.x), GammaToLinear(vGamma.y), GammaToLinear(vGamma.z));
}

wdVec3 wdColor::LinearToGamma(const wdVec3& vLinear)
{
  // assuming we have linear color (not CIE xyY or CIE XYZ)
  return wdVec3(LinearToGamma(vLinear.x), LinearToGamma(vLinear.y), LinearToGamma(vLinear.z));
}

const wdColor wdColor::AliceBlue(wdColorGammaUB(0xF0, 0xF8, 0xFF));
const wdColor wdColor::AntiqueWhite(wdColorGammaUB(0xFA, 0xEB, 0xD7));
const wdColor wdColor::Aqua(wdColorGammaUB(0x00, 0xFF, 0xFF));
const wdColor wdColor::Aquamarine(wdColorGammaUB(0x7F, 0xFF, 0xD4));
const wdColor wdColor::Azure(wdColorGammaUB(0xF0, 0xFF, 0xFF));
const wdColor wdColor::Beige(wdColorGammaUB(0xF5, 0xF5, 0xDC));
const wdColor wdColor::Bisque(wdColorGammaUB(0xFF, 0xE4, 0xC4));
const wdColor wdColor::Black(wdColorGammaUB(0x00, 0x00, 0x00));
const wdColor wdColor::BlanchedAlmond(wdColorGammaUB(0xFF, 0xEB, 0xCD));
const wdColor wdColor::Blue(wdColorGammaUB(0x00, 0x00, 0xFF));
const wdColor wdColor::BlueViolet(wdColorGammaUB(0x8A, 0x2B, 0xE2));
const wdColor wdColor::Brown(wdColorGammaUB(0xA5, 0x2A, 0x2A));
const wdColor wdColor::BurlyWood(wdColorGammaUB(0xDE, 0xB8, 0x87));
const wdColor wdColor::CadetBlue(wdColorGammaUB(0x5F, 0x9E, 0xA0));
const wdColor wdColor::Chartreuse(wdColorGammaUB(0x7F, 0xFF, 0x00));
const wdColor wdColor::Chocolate(wdColorGammaUB(0xD2, 0x69, 0x1E));
const wdColor wdColor::Coral(wdColorGammaUB(0xFF, 0x7F, 0x50));
const wdColor wdColor::CornflowerBlue(wdColorGammaUB(0x64, 0x95, 0xED)); // The Original!
const wdColor wdColor::Cornsilk(wdColorGammaUB(0xFF, 0xF8, 0xDC));
const wdColor wdColor::Crimson(wdColorGammaUB(0xDC, 0x14, 0x3C));
const wdColor wdColor::Cyan(wdColorGammaUB(0x00, 0xFF, 0xFF));
const wdColor wdColor::DarkBlue(wdColorGammaUB(0x00, 0x00, 0x8B));
const wdColor wdColor::DarkCyan(wdColorGammaUB(0x00, 0x8B, 0x8B));
const wdColor wdColor::DarkGoldenRod(wdColorGammaUB(0xB8, 0x86, 0x0B));
const wdColor wdColor::DarkGray(wdColorGammaUB(0xA9, 0xA9, 0xA9));
const wdColor wdColor::DarkGrey(wdColorGammaUB(0xA9, 0xA9, 0xA9));
const wdColor wdColor::DarkGreen(wdColorGammaUB(0x00, 0x64, 0x00));
const wdColor wdColor::DarkKhaki(wdColorGammaUB(0xBD, 0xB7, 0x6B));
const wdColor wdColor::DarkMagenta(wdColorGammaUB(0x8B, 0x00, 0x8B));
const wdColor wdColor::DarkOliveGreen(wdColorGammaUB(0x55, 0x6B, 0x2F));
const wdColor wdColor::DarkOrange(wdColorGammaUB(0xFF, 0x8C, 0x00));
const wdColor wdColor::DarkOrchid(wdColorGammaUB(0x99, 0x32, 0xCC));
const wdColor wdColor::DarkRed(wdColorGammaUB(0x8B, 0x00, 0x00));
const wdColor wdColor::DarkSalmon(wdColorGammaUB(0xE9, 0x96, 0x7A));
const wdColor wdColor::DarkSeaGreen(wdColorGammaUB(0x8F, 0xBC, 0x8F));
const wdColor wdColor::DarkSlateBlue(wdColorGammaUB(0x48, 0x3D, 0x8B));
const wdColor wdColor::DarkSlateGray(wdColorGammaUB(0x2F, 0x4F, 0x4F));
const wdColor wdColor::DarkSlateGrey(wdColorGammaUB(0x2F, 0x4F, 0x4F));
const wdColor wdColor::DarkTurquoise(wdColorGammaUB(0x00, 0xCE, 0xD1));
const wdColor wdColor::DarkViolet(wdColorGammaUB(0x94, 0x00, 0xD3));
const wdColor wdColor::DeepPink(wdColorGammaUB(0xFF, 0x14, 0x93));
const wdColor wdColor::DeepSkyBlue(wdColorGammaUB(0x00, 0xBF, 0xFF));
const wdColor wdColor::DimGray(wdColorGammaUB(0x69, 0x69, 0x69));
const wdColor wdColor::DimGrey(wdColorGammaUB(0x69, 0x69, 0x69));
const wdColor wdColor::DodgerBlue(wdColorGammaUB(0x1E, 0x90, 0xFF));
const wdColor wdColor::FireBrick(wdColorGammaUB(0xB2, 0x22, 0x22));
const wdColor wdColor::FloralWhite(wdColorGammaUB(0xFF, 0xFA, 0xF0));
const wdColor wdColor::ForestGreen(wdColorGammaUB(0x22, 0x8B, 0x22));
const wdColor wdColor::Fuchsia(wdColorGammaUB(0xFF, 0x00, 0xFF));
const wdColor wdColor::Gainsboro(wdColorGammaUB(0xDC, 0xDC, 0xDC));
const wdColor wdColor::GhostWhite(wdColorGammaUB(0xF8, 0xF8, 0xFF));
const wdColor wdColor::Gold(wdColorGammaUB(0xFF, 0xD7, 0x00));
const wdColor wdColor::GoldenRod(wdColorGammaUB(0xDA, 0xA5, 0x20));
const wdColor wdColor::Gray(wdColorGammaUB(0x80, 0x80, 0x80));
const wdColor wdColor::Grey(wdColorGammaUB(0x80, 0x80, 0x80));
const wdColor wdColor::Green(wdColorGammaUB(0x00, 0x80, 0x00));
const wdColor wdColor::GreenYellow(wdColorGammaUB(0xAD, 0xFF, 0x2F));
const wdColor wdColor::HoneyDew(wdColorGammaUB(0xF0, 0xFF, 0xF0));
const wdColor wdColor::HotPink(wdColorGammaUB(0xFF, 0x69, 0xB4));
const wdColor wdColor::IndianRed(wdColorGammaUB(0xCD, 0x5C, 0x5C));
const wdColor wdColor::Indigo(wdColorGammaUB(0x4B, 0x00, 0x82));
const wdColor wdColor::Ivory(wdColorGammaUB(0xFF, 0xFF, 0xF0));
const wdColor wdColor::Khaki(wdColorGammaUB(0xF0, 0xE6, 0x8C));
const wdColor wdColor::Lavender(wdColorGammaUB(0xE6, 0xE6, 0xFA));
const wdColor wdColor::LavenderBlush(wdColorGammaUB(0xFF, 0xF0, 0xF5));
const wdColor wdColor::LawnGreen(wdColorGammaUB(0x7C, 0xFC, 0x00));
const wdColor wdColor::LemonChiffon(wdColorGammaUB(0xFF, 0xFA, 0xCD));
const wdColor wdColor::LightBlue(wdColorGammaUB(0xAD, 0xD8, 0xE6));
const wdColor wdColor::LightCoral(wdColorGammaUB(0xF0, 0x80, 0x80));
const wdColor wdColor::LightCyan(wdColorGammaUB(0xE0, 0xFF, 0xFF));
const wdColor wdColor::LightGoldenRodYellow(wdColorGammaUB(0xFA, 0xFA, 0xD2));
const wdColor wdColor::LightGray(wdColorGammaUB(0xD3, 0xD3, 0xD3));
const wdColor wdColor::LightGrey(wdColorGammaUB(0xD3, 0xD3, 0xD3));
const wdColor wdColor::LightGreen(wdColorGammaUB(0x90, 0xEE, 0x90));
const wdColor wdColor::LightPink(wdColorGammaUB(0xFF, 0xB6, 0xC1));
const wdColor wdColor::LightSalmon(wdColorGammaUB(0xFF, 0xA0, 0x7A));
const wdColor wdColor::LightSeaGreen(wdColorGammaUB(0x20, 0xB2, 0xAA));
const wdColor wdColor::LightSkyBlue(wdColorGammaUB(0x87, 0xCE, 0xFA));
const wdColor wdColor::LightSlateGray(wdColorGammaUB(0x77, 0x88, 0x99));
const wdColor wdColor::LightSlateGrey(wdColorGammaUB(0x77, 0x88, 0x99));
const wdColor wdColor::LightSteelBlue(wdColorGammaUB(0xB0, 0xC4, 0xDE));
const wdColor wdColor::LightYellow(wdColorGammaUB(0xFF, 0xFF, 0xE0));
const wdColor wdColor::Lime(wdColorGammaUB(0x00, 0xFF, 0x00));
const wdColor wdColor::LimeGreen(wdColorGammaUB(0x32, 0xCD, 0x32));
const wdColor wdColor::Linen(wdColorGammaUB(0xFA, 0xF0, 0xE6));
const wdColor wdColor::Magenta(wdColorGammaUB(0xFF, 0x00, 0xFF));
const wdColor wdColor::Maroon(wdColorGammaUB(0x80, 0x00, 0x00));
const wdColor wdColor::MediumAquaMarine(wdColorGammaUB(0x66, 0xCD, 0xAA));
const wdColor wdColor::MediumBlue(wdColorGammaUB(0x00, 0x00, 0xCD));
const wdColor wdColor::MediumOrchid(wdColorGammaUB(0xBA, 0x55, 0xD3));
const wdColor wdColor::MediumPurple(wdColorGammaUB(0x93, 0x70, 0xDB));
const wdColor wdColor::MediumSeaGreen(wdColorGammaUB(0x3C, 0xB3, 0x71));
const wdColor wdColor::MediumSlateBlue(wdColorGammaUB(0x7B, 0x68, 0xEE));
const wdColor wdColor::MediumSpringGreen(wdColorGammaUB(0x00, 0xFA, 0x9A));
const wdColor wdColor::MediumTurquoise(wdColorGammaUB(0x48, 0xD1, 0xCC));
const wdColor wdColor::MediumVioletRed(wdColorGammaUB(0xC7, 0x15, 0x85));
const wdColor wdColor::MidnightBlue(wdColorGammaUB(0x19, 0x19, 0x70));
const wdColor wdColor::MintCream(wdColorGammaUB(0xF5, 0xFF, 0xFA));
const wdColor wdColor::MistyRose(wdColorGammaUB(0xFF, 0xE4, 0xE1));
const wdColor wdColor::Moccasin(wdColorGammaUB(0xFF, 0xE4, 0xB5));
const wdColor wdColor::NavajoWhite(wdColorGammaUB(0xFF, 0xDE, 0xAD));
const wdColor wdColor::Navy(wdColorGammaUB(0x00, 0x00, 0x80));
const wdColor wdColor::OldLace(wdColorGammaUB(0xFD, 0xF5, 0xE6));
const wdColor wdColor::Olive(wdColorGammaUB(0x80, 0x80, 0x00));
const wdColor wdColor::OliveDrab(wdColorGammaUB(0x6B, 0x8E, 0x23));
const wdColor wdColor::Orange(wdColorGammaUB(0xFF, 0xA5, 0x00));
const wdColor wdColor::OrangeRed(wdColorGammaUB(0xFF, 0x45, 0x00));
const wdColor wdColor::Orchid(wdColorGammaUB(0xDA, 0x70, 0xD6));
const wdColor wdColor::PaleGoldenRod(wdColorGammaUB(0xEE, 0xE8, 0xAA));
const wdColor wdColor::PaleGreen(wdColorGammaUB(0x98, 0xFB, 0x98));
const wdColor wdColor::PaleTurquoise(wdColorGammaUB(0xAF, 0xEE, 0xEE));
const wdColor wdColor::PaleVioletRed(wdColorGammaUB(0xDB, 0x70, 0x93));
const wdColor wdColor::PapayaWhip(wdColorGammaUB(0xFF, 0xEF, 0xD5));
const wdColor wdColor::PeachPuff(wdColorGammaUB(0xFF, 0xDA, 0xB9));
const wdColor wdColor::Peru(wdColorGammaUB(0xCD, 0x85, 0x3F));
const wdColor wdColor::Pink(wdColorGammaUB(0xFF, 0xC0, 0xCB));
const wdColor wdColor::Plum(wdColorGammaUB(0xDD, 0xA0, 0xDD));
const wdColor wdColor::PowderBlue(wdColorGammaUB(0xB0, 0xE0, 0xE6));
const wdColor wdColor::Purple(wdColorGammaUB(0x80, 0x00, 0x80));
const wdColor wdColor::RebeccaPurple(wdColorGammaUB(0x66, 0x33, 0x99));
const wdColor wdColor::Red(wdColorGammaUB(0xFF, 0x00, 0x00));
const wdColor wdColor::RosyBrown(wdColorGammaUB(0xBC, 0x8F, 0x8F));
const wdColor wdColor::RoyalBlue(wdColorGammaUB(0x41, 0x69, 0xE1));
const wdColor wdColor::SaddleBrown(wdColorGammaUB(0x8B, 0x45, 0x13));
const wdColor wdColor::Salmon(wdColorGammaUB(0xFA, 0x80, 0x72));
const wdColor wdColor::SandyBrown(wdColorGammaUB(0xF4, 0xA4, 0x60));
const wdColor wdColor::SeaGreen(wdColorGammaUB(0x2E, 0x8B, 0x57));
const wdColor wdColor::SeaShell(wdColorGammaUB(0xFF, 0xF5, 0xEE));
const wdColor wdColor::Sienna(wdColorGammaUB(0xA0, 0x52, 0x2D));
const wdColor wdColor::Silver(wdColorGammaUB(0xC0, 0xC0, 0xC0));
const wdColor wdColor::SkyBlue(wdColorGammaUB(0x87, 0xCE, 0xEB));
const wdColor wdColor::SlateBlue(wdColorGammaUB(0x6A, 0x5A, 0xCD));
const wdColor wdColor::SlateGray(wdColorGammaUB(0x70, 0x80, 0x90));
const wdColor wdColor::SlateGrey(wdColorGammaUB(0x70, 0x80, 0x90));
const wdColor wdColor::Snow(wdColorGammaUB(0xFF, 0xFA, 0xFA));
const wdColor wdColor::SpringGreen(wdColorGammaUB(0x00, 0xFF, 0x7F));
const wdColor wdColor::SteelBlue(wdColorGammaUB(0x46, 0x82, 0xB4));
const wdColor wdColor::Tan(wdColorGammaUB(0xD2, 0xB4, 0x8C));
const wdColor wdColor::Teal(wdColorGammaUB(0x00, 0x80, 0x80));
const wdColor wdColor::Thistle(wdColorGammaUB(0xD8, 0xBF, 0xD8));
const wdColor wdColor::Tomato(wdColorGammaUB(0xFF, 0x63, 0x47));
const wdColor wdColor::Turquoise(wdColorGammaUB(0x40, 0xE0, 0xD0));
const wdColor wdColor::Violet(wdColorGammaUB(0xEE, 0x82, 0xEE));
const wdColor wdColor::Wheat(wdColorGammaUB(0xF5, 0xDE, 0xB3));
const wdColor wdColor::White(wdColorGammaUB(0xFF, 0xFF, 0xFF));
const wdColor wdColor::WhiteSmoke(wdColorGammaUB(0xF5, 0xF5, 0xF5));
const wdColor wdColor::Yellow(wdColorGammaUB(0xFF, 0xFF, 0x00));
const wdColor wdColor::YellowGreen(wdColorGammaUB(0x9A, 0xCD, 0x32));

wdColor wdColor::ZeroColor()
{
  return wdColor(0.0f, 0.0f, 0.0f, 0.0f);
}

WD_STATICLINK_FILE(Foundation, Foundation_Math_Implementation_Color);
