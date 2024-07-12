#pragma once

#include <Foundation/Math/Mat4.h>

template <typename Type>
NS_FORCE_INLINE nsPlaneTemplate<Type>::nsPlaneTemplate()
{
#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  const Type TypeNaN = nsMath::NaN<Type>();
  m_vNormal.Set(TypeNaN);
  m_fNegDistance = TypeNaN;
#endif
}

template <typename Type>
nsPlaneTemplate<Type> nsPlaneTemplate<Type>::MakeInvalid()
{
  nsPlaneTemplate<Type> res;
  res.m_vNormal.Set(0);
  res.m_fNegDistance = 0;
  return res;
}

template <typename Type>
nsPlaneTemplate<Type> nsPlaneTemplate<Type>::MakeFromNormalAndPoint(const nsVec3Template<Type>& vNormal, const nsVec3Template<Type>& vPointOnPlane)
{
  NS_ASSERT_DEV(vNormal.IsNormalized(), "Normal must be normalized.");

  nsPlaneTemplate<Type> res;
  res.m_vNormal = vNormal;
  res.m_fNegDistance = -vNormal.Dot(vPointOnPlane);
  return res;
}

template <typename Type>
nsPlaneTemplate<Type> nsPlaneTemplate<Type>::MakeFromPoints(const nsVec3Template<Type>& v1, const nsVec3Template<Type>& v2, const nsVec3Template<Type>& v3)
{
  nsPlaneTemplate<Type> res;
  NS_VERIFY(res.m_vNormal.CalculateNormal(v1, v2, v3).Succeeded(), "The 3 provided points do not form a plane");

  res.m_fNegDistance = -res.m_vNormal.Dot(v1);
  return res;
}

template <typename Type>
nsVec4Template<Type> nsPlaneTemplate<Type>::GetAsVec4() const
{
  return nsVec4(m_vNormal.x, m_vNormal.y, m_vNormal.z, m_fNegDistance);
}

template <typename Type>
nsResult nsPlaneTemplate<Type>::SetFromPoints(const nsVec3Template<Type>& v1, const nsVec3Template<Type>& v2, const nsVec3Template<Type>& v3)
{
  if (m_vNormal.CalculateNormal(v1, v2, v3) == NS_FAILURE)
    return NS_FAILURE;

  m_fNegDistance = -m_vNormal.Dot(v1);
  return NS_SUCCESS;
}

template <typename Type>
nsResult nsPlaneTemplate<Type>::SetFromPoints(const nsVec3Template<Type>* const pVertices)
{
  if (m_vNormal.CalculateNormal(pVertices[0], pVertices[1], pVertices[2]) == NS_FAILURE)
    return NS_FAILURE;

  m_fNegDistance = -m_vNormal.Dot(pVertices[0]);
  return NS_SUCCESS;
}

template <typename Type>
nsResult nsPlaneTemplate<Type>::SetFromDirections(const nsVec3Template<Type>& vTangent1, const nsVec3Template<Type>& vTangent2, const nsVec3Template<Type>& vPointOnPlane)
{
  nsVec3Template<Type> vNormal = vTangent1.CrossRH(vTangent2);
  nsResult res = vNormal.NormalizeIfNotZero();

  m_vNormal = vNormal;
  m_fNegDistance = -vNormal.Dot(vPointOnPlane);
  return res;
}

template <typename Type>
void nsPlaneTemplate<Type>::Transform(const nsMat3Template<Type>& m)
{
  nsVec3Template<Type> vPointOnPlane = m_vNormal * -m_fNegDistance;

  // Transform the normal
  nsVec3Template<Type> vTransformedNormal = m.TransformDirection(m_vNormal);

  // Normalize the normal vector
  const bool normalizeSucceeded = vTransformedNormal.NormalizeIfNotZero().Succeeded();
  NS_ASSERT_DEBUG(normalizeSucceeded, "");
  NS_IGNORE_UNUSED(normalizeSucceeded);

  // If the plane's distance is already infinite, there won't be any meaningful change
  // to it as a result of the transformation.
  if (!nsMath::IsFinite(m_fNegDistance))
  {
    m_vNormal = vTransformedNormal;
  }
  else
  {
    *this = nsPlane::MakeFromNormalAndPoint(vTransformedNormal, m * vPointOnPlane);
  }
}

