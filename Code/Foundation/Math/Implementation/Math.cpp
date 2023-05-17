#include <Foundation/FoundationPCH.h>

#include <Foundation/Math/Mat3.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Math/Quat.h>
#include <Foundation/Math/Vec3.h>
#include <Foundation/Reflection/Reflection.h>

// Default are D3D convention before a renderer is initialized.
wdClipSpaceDepthRange::Enum wdClipSpaceDepthRange::Default = wdClipSpaceDepthRange::ZeroToOne;
wdClipSpaceYMode::Enum wdClipSpaceYMode::RenderToTextureDefault = wdClipSpaceYMode::Regular;

wdHandedness::Enum wdHandedness::Default = wdHandedness::LeftHanded;

bool wdMath::IsPowerOf(wdInt32 value, wdInt32 iBase)
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

wdUInt32 wdMath::PowerOfTwo_Floor(wdUInt32 uiNpot)
{
  if (IsPowerOf2(uiNpot))
    return (uiNpot);

  for (wdUInt32 i = 1; i <= (sizeof(uiNpot) * 8); ++i)
  {
    uiNpot >>= 1;

    if (uiNpot == 1)
      return (uiNpot << i);
  }

  return (1);
}

wdUInt32 wdMath::PowerOfTwo_Ceil(wdUInt32 uiNpot)
{
  if (IsPowerOf2(uiNpot))
    return (uiNpot);

  for (wdUInt32 i = 1; i <= (sizeof(uiNpot) * 8); ++i)
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


wdUInt32 wdMath::GreatestCommonDivisor(wdUInt32 a, wdUInt32 b)
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

  wdUInt32 shift = FirstBitLow(a | b);
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

wdResult wdMath::TryMultiply32(wdUInt32& out_uiResult, wdUInt32 a, wdUInt32 b, wdUInt32 c, wdUInt32 d)
{
  wdUInt64 result = static_cast<wdUInt64>(a) * static_cast<wdUInt64>(b);

  if (result > 0xFFFFFFFFllu)
  {
    return WD_FAILURE;
  }

  result *= static_cast<wdUInt64>(c);

  if (result > 0xFFFFFFFFllu)
  {
    return WD_FAILURE;
  }

  result *= static_cast<wdUInt64>(d);

  if (result > 0xFFFFFFFFllu)
  {
    return WD_FAILURE;
  }

  out_uiResult = static_cast<wdUInt32>(result & 0xFFFFFFFFllu);
  return WD_SUCCESS;
}

wdUInt32 wdMath::SafeMultiply32(wdUInt32 a, wdUInt32 b, wdUInt32 c, wdUInt32 d)
{
  wdUInt32 result = 0;
  if (TryMultiply32(result, a, b, c, d).Succeeded())
  {
    return result;
  }

  WD_REPORT_FAILURE("Safe multiplication failed: {0} * {1} * {2} * {3} exceeds UInt32 range.", a, b, c, d);
  std::terminate();
  return 0;
}

wdResult wdMath::TryMultiply64(wdUInt64& out_uiResult, wdUInt64 a, wdUInt64 b, wdUInt64 c, wdUInt64 d)
{
  if (a == 0 || b == 0 || c == 0 || d == 0)
  {
    out_uiResult = 0;
    return WD_SUCCESS;
  }

#if WD_ENABLED(WD_PLATFORM_ARCH_X86) && WD_ENABLED(WD_PLATFORM_64BIT) && WD_ENABLED(WD_COMPILER_MSVC)

  wdUInt64 uiHighBits = 0;

  const wdUInt64 ab = _umul128(a, b, &uiHighBits);
  if (uiHighBits != 0)
  {
    return WD_FAILURE;
  }

  const wdUInt64 abc = _umul128(ab, c, &uiHighBits);
  if (uiHighBits != 0)
  {
    return WD_FAILURE;
  }

  const wdUInt64 abcd = _umul128(abc, d, &uiHighBits);
  if (uiHighBits != 0)
  {
    return WD_FAILURE;
  }

#else
  const wdUInt64 ab = a * b;
  const wdUInt64 abc = ab * c;
  const wdUInt64 abcd = abc * d;

  if (a > 1 && b > 1 && (ab / a != b))
  {
    return WD_FAILURE;
  }

  if (c > 1 && (abc / c != ab))
  {
    return WD_FAILURE;
  }

  if (d > 1 && (abcd / d != abc))
  {
    return WD_FAILURE;
  }

#endif

  out_uiResult = abcd;
  return WD_SUCCESS;
}

wdUInt64 wdMath::SafeMultiply64(wdUInt64 a, wdUInt64 b, wdUInt64 c, wdUInt64 d)
{
  wdUInt64 result = 0;
  if (TryMultiply64(result, a, b, c, d).Succeeded())
  {
    return result;
  }

  WD_REPORT_FAILURE("Safe multiplication failed: {0} * {1} * {2} * {3} exceeds wdUInt64 range.", a, b, c, d);
  std::terminate();
  return 0;
}

#if WD_ENABLED(WD_PLATFORM_32BIT)
size_t wdMath::SafeConvertToSizeT(wdUInt64 uiValue)
{
  size_t result = 0;
  if (TryConvertToSizeT(result, uiValue).Succeeded())
  {
    return result;
  }

  WD_REPORT_FAILURE("Given value ({}) can't be converted to size_t because it is too big.", uiValue);
  std::terminate();
  return 0;
}
#endif

void wdAngle::NormalizeRange()
{
  const float fTwoPi = 2.0f * Pi<float>();

  const float fTwoPiTen = 10.0f * Pi<float>();

  if (m_fRadian > fTwoPiTen || m_fRadian < -fTwoPiTen)
  {
    m_fRadian = wdMath::Mod(m_fRadian, fTwoPi);
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

wdVec3 wdBasisAxis::GetBasisVector(Enum basisAxis)
{
  switch (basisAxis)
  {
    case wdBasisAxis::PositiveX:
      return wdVec3(1.0f, 0.0f, 0.0f);

    case wdBasisAxis::NegativeX:
      return wdVec3(-1.0f, 0.0f, 0.0f);

    case wdBasisAxis::PositiveY:
      return wdVec3(0.0f, 1.0f, 0.0f);

    case wdBasisAxis::NegativeY:
      return wdVec3(0.0f, -1.0f, 0.0f);

    case wdBasisAxis::PositiveZ:
      return wdVec3(0.0f, 0.0f, 1.0f);

    case wdBasisAxis::NegativeZ:
      return wdVec3(0.0f, 0.0f, -1.0f);

    default:
      WD_REPORT_FAILURE("Invalid basis dir {0}", basisAxis);
      return wdVec3::ZeroVector();
  }
}

wdMat3 wdBasisAxis::CalculateTransformationMatrix(Enum forwardDir, Enum rightDir, Enum dir, float fUniformScale /*= 1.0f*/, float fScaleX /*= 1.0f*/, float fScaleY /*= 1.0f*/, float fScaleZ /*= 1.0f*/)
{
  wdMat3 mResult;
  mResult.SetRow(0, wdBasisAxis::GetBasisVector(forwardDir) * fUniformScale * fScaleX);
  mResult.SetRow(1, wdBasisAxis::GetBasisVector(rightDir) * fUniformScale * fScaleY);
  mResult.SetRow(2, wdBasisAxis::GetBasisVector(dir) * fUniformScale * fScaleZ);

  return mResult;
}


wdQuat wdBasisAxis::GetBasisRotation_PosX(Enum axis)
{
  wdQuat rotAxis;
  switch (axis)
  {
    case wdBasisAxis::PositiveX:
      rotAxis.SetIdentity();
      break;
    case wdBasisAxis::PositiveY:
      rotAxis.SetFromAxisAndAngle(wdVec3(0, 0, 1), wdAngle::Degree(90));
      break;
    case wdBasisAxis::PositiveZ:
      rotAxis.SetFromAxisAndAngle(wdVec3(0, 1, 0), wdAngle::Degree(-90));
      break;
    case wdBasisAxis::NegativeX:
      rotAxis.SetFromAxisAndAngle(wdVec3(0, 1, 0), wdAngle::Degree(180));
      break;
    case wdBasisAxis::NegativeY:
      rotAxis.SetFromAxisAndAngle(wdVec3(0, 0, 1), wdAngle::Degree(-90));
      break;
    case wdBasisAxis::NegativeZ:
      rotAxis.SetFromAxisAndAngle(wdVec3(0, 1, 0), wdAngle::Degree(90));
      break;
  }

  return rotAxis;
}

wdQuat wdBasisAxis::GetBasisRotation(Enum identity, Enum axis)
{
  wdQuat rotId;
  switch (identity)
  {
    case wdBasisAxis::PositiveX:
      rotId.SetIdentity();
      break;
    case wdBasisAxis::PositiveY:
      rotId.SetFromAxisAndAngle(wdVec3(0, 0, 1), wdAngle::Degree(-90));
      break;
    case wdBasisAxis::PositiveZ:
      rotId.SetFromAxisAndAngle(wdVec3(0, 1, 0), wdAngle::Degree(90));
      break;
    case wdBasisAxis::NegativeX:
      rotId.SetFromAxisAndAngle(wdVec3(0, 1, 0), wdAngle::Degree(180));
      break;
    case wdBasisAxis::NegativeY:
      rotId.SetFromAxisAndAngle(wdVec3(0, 0, 1), wdAngle::Degree(90));
      break;
    case wdBasisAxis::NegativeZ:
      rotId.SetFromAxisAndAngle(wdVec3(0, 1, 0), wdAngle::Degree(90));
      break;
  }

  wdQuat rotAxis;
  switch (axis)
  {
    case wdBasisAxis::PositiveX:
      rotAxis.SetIdentity();
      break;
    case wdBasisAxis::PositiveY:
      rotAxis.SetFromAxisAndAngle(wdVec3(0, 0, 1), wdAngle::Degree(90));
      break;
    case wdBasisAxis::PositiveZ:
      rotAxis.SetFromAxisAndAngle(wdVec3(0, 1, 0), wdAngle::Degree(-90));
      break;
    case wdBasisAxis::NegativeX:
      rotAxis.SetFromAxisAndAngle(wdVec3(0, 1, 0), wdAngle::Degree(180));
      break;
    case wdBasisAxis::NegativeY:
      rotAxis.SetFromAxisAndAngle(wdVec3(0, 0, 1), wdAngle::Degree(-90));
      break;
    case wdBasisAxis::NegativeZ:
      rotAxis.SetFromAxisAndAngle(wdVec3(0, 1, 0), wdAngle::Degree(90));
      break;
  }

  return rotAxis * rotId;
}

wdBasisAxis::Enum wdBasisAxis::GetOrthogonalAxis(Enum axis1, Enum axis2, bool bFlip)
{
  const wdVec3 a1 = wdBasisAxis::GetBasisVector(axis1);
  const wdVec3 a2 = wdBasisAxis::GetBasisVector(axis2);

  wdVec3 c = a1.CrossRH(a2);

  if (bFlip)
    c = -c;

  if (c.IsEqual(wdVec3::UnitXAxis(), 0.01f))
    return wdBasisAxis::PositiveX;
  if (c.IsEqual(-wdVec3::UnitXAxis(), 0.01f))
    return wdBasisAxis::NegativeX;

  if (c.IsEqual(wdVec3::UnitYAxis(), 0.01f))
    return wdBasisAxis::PositiveY;
  if (c.IsEqual(-wdVec3::UnitYAxis(), 0.01f))
    return wdBasisAxis::NegativeY;

  if (c.IsEqual(wdVec3::UnitZAxis(), 0.01f))
    return wdBasisAxis::PositiveZ;
  if (c.IsEqual(-wdVec3::UnitZAxis(), 0.01f))
    return wdBasisAxis::NegativeZ;

  return axis1;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_STATIC_REFLECTED_ENUM(wdComparisonOperator, 1)
  WD_ENUM_CONSTANTS(wdComparisonOperator::Equal, wdComparisonOperator::NotEqual)
  WD_ENUM_CONSTANTS(wdComparisonOperator::Less, wdComparisonOperator::LessEqual)
  WD_ENUM_CONSTANTS(wdComparisonOperator::Greater, wdComparisonOperator::GreaterEqual)
WD_END_STATIC_REFLECTED_ENUM;
// clang-format on

// static
bool wdComparisonOperator::Compare(wdComparisonOperator::Enum cmp, double f1, double f2)
{
  switch (cmp)
  {
    case wdComparisonOperator::Equal:
      return f1 == f2;
    case wdComparisonOperator::NotEqual:
      return f1 != f2;
    case wdComparisonOperator::Less:
      return f1 < f2;
    case wdComparisonOperator::LessEqual:
      return f1 <= f2;
    case wdComparisonOperator::Greater:
      return f1 > f2;
    case wdComparisonOperator::GreaterEqual:
      return f1 >= f2;

      WD_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  return false;
}


WD_STATICLINK_FILE(Foundation, Foundation_Math_Implementation_Math);
