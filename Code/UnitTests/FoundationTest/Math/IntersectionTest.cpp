#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/Intersection.h>
#include <Foundation/Math/Mat4.h>

NS_CREATE_SIMPLE_TEST(Math, Intersection)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "RayPolygonIntersection")
  {
    for (nsUInt32 i = 0; i < 100; ++i)
    {
      nsMat4 m;
      m = nsMat4::MakeAxisRotation(nsVec3(i + 1.0f, i * 3.0f, i * 7.0f).GetNormalized(), nsAngle::MakeFromDegree((float)i));
      m.SetTranslationVector(nsVec3((float)i, i * 2.0f, i * 3.0f));

      nsVec3 Vertices[8] = {m.TransformPosition(nsVec3(-10, -10, 0)), nsVec3(-10, -10, 0), m.TransformPosition(nsVec3(10, -10, 0)),
        nsVec3(10, -10, 0), m.TransformPosition(nsVec3(10, 10, 0)), nsVec3(10, 10, 0), m.TransformPosition(nsVec3(-10, 10, 0)), nsVec3(-10, 10, 0)};

      for (float y = -14.5; y <= 14.5f; y += 2.0f)
      {
        for (float x = -14.5; x <= 14.5f; x += 2.0f)
        {
          const nsVec3 vRayDir = m.TransformDirection(nsVec3(x, y, -10.0f));
          const nsVec3 vRayStart = m.TransformPosition(nsVec3(x, y, 0.0f)) - vRayDir * 3.0f;

          const bool bIntersects = (x >= -10.0f && x <= 10.0f && y >= -10.0f && y <= 10.0f);

          float fIntersection;
          nsVec3 vIntersection;
          NS_TEST_BOOL(nsIntersectionUtils::RayPolygonIntersection(
                         vRayStart, vRayDir, Vertices, 4, &fIntersection, &vIntersection, sizeof(nsVec3) * 2) == bIntersects);

          if (bIntersects)
          {
            NS_TEST_FLOAT(fIntersection, 3.0f, 0.0001f);
            NS_TEST_VEC3(vIntersection, m.TransformPosition(nsVec3(x, y, 0.0f)), 0.0001f);
          }
        }
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ClosestPoint_PointLineSegment")
  {
    for (nsUInt32 i = 0; i < 100; ++i)
    {
      nsMat4 m;
      m = nsMat4::MakeAxisRotation(nsVec3(i + 1.0f, i * 3.0f, i * 7.0f).GetNormalized(), nsAngle::MakeFromDegree((float)i));
      m.SetTranslationVector(nsVec3((float)i, i * 2.0f, i * 3.0f));

      nsVec3 vSegment0 = m.TransformPosition(nsVec3(-10, 1, 2));
      nsVec3 vSegment1 = m.TransformPosition(nsVec3(10, 1, 2));

      for (float f = -20; f <= -10; f += 0.5f)
      {
        const nsVec3 vPos = m.TransformPosition(nsVec3(f, 10.0f, 20.0f));

        float fFraction = -1.0f;
        const nsVec3 vClosest = nsIntersectionUtils::ClosestPoint_PointLineSegment(vPos, vSegment0, vSegment1, &fFraction);

        NS_TEST_FLOAT(fFraction, 0.0f, 0.0001f);
        NS_TEST_VEC3(vClosest, vSegment0, 0.0001f);
      }

      for (float f = -10; f <= 10; f += 0.5f)
      {
        const nsVec3 vPos = m.TransformPosition(nsVec3(f, 10.0f, 20.0f));

        float fFraction = -1.0f;
        const nsVec3 vClosest = nsIntersectionUtils::ClosestPoint_PointLineSegment(vPos, vSegment0, vSegment1, &fFraction);

        NS_TEST_FLOAT(fFraction, (f + 10.0f) / 20.0f, 0.0001f);
        NS_TEST_VEC3(vClosest, m.TransformPosition(nsVec3(f, 1, 2)), 0.0001f);
      }

      for (float f = 10; f <= 20; f += 0.5f)
      {
        const nsVec3 vPos = m.TransformPosition(nsVec3(f, 10.0f, 20.0f));

        float fFraction = -1.0f;
        const nsVec3 vClosest = nsIntersectionUtils::ClosestPoint_PointLineSegment(vPos, vSegment0, vSegment1, &fFraction);

        NS_TEST_FLOAT(fFraction, 1.0f, 0.0001f);
        NS_TEST_VEC3(vClosest, vSegment1, 0.0001f);
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Ray2DLine2D")
  {
    for (nsUInt32 i = 0; i < 100; ++i)
    {
      nsMat4 m;
      m = nsMat4::MakeRotationZ(nsAngle::MakeFromDegree((float)i));
      m.SetTranslationVector(nsVec3((float)i, i * 2.0f, i * 3.0f));

      const nsVec2 vSegment0 = m.TransformPosition(nsVec3(23, 42, 0)).GetAsVec2();
      const nsVec2 vSegmentDir = m.TransformDirection(nsVec3(13, 15, 0)).GetAsVec2();

      const nsVec2 vSegment1 = vSegment0 + vSegmentDir;

      for (float f = -1.1f; f < 2.0f; f += 0.2f)
      {
        const bool bIntersection = (f >= 0.0f && f <= 1.0f);
        const nsVec2 vSegmentPos = vSegment0 + f * vSegmentDir;

        const nsVec2 vRayDir = nsVec2(2.0f, f);
        const nsVec2 vRayStart = vSegmentPos - vRayDir * 5.0f;

        float fIntersection;
        nsVec2 vIntersection;
        NS_TEST_BOOL(nsIntersectionUtils::Ray2DLine2D(vRayStart, vRayDir, vSegment0, vSegment1, &fIntersection, &vIntersection) == bIntersection);

        if (bIntersection)
        {
          NS_TEST_FLOAT(fIntersection, 5.0f, 0.0001f);
          NS_TEST_VEC2(vIntersection, vSegmentPos, 0.0001f);
        }
      };
    }
  }
}
