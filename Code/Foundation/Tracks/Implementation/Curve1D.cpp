#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/Stream.h>
#include <Foundation/Tracks/Curve1D.h>

wdCurve1D::ControlPoint::ControlPoint()
{
  m_Position.SetZero();
  m_LeftTangent.SetZero();
  m_RightTangent.SetZero();
  m_uiOriginalIndex = 0;
}

wdCurve1D::wdCurve1D()
{
  Clear();
}

void wdCurve1D::Clear()
{
  m_fMinX = 0;
  m_fMaxX = 0;
  m_fMinY = 0;
  m_fMaxY = 0;

  m_ControlPoints.Clear();
}

bool wdCurve1D::IsEmpty() const
{
  return m_ControlPoints.IsEmpty();
}

wdCurve1D::ControlPoint& wdCurve1D::AddControlPoint(double x)
{
  auto& cp = m_ControlPoints.ExpandAndGetRef();
  cp.m_uiOriginalIndex = static_cast<wdUInt16>(m_ControlPoints.GetCount() - 1);
  cp.m_Position.x = x;
  cp.m_Position.y = 0;
  cp.m_LeftTangent.x = -0.1f;
  cp.m_LeftTangent.y = 0.0f;
  cp.m_RightTangent.x = +0.1f;
  cp.m_RightTangent.y = 0.0f;

  return cp;
}

void wdCurve1D::QueryExtents(double& ref_fMinx, double& ref_fMaxx) const
{
  ref_fMinx = m_fMinX;
  ref_fMaxx = m_fMaxX;
}

void wdCurve1D::QueryExtremeValues(double& ref_fMinVal, double& ref_fMaxVal) const
{
  ref_fMinVal = m_fMinY;
  ref_fMaxVal = m_fMaxY;
}

wdUInt32 wdCurve1D::GetNumControlPoints() const
{
  return m_ControlPoints.GetCount();
}

void wdCurve1D::SortControlPoints()
{
  m_ControlPoints.Sort();

  RecomputeExtents();
}

wdInt32 wdCurve1D::FindApproxControlPoint(double x) const
{
  wdUInt32 uiLowIdx = 0;
  wdUInt32 uiHighIdx = m_LinearApproximation.GetCount();

  // do a binary search to reduce the search space
  while (uiHighIdx - uiLowIdx > 8)
  {
    const wdUInt32 uiMidIdx = uiLowIdx + ((uiHighIdx - uiLowIdx) >> 1); // lerp

    // doesn't matter whether to use > or >=
    if (m_LinearApproximation[uiMidIdx].x > x)
      uiHighIdx = uiMidIdx;
    else
      uiLowIdx = uiMidIdx;
  }

  // now do a linear search to find the final item
  for (wdUInt32 idx = uiLowIdx; idx < uiHighIdx; ++idx)
  {
    if (m_LinearApproximation[idx].x >= x)
    {
      // when m_LinearApproximation[0].x >= x, we want to return -1
      return ((wdInt32)idx) - 1;
    }
  }

  // return last index
  return (wdInt32)uiHighIdx - 1;
}

double wdCurve1D::Evaluate(double x) const
{
  WD_ASSERT_DEBUG(!m_LinearApproximation.IsEmpty(), "Cannot evaluate curve without precomputing curve approximation data first. Call CreateLinearApproximation() on curve before calling Evaluate().");

  if (m_LinearApproximation.GetCount() >= 2)
  {
    const wdUInt32 numCPs = m_LinearApproximation.GetCount();
    const wdInt32 iControlPoint = FindApproxControlPoint(x);

    if (iControlPoint == -1)
    {
      // clamp to left value
      return m_LinearApproximation[0].y;
    }
    else if (iControlPoint == numCPs - 1)
    {
      // clamp to right value
      return m_LinearApproximation[numCPs - 1].y;
    }
    else
    {
      const double v1 = m_LinearApproximation[iControlPoint].y;
      const double v2 = m_LinearApproximation[iControlPoint + 1].y;

      // interpolate
      double lerpX = x - m_LinearApproximation[iControlPoint].x;
      const double len = (m_LinearApproximation[iControlPoint + 1].x - m_LinearApproximation[iControlPoint].x);

      if (len <= 0)
        lerpX = 0;
      else
        lerpX /= len; // TODO remove division ?

      return wdMath::Lerp(v1, v2, lerpX);
    }
  }
  else if (m_LinearApproximation.GetCount() == 1)
  {
    return m_LinearApproximation[0].y;
  }

  return 0;
}

