#pragma once

#include <Foundation/Math/Frustum.h>

NS_FORCE_INLINE bool nsFrustum::Overlaps(const nsSimdBBox& object) const
{
  nsSimdVec4f center2 = object.m_Min + object.m_Max;
  nsSimdVec4f extents = object.GetExtents();

  // We're working with center and extents scaled by two - but the plane equation still works out
  // correctly since we set W = 2 here.
  center2.SetW(nsSimdFloat(2.0f));
  extents.SetW(nsSimdFloat::MakeZero());

#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
  nsSimdVec4f minusZero;
  minusZero.Set(-0.0f);
#endif

  for (nsUInt32 plane = 0; plane < PLANE_COUNT; ++plane)
  {
    nsSimdVec4f equation;
    equation.Load<4>(m_Planes[plane].m_vNormal.GetData());

    // Change signs of extents to match signs of plane normal
    nsSimdVec4f maxExtent;

#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
    // Specialized for SSE - this is faster than FlipSign for multiple calls since we can preload the constant -0.0f
    maxExtent.m_v = _mm_xor_ps(extents.m_v, _mm_andnot_ps(equation.m_v, minusZero.m_v));
#else
    maxExtent = extents.FlipSign(equation >= nsSimdVec4f::MakeZero());
#endif

    // Compute AABB corner which is the furthest along the plane normal
    const nsSimdVec4f offset = center2 + maxExtent;

    if (equation.Dot<4>(offset) > nsSimdFloat::MakeZero())
    {
      // outside
      return false;
    }
  }

  // inside or intersect
  return true;
}

NS_FORCE_INLINE bool nsFrustum::Overlaps(const nsSimdBSphere& object) const
{
  // Calculate the minimum distance of the given sphere's center to all frustum planes.
  nsSimdFloat xSphere = object.m_CenterAndRadius.x();
  nsSimdFloat ySphere = object.m_CenterAndRadius.y();
  nsSimdFloat zSphere = object.m_CenterAndRadius.z();
  nsSimdVec4f radius;
  radius.Set(object.m_CenterAndRadius.w());

  nsSimdVec4f xPlanes0;
  nsSimdVec4f yPlanes0;
  nsSimdVec4f zPlanes0;
  nsSimdVec4f dPlanes0;

  xPlanes0.Load<4>(m_Planes[0].m_vNormal.GetData());
  yPlanes0.Load<4>(m_Planes[1].m_vNormal.GetData());
  zPlanes0.Load<4>(m_Planes[2].m_vNormal.GetData());
  dPlanes0.Load<4>(m_Planes[3].m_vNormal.GetData());

  {
    nsSimdMat4f tmp;
    tmp.SetRows(xPlanes0, yPlanes0, zPlanes0, dPlanes0);
    xPlanes0 = -tmp.m_col0;
    yPlanes0 = -tmp.m_col1;
    zPlanes0 = -tmp.m_col2;
    dPlanes0 = -tmp.m_col3;
  }

  nsSimdVec4f minDist0;
  minDist0 = dPlanes0 + xPlanes0 * xSphere;
  minDist0 += yPlanes0 * ySphere;
  minDist0 += zPlanes0 * zSphere;

  nsSimdVec4f xPlanes1;
  nsSimdVec4f yPlanes1;
  nsSimdVec4f zPlanes1;
  nsSimdVec4f dPlanes1;

  xPlanes1.Load<4>(m_Planes[4].m_vNormal.GetData());
  yPlanes1.Load<4>(m_Planes[5].m_vNormal.GetData());
  zPlanes1.Load<4>(m_Planes[5].m_vNormal.GetData());
  dPlanes1.Load<4>(m_Planes[5].m_vNormal.GetData());

  {
    nsSimdMat4f tmp;
    tmp.SetRows(xPlanes1, yPlanes1, zPlanes1, dPlanes1);
    xPlanes1 = -tmp.m_col0;
    yPlanes1 = -tmp.m_col1;
    zPlanes1 = -tmp.m_col2;
    dPlanes1 = -tmp.m_col3;
  }


  nsSimdVec4f minDist1 = dPlanes1 + xPlanes1 * xSphere;
  minDist1 += yPlanes1 * ySphere;
  minDist1 += zPlanes1 * zSphere;

  nsSimdVec4f minDist = minDist0.CompMin(minDist1);

  // Add the sphere radius to cater for the sphere's extents.
  minDist += radius;

  // If the distance is still less than zero, the sphere is completely "outside" of at least one plane.
  return (minDist < nsSimdVec4f::MakeZero()).NoneSet();
}
