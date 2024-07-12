#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/Implementation/AllClasses_inl.h>
#include <Foundation/Math/Mat4.h>

NS_CREATE_SIMPLE_TEST(Math, Mat4)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Default Constructor")
  {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    if (nsMath::SupportsNaN<nsMat3T::ComponentType>())
    {
      // In debug the default constructor initializes everything with NaN.
      nsMat4T m;
      NS_TEST_BOOL(nsMath::IsNaN(m.m_fElementsCM[0]) && nsMath::IsNaN(m.m_fElementsCM[1]) && nsMath::IsNaN(m.m_fElementsCM[2]) &&
                   nsMath::IsNaN(m.m_fElementsCM[3]) && nsMath::IsNaN(m.m_fElementsCM[4]) && nsMath::IsNaN(m.m_fElementsCM[5]) &&
                   nsMath::IsNaN(m.m_fElementsCM[6]) && nsMath::IsNaN(m.m_fElementsCM[7]) && nsMath::IsNaN(m.m_fElementsCM[8]) &&
                   nsMath::IsNaN(m.m_fElementsCM[9]) && nsMath::IsNaN(m.m_fElementsCM[10]) && nsMath::IsNaN(m.m_fElementsCM[11]) &&
                   nsMath::IsNaN(m.m_fElementsCM[12]) && nsMath::IsNaN(m.m_fElementsCM[13]) && nsMath::IsNaN(m.m_fElementsCM[14]) &&
                   nsMath::IsNaN(m.m_fElementsCM[15]));
    }
#else
    // Placement new of the default constructor should not have any effect on the previous data.
    nsMat4T::ComponentType testBlock[16] = {(nsMat4T::ComponentType)1, (nsMat4T::ComponentType)2, (nsMat4T::ComponentType)3,
      (nsMat4T::ComponentType)4, (nsMat4T::ComponentType)5, (nsMat4T::ComponentType)6, (nsMat4T::ComponentType)7, (nsMat4T::ComponentType)8,
      (nsMat4T::ComponentType)9, (nsMat4T::ComponentType)10, (nsMat4T::ComponentType)11, (nsMat4T::ComponentType)12, (nsMat4T::ComponentType)13,
      (nsMat4T::ComponentType)14, (nsMat4T::ComponentType)15, (nsMat4T::ComponentType)16};
    nsMat4T* m = ::new ((void*)&testBlock[0]) nsMat4T;

    NS_TEST_BOOL(m->m_fElementsCM[0] == 1.0f && m->m_fElementsCM[1] == 2.0f && m->m_fElementsCM[2] == 3.0f && m->m_fElementsCM[3] == 4.0f &&
                 m->m_fElementsCM[4] == 5.0f && m->m_fElementsCM[5] == 6.0f && m->m_fElementsCM[6] == 7.0f && m->m_fElementsCM[7] == 8.0f &&
                 m->m_fElementsCM[8] == 9.0f && m->m_fElementsCM[9] == 10.0f && m->m_fElementsCM[10] == 11.0f && m->m_fElementsCM[11] == 12.0f &&
                 m->m_fElementsCM[12] == 13.0f && m->m_fElementsCM[13] == 14.0f && m->m_fElementsCM[14] == 15.0f && m->m_fElementsCM[15] == 16.0f);
#endif
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor (Array Data)")
  {
    const nsMathTestType data[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    {
      nsMat4T m = nsMat4T::MakeFromColumnMajorArray(data);

      NS_TEST_BOOL(m.m_fElementsCM[0] == 1.0f && m.m_fElementsCM[1] == 2.0f && m.m_fElementsCM[2] == 3.0f && m.m_fElementsCM[3] == 4.0f &&
                   m.m_fElementsCM[4] == 5.0f && m.m_fElementsCM[5] == 6.0f && m.m_fElementsCM[6] == 7.0f && m.m_fElementsCM[7] == 8.0f &&
                   m.m_fElementsCM[8] == 9.0f && m.m_fElementsCM[9] == 10.0f && m.m_fElementsCM[10] == 11.0f && m.m_fElementsCM[11] == 12.0f &&
                   m.m_fElementsCM[12] == 13.0f && m.m_fElementsCM[13] == 14.0f && m.m_fElementsCM[14] == 15.0f && m.m_fElementsCM[15] == 16.0f);
    }

    {
      nsMat4T m = nsMat4T::MakeFromRowMajorArray(data);

      NS_TEST_BOOL(m.m_fElementsCM[0] == 1.0f && m.m_fElementsCM[1] == 5.0f && m.m_fElementsCM[2] == 9.0f && m.m_fElementsCM[3] == 13.0f &&
                   m.m_fElementsCM[4] == 2.0f && m.m_fElementsCM[5] == 6.0f && m.m_fElementsCM[6] == 10.0f && m.m_fElementsCM[7] == 14.0f &&
                   m.m_fElementsCM[8] == 3.0f && m.m_fElementsCM[9] == 7.0f && m.m_fElementsCM[10] == 11.0f && m.m_fElementsCM[11] == 15.0f &&
                   m.m_fElementsCM[12] == 4.0f && m.m_fElementsCM[13] == 8.0f && m.m_fElementsCM[14] == 12.0f && m.m_fElementsCM[15] == 16.0f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor (Elements)")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    NS_TEST_FLOAT(m.Element(0, 0), 1, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 0), 2, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 0), 3, 0.00001f);
    NS_TEST_FLOAT(m.Element(3, 0), 4, 0.00001f);
    NS_TEST_FLOAT(m.Element(0, 1), 5, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 1), 6, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 1), 7, 0.00001f);
    NS_TEST_FLOAT(m.Element(3, 1), 8, 0.00001f);
    NS_TEST_FLOAT(m.Element(0, 2), 9, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 2), 10, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 2), 11, 0.00001f);
    NS_TEST_FLOAT(m.Element(3, 2), 12, 0.00001f);
    NS_TEST_FLOAT(m.Element(0, 3), 13, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 3), 14, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 3), 15, 0.00001f);
    NS_TEST_FLOAT(m.Element(3, 3), 16, 0.00001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor (composite)")
  {
    nsMat3T mr = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);
    nsVec3T vt(10, 11, 12);

    nsMat4T m(mr, vt);

    NS_TEST_FLOAT(m.Element(0, 0), 1, 0);
    NS_TEST_FLOAT(m.Element(1, 0), 2, 0);
    NS_TEST_FLOAT(m.Element(2, 0), 3, 0);
    NS_TEST_FLOAT(m.Element(3, 0), 10, 0);
    NS_TEST_FLOAT(m.Element(0, 1), 4, 0);
    NS_TEST_FLOAT(m.Element(1, 1), 5, 0);
    NS_TEST_FLOAT(m.Element(2, 1), 6, 0);
    NS_TEST_FLOAT(m.Element(3, 1), 11, 0);
    NS_TEST_FLOAT(m.Element(0, 2), 7, 0);
    NS_TEST_FLOAT(m.Element(1, 2), 8, 0);
    NS_TEST_FLOAT(m.Element(2, 2), 9, 0);
    NS_TEST_FLOAT(m.Element(3, 2), 12, 0);
    NS_TEST_FLOAT(m.Element(0, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(3, 3), 1, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetFromArray")
  {
    const nsMathTestType data[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    {
      nsMat4T m = nsMat4T::MakeFromColumnMajorArray(data);

      NS_TEST_BOOL(m.m_fElementsCM[0] == 1.0f && m.m_fElementsCM[1] == 2.0f && m.m_fElementsCM[2] == 3.0f && m.m_fElementsCM[3] == 4.0f &&
                   m.m_fElementsCM[4] == 5.0f && m.m_fElementsCM[5] == 6.0f && m.m_fElementsCM[6] == 7.0f && m.m_fElementsCM[7] == 8.0f &&
                   m.m_fElementsCM[8] == 9.0f && m.m_fElementsCM[9] == 10.0f && m.m_fElementsCM[10] == 11.0f && m.m_fElementsCM[11] == 12.0f &&
                   m.m_fElementsCM[12] == 13.0f && m.m_fElementsCM[13] == 14.0f && m.m_fElementsCM[14] == 15.0f && m.m_fElementsCM[15] == 16.0f);
    }

    {
      nsMat4T m = nsMat4T::MakeFromRowMajorArray(data);

      NS_TEST_BOOL(m.m_fElementsCM[0] == 1.0f && m.m_fElementsCM[1] == 5.0f && m.m_fElementsCM[2] == 9.0f && m.m_fElementsCM[3] == 13.0f &&
                   m.m_fElementsCM[4] == 2.0f && m.m_fElementsCM[5] == 6.0f && m.m_fElementsCM[6] == 10.0f && m.m_fElementsCM[7] == 14.0f &&
                   m.m_fElementsCM[8] == 3.0f && m.m_fElementsCM[9] == 7.0f && m.m_fElementsCM[10] == 11.0f && m.m_fElementsCM[11] == 15.0f &&
                   m.m_fElementsCM[12] == 4.0f && m.m_fElementsCM[13] == 8.0f && m.m_fElementsCM[14] == 12.0f && m.m_fElementsCM[15] == 16.0f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetElements")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    NS_TEST_FLOAT(m.Element(0, 0), 1, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 0), 2, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 0), 3, 0.00001f);
    NS_TEST_FLOAT(m.Element(3, 0), 4, 0.00001f);
    NS_TEST_FLOAT(m.Element(0, 1), 5, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 1), 6, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 1), 7, 0.00001f);
    NS_TEST_FLOAT(m.Element(3, 1), 8, 0.00001f);
    NS_TEST_FLOAT(m.Element(0, 2), 9, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 2), 10, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 2), 11, 0.00001f);
    NS_TEST_FLOAT(m.Element(3, 2), 12, 0.00001f);
    NS_TEST_FLOAT(m.Element(0, 3), 13, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 3), 14, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 3), 15, 0.00001f);
    NS_TEST_FLOAT(m.Element(3, 3), 16, 0.00001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeTransformation")
  {
    nsMat3T mr = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);
    nsVec3T vt(10, 11, 12);

    nsMat4T m = nsMat4T::MakeTransformation(mr, vt);

    NS_TEST_FLOAT(m.Element(0, 0), 1, 0);
    NS_TEST_FLOAT(m.Element(1, 0), 2, 0);
    NS_TEST_FLOAT(m.Element(2, 0), 3, 0);
    NS_TEST_FLOAT(m.Element(3, 0), 10, 0);
    NS_TEST_FLOAT(m.Element(0, 1), 4, 0);
    NS_TEST_FLOAT(m.Element(1, 1), 5, 0);
    NS_TEST_FLOAT(m.Element(2, 1), 6, 0);
    NS_TEST_FLOAT(m.Element(3, 1), 11, 0);
    NS_TEST_FLOAT(m.Element(0, 2), 7, 0);
    NS_TEST_FLOAT(m.Element(1, 2), 8, 0);
    NS_TEST_FLOAT(m.Element(2, 2), 9, 0);
    NS_TEST_FLOAT(m.Element(3, 2), 12, 0);
    NS_TEST_FLOAT(m.Element(0, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(3, 3), 1, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetAsArray")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    nsMathTestType data[16];

    m.GetAsArray(data, nsMatrixLayout::ColumnMajor);
    NS_TEST_FLOAT(data[0], 1, 0.0001f);
    NS_TEST_FLOAT(data[1], 5, 0.0001f);
    NS_TEST_FLOAT(data[2], 9, 0.0001f);
    NS_TEST_FLOAT(data[3], 13, 0.0001f);
    NS_TEST_FLOAT(data[4], 2, 0.0001f);
    NS_TEST_FLOAT(data[5], 6, 0.0001f);
    NS_TEST_FLOAT(data[6], 10, 0.0001f);
    NS_TEST_FLOAT(data[7], 14, 0.0001f);
    NS_TEST_FLOAT(data[8], 3, 0.0001f);
    NS_TEST_FLOAT(data[9], 7, 0.0001f);
    NS_TEST_FLOAT(data[10], 11, 0.0001f);
    NS_TEST_FLOAT(data[11], 15, 0.0001f);
    NS_TEST_FLOAT(data[12], 4, 0.0001f);
    NS_TEST_FLOAT(data[13], 8, 0.0001f);
    NS_TEST_FLOAT(data[14], 12, 0.0001f);
    NS_TEST_FLOAT(data[15], 16, 0.0001f);

    m.GetAsArray(data, nsMatrixLayout::RowMajor);
    NS_TEST_FLOAT(data[0], 1, 0.0001f);
    NS_TEST_FLOAT(data[1], 2, 0.0001f);
    NS_TEST_FLOAT(data[2], 3, 0.0001f);
    NS_TEST_FLOAT(data[3], 4, 0.0001f);
    NS_TEST_FLOAT(data[4], 5, 0.0001f);
    NS_TEST_FLOAT(data[5], 6, 0.0001f);
    NS_TEST_FLOAT(data[6], 7, 0.0001f);
    NS_TEST_FLOAT(data[7], 8, 0.0001f);
    NS_TEST_FLOAT(data[8], 9, 0.0001f);
    NS_TEST_FLOAT(data[9], 10, 0.0001f);
    NS_TEST_FLOAT(data[10], 11, 0.0001f);
    NS_TEST_FLOAT(data[11], 12, 0.0001f);
    NS_TEST_FLOAT(data[12], 13, 0.0001f);
    NS_TEST_FLOAT(data[13], 14, 0.0001f);
    NS_TEST_FLOAT(data[14], 15, 0.0001f);
    NS_TEST_FLOAT(data[15], 16, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetZero")
  {
    nsMat4T m;
    m.SetZero();

    for (nsUInt32 i = 0; i < 16; ++i)
      NS_TEST_FLOAT(m.m_fElementsCM[i], 0.0f, 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetIdentity")
  {
    nsMat4T m;
    m.SetIdentity();

    NS_TEST_FLOAT(m.Element(0, 0), 1, 0);
    NS_TEST_FLOAT(m.Element(1, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(3, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 1), 1, 0);
    NS_TEST_FLOAT(m.Element(2, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(3, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 2), 1, 0);
    NS_TEST_FLOAT(m.Element(3, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(3, 3), 1, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetTranslationMatrix")
  {
    nsMat4T m = nsMat4::MakeTranslation(nsVec3T(2, 3, 4));

    NS_TEST_FLOAT(m.Element(0, 0), 1, 0);
    NS_TEST_FLOAT(m.Element(1, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(3, 0), 2, 0);
    NS_TEST_FLOAT(m.Element(0, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 1), 1, 0);
    NS_TEST_FLOAT(m.Element(2, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(3, 1), 3, 0);
    NS_TEST_FLOAT(m.Element(0, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 2), 1, 0);
    NS_TEST_FLOAT(m.Element(3, 2), 4, 0);
    NS_TEST_FLOAT(m.Element(0, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(3, 3), 1, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetScalingMatrix")
  {
    nsMat4T m = nsMat4::MakeScaling(nsVec3T(2, 3, 4));

    NS_TEST_FLOAT(m.Element(0, 0), 2, 0);
    NS_TEST_FLOAT(m.Element(1, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(3, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 1), 3, 0);
    NS_TEST_FLOAT(m.Element(2, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(3, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 2), 4, 0);
    NS_TEST_FLOAT(m.Element(3, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(3, 3), 1, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetRotationMatrixX")
  {
    nsMat4T m;

    m = nsMat4::MakeRotationX(nsAngle::MakeFromDegree(90));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(1, -3, 2), 0.0001f));

    m = nsMat4::MakeRotationX(nsAngle::MakeFromDegree(180));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(1, -2, -3), 0.0001f));

    m = nsMat4::MakeRotationX(nsAngle::MakeFromDegree(270));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(1, 3, -2), 0.0001f));

    m = nsMat4::MakeRotationX(nsAngle::MakeFromDegree(360));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(1, 2, 3), 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetRotationMatrixY")
  {
    nsMat4T m;

    m = nsMat4::MakeRotationY(nsAngle::MakeFromDegree(90));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(3, 2, -1), 0.0001f));

    m = nsMat4::MakeRotationY(nsAngle::MakeFromDegree(180));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(-1, 2, -3), 0.0001f));

    m = nsMat4::MakeRotationY(nsAngle::MakeFromDegree(270));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(-3, 2, 1), 0.0001f));

    m = nsMat4::MakeRotationY(nsAngle::MakeFromDegree(360));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(1, 2, 3), 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetRotationMatrixZ")
  {
    nsMat4T m;

    m = nsMat4::MakeRotationZ(nsAngle::MakeFromDegree(90));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(-2, 1, 3), 0.0001f));

    m = nsMat4::MakeRotationZ(nsAngle::MakeFromDegree(180));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(-1, -2, 3), 0.0001f));

    m = nsMat4::MakeRotationZ(nsAngle::MakeFromDegree(270));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(2, -1, 3), 0.0001f));

    m = nsMat4::MakeRotationZ(nsAngle::MakeFromDegree(360));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(1, 2, 3), 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetRotationMatrix")
  {
    nsMat4T m;

    m = nsMat4::MakeAxisRotation(nsVec3T(1, 0, 0), nsAngle::MakeFromDegree(90));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(1, -3, 2), nsMath::DefaultEpsilon<nsMat3T::ComponentType>()));

    m = nsMat4::MakeAxisRotation(nsVec3T(1, 0, 0), nsAngle::MakeFromDegree(180));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(1, -2, -3), nsMath::DefaultEpsilon<nsMat3T::ComponentType>()));

    m = nsMat4::MakeAxisRotation(nsVec3T(1, 0, 0), nsAngle::MakeFromDegree(270));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(1, 3, -2), nsMath::DefaultEpsilon<nsMat3T::ComponentType>()));

    m = nsMat4::MakeAxisRotation(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(90));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(3, 2, -1), nsMath::DefaultEpsilon<nsMat3T::ComponentType>()));

    m = nsMat4::MakeAxisRotation(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(180));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(-1, 2, -3), nsMath::DefaultEpsilon<nsMat3T::ComponentType>()));

    m = nsMat4::MakeAxisRotation(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(270));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(-3, 2, 1), nsMath::DefaultEpsilon<nsMat3T::ComponentType>()));

    m = nsMat4::MakeAxisRotation(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(90));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(-2, 1, 3), nsMath::DefaultEpsilon<nsMat3T::ComponentType>()));

    m = nsMat4::MakeAxisRotation(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(180));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(-1, -2, 3), nsMath::DefaultEpsilon<nsMat3T::ComponentType>()));

    m = nsMat4::MakeAxisRotation(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(270));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(2, -1, 3), nsMath::DefaultEpsilon<nsMat3T::ComponentType>()));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeIdentity")
  {
    nsMat4T m = nsMat4T::MakeIdentity();

    NS_TEST_FLOAT(m.Element(0, 0), 1, 0);
    NS_TEST_FLOAT(m.Element(1, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(3, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 1), 1, 0);
    NS_TEST_FLOAT(m.Element(2, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(3, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 2), 1, 0);
    NS_TEST_FLOAT(m.Element(3, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(3, 3), 1, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeZero")
  {
    nsMat4T m = nsMat4T::MakeZero();

    NS_TEST_FLOAT(m.Element(0, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(3, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(3, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(3, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 3), 0, 0);
    NS_TEST_FLOAT(m.Element(3, 3), 0, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Transpose")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    m.Transpose();

    NS_TEST_FLOAT(m.Element(0, 0), 1, 0);
    NS_TEST_FLOAT(m.Element(1, 0), 5, 0);
    NS_TEST_FLOAT(m.Element(2, 0), 9, 0);
    NS_TEST_FLOAT(m.Element(3, 0), 13, 0);
    NS_TEST_FLOAT(m.Element(0, 1), 2, 0);
    NS_TEST_FLOAT(m.Element(1, 1), 6, 0);
    NS_TEST_FLOAT(m.Element(2, 1), 10, 0);
    NS_TEST_FLOAT(m.Element(3, 1), 14, 0);
    NS_TEST_FLOAT(m.Element(0, 2), 3, 0);
    NS_TEST_FLOAT(m.Element(1, 2), 7, 0);
    NS_TEST_FLOAT(m.Element(2, 2), 11, 0);
    NS_TEST_FLOAT(m.Element(3, 2), 15, 0);
    NS_TEST_FLOAT(m.Element(0, 3), 4, 0);
    NS_TEST_FLOAT(m.Element(1, 3), 8, 0);
    NS_TEST_FLOAT(m.Element(2, 3), 12, 0);
    NS_TEST_FLOAT(m.Element(3, 3), 16, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetTranspose")
  {
    nsMat4T m0 = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    nsMat4T m = m0.GetTranspose();

    NS_TEST_FLOAT(m.Element(0, 0), 1, 0);
    NS_TEST_FLOAT(m.Element(1, 0), 5, 0);
    NS_TEST_FLOAT(m.Element(2, 0), 9, 0);
    NS_TEST_FLOAT(m.Element(3, 0), 13, 0);
    NS_TEST_FLOAT(m.Element(0, 1), 2, 0);
    NS_TEST_FLOAT(m.Element(1, 1), 6, 0);
    NS_TEST_FLOAT(m.Element(2, 1), 10, 0);
    NS_TEST_FLOAT(m.Element(3, 1), 14, 0);
    NS_TEST_FLOAT(m.Element(0, 2), 3, 0);
    NS_TEST_FLOAT(m.Element(1, 2), 7, 0);
    NS_TEST_FLOAT(m.Element(2, 2), 11, 0);
    NS_TEST_FLOAT(m.Element(3, 2), 15, 0);
    NS_TEST_FLOAT(m.Element(0, 3), 4, 0);
    NS_TEST_FLOAT(m.Element(1, 3), 8, 0);
    NS_TEST_FLOAT(m.Element(2, 3), 12, 0);
    NS_TEST_FLOAT(m.Element(3, 3), 16, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Invert")
  {
    for (float x = 1.0f; x < 360.0f; x += 10.0f)
    {
      for (float y = 2.0f; y < 360.0f; y += 17.0f)
      {
        for (float z = 3.0f; z < 360.0f; z += 23.0f)
        {
          nsMat4T m, inv;
          m = nsMat4::MakeAxisRotation(nsVec3T(x, y, z).GetNormalized(), nsAngle::MakeFromDegree(19.0f));
          inv = m;
          NS_TEST_BOOL(inv.Invert() == NS_SUCCESS);

          nsVec3T v = m * nsVec3T(1, 1, 1);
          nsVec3T vinv = inv * v;

          NS_TEST_VEC3(vinv, nsVec3T(1, 1, 1), nsMath::DefaultEpsilon<nsMathTestType>());
        }
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetInverse")
  {
    for (float x = 1.0f; x < 360.0f; x += 9.0f)
    {
      for (float y = 2.0f; y < 360.0f; y += 19.0f)
      {
        for (float z = 3.0f; z < 360.0f; z += 21.0f)
        {
          nsMat4T m, inv;
          m = nsMat4::MakeAxisRotation(nsVec3T(x, y, z).GetNormalized(), nsAngle::MakeFromDegree(83.0f));
          inv = m.GetInverse();

          nsVec3T v = m * nsVec3T(1, 1, 1);
          nsVec3T vinv = inv * v;

          NS_TEST_VEC3(vinv, nsVec3T(1, 1, 1), nsMath::DefaultEpsilon<nsMathTestType>());
        }
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsZero")
  {
    nsMat4T m;

    m.SetIdentity();
    NS_TEST_BOOL(!m.IsZero());

    m.SetZero();
    NS_TEST_BOOL(m.IsZero());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsIdentity")
  {
    nsMat4T m;

    m.SetIdentity();
    NS_TEST_BOOL(m.IsIdentity());

    m.SetZero();
    NS_TEST_BOOL(!m.IsIdentity());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsValid")
  {
    if (nsMath::SupportsNaN<nsMat3T::ComponentType>())
    {
      nsMat4T m;

      m.SetZero();
      NS_TEST_BOOL(m.IsValid());

      m.m_fElementsCM[0] = nsMath::NaN<nsMat4T::ComponentType>();
      NS_TEST_BOOL(!m.IsValid());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetRow")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    NS_TEST_VEC4(m.GetRow(0), nsVec4T(1, 2, 3, 4), 0.0f);
    NS_TEST_VEC4(m.GetRow(1), nsVec4T(5, 6, 7, 8), 0.0f);
    NS_TEST_VEC4(m.GetRow(2), nsVec4T(9, 10, 11, 12), 0.0f);
    NS_TEST_VEC4(m.GetRow(3), nsVec4T(13, 14, 15, 16), 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetRow")
  {
    nsMat4T m;
    m.SetZero();

    m.SetRow(0, nsVec4T(1, 2, 3, 4));
    NS_TEST_VEC4(m.GetRow(0), nsVec4T(1, 2, 3, 4), 0.0f);

    m.SetRow(1, nsVec4T(5, 6, 7, 8));
    NS_TEST_VEC4(m.GetRow(1), nsVec4T(5, 6, 7, 8), 0.0f);

    m.SetRow(2, nsVec4T(9, 10, 11, 12));
    NS_TEST_VEC4(m.GetRow(2), nsVec4T(9, 10, 11, 12), 0.0f);

    m.SetRow(3, nsVec4T(13, 14, 15, 16));
    NS_TEST_VEC4(m.GetRow(3), nsVec4T(13, 14, 15, 16), 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetColumn")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    NS_TEST_VEC4(m.GetColumn(0), nsVec4T(1, 5, 9, 13), 0.0f);
    NS_TEST_VEC4(m.GetColumn(1), nsVec4T(2, 6, 10, 14), 0.0f);
    NS_TEST_VEC4(m.GetColumn(2), nsVec4T(3, 7, 11, 15), 0.0f);
    NS_TEST_VEC4(m.GetColumn(3), nsVec4T(4, 8, 12, 16), 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetColumn")
  {
    nsMat4T m;
    m.SetZero();

    m.SetColumn(0, nsVec4T(1, 2, 3, 4));
    NS_TEST_VEC4(m.GetColumn(0), nsVec4T(1, 2, 3, 4), 0.0f);

    m.SetColumn(1, nsVec4T(5, 6, 7, 8));
    NS_TEST_VEC4(m.GetColumn(1), nsVec4T(5, 6, 7, 8), 0.0f);

    m.SetColumn(2, nsVec4T(9, 10, 11, 12));
    NS_TEST_VEC4(m.GetColumn(2), nsVec4T(9, 10, 11, 12), 0.0f);

    m.SetColumn(3, nsVec4T(13, 14, 15, 16));
    NS_TEST_VEC4(m.GetColumn(3), nsVec4T(13, 14, 15, 16), 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetDiagonal")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    NS_TEST_VEC4(m.GetDiagonal(), nsVec4T(1, 6, 11, 16), 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetDiagonal")
  {
    nsMat4T m;
    m.SetZero();

    m.SetDiagonal(nsVec4T(1, 2, 3, 4));
    NS_TEST_VEC4(m.GetColumn(0), nsVec4T(1, 0, 0, 0), 0.0f);
    NS_TEST_VEC4(m.GetColumn(1), nsVec4T(0, 2, 0, 0), 0.0f);
    NS_TEST_VEC4(m.GetColumn(2), nsVec4T(0, 0, 3, 0), 0.0f);
    NS_TEST_VEC4(m.GetColumn(3), nsVec4T(0, 0, 0, 4), 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetTranslationVector")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    NS_TEST_VEC3(m.GetTranslationVector(), nsVec3T(4, 8, 12), 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetTranslationVector")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    m.SetTranslationVector(nsVec3T(17, 18, 19));
    NS_TEST_VEC4(m.GetRow(0), nsVec4T(1, 2, 3, 17), 0.0f);
    NS_TEST_VEC4(m.GetRow(1), nsVec4T(5, 6, 7, 18), 0.0f);
    NS_TEST_VEC4(m.GetRow(2), nsVec4T(9, 10, 11, 19), 0.0f);
    NS_TEST_VEC4(m.GetRow(3), nsVec4T(13, 14, 15, 16), 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetRotationalPart")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    nsMat3T r = nsMat3T::MakeFromValues(17, 18, 19, 20, 21, 22, 23, 24, 25);

    m.SetRotationalPart(r);
    NS_TEST_VEC4(m.GetRow(0), nsVec4T(17, 18, 19, 4), 0.0f);
    NS_TEST_VEC4(m.GetRow(1), nsVec4T(20, 21, 22, 8), 0.0f);
    NS_TEST_VEC4(m.GetRow(2), nsVec4T(23, 24, 25, 12), 0.0f);
    NS_TEST_VEC4(m.GetRow(3), nsVec4T(13, 14, 15, 16), 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetRotationalPart")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    nsMat3T r = m.GetRotationalPart();
    NS_TEST_VEC3(r.GetRow(0), nsVec3T(1, 2, 3), 0.0f);
    NS_TEST_VEC3(r.GetRow(1), nsVec3T(5, 6, 7), 0.0f);
    NS_TEST_VEC3(r.GetRow(2), nsVec3T(9, 10, 11), 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetScalingFactors")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    nsVec3T s = m.GetScalingFactors();
    NS_TEST_VEC3(s,
      nsVec3T(nsMath::Sqrt((nsMathTestType)(1 * 1 + 5 * 5 + 9 * 9)), nsMath::Sqrt((nsMathTestType)(2 * 2 + 6 * 6 + 10 * 10)),
        nsMath::Sqrt((nsMathTestType)(3 * 3 + 7 * 7 + 11 * 11))),
      0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetScalingFactors")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    NS_TEST_BOOL(m.SetScalingFactors(nsVec3T(1, 2, 3)) == NS_SUCCESS);

    nsVec3T s = m.GetScalingFactors();
    NS_TEST_VEC3(s, nsVec3T(1, 2, 3), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TransformDirection")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    const nsVec3T r = m.TransformDirection(nsVec3T(1, 2, 3));

    NS_TEST_VEC3(r, nsVec3T(1 * 1 + 2 * 2 + 3 * 3, 1 * 5 + 2 * 6 + 3 * 7, 1 * 9 + 2 * 10 + 3 * 11), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TransformDirection(array)")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    nsVec3T data[3] = {nsVec3T(1, 2, 3), nsVec3T(4, 5, 6), nsVec3T(7, 8, 9)};

    m.TransformDirection(data, 2);

    NS_TEST_VEC3(data[0], nsVec3T(1 * 1 + 2 * 2 + 3 * 3, 1 * 5 + 2 * 6 + 3 * 7, 1 * 9 + 2 * 10 + 3 * 11), 0.0001f);
    NS_TEST_VEC3(data[1], nsVec3T(4 * 1 + 5 * 2 + 6 * 3, 4 * 5 + 5 * 6 + 6 * 7, 4 * 9 + 5 * 10 + 6 * 11), 0.0001f);
    NS_TEST_VEC3(data[2], nsVec3T(7, 8, 9), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TransformPosition")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    const nsVec3T r = m.TransformPosition(nsVec3T(1, 2, 3));

    NS_TEST_VEC3(r, nsVec3T(1 * 1 + 2 * 2 + 3 * 3 + 4, 1 * 5 + 2 * 6 + 3 * 7 + 8, 1 * 9 + 2 * 10 + 3 * 11 + 12), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TransformPosition(array)")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    nsVec3T data[3] = {nsVec3T(1, 2, 3), nsVec3T(4, 5, 6), nsVec3T(7, 8, 9)};

    m.TransformPosition(data, 2);

    NS_TEST_VEC3(data[0], nsVec3T(1 * 1 + 2 * 2 + 3 * 3 + 4, 1 * 5 + 2 * 6 + 3 * 7 + 8, 1 * 9 + 2 * 10 + 3 * 11 + 12), 0.0001f);
    NS_TEST_VEC3(data[1], nsVec3T(4 * 1 + 5 * 2 + 6 * 3 + 4, 4 * 5 + 5 * 6 + 6 * 7 + 8, 4 * 9 + 5 * 10 + 6 * 11 + 12), 0.0001f);
    NS_TEST_VEC3(data[2], nsVec3T(7, 8, 9), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Transform")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    const nsVec4T r = m.Transform(nsVec4T(1, 2, 3, 4));

    NS_TEST_VEC4(r,
      nsVec4T(1 * 1 + 2 * 2 + 3 * 3 + 4 * 4, 1 * 5 + 2 * 6 + 3 * 7 + 8 * 4, 1 * 9 + 2 * 10 + 3 * 11 + 12 * 4, 1 * 13 + 2 * 14 + 3 * 15 + 4 * 16),
      0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Transform(array)")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    nsVec4T data[3] = {nsVec4T(1, 2, 3, 4), nsVec4T(5, 6, 7, 8), nsVec4T(9, 10, 11, 12)};

    m.Transform(data, 2);

    NS_TEST_VEC4(data[0],
      nsVec4T(1 * 1 + 2 * 2 + 3 * 3 + 4 * 4, 1 * 5 + 2 * 6 + 3 * 7 + 8 * 4, 1 * 9 + 2 * 10 + 3 * 11 + 12 * 4, 1 * 13 + 2 * 14 + 3 * 15 + 4 * 16),
      0.0001f);
    NS_TEST_VEC4(data[1],
      nsVec4T(5 * 1 + 6 * 2 + 7 * 3 + 8 * 4, 5 * 5 + 6 * 6 + 7 * 7 + 8 * 8, 5 * 9 + 6 * 10 + 7 * 11 + 12 * 8, 5 * 13 + 6 * 14 + 7 * 15 + 8 * 16),
      0.0001f);
    NS_TEST_VEC4(data[2], nsVec4T(9, 10, 11, 12), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator*=")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    m *= 2.0f;

    NS_TEST_VEC4(m.GetRow(0), nsVec4T(2, 4, 6, 8), 0.0001f);
    NS_TEST_VEC4(m.GetRow(1), nsVec4T(10, 12, 14, 16), 0.0001f);
    NS_TEST_VEC4(m.GetRow(2), nsVec4T(18, 20, 22, 24), 0.0001f);
    NS_TEST_VEC4(m.GetRow(3), nsVec4T(26, 28, 30, 32), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator/=")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    m *= 4.0f;
    m /= 2.0f;

    NS_TEST_VEC4(m.GetRow(0), nsVec4T(2, 4, 6, 8), 0.0001f);
    NS_TEST_VEC4(m.GetRow(1), nsVec4T(10, 12, 14, 16), 0.0001f);
    NS_TEST_VEC4(m.GetRow(2), nsVec4T(18, 20, 22, 24), 0.0001f);
    NS_TEST_VEC4(m.GetRow(3), nsVec4T(26, 28, 30, 32), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsIdentical")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    nsMat4T m2 = m;

    NS_TEST_BOOL(m.IsIdentical(m2));

    m2.m_fElementsCM[0] += 0.00001f;
    NS_TEST_BOOL(!m.IsIdentical(m2));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqual")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    nsMat4T m2 = m;

    NS_TEST_BOOL(m.IsEqual(m2, 0.0001f));

    m2.m_fElementsCM[0] += 0.00001f;
    NS_TEST_BOOL(m.IsEqual(m2, 0.0001f));
    NS_TEST_BOOL(!m.IsEqual(m2, 0.000001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator*(mat, mat)")
  {
    nsMat4T m1 = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    nsMat4T m2 = nsMat4T::MakeFromValues(-1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16);

    nsMat4T r = m1 * m2;

    NS_TEST_VEC4(r.GetColumn(0),
      nsVec4T(-1 * 1 + -5 * 2 + -9 * 3 + -13 * 4, -1 * 5 + -5 * 6 + -9 * 7 + -13 * 8, -1 * 9 + -5 * 10 + -9 * 11 + -13 * 12,
        -1 * 13 + -5 * 14 + -9 * 15 + -13 * 16),
      0.001f);
    NS_TEST_VEC4(r.GetColumn(1),
      nsVec4T(-2 * 1 + -6 * 2 + -10 * 3 + -14 * 4, -2 * 5 + -6 * 6 + -10 * 7 + -14 * 8, -2 * 9 + -6 * 10 + -10 * 11 + -14 * 12,
        -2 * 13 + -6 * 14 + -10 * 15 + -14 * 16),
      0.001f);
    NS_TEST_VEC4(r.GetColumn(2),
      nsVec4T(-3 * 1 + -7 * 2 + -11 * 3 + -15 * 4, -3 * 5 + -7 * 6 + -11 * 7 + -15 * 8, -3 * 9 + -7 * 10 + -11 * 11 + -15 * 12,
        -3 * 13 + -7 * 14 + -11 * 15 + -15 * 16),
      0.001f);
    NS_TEST_VEC4(r.GetColumn(3),
      nsVec4T(-4 * 1 + -8 * 2 + -12 * 3 + -16 * 4, -4 * 5 + -8 * 6 + -12 * 7 + -16 * 8, -4 * 9 + -8 * 10 + -12 * 11 + -16 * 12,
        -4 * 13 + -8 * 14 + -12 * 15 + -16 * 16),
      0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator*(mat, vec3)")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    const nsVec3T r = m * nsVec3T(1, 2, 3);

    NS_TEST_VEC3(r, nsVec3T(1 * 1 + 2 * 2 + 3 * 3 + 4, 1 * 5 + 2 * 6 + 3 * 7 + 8, 1 * 9 + 2 * 10 + 3 * 11 + 12), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator*(mat, vec4)")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    const nsVec4T r = m * nsVec4T(1, 2, 3, 4);

    NS_TEST_VEC4(r,
      nsVec4T(1 * 1 + 2 * 2 + 3 * 3 + 4 * 4, 1 * 5 + 2 * 6 + 3 * 7 + 4 * 8, 1 * 9 + 2 * 10 + 3 * 11 + 4 * 12, 1 * 13 + 2 * 14 + 3 * 15 + 4 * 16),
      0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator*(mat, float) | operator*(float, mat)")
  {
    nsMat4T m0 = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    nsMat4T m = m0 * (nsMathTestType)2;
    nsMat4T m2 = (nsMathTestType)2 * m0;

    NS_TEST_VEC4(m.GetRow(0), nsVec4T(2, 4, 6, 8), 0.0001f);
    NS_TEST_VEC4(m.GetRow(1), nsVec4T(10, 12, 14, 16), 0.0001f);
    NS_TEST_VEC4(m.GetRow(2), nsVec4T(18, 20, 22, 24), 0.0001f);
    NS_TEST_VEC4(m.GetRow(3), nsVec4T(26, 28, 30, 32), 0.0001f);

    NS_TEST_VEC4(m2.GetRow(0), nsVec4T(2, 4, 6, 8), 0.0001f);
    NS_TEST_VEC4(m2.GetRow(1), nsVec4T(10, 12, 14, 16), 0.0001f);
    NS_TEST_VEC4(m2.GetRow(2), nsVec4T(18, 20, 22, 24), 0.0001f);
    NS_TEST_VEC4(m2.GetRow(3), nsVec4T(26, 28, 30, 32), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator/(mat, float)")
  {
    nsMat4T m0 = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    m0 *= (nsMathTestType)4;

    nsMat4T m = m0 / (nsMathTestType)2;

    NS_TEST_VEC4(m.GetRow(0), nsVec4T(2, 4, 6, 8), 0.0001f);
    NS_TEST_VEC4(m.GetRow(1), nsVec4T(10, 12, 14, 16), 0.0001f);
    NS_TEST_VEC4(m.GetRow(2), nsVec4T(18, 20, 22, 24), 0.0001f);
    NS_TEST_VEC4(m.GetRow(3), nsVec4T(26, 28, 30, 32), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator+(mat, mat) | operator-(mat, mat)")
  {
    nsMat4T m0 = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    nsMat4T m1 = nsMat4T::MakeFromValues(-1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16);

    NS_TEST_BOOL((m0 + m1).IsZero());
    NS_TEST_BOOL((m0 - m1).IsEqual(m0 * (nsMathTestType)2, 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator== (mat, mat) | operator!= (mat, mat)")
  {
    nsMat4T m = nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    nsMat4T m2 = m;

    NS_TEST_BOOL(m == m2);

    m2.m_fElementsCM[0] += 0.00001f;

    NS_TEST_BOOL(m != m2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsNaN")
  {
    if (nsMath::SupportsNaN<nsMathTestType>())
    {
      nsMat4T m;

      m.SetIdentity();
      NS_TEST_BOOL(!m.IsNaN());

      for (nsUInt32 i = 0; i < 16; ++i)
      {
        m.SetIdentity();
        m.m_fElementsCM[i] = nsMath::NaN<nsMathTestType>();

        NS_TEST_BOOL(m.IsNaN());
      }
    }
  }
}