template <typename Type>
void nsPlaneTemplate<Type>::Transform(const nsMat4Template<Type>& m)
{
  nsVec3Template<Type> vPointOnPlane = m_vNormal * -m_fNegDistance;

  // Transform the normal
  nsVec3Template<Type> vTransformedNormal = m.TransformDirection(m_vNormal);

  // Normalize the normal vector
  const bool normalizeSucceeded = vTransformedNormal.NormalizeIfNotZero().Succeeded();
  NS_ASSERT_DEBUG(normalizeSucceeded, "");
  NS_IGNORE_UNUSED(normalizeSucceeded);

  // If the plane's distance is already infinite, there won't be any meaningful change
  // to it as a result of the transformation.
  if (!nsMath::IsFinite(m_fNegDistance))
  {
    m_vNormal = vTransformedNormal;
  }
  else
  {
    *this = nsPlane::MakeFromNormalAndPoint(vTransformedNormal, m * vPointOnPlane);
  }
}

template <typename Type>
NS_FORCE_INLINE void nsPlaneTemplate<Type>::Flip()
{
  m_fNegDistance = -m_fNegDistance;
  m_vNormal = -m_vNormal;
}

template <typename Type>
NS_FORCE_INLINE Type nsPlaneTemplate<Type>::GetDistanceTo(const nsVec3Template<Type>& vPoint) const
{
  return (m_vNormal.Dot(vPoint) + m_fNegDistance);
}

template <typename Type>
NS_FORCE_INLINE nsPositionOnPlane::Enum nsPlaneTemplate<Type>::GetPointPosition(const nsVec3Template<Type>& vPoint) const
{
  return (m_vNormal.Dot(vPoint) < -m_fNegDistance ? nsPositionOnPlane::Back : nsPositionOnPlane::Front);
}

template <typename Type>
nsPositionOnPlane::Enum nsPlaneTemplate<Type>::GetPointPosition(const nsVec3Template<Type>& vPoint, Type fPlaneHalfWidth) const
{
  const Type f = m_vNormal.Dot(vPoint);

  if (f + fPlaneHalfWidth < -m_fNegDistance)
    return nsPositionOnPlane::Back;

  if (f - fPlaneHalfWidth > -m_fNegDistance)
    return nsPositionOnPlane::Front;

  return nsPositionOnPlane::OnPlane;
}

template <typename Type>
NS_FORCE_INLINE const nsVec3Template<Type> nsPlaneTemplate<Type>::ProjectOntoPlane(const nsVec3Template<Type>& vPoint) const
{
  return vPoint - m_vNormal * (m_vNormal.Dot(vPoint) + m_fNegDistance);
}

template <typename Type>
NS_FORCE_INLINE const nsVec3Template<Type> nsPlaneTemplate<Type>::Mirror(const nsVec3Template<Type>& vPoint) const
{
  return vPoint - (Type)2 * GetDistanceTo(vPoint) * m_vNormal;
}

template <typename Type>
const nsVec3Template<Type> nsPlaneTemplate<Type>::GetCoplanarDirection(const nsVec3Template<Type>& vDirection) const
{
  nsVec3Template<Type> res = vDirection;
  res.MakeOrthogonalTo(m_vNormal);
  return res;
}

template <typename Type>
bool nsPlaneTemplate<Type>::IsIdentical(const nsPlaneTemplate& rhs) const
{
  return m_vNormal.IsIdentical(rhs.m_vNormal) && m_fNegDistance == rhs.m_fNegDistance;
}