double wdCurve1D::ConvertNormalizedPos(double fPos) const
{
  double fMin, fMax;
  QueryExtents(fMin, fMax);

  return wdMath::Lerp(fMin, fMax, fPos);
}


double wdCurve1D::NormalizeValue(double value) const
{
  double fMin, fMax;
  QueryExtremeValues(fMin, fMax);

  if (fMin >= fMax)
    return 0;

  return (value - fMin) / (fMax - fMin);
}

wdUInt64 wdCurve1D::GetHeapMemoryUsage() const
{
  return m_ControlPoints.GetHeapMemoryUsage();
}

void wdCurve1D::Save(wdStreamWriter& inout_stream) const
{
  const wdUInt8 uiVersion = 4;

  inout_stream << uiVersion;

  const wdUInt32 numCp = m_ControlPoints.GetCount();

  inout_stream << numCp;

  for (const auto& cp : m_ControlPoints)
  {
    inout_stream << cp.m_Position;
    inout_stream << cp.m_LeftTangent;
    inout_stream << cp.m_RightTangent;
    inout_stream << cp.m_TangentModeRight;
    inout_stream << cp.m_TangentModeLeft;
  }
}

void wdCurve1D::Load(wdStreamReader& inout_stream)
{
  wdUInt8 uiVersion = 0;

  inout_stream >> uiVersion;
  WD_ASSERT_DEV(uiVersion <= 4, "Incorrect version '{0}' for wdCurve1D", uiVersion);

  wdUInt32 numCp = 0;

  inout_stream >> numCp;

  m_ControlPoints.SetCountUninitialized(numCp);

  if (uiVersion <= 2)
  {
    for (auto& cp : m_ControlPoints)
    {
      wdVec2 pos;
      inout_stream >> pos;
      cp.m_Position.Set(pos.x, pos.y);

      if (uiVersion >= 2)
      {
        inout_stream >> cp.m_LeftTangent;
        inout_stream >> cp.m_RightTangent;
      }
    }
  }
  else
  {
    for (auto& cp : m_ControlPoints)
    {
      inout_stream >> cp.m_Position;
      inout_stream >> cp.m_LeftTangent;
      inout_stream >> cp.m_RightTangent;

      if (uiVersion >= 4)
      {
        inout_stream >> cp.m_TangentModeRight;
        inout_stream >> cp.m_TangentModeLeft;
      }
    }
  }
}

