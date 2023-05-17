#include <Foundation/FoundationPCH.h>

#include <Foundation/Math/Intersection.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Math/Plane.h>

bool wdIntersectionUtils::RayPolygonIntersection(const wdVec3& vRayStartPos, const wdVec3& vRayDir, const wdVec3* pPolygonVertices,
  wdUInt32 uiNumVertices, float* out_pIntersectionTime, wdVec3* out_pIntersectionPoint, wdUInt32 uiVertexStride)
{
  WD_ASSERT_DEBUG(uiNumVertices >= 3, "A polygon must have at least three vertices.");
  WD_ASSERT_DEBUG(uiVertexStride >= sizeof(wdVec3), "The vertex stride is invalid.");

  wdPlane p(*pPolygonVertices, *wdMemoryUtils::AddByteOffset(pPolygonVertices, uiVertexStride),
    *wdMemoryUtils::AddByteOffset(pPolygonVertices, uiVertexStride * 2));

  WD_ASSERT_DEBUG(p.IsValid(), "The given polygon's plane is invalid (computed from the first three vertices only).");

  wdVec3 vIntersection;

  if (!p.GetRayIntersection(vRayStartPos, vRayDir, out_pIntersectionTime, &vIntersection))
    return false;

  if (out_pIntersectionPoint)
    *out_pIntersectionPoint = vIntersection;

  // start with the last point as the 'wrap around' position
  wdVec3 vPrevPoint = *wdMemoryUtils::AddByteOffset(pPolygonVertices, wdMath::SafeMultiply32(uiVertexStride, (uiNumVertices - 1)));

  // for each polygon edge
  for (wdUInt32 i = 0; i < uiNumVertices; ++i)
  {
    const wdVec3 vThisPoint = *wdMemoryUtils::AddByteOffset(pPolygonVertices, wdMath::SafeMultiply32(uiVertexStride, i));

    const wdPlane EdgePlane(vThisPoint, vPrevPoint, vPrevPoint + p.m_vNormal);

    // if the intersection point is outside of any of the edge planes, it is not inside the (convex) polygon
    if (EdgePlane.GetPointPosition(vIntersection) == wdPositionOnPlane::Back)
      return false;

    vPrevPoint = vThisPoint;
  }

  // inside all edge planes -> inside the polygon -> there is a proper intersection
  return true;
}

wdVec3 wdIntersectionUtils::ClosestPoint_PointLineSegment(
  const wdVec3& vStartPoint, const wdVec3& vLineSegmentPos0, const wdVec3& vLineSegmentPos1, float* out_pFractionAlongSegment)
{
  const wdVec3 vLineDir = vLineSegmentPos1 - vLineSegmentPos0;
  const wdVec3 vToStartPoint = vStartPoint - vLineSegmentPos0;

  const float fProjected = vToStartPoint.Dot(vLineDir);

  float fPosAlongSegment;

  // clamp t to [0; 1] range, and only do the division etc. when necessary
  if (fProjected <= 0.0f)
  {
    fPosAlongSegment = 0.0f;
  }
  else
  {
    const float fSquaredDirLen = vLineDir.GetLengthSquared();

    if (fProjected >= fSquaredDirLen)
    {
      fPosAlongSegment = 1.0f;
    }
    else
    {
      fPosAlongSegment = fProjected / fSquaredDirLen;
    }
  }

  if (out_pFractionAlongSegment)
    *out_pFractionAlongSegment = fPosAlongSegment;

  return vLineSegmentPos0 + fPosAlongSegment * vLineDir;
}

bool wdIntersectionUtils::Ray2DLine2D(const wdVec2& vRayStartPos, const wdVec2& vRayDir, const wdVec2& vLineSegmentPos0,
  const wdVec2& vLineSegmentPos1, float* out_pIntersectionTime, wdVec2* out_pIntersectionPoint)
{
  const wdVec2 vLineDir = vLineSegmentPos1 - vLineSegmentPos0;

  // 2D Plane
  const wdVec2 vPlaneNormal = vLineDir.GetOrthogonalVector();
  const float fPlaneNegDist = -vPlaneNormal.Dot(vLineSegmentPos0);

  wdVec2 vIntersection;
  float fIntersectionTime;

  // 2D Plane ray intersection test
  {
    const float fPlaneSide = vPlaneNormal.Dot(vRayStartPos) + fPlaneNegDist;
    const float fCosAlpha = vPlaneNormal.Dot(vRayDir);

    if (fCosAlpha == 0) // ray is orthogonal to plane
      return false;

    if (wdMath::Sign(fPlaneSide) == wdMath::Sign(fCosAlpha)) // ray points away from the plane
      return false;

    fIntersectionTime = -fPlaneSide / fCosAlpha;

    vIntersection = vRayStartPos + fIntersectionTime * vRayDir;
  }

  const wdVec2 vToIntersection = vIntersection - vLineSegmentPos0;

  const float fProjected = vLineDir.Dot(vToIntersection);

  if (fProjected < 0.0f)
    return false;

  if (fProjected > vLineDir.GetLengthSquared())
    return false;

  if (out_pIntersectionTime)
    *out_pIntersectionTime = fIntersectionTime;

  if (out_pIntersectionPoint)
    *out_pIntersectionPoint = vIntersection;

  return true;
}

bool wdIntersectionUtils::IsPointOnLine(const wdVec3& vLineStart, const wdVec3& vLineEnd, const wdVec3& vPoint, float fMaxDist /*= 0.01f*/)
{
  const wdVec3 vClosest = ClosestPoint_PointLineSegment(vPoint, vLineStart, vLineEnd);
  const float fClosestDistSqr = (vClosest - vPoint).GetLengthSquared();

  return (fClosestDistSqr <= fMaxDist * fMaxDist);
}

WD_STATICLINK_FILE(Foundation, Foundation_Math_Implementation_Intersection);