template <typename Type>
bool nsPlaneTemplate<Type>::IsEqual(const nsPlaneTemplate& rhs, Type fEpsilon) const
{
  return m_vNormal.IsEqual(rhs.m_vNormal, fEpsilon) && nsMath::IsEqual(m_fNegDistance, rhs.m_fNegDistance, fEpsilon);
}

template <typename Type>
NS_ALWAYS_INLINE bool operator==(const nsPlaneTemplate<Type>& lhs, const nsPlaneTemplate<Type>& rhs)
{
  return lhs.IsIdentical(rhs);
}

template <typename Type>
NS_ALWAYS_INLINE bool operator!=(const nsPlaneTemplate<Type>& lhs, const nsPlaneTemplate<Type>& rhs)
{
  return !lhs.IsIdentical(rhs);
}

template <typename Type>
bool nsPlaneTemplate<Type>::FlipIfNecessary(const nsVec3Template<Type>& vPoint, bool bPlaneShouldFacePoint)
{
  if ((GetPointPosition(vPoint) == nsPositionOnPlane::Front) != bPlaneShouldFacePoint)
  {
    Flip();
    return true;
  }

  return false;
}

template <typename Type>
bool nsPlaneTemplate<Type>::IsValid() const
{
  return !IsNaN() && m_vNormal.IsNormalized(nsMath::DefaultEpsilon<Type>());
}

template <typename Type>
bool nsPlaneTemplate<Type>::IsNaN() const
{
  return nsMath::IsNaN(m_fNegDistance) || m_vNormal.IsNaN();
}

template <typename Type>
bool nsPlaneTemplate<Type>::IsFinite() const
{
  return m_vNormal.IsValid() && nsMath::IsFinite(m_fNegDistance);
}

/*! The given vertices can be partially equal or lie on the same line. The algorithm will try to find 3 vertices, that
  form a plane, and deduce the normal from them. This algorithm is much slower, than all the other methods, so only
  use it, when you know, that your data can contain such configurations. */
template <typename Type>
nsResult nsPlaneTemplate<Type>::SetFromPoints(const nsVec3Template<Type>* const pVertices, nsUInt32 uiMaxVertices)
{
  nsInt32 iPoints[3];

  if (FindSupportPoints(pVertices, uiMaxVertices, iPoints[0], iPoints[1], iPoints[2]) == NS_FAILURE)
  {
    SetFromPoints(pVertices).IgnoreResult();
    return NS_FAILURE;
  }

  SetFromPoints(pVertices[iPoints[0]], pVertices[iPoints[1]], pVertices[iPoints[2]]).IgnoreResult();
  return NS_SUCCESS;
}

template <typename Type>
nsResult nsPlaneTemplate<Type>::FindSupportPoints(const nsVec3Template<Type>* const pVertices, int iMaxVertices, int& out_i1, int& out_i2, int& out_i3)
{
  const nsVec3Template<Type> v1 = pVertices[0];

  bool bFoundSecond = false;

  int i = 1;
  while (i < iMaxVertices)
  {
    if (pVertices[i].IsEqual(v1, 0.001f) == false)
    {
      bFoundSecond = true;
      break;
    }

    ++i;
  }

  if (!bFoundSecond)
    return NS_FAILURE;

  const nsVec3Template<Type> v2 = pVertices[i];

  const nsVec3Template<Type> vDir1 = (v1 - v2).GetNormalized();

  out_i1 = 0;
  out_i2 = i;

  ++i;

  while (i < iMaxVertices)
  {
    // check for inequality, then for non-collinearity
    if ((pVertices[i].IsEqual(v2, 0.001f) == false) && (nsMath::Abs((pVertices[i] - v2).GetNormalized().Dot(vDir1)) < (Type)0.999))
    {
      out_i3 = i;
      return NS_SUCCESS;
    }

    ++i;
  }

  return NS_FAILURE;
}