void wdCurve1D::CreateLinearApproximation(double fMaxError /*= 0.01f*/, wdUInt8 uiMaxSubDivs /*= 8*/)
{
  m_LinearApproximation.Clear();

  /// \todo Since we do this, we actually don't need the linear approximation anymore and could just evaluate the full curve
  ApplyTangentModes();

  ClampTangents();

  if (m_ControlPoints.IsEmpty())
  {
    m_LinearApproximation.PushBack(wdVec2d::ZeroVector());
    return;
  }

  for (wdUInt32 i = 1; i < m_ControlPoints.GetCount(); ++i)
  {
    WD_ASSERT_DEBUG(m_ControlPoints[i - 1].m_Position.x <= m_ControlPoints[i].m_Position.x, "Curve control points are not sorted. Call SortControlPoints() before CreateLinearApproximation().");

    double fMinY, fMaxY;
    ApproximateMinMaxValues(m_ControlPoints[i - 1], m_ControlPoints[i], fMinY, fMaxY);

    const double rangeY = wdMath::Max(0.1, fMaxY - fMinY);
    const double fMaxErrorY = fMaxError * rangeY;
    const double fMaxErrorX = (m_ControlPoints[i].m_Position.x - m_ControlPoints[i - 1].m_Position.x) * fMaxError;


    m_LinearApproximation.PushBack(m_ControlPoints[i - 1].m_Position);

    ApproximateCurve(m_ControlPoints[i - 1].m_Position,
      m_ControlPoints[i - 1].m_Position + wdVec2d(m_ControlPoints[i - 1].m_RightTangent.x, m_ControlPoints[i - 1].m_RightTangent.y),
      m_ControlPoints[i].m_Position + wdVec2d(m_ControlPoints[i].m_LeftTangent.x, m_ControlPoints[i].m_LeftTangent.y), m_ControlPoints[i].m_Position,
      fMaxErrorX, fMaxErrorY, uiMaxSubDivs);
  }

  m_LinearApproximation.PushBack(m_ControlPoints.PeekBack().m_Position);

  RecomputeLinearApproxExtremes();
}

void wdCurve1D::RecomputeExtents()
{
  m_fMinX = wdMath::MaxValue<float>();
  m_fMaxX = -wdMath::MaxValue<float>();

  for (const auto& cp : m_ControlPoints)
  {
    m_fMinX = wdMath::Min(m_fMinX, cp.m_Position.x);
    m_fMaxX = wdMath::Max(m_fMaxX, cp.m_Position.x);

    // ignore X values that could go outside the control point range due to Bwdier curve interpolation
    // we just assume the curve is always restricted along X by the CPs

    // m_fMinX = wdMath::Min(m_fMinX, cp.m_Position.x + cp.m_LeftTangent.x);
    // m_fMaxX = wdMath::Max(m_fMaxX, cp.m_Position.x + cp.m_LeftTangent.x);

    // m_fMinX = wdMath::Min(m_fMinX, cp.m_Position.x + cp.m_RightTangent.x);
    // m_fMaxX = wdMath::Max(m_fMaxX, cp.m_Position.x + cp.m_RightTangent.x);
  }
}


void wdCurve1D::RecomputeLinearApproxExtremes()
{
  m_fMinY = wdMath::MaxValue<float>();
  m_fMaxY = -wdMath::MaxValue<float>();

  for (const auto& cp : m_LinearApproximation)
  {
    m_fMinY = wdMath::Min(m_fMinY, cp.y);
    m_fMaxY = wdMath::Max(m_fMaxY, cp.y);
  }
}

void wdCurve1D::ApproximateMinMaxValues(const ControlPoint& lhs, const ControlPoint& rhs, double& fMinY, double& fMaxY)
{
  fMinY = wdMath::Min(lhs.m_Position.y, rhs.m_Position.y);
  fMaxY = wdMath::Max(lhs.m_Position.y, rhs.m_Position.y);

  fMinY = wdMath::Min(fMinY, lhs.m_Position.y + lhs.m_RightTangent.y);
  fMaxY = wdMath::Max(fMaxY, lhs.m_Position.y + lhs.m_RightTangent.y);

  fMinY = wdMath::Min(fMinY, rhs.m_Position.y + rhs.m_LeftTangent.y);
  fMaxY = wdMath::Max(fMaxY, rhs.m_Position.y + rhs.m_LeftTangent.y);
}

