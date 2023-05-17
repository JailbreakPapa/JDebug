#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Math/Vec3.h>

namespace wdIntersectionUtils
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
  ///   The stride in bytes between each vertex in the pPolygonVertices array. If the array is tightly packed, this will equal sizeof(wdVec3), but it
  ///   can be larger, if the vertices are interleaved with other data.
  /// \return
  ///   True, if the ray intersects the polygon, false otherwise.
  WD_FOUNDATION_DLL bool RayPolygonIntersection(const wdVec3& vRayStartPos, const wdVec3& vRayDir, const wdVec3* pPolygonVertices,
    wdUInt32 uiNumVertices, float* out_pIntersectionTime = nullptr, wdVec3* out_pIntersectionPoint = nullptr,
    wdUInt32 uiVertexStride = sizeof(wdVec3)); // [tested]


  /// \brief Returns point on the line segment that is closest to \a vStartPoint. Optionally also returns the fraction along the segment, where that
  /// point is located.
  WD_FOUNDATION_DLL wdVec3 ClosestPoint_PointLineSegment(const wdVec3& vStartPoint, const wdVec3& vLineSegmentPos0, const wdVec3& vLineSegmentPos1,
    float* out_pFractionAlongSegment = nullptr); // [tested]

  /// \brief Computes the intersection point and time of the 2D ray with the 2D line segment. Returns true, if there is an intersection.
  WD_FOUNDATION_DLL bool Ray2DLine2D(const wdVec2& vRayStartPos, const wdVec2& vRayDir, const wdVec2& vLineSegmentPos0,
    const wdVec2& vLineSegmentPos1, float* out_pIntersectionTime = nullptr, wdVec2* out_pIntersectionPoint = nullptr); // [tested]

  /// \brief Tests whether a point is located on a line
  WD_FOUNDATION_DLL bool IsPointOnLine(const wdVec3& vLineStart, const wdVec3& vLineEnd, const wdVec3& vPoint, float fMaxDist = 0.01f);
} // namespace wdIntersectionUtils