template <typename Type>
nsPositionOnPlane::Enum nsPlaneTemplate<Type>::GetObjectPosition(const nsVec3Template<Type>* const pPoints, nsUInt32 uiVertices) const
{
  bool bFront = false;
  bool bBack = false;

  for (nsUInt32 i = 0; i < uiVertices; ++i)
  {
    switch (GetPointPosition(pPoints[i]))
    {
      case nsPositionOnPlane::Front:
        if (bBack)
          return (nsPositionOnPlane::Spanning);
        bFront = true;
        break;
      case nsPositionOnPlane::Back:
        if (bFront)
          return (nsPositionOnPlane::Spanning);
        bBack = true;
        break;

      default:
        break;
    }
  }

  return (bFront ? nsPositionOnPlane::Front : nsPositionOnPlane::Back);
}

template <typename Type>
nsPositionOnPlane::Enum nsPlaneTemplate<Type>::GetObjectPosition(const nsVec3Template<Type>* const pPoints, nsUInt32 uiVertices, Type fPlaneHalfWidth) const
{
  bool bFront = false;
  bool bBack = false;

  for (nsUInt32 i = 0; i < uiVertices; ++i)
  {
    switch (GetPointPosition(pPoints[i], fPlaneHalfWidth))
    {
      case nsPositionOnPlane::Front:
        if (bBack)
          return (nsPositionOnPlane::Spanning);
        bFront = true;
        break;
      case nsPositionOnPlane::Back:
        if (bFront)
          return (nsPositionOnPlane::Spanning);
        bBack = true;
        break;

      default:
        break;
    }
  }

  if (bFront)
    return (nsPositionOnPlane::Front);
  if (bBack)
    return (nsPositionOnPlane::Back);

  return (nsPositionOnPlane::OnPlane);
}

template <typename Type>
bool nsPlaneTemplate<Type>::GetRayIntersection(const nsVec3Template<Type>& vRayStartPos, const nsVec3Template<Type>& vRayDir, Type* out_pIntersectionDistance, nsVec3Template<Type>* out_pIntersection) const
{
  NS_ASSERT_DEBUG(vRayStartPos.IsValid(), "Ray start position must be valid.");
  NS_ASSERT_DEBUG(vRayDir.IsValid(), "Ray direction must be valid.");

  const Type fPlaneSide = GetDistanceTo(vRayStartPos);
  const Type fCosAlpha = m_vNormal.Dot(vRayDir);

  if (fCosAlpha == 0)                                      // ray is orthogonal to plane
    return false;

  if (nsMath::Sign(fPlaneSide) == nsMath::Sign(fCosAlpha)) // ray points away from the plane
    return false;

  const Type fTime = -fPlaneSide / fCosAlpha;

  if (out_pIntersectionDistance)
    *out_pIntersectionDistance = fTime;

  if (out_pIntersection)
    *out_pIntersection = vRayStartPos + fTime * vRayDir;

  return true;
}

template <typename Type>
bool nsPlaneTemplate<Type>::GetRayIntersectionBiDirectional(const nsVec3Template<Type>& vRayStartPos, const nsVec3Template<Type>& vRayDir, Type* out_pIntersectionDistance, nsVec3Template<Type>* out_pIntersection) const
{
  NS_ASSERT_DEBUG(vRayStartPos.IsValid(), "Ray start position must be valid.");
  NS_ASSERT_DEBUG(vRayDir.IsValid(), "Ray direction must be valid.");

  const Type fPlaneSide = GetDistanceTo(vRayStartPos);
  const Type fCosAlpha = m_vNormal.Dot(vRayDir);

  if (fCosAlpha == 0) // ray is orthogonal to plane
    return false;

  const Type fTime = -fPlaneSide / fCosAlpha;

  if (out_pIntersectionDistance)
    *out_pIntersectionDistance = fTime;

  if (out_pIntersection)
    *out_pIntersection = vRayStartPos + fTime * vRayDir;

  return true;
}

