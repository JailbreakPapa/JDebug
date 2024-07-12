#include <Foundation/FoundationPCH.h>

#include <Foundation/Math/CurveFunctions.h>
#include <Foundation/Math/Mat3.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Math/Quat.h>
#include <Foundation/Math/Vec3.h>
#include <Foundation/Reflection/Reflection.h>

// Default are D3D convention before a renderer is initialized.
nsClipSpaceDepthRange::Enum nsClipSpaceDepthRange::Default = nsClipSpaceDepthRange::ZeroToOne;
nsClipSpaceYMode::Enum nsClipSpaceYMode::RenderToTextureDefault = nsClipSpaceYMode::Regular;

nsHandedness::Enum nsHandedness::Default = nsHandedness::LeftHanded;

bool nsMath::IsPowerOf(nsInt32 value, nsInt32 iBase)
{
  if (value == 1)
    return true;

  while (value > iBase)
  {
    if (value % iBase == 0)
      value /= iBase;
    else
      return false;
  }

  return (value == iBase);
}

nsUInt32 nsMath::PowerOfTwo_Floor(nsUInt32 uiNpot)
{
  return static_cast<nsUInt32>(PowerOfTwo_Floor(static_cast<nsUInt64>(uiNpot)));
}

nsUInt64 nsMath::PowerOfTwo_Floor(nsUInt64 uiNpot)
{
  if (IsPowerOf2(uiNpot))
    return (uiNpot);

  for (nsUInt32 i = 1; i <= (sizeof(uiNpot) * 8); ++i)
  {
    uiNpot >>= 1;

    if (uiNpot == 1)
      return (uiNpot << i);
  }

  return (1);
}

nsUInt32 nsMath::PowerOfTwo_Ceil(nsUInt32 uiNpot)
{
  return static_cast<nsUInt32>(PowerOfTwo_Ceil(static_cast<nsUInt64>(uiNpot)));
}

nsUInt64 nsMath::PowerOfTwo_Ceil(nsUInt64 uiNpot)
{
  if (IsPowerOf2(uiNpot))
    return (uiNpot);

  for (nsUInt32 i = 1; i <= (sizeof(uiNpot) * 8); ++i)
  {
    uiNpot >>= 1;

    if (uiNpot == 1)
    {
      // note: left shift by 32 bits is undefined behavior and typically just returns the left operand unchanged
      // so for npot values larger than 1^31 we do run into this code path, but instead of returning 0, as one may expect, it will usually return 1
      return uiNpot << (i + 1u);
    }
  }

  return (1u);
}


nsUInt32 nsMath::GreatestCommonDivisor(nsUInt32 a, nsUInt32 b)
{
  // https://lemire.me/blog/2013/12/26/fastest-way-to-compute-the-greatest-common-divisor/
  if (a == 0)
  {
    return a;
  }
  if (b == 0)
  {
    return b;
  }

  nsUInt32 shift = FirstBitLow(a | b);
  a >>= FirstBitLow(a);
  do
  {
    b >>= FirstBitLow(b);
    if (a > b)
    {
      Swap(a, b);
    }
    b = b - a;
  } while (b != 0);
  return a << shift;
}

nsResult nsMath::TryMultiply32(nsUInt32& out_uiResult, nsUInt32 a, nsUInt32 b, nsUInt32 c, nsUInt32 d)
{
  nsUInt64 result = static_cast<nsUInt64>(a) * static_cast<nsUInt64>(b);

  if (result > 0xFFFFFFFFllu)
  {
    return NS_FAILURE;
  }

  result *= static_cast<nsUInt64>(c);

  if (result > 0xFFFFFFFFllu)
  {
    return NS_FAILURE;
  }

  result *= static_cast<nsUInt64>(d);

  if (result > 0xFFFFFFFFllu)
  {
    return NS_FAILURE;
  }

  out_uiResult = static_cast<nsUInt32>(result & 0xFFFFFFFFllu);
  return NS_SUCCESS;
}

nsUInt32 nsMath::SafeMultiply32(nsUInt32 a, nsUInt32 b, nsUInt32 c, nsUInt32 d)
{
  nsUInt32 result = 0;
  if (TryMultiply32(result, a, b, c, d).Succeeded())
  {
    return result;
  }

  NS_REPORT_FAILURE("Safe multiplication failed: {0} * {1} * {2} * {3} exceeds UInt32 range.", a, b, c, d);
  std::terminate();
}