void wdCurve1D::ApproximateCurve(
  const wdVec2d& p0, const wdVec2d& p1, const wdVec2d& p2, const wdVec2d& p3, double fMaxErrorX, double fMaxErrorY, wdInt32 iSubDivLeft)
{
  const wdVec2d cubicCenter = wdMath::EvaluateBwdierCurve(0.5, p0, p1, p2, p3);

  ApproximateCurvePiece(p0, p1, p2, p3, 0.0f, p0, 0.5, cubicCenter, fMaxErrorX, fMaxErrorY, iSubDivLeft);

  // always insert the center point
  // with an S curve the cubicCenter and the linearCenter can be identical even though the rest of the curve is absolutely not linear
  m_LinearApproximation.PushBack(cubicCenter);

  ApproximateCurvePiece(p0, p1, p2, p3, 0.5, cubicCenter, 1.0, p3, fMaxErrorX, fMaxErrorY, iSubDivLeft);
}

void wdCurve1D::ApproximateCurvePiece(const wdVec2d& p0, const wdVec2d& p1, const wdVec2d& p2, const wdVec2d& p3, double tLeft, const wdVec2d& pLeft,
  double tRight, const wdVec2d& pRight, double fMaxErrorX, double fMaxErrorY, wdInt32 iSubDivLeft)
{
  // this is a safe guard
  if (iSubDivLeft <= 0)
    return;

  const double tCenter = wdMath::Lerp(tLeft, tRight, 0.5);

  const wdVec2d cubicCenter = wdMath::EvaluateBwdierCurve(tCenter, p0, p1, p2, p3);
  const wdVec2d linearCenter = wdMath::Lerp(pLeft, pRight, 0.5);

  // check whether the linear interpolation between pLeft and pRight would already result in a good enough approximation
  // if not, subdivide the curve further

  const double fThisErrorX = wdMath::Abs(cubicCenter.x - linearCenter.x);
  const double fThisErrorY = wdMath::Abs(cubicCenter.y - linearCenter.y);

  if (fThisErrorX < fMaxErrorX && fThisErrorY < fMaxErrorY)
    return;

  ApproximateCurvePiece(p0, p1, p2, p3, tLeft, pLeft, tCenter, cubicCenter, fMaxErrorX, fMaxErrorY, iSubDivLeft - 1);

  m_LinearApproximation.PushBack(cubicCenter);

  ApproximateCurvePiece(p0, p1, p2, p3, tCenter, cubicCenter, tRight, pRight, fMaxErrorX, fMaxErrorY, iSubDivLeft - 1);
}

void wdCurve1D::ClampTangents()
{
  if (m_ControlPoints.GetCount() < 2)
    return;

  for (wdUInt32 i = 1; i < m_ControlPoints.GetCount() - 1; ++i)
  {
    auto& tCP = m_ControlPoints[i];
    const auto& pCP = m_ControlPoints[i - 1];
    const auto& nCP = m_ControlPoints[i + 1];

    wdVec2d lpt = tCP.m_Position + wdVec2d(tCP.m_LeftTangent.x, tCP.m_LeftTangent.y);
    wdVec2d rpt = tCP.m_Position + wdVec2d(tCP.m_RightTangent.x, tCP.m_RightTangent.y);

    lpt.x = wdMath::Clamp(lpt.x, pCP.m_Position.x, tCP.m_Position.x);
    rpt.x = wdMath::Clamp(rpt.x, tCP.m_Position.x, nCP.m_Position.x);

    const wdVec2d tangentL = lpt - tCP.m_Position;
    const wdVec2d tangentR = rpt - tCP.m_Position;

    tCP.m_LeftTangent.Set((float)tangentL.x, (float)tangentL.y);
    tCP.m_RightTangent.Set((float)tangentR.x, (float)tangentR.y);
  }

  // first CP
  {
    auto& tCP = m_ControlPoints[0];
    const auto& nCP = m_ControlPoints[1];

    wdVec2d rpt = tCP.m_Position + wdVec2d(tCP.m_RightTangent.x, tCP.m_RightTangent.y);
    rpt.x = wdMath::Clamp(rpt.x, tCP.m_Position.x, nCP.m_Position.x);

    const wdVec2d tangentR = rpt - tCP.m_Position;
    tCP.m_RightTangent.Set((float)tangentR.x, (float)tangentR.y);
  }

  // last CP
  {
    auto& tCP = m_ControlPoints[m_ControlPoints.GetCount() - 1];
    const auto& pCP = m_ControlPoints[m_ControlPoints.GetCount() - 2];

    wdVec2d lpt = tCP.m_Position + wdVec2d(tCP.m_LeftTangent.x, tCP.m_LeftTangent.y);
    lpt.x = wdMath::Clamp(lpt.x, pCP.m_Position.x, tCP.m_Position.x);

    const wdVec2d tangentL = lpt - tCP.m_Position;
    tCP.m_LeftTangent.Set((float)tangentL.x, (float)tangentL.y);
  }
}

