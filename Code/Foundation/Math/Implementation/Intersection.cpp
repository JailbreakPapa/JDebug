#include <Foundation/FoundationPCH.h>

#include <Foundation/Math/Intersection.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Math/Plane.h>

bool nsIntersectionUtils::RayPolygonIntersection(const nsVec3& vRayStartPos, const nsVec3& vRayDir, const nsVec3* pPolygonVertices,
  nsUInt32 uiNumVertices, float* out_pIntersectionTime, nsVec3* out_pIntersectionPoint, nsUInt32 uiVertexStride)
{
  NS_ASSERT_DEBUG(uiNumVertices >= 3, "A polygon must have at least three vertices.");
  NS_ASSERT_DEBUG(uiVertexStride >= sizeof(nsVec3), "The vertex stride is invalid.");

  nsPlane p = nsPlane::MakeFromPoints(*pPolygonVertices, *nsMemoryUtils::AddByteOffset(pPolygonVertices, uiVertexStride), *nsMemoryUtils::AddByteOffset(pPolygonVertices, uiVertexStride * 2));

  NS_ASSERT_DEBUG(p.IsValid(), "The given polygon's plane is invalid (computed from the first three vertices only).");

  nsVec3 vIntersection;

  if (!p.GetRayIntersection(vRayStartPos, vRayDir, out_pIntersectionTime, &vIntersection))
    return false;

  if (out_pIntersectionPoint)
    *out_pIntersectionPoint = vIntersection;

  // start with the last point as the 'wrap around' position
  nsVec3 vPrevPoint = *nsMemoryUtils::AddByteOffset(pPolygonVertices, nsMath::SafeMultiply32(uiVertexStride, (uiNumVertices - 1)));

  // for each polygon edge
  for (nsUInt32 i = 0; i < uiNumVertices; ++i)
  {
    const nsVec3 vThisPoint = *nsMemoryUtils::AddByteOffset(pPolygonVertices, nsMath::SafeMultiply32(uiVertexStride, i));

    nsPlane EdgePlane = nsPlane::MakeFromPoints(vThisPoint, vPrevPoint, vPrevPoint + p.m_vNormal);

    // if the intersection point is outside of any of the edge planes, it is not inside the (convex) polygon
    if (EdgePlane.GetPointPosition(vIntersection) == nsPositionOnPlane::Back)
      return false;

    vPrevPoint = vThisPoint;
  }

  // inside all edge planes -> inside the polygon -> there is a proper intersection
  return true;
}

nsVec3 nsIntersectionUtils::ClosestPoint_PointLineSegment(
  const nsVec3& vStartPoint, const nsVec3& vLineSegmentPos0, const nsVec3& vLineSegmentPos1, float* out_pFractionAlongSegment)
{
  const nsVec3 vLineDir = vLineSegmentPos1 - vLineSegmentPos0;
  const nsVec3 vToStartPoint = vStartPoint - vLineSegmentPos0;

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

bool nsIntersectionUtils::Ray2DLine2D(const nsVec2& vRayStartPos, const nsVec2& vRayDir, const nsVec2& vLineSegmentPos0,
  const nsVec2& vLineSegmentPos1, float* out_pIntersectionTime, nsVec2* out_pIntersectionPoint)
{
  const nsVec2 vLineDir = vLineSegmentPos1 - vLineSegmentPos0;

  // 2D Plane
  const nsVec2 vPlaneNormal = vLineDir.GetOrthogonalVector();
  const float fPlaneNegDist = -vPlaneNormal.Dot(vLineSegmentPos0);

  nsVec2 vIntersection;
  float fIntersectionTime;

  // 2D Plane ray intersection test
  {
    const float fPlaneSide = vPlaneNormal.Dot(vRayStartPos) + fPlaneNegDist;
    const float fCosAlpha = vPlaneNormal.Dot(vRayDir);

    if (fCosAlpha == 0)                                      // ray is orthogonal to plane
      return false;

    if (nsMath::Sign(fPlaneSide) == nsMath::Sign(fCosAlpha)) // ray points away from the plane
      return false;

    fIntersectionTime = -fPlaneSide / fCosAlpha;

    vIntersection = vRayStartPos + fIntersectionTime * vRayDir;
  }

  const nsVec2 vToIntersection = vIntersection - vLineSegmentPos0;

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

bool nsIntersectionUtils::IsPointOnLine(const nsVec3& vLineStart, const nsVec3& vLineEnd, const nsVec3& vPoint, float fMaxDist /*= 0.01f*/)
{
  const nsVec3 vClosest = ClosestPoint_PointLineSegment(vPoint, vLineStart, vLineEnd);
  const float fClosestDistSqr = (vClosest - vPoint).GetLengthSquared();

  return (fClosestDistSqr <= fMaxDist * fMaxDist);
}
