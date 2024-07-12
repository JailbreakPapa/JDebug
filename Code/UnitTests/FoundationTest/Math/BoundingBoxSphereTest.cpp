#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/BoundingBoxSphere.h>

NS_CREATE_SIMPLE_TEST(Math, BoundingBoxSphere)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    nsBoundingBoxSphereT b = nsBoundingBoxSphereT::MakeFromCenterExtents(nsVec3T(-1, -2, -3), nsVec3T(1, 2, 3), 2);

    NS_TEST_BOOL(b.m_vCenter == nsVec3T(-1, -2, -3));
    NS_TEST_BOOL(b.m_vBoxHalfExtends == nsVec3T(1, 2, 3));
    NS_TEST_BOOL(b.m_fSphereRadius == 2);

    nsBoundingBoxT box = nsBoundingBoxT::MakeFromMinMax(nsVec3T(1, 1, 1), nsVec3T(3, 3, 3));
    nsBoundingSphereT sphere = nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(2, 2, 2), 1);

    b = nsBoundingBoxSphereT::MakeFromBoxAndSphere(box, sphere);

    NS_TEST_BOOL(b.m_vCenter == nsVec3T(2, 2, 2));
    NS_TEST_BOOL(b.m_vBoxHalfExtends == nsVec3T(1, 1, 1));
    NS_TEST_BOOL(b.m_fSphereRadius == 1);
    NS_TEST_BOOL(b.GetBox() == box);
    NS_TEST_BOOL(b.GetSphere() == sphere);

    b = nsBoundingBoxSphereT::MakeFromBox(box);

    NS_TEST_BOOL(b.m_vCenter == nsVec3T(2, 2, 2));
    NS_TEST_BOOL(b.m_vBoxHalfExtends == nsVec3T(1, 1, 1));
    NS_TEST_FLOAT(b.m_fSphereRadius, nsMath::Sqrt(nsMathTestType(3)), 0.00001f);
    NS_TEST_BOOL(b.GetBox() == box);

    b = nsBoundingBoxSphereT::MakeFromSphere(sphere);

    NS_TEST_BOOL(b.m_vCenter == nsVec3T(2, 2, 2));
    NS_TEST_BOOL(b.m_vBoxHalfExtends == nsVec3T(1, 1, 1));
    NS_TEST_BOOL(b.m_fSphereRadius == 1);
    NS_TEST_BOOL(b.GetSphere() == sphere);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetFromPoints")
  {
    nsVec3T p[6] = {
      nsVec3T(-4, 0, 0),
      nsVec3T(5, 0, 0),
      nsVec3T(0, -6, 0),
      nsVec3T(0, 7, 0),
      nsVec3T(0, 0, -8),
      nsVec3T(0, 0, 9),
    };

    nsBoundingBoxSphereT b = nsBoundingBoxSphereT::MakeFromPoints(p, 6);

    NS_TEST_BOOL(b.m_vCenter == nsVec3T(0.5, 0.5, 0.5));
    NS_TEST_BOOL(b.m_vBoxHalfExtends == nsVec3T(4.5, 6.5, 8.5));
    NS_TEST_FLOAT(b.m_fSphereRadius, nsVec3T(0.5, 0.5, 8.5).GetLength(), 0.00001f);
    NS_TEST_BOOL(b.m_fSphereRadius <= b.m_vBoxHalfExtends.GetLength());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetInvalid")
  {
    nsBoundingBoxSphereT b = nsBoundingBoxSphereT::MakeInvalid();

    NS_TEST_BOOL(!b.IsValid());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExpandToInclude")
  {
    nsBoundingBoxSphereT b1 = nsBoundingBoxSphereT::MakeInvalid();
    nsBoundingBoxSphereT b2 = nsBoundingBoxSphereT::MakeFromBox(nsBoundingBoxT::MakeFromMinMax(nsVec3T(2, 2, 2), nsVec3T(4, 4, 4)));

    b1.ExpandToInclude(b2);
    NS_TEST_BOOL(b1 == b2);

    nsBoundingSphereT sphere = nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(2, 2, 2), 2);
    b2 = nsBoundingBoxSphereT::MakeFromSphere(sphere);

    b1.ExpandToInclude(b2);
    NS_TEST_BOOL(b1 != b2);

    NS_TEST_BOOL(b1.m_vCenter == nsVec3T(2, 2, 2));
    NS_TEST_BOOL(b1.m_vBoxHalfExtends == nsVec3T(2, 2, 2));
    NS_TEST_FLOAT(b1.m_fSphereRadius, nsMath::Sqrt(nsMathTestType(3)) * 2, 0.00001f);
    NS_TEST_BOOL(b1.m_fSphereRadius <= b1.m_vBoxHalfExtends.GetLength());

    b1 = nsBoundingBoxSphereT::MakeInvalid();
    b2 = nsBoundingBoxSphereT::MakeFromBox(nsBoundingBoxT::MakeFromMinMax(nsVec3T(0.25, 0.25, 0.25), nsVec3T(0.5, 0.5, 0.5)));

    b1.ExpandToInclude(b2);
    NS_TEST_BOOL(b1 == b2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Transform")
  {
    nsBoundingBoxSphereT b = nsBoundingBoxSphereT::MakeFromCenterExtents(nsVec3T(1), nsVec3T(5), 5);

    nsMat4T m;
    m = nsMat4::MakeScaling(nsVec3T(-2, -3, -2));
    m.SetTranslationVector(nsVec3T(1, 1, 1));

    b.Transform(m);

    NS_TEST_BOOL(b.m_vCenter == nsVec3T(-1, -2, -1));
    NS_TEST_BOOL(b.m_vBoxHalfExtends == nsVec3T(10, 15, 10));
    NS_TEST_BOOL(b.m_fSphereRadius == 15);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsNaN")
  {
    if (nsMath::SupportsNaN<nsMathTestType>())
    {
      nsBoundingBoxSphereT b;

      b = nsBoundingBoxSphereT::MakeInvalid();
      NS_TEST_BOOL(!b.IsNaN());

      b = nsBoundingBoxSphereT::MakeInvalid();
      b.m_vCenter.x = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(b.IsNaN());

      b = nsBoundingBoxSphereT::MakeInvalid();
      b.m_vCenter.y = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(b.IsNaN());

      b = nsBoundingBoxSphereT::MakeInvalid();
      b.m_vCenter.z = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(b.IsNaN());

      b = nsBoundingBoxSphereT::MakeInvalid();
      b.m_vBoxHalfExtends.x = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(b.IsNaN());

      b = nsBoundingBoxSphereT::MakeInvalid();
      b.m_vBoxHalfExtends.y = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(b.IsNaN());

      b = nsBoundingBoxSphereT::MakeInvalid();
      b.m_vBoxHalfExtends.z = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(b.IsNaN());

      b = nsBoundingBoxSphereT::MakeInvalid();
      b.m_fSphereRadius = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(b.IsNaN());
    }
  }
}