void wdCurve1D::ApplyTangentModes()
{
  if (m_ControlPoints.GetCount() < 2)
    return;

  for (wdUInt32 i = 1; i < m_ControlPoints.GetCount() - 1; ++i)
  {
    const auto& cp = m_ControlPoints[i];

    WD_ASSERT_DEBUG(cp.m_Position.x >= m_ControlPoints[i - 1].m_Position.x, "Curve control points are not sorted. Call SortControlPoints() before CreateLinearApproximation().");
    WD_ASSERT_DEBUG(m_ControlPoints[i + 1].m_Position.x >= cp.m_Position.x, "Curve control points are not sorted. Call SortControlPoints() before CreateLinearApproximation().");

    if (cp.m_TangentModeLeft == wdCurveTangentMode::FixedLength)
      MakeFixedLengthTangentLeft(i);
    else if (cp.m_TangentModeLeft == wdCurveTangentMode::Linear)
      MakeLinearTangentLeft(i);
    else if (cp.m_TangentModeLeft == wdCurveTangentMode::Auto)
      MakeAutoTangentLeft(i);

    if (cp.m_TangentModeRight == wdCurveTangentMode::FixedLength)
      MakeFixedLengthTangentRight(i);
    else if (cp.m_TangentModeRight == wdCurveTangentMode::Linear)
      MakeLinearTangentRight(i);
    else if (cp.m_TangentModeRight == wdCurveTangentMode::Auto)
      MakeAutoTangentRight(i);
  }

  // first CP
  {
    const wdUInt32 i = 0;
    const auto& cp = m_ControlPoints[i];

    if (cp.m_TangentModeRight == wdCurveTangentMode::FixedLength)
      MakeFixedLengthTangentRight(i);
    else if (cp.m_TangentModeRight == wdCurveTangentMode::Linear)
      MakeLinearTangentRight(i);
    else if (cp.m_TangentModeRight == wdCurveTangentMode::Auto)
      MakeLinearTangentRight(i); // note: first point will always be linear in auto mode
  }

  // last CP
  {
    const wdUInt32 i = m_ControlPoints.GetCount() - 1;
    const auto& cp = m_ControlPoints[i];

    if (cp.m_TangentModeLeft == wdCurveTangentMode::FixedLength)
      MakeFixedLengthTangentLeft(i);
    else if (cp.m_TangentModeLeft == wdCurveTangentMode::Linear)
      MakeLinearTangentLeft(i);
    else if (cp.m_TangentModeLeft == wdCurveTangentMode::Auto)
      MakeLinearTangentLeft(i); // note: last point will always be linear in auto mode
  }
}

void wdCurve1D::MakeFixedLengthTangentLeft(wdUInt32 uiCpIdx)
{
  auto& tCP = m_ControlPoints[uiCpIdx];
  const auto& pCP = m_ControlPoints[uiCpIdx - 1];

  const double lengthL = (pCP.m_Position.x - tCP.m_Position.x) * 0.3333333333;

  if (lengthL >= -0.0000001)
  {
    tCP.m_LeftTangent.SetZero();
  }
  else
  {
    const double tLen = wdMath::Min((double)tCP.m_LeftTangent.x, -0.001);

    const double fNormL = lengthL / tLen;
    tCP.m_LeftTangent.x = (float)lengthL;
    tCP.m_LeftTangent.y *= (float)fNormL;
  }
}