nsResult nsMath::TryMultiply64(nsUInt64& out_uiResult, nsUInt64 a, nsUInt64 b, nsUInt64 c, nsUInt64 d)
{
  if (a == 0 || b == 0 || c == 0 || d == 0)
  {
    out_uiResult = 0;
    return NS_SUCCESS;
  }

#if NS_ENABLED(NS_PLATFORM_ARCH_X86) && NS_ENABLED(NS_PLATFORM_64BIT) && NS_ENABLED(NS_COMPILER_MSVC)

  nsUInt64 uiHighBits = 0;

  const nsUInt64 ab = _umul128(a, b, &uiHighBits);
  if (uiHighBits != 0)
  {
    return NS_FAILURE;
  }

  const nsUInt64 abc = _umul128(ab, c, &uiHighBits);
  if (uiHighBits != 0)
  {
    return NS_FAILURE;
  }

  const nsUInt64 abcd = _umul128(abc, d, &uiHighBits);
  if (uiHighBits != 0)
  {
    return NS_FAILURE;
  }

#else
  const nsUInt64 ab = a * b;
  const nsUInt64 abc = ab * c;
  const nsUInt64 abcd = abc * d;

  if (a > 1 && b > 1 && (ab / a != b))
  {
    return NS_FAILURE;
  }

  if (c > 1 && (abc / c != ab))
  {
    return NS_FAILURE;
  }

  if (d > 1 && (abcd / d != abc))
  {
    return NS_FAILURE;
  }

#endif

  out_uiResult = abcd;
  return NS_SUCCESS;
}

nsUInt64 nsMath::SafeMultiply64(nsUInt64 a, nsUInt64 b, nsUInt64 c, nsUInt64 d)
{
  nsUInt64 result = 0;
  if (TryMultiply64(result, a, b, c, d).Succeeded())
  {
    return result;
  }

  NS_REPORT_FAILURE("Safe multiplication failed: {0} * {1} * {2} * {3} exceeds nsUInt64 range.", a, b, c, d);
  std::terminate();
}

#if NS_ENABLED(NS_PLATFORM_32BIT)
size_t nsMath::SafeConvertToSizeT(nsUInt64 uiValue)
{
  size_t result = 0;
  if (TryConvertToSizeT(result, uiValue).Succeeded())
  {
    return result;
  }

  NS_REPORT_FAILURE("Given value ({}) can't be converted to size_t because it is too big.", uiValue);
  std::terminate();
}
#endif

void nsAngle::NormalizeRange()
{
  constexpr float fTwoPi = 2.0f * Pi<float>();
  constexpr float fTwoPiTen = 10.0f * Pi<float>();

  if (m_fRadian > fTwoPiTen || m_fRadian < -fTwoPiTen)
  {
    m_fRadian = nsMath::Mod(m_fRadian, fTwoPi);
  }

  while (m_fRadian >= fTwoPi)
  {
    m_fRadian -= fTwoPi;
  }

  while (m_fRadian < 0.0f)
  {
    m_fRadian += fTwoPi;
  }
}

float nsMath::ReplaceNaN(float fValue, float fFallback)
{
  // ATTENTION: if this is a template, inline or constexpr function, the current MSVC (17.6)
  // seems to generate incorrect code and the IsNaN check doesn't detect NaNs.
  // As an out-of-line function it works.

  if (nsMath::IsNaN(fValue))
    return fFallback;

  return fValue;
}

double nsMath::ReplaceNaN(double fValue, double fFallback)
{
  // ATTENTION: if this is a template, inline or constexpr function, the current MSVC (17.6)
  // seems to generate incorrect code and the IsNaN check doesn't detect NaNs.
  // As an out-of-line function it works.

  if (nsMath::IsNaN(fValue))
    return fFallback;

  return fValue;
}

nsVec3 nsBasisAxis::GetBasisVector(Enum basisAxis)
{
  switch (basisAxis)
  {
    case nsBasisAxis::PositiveX:
      return nsVec3(1.0f, 0.0f, 0.0f);

    case nsBasisAxis::NegativeX:
      return nsVec3(-1.0f, 0.0f, 0.0f);

    case nsBasisAxis::PositiveY:
      return nsVec3(0.0f, 1.0f, 0.0f);

    case nsBasisAxis::NegativeY:
      return nsVec3(0.0f, -1.0f, 0.0f);

    case nsBasisAxis::PositiveZ:
      return nsVec3(0.0f, 0.0f, 1.0f);

    case nsBasisAxis::NegativeZ:
      return nsVec3(0.0f, 0.0f, -1.0f);

    default:
      NS_REPORT_FAILURE("Invalid basis dir {0}", basisAxis);
      return nsVec3::MakeZero();
  }
}