template <typename Type>
bool nsPlaneTemplate<Type>::GetLineSegmentIntersection(const nsVec3Template<Type>& vLineStartPos, const nsVec3Template<Type>& vLineEndPos, Type* out_pHitFraction, nsVec3Template<Type>* out_pIntersection) const
{
  Type fTime = 0;

  if (!GetRayIntersection(vLineStartPos, vLineEndPos - vLineStartPos, &fTime, out_pIntersection))
    return false;

  if (out_pHitFraction)
    *out_pHitFraction = fTime;

  return (fTime <= 1);
}

template <typename Type>
Type nsPlaneTemplate<Type>::GetMinimumDistanceTo(const nsVec3Template<Type>* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride /* = sizeof (nsVec3Template<Type>) */) const
{
  NS_ASSERT_DEBUG(pPoints != nullptr, "Array may not be nullptr.");
  NS_ASSERT_DEBUG(uiStride >= sizeof(nsVec3Template<Type>), "Stride must be at least sizeof(nsVec3Template) to not have overlapping data.");
  NS_ASSERT_DEBUG(uiNumPoints >= 1, "Array must contain at least one point.");

  Type fMinDist = nsMath::MaxValue<Type>();

  const nsVec3Template<Type>* pCurPoint = pPoints;

  for (nsUInt32 i = 0; i < uiNumPoints; ++i)
  {
    fMinDist = nsMath::Min(m_vNormal.Dot(*pCurPoint), fMinDist);

    pCurPoint = nsMemoryUtils::AddByteOffset(pCurPoint, uiStride);
  }

  return fMinDist + m_fNegDistance;
}

template <typename Type>
void nsPlaneTemplate<Type>::GetMinMaxDistanceTo(Type& out_fMin, Type& out_fMax, const nsVec3Template<Type>* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride /* = sizeof (nsVec3Template<Type>) */) const
{
  NS_ASSERT_DEBUG(pPoints != nullptr, "Array may not be nullptr.");
  NS_ASSERT_DEBUG(uiStride >= sizeof(nsVec3Template<Type>), "Stride must be at least sizeof(nsVec3Template) to not have overlapping data.");
  NS_ASSERT_DEBUG(uiNumPoints >= 1, "Array must contain at least one point.");

  out_fMin = nsMath::MaxValue<Type>();
  out_fMax = -nsMath::MaxValue<Type>();

  const nsVec3Template<Type>* pCurPoint = pPoints;

  for (nsUInt32 i = 0; i < uiNumPoints; ++i)
  {
    const Type f = m_vNormal.Dot(*pCurPoint);

    out_fMin = nsMath::Min(f, out_fMin);
    out_fMax = nsMath::Max(f, out_fMax);

    pCurPoint = nsMemoryUtils::AddByteOffset(pCurPoint, uiStride);
  }

  out_fMin += m_fNegDistance;
  out_fMax += m_fNegDistance;
}

template <typename Type>
nsResult nsPlaneTemplate<Type>::GetPlanesIntersectionPoint(const nsPlaneTemplate& p0, const nsPlaneTemplate& p1, const nsPlaneTemplate& p2, nsVec3Template<Type>& out_vResult)
{
  const nsVec3Template<Type> n1(p0.m_vNormal);
  const nsVec3Template<Type> n2(p1.m_vNormal);
  const nsVec3Template<Type> n3(p2.m_vNormal);

  const Type det = n1.Dot(n2.CrossRH(n3));

  if (nsMath::IsZero<Type>(det, nsMath::LargeEpsilon<Type>()))
    return NS_FAILURE;

  out_vResult = (-p0.m_fNegDistance * n2.CrossRH(n3) + -p1.m_fNegDistance * n3.CrossRH(n1) + -p2.m_fNegDistance * n1.CrossRH(n2)) / det;

  return NS_SUCCESS;
}

#include <Foundation/Math/Implementation/AllClasses_inl.h>