void wdCurve1D::MakeFixedLengthTangentRight(wdUInt32 uiCpIdx)
{
  auto& tCP = m_ControlPoints[uiCpIdx];
  const auto& nCP = m_ControlPoints[uiCpIdx + 1];

  const double lengthR = (nCP.m_Position.x - tCP.m_Position.x) * 0.3333333333;

  if (lengthR <= 0.0000001)
  {
    tCP.m_RightTangent.SetZero();
  }
  else
  {
    const double tLen = wdMath::Max((double)tCP.m_RightTangent.x, 0.001);

    const double fNormR = lengthR / tLen;
    tCP.m_RightTangent.x = (float)lengthR;
    tCP.m_RightTangent.y *= (float)fNormR;
  }
}

void wdCurve1D::MakeLinearTangentLeft(wdUInt32 uiCpIdx)
{
  auto& tCP = m_ControlPoints[uiCpIdx];
  const auto& pCP = m_ControlPoints[uiCpIdx - 1];

  const wdVec2d tangent = (pCP.m_Position - tCP.m_Position) * 0.3333333333;
  tCP.m_LeftTangent.Set((float)tangent.x, (float)tangent.y);
}

void wdCurve1D::MakeLinearTangentRight(wdUInt32 uiCpIdx)
{
  auto& tCP = m_ControlPoints[uiCpIdx];
  const auto& nCP = m_ControlPoints[uiCpIdx + 1];

  const wdVec2d tangent = (nCP.m_Position - tCP.m_Position) * 0.3333333333;
  tCP.m_RightTangent.Set((float)tangent.x, (float)tangent.y);
}

void wdCurve1D::MakeAutoTangentLeft(wdUInt32 uiCpIdx)
{
  auto& tCP = m_ControlPoints[uiCpIdx];
  const auto& pCP = m_ControlPoints[uiCpIdx - 1];
  const auto& nCP = m_ControlPoints[uiCpIdx + 1];

  const double len = (nCP.m_Position.x - pCP.m_Position.x);
  if (len <= 0)
    return;

  const double fLerpFactor = (tCP.m_Position.x - pCP.m_Position.x) / len;

  const wdVec2d dirP = (tCP.m_Position - pCP.m_Position) * 0.3333333333;
  const wdVec2d dirN = (nCP.m_Position - tCP.m_Position) * 0.3333333333;

  const wdVec2d tangent = wdMath::Lerp(dirP, dirN, fLerpFactor);

  tCP.m_LeftTangent.Set(-(float)tangent.x, -(float)tangent.y);
}

void wdCurve1D::MakeAutoTangentRight(wdUInt32 uiCpIdx)
{
  auto& tCP = m_ControlPoints[uiCpIdx];
  const auto& pCP = m_ControlPoints[uiCpIdx - 1];
  const auto& nCP = m_ControlPoints[uiCpIdx + 1];

  const double len = (nCP.m_Position.x - pCP.m_Position.x);
  if (len <= 0)
    return;

  const double fLerpFactor = (tCP.m_Position.x - pCP.m_Position.x) / len;

  const wdVec2d dirP = (tCP.m_Position - pCP.m_Position) * 0.3333333333;
  const wdVec2d dirN = (nCP.m_Position - tCP.m_Position) * 0.3333333333;

  const wdVec2d tangent = wdMath::Lerp(dirP, dirN, fLerpFactor);

  tCP.m_RightTangent.Set((float)tangent.x, (float)tangent.y);
}

WD_STATICLINK_FILE(Foundation, Foundation_Tracks_Implementation_Curve1D);