nsMat3 nsBasisAxis::CalculateTransformationMatrix(Enum forwardDir, Enum rightDir, Enum dir, float fUniformScale /*= 1.0f*/, float fScaleX /*= 1.0f*/, float fScaleY /*= 1.0f*/, float fScaleZ /*= 1.0f*/)
{
  nsMat3 mResult;
  mResult.SetRow(0, nsBasisAxis::GetBasisVector(forwardDir) * fUniformScale * fScaleX);
  mResult.SetRow(1, nsBasisAxis::GetBasisVector(rightDir) * fUniformScale * fScaleY);
  mResult.SetRow(2, nsBasisAxis::GetBasisVector(dir) * fUniformScale * fScaleZ);

  return mResult;
}


nsQuat nsBasisAxis::GetBasisRotation_PosX(Enum axis)
{
  return nsQuat::MakeShortestRotation(nsVec3::MakeAxisX(), GetBasisVector(axis));
}

nsQuat nsBasisAxis::GetBasisRotation(Enum identity, Enum axis)
{
  return nsQuat::MakeShortestRotation(GetBasisVector(identity), GetBasisVector(axis));
}

nsBasisAxis::Enum nsBasisAxis::GetOrthogonalAxis(Enum axis1, Enum axis2, bool bFlip)
{
  const nsVec3 a1 = nsBasisAxis::GetBasisVector(axis1);
  const nsVec3 a2 = nsBasisAxis::GetBasisVector(axis2);

  nsVec3 c = a1.CrossRH(a2);

  if (bFlip)
    c = -c;

  if (c.IsEqual(nsVec3::MakeAxisX(), 0.01f))
    return nsBasisAxis::PositiveX;
  if (c.IsEqual(-nsVec3::MakeAxisX(), 0.01f))
    return nsBasisAxis::NegativeX;

  if (c.IsEqual(nsVec3::MakeAxisY(), 0.01f))
    return nsBasisAxis::PositiveY;
  if (c.IsEqual(-nsVec3::MakeAxisY(), 0.01f))
    return nsBasisAxis::NegativeY;

  if (c.IsEqual(nsVec3::MakeAxisZ(), 0.01f))
    return nsBasisAxis::PositiveZ;
  if (c.IsEqual(-nsVec3::MakeAxisZ(), 0.01f))
    return nsBasisAxis::NegativeZ;

  return axis1;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsComparisonOperator, 1)
  NS_ENUM_CONSTANTS(nsComparisonOperator::Equal, nsComparisonOperator::NotEqual)
  NS_ENUM_CONSTANTS(nsComparisonOperator::Less, nsComparisonOperator::LessEqual)
  NS_ENUM_CONSTANTS(nsComparisonOperator::Greater, nsComparisonOperator::GreaterEqual)
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_STATIC_REFLECTED_ENUM(nsCurveFunction, 1)
 NS_ENUM_CONSTANT(nsCurveFunction::Linear),
 NS_ENUM_CONSTANT(nsCurveFunction::ConstantZero),
 NS_ENUM_CONSTANT(nsCurveFunction::ConstantOne),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInSine),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseOutSine),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInOutSine),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInQuad),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseOutQuad),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInOutQuad),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInCubic),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseOutCubic),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInOutCubic),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInQuartic),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseOutQuartic),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInOutQuartic),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInQuintic),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseOutQuintic),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInOutQuintic),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInExpo),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseOutExpo),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInOutExpo),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInCirc),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseOutCirc),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInOutCirc),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInBack),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseOutBack),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInOutBack), 
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInElastic), 
 NS_ENUM_CONSTANT(nsCurveFunction::EaseOutElastic),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInOutElastic),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInBounce),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseOutBounce),
 NS_ENUM_CONSTANT(nsCurveFunction::EaseInOutBounce),
 NS_ENUM_CONSTANT(nsCurveFunction::Conical),
 NS_ENUM_CONSTANT(nsCurveFunction::FadeInHoldFadeOut),
 NS_ENUM_CONSTANT(nsCurveFunction::FadeInFadeOut),
 NS_ENUM_CONSTANT(nsCurveFunction::Bell),
NS_END_STATIC_REFLECTED_ENUM;
// clang-format on

NS_STATICLINK_FILE(Foundation, Foundation_Math_Implementation_Math);
