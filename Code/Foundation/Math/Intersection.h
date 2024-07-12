#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Math/Vec3.h>

namespace nsIntersectionUtils
{
  /// \brief Checks whether a ray intersects with a polygon.
  ///
  /// \param vRayStartPos
  ///   The start position of the ray.
  /// \param vRayDir
  ///   The direction of the ray. This does not need to be normalized. Depending on its length, out_fIntersectionTime will be scaled differently.
  /// \param pPolygonVertices
  ///   Pointer to the first vertex of the polygon.
  /// \param uiNumVertices
  ///   The number of vertices in the polygon.
  /// \param out_fIntersectionTime
  ///   The 'time' at which the ray intersects the polygon. If \a vRayDir is normalized, this is the exact distance.
  ///   out_fIntersectionPoint == vRayStartPos + vRayDir * out_fIntersectionTime
  ///   This parameter is optional and may be set to nullptr.
  /// \param out_fIntersectionPoint
  ///   The point where the ray intersects the polygon.
  ///   out_fIntersectionPoint == vRayStartPos + vRayDir * out_fIntersectionTime
  ///   This parameter is optional and may be set to nullptr.
  /// \param uiVertexStride
  ///   The stride in bytes between each vertex in the pPolygonVertices array. If the array is tightly packed, this will equal sizeof(nsVec3), but it
  ///   can be larger, if the vertices are interleaved with other data.
  /// \return
  ///   True, if the ray intersects the polygon, false otherwise.
  NS_FOUNDATION_DLL bool RayPolygonIntersection(const nsVec3& vRayStartPos, const nsVec3& vRayDir, const nsVec3* pPolygonVertices,
    nsUInt32 uiNumVertices, float* out_pIntersectionTime = nullptr, nsVec3* out_pIntersectionPoint = nullptr,
    nsUInt32 uiVertexStride = sizeof(nsVec3)); // [tested]


  /// \brief Returns point on the line segment that is closest to \a vStartPoint. Optionally also returns the fraction along the segment, where that
  /// point is located.
  NS_FOUNDATION_DLL nsVec3 ClosestPoint_PointLineSegment(const nsVec3& vStartPoint, const nsVec3& vLineSegmentPos0, const nsVec3& vLineSegmentPos1,
    float* out_pFractionAlongSegment = nullptr); // [tested]

  /// \brief Computes the intersection point and time of the 2D ray with the 2D line segment. Returns true, if there is an intersection.
  NS_FOUNDATION_DLL bool Ray2DLine2D(const nsVec2& vRayStartPos, const nsVec2& vRayDir, const nsVec2& vLineSegmentPos0,
    const nsVec2& vLineSegmentPos1, float* out_pIntersectionTime = nullptr, nsVec2* out_pIntersectionPoint = nullptr); // [tested]

  /// \brief Tests whether a point is located on a line
  NS_FOUNDATION_DLL bool IsPointOnLine(const nsVec3& vLineStart, const nsVec3& vLineEnd, const nsVec3& vPoint, float fMaxDist = 0.01f);
} // namespace nsIntersectionUtils
