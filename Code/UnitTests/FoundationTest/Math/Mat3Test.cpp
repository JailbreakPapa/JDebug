#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/Implementation/AllClasses_inl.h>
#include <Foundation/Math/Mat3.h>

NS_CREATE_SIMPLE_TEST(Math, Mat3)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Default Constructor")
  {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    if (nsMath::SupportsNaN<nsMathTestType>())
    {
      // In debug the default constructor initializes everything with NaN.
      nsMat3T m;
      NS_TEST_BOOL(nsMath::IsNaN(m.m_fElementsCM[0]) && nsMath::IsNaN(m.m_fElementsCM[1]) && nsMath::IsNaN(m.m_fElementsCM[2]) &&
                   nsMath::IsNaN(m.m_fElementsCM[3]) && nsMath::IsNaN(m.m_fElementsCM[4]) && nsMath::IsNaN(m.m_fElementsCM[5]) &&
                   nsMath::IsNaN(m.m_fElementsCM[6]) && nsMath::IsNaN(m.m_fElementsCM[7]) && nsMath::IsNaN(m.m_fElementsCM[8]));
    }
#else
    // Placement new of the default constructor should not have any effect on the previous data.
    nsMat3T::ComponentType testBlock[9] = {(nsMat3T::ComponentType)1, (nsMat3T::ComponentType)2, (nsMat3T::ComponentType)3, (nsMat3T::ComponentType)4,
      (nsMat3T::ComponentType)5, (nsMat3T::ComponentType)6, (nsMat3T::ComponentType)7, (nsMat3T::ComponentType)8, (nsMat3T::ComponentType)9};

    nsMat3T* m = ::new ((void*)&testBlock[0]) nsMat3T;

    NS_TEST_BOOL(m->m_fElementsCM[0] == (nsMat3T::ComponentType)1 && m->m_fElementsCM[1] == (nsMat3T::ComponentType)2 &&
                 m->m_fElementsCM[2] == (nsMat3T::ComponentType)3 && m->m_fElementsCM[3] == (nsMat3T::ComponentType)4 &&
                 m->m_fElementsCM[4] == (nsMat3T::ComponentType)5 && m->m_fElementsCM[5] == (nsMat3T::ComponentType)6 &&
                 m->m_fElementsCM[6] == (nsMat3T::ComponentType)7 && m->m_fElementsCM[7] == (nsMat3T::ComponentType)8 &&
                 m->m_fElementsCM[8] == (nsMat3T::ComponentType)9);
#endif
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor (Array Data)")
  {
    const nsMathTestType data[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    {
      nsMat3T m = nsMat3T::MakeFromColumnMajorArray(data);

      NS_TEST_BOOL(m.m_fElementsCM[0] == 1.0f && m.m_fElementsCM[1] == 2.0f && m.m_fElementsCM[2] == 3.0f && m.m_fElementsCM[3] == 4.0f &&
                   m.m_fElementsCM[4] == 5.0f && m.m_fElementsCM[5] == 6.0f && m.m_fElementsCM[6] == 7.0f && m.m_fElementsCM[7] == 8.0f &&
                   m.m_fElementsCM[8] == 9.0f);
    }

    {
      nsMat3T m = nsMat3T::MakeFromRowMajorArray(data);

      NS_TEST_BOOL(m.m_fElementsCM[0] == 1.0f && m.m_fElementsCM[1] == 4.0f && m.m_fElementsCM[2] == 7.0f && m.m_fElementsCM[3] == 2.0f &&
                   m.m_fElementsCM[4] == 5.0f && m.m_fElementsCM[5] == 8.0f && m.m_fElementsCM[6] == 3.0f && m.m_fElementsCM[7] == 6.0f &&
                   m.m_fElementsCM[8] == 9.0f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor (Elements)")
  {
    nsMat3T m = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);

    NS_TEST_FLOAT(m.Element(0, 0), 1, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 0), 2, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 0), 3, 0.00001f);
    NS_TEST_FLOAT(m.Element(0, 1), 4, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 1), 5, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 1), 6, 0.00001f);
    NS_TEST_FLOAT(m.Element(0, 2), 7, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 2), 8, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 2), 9, 0.00001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetFromArray")
  {
    const nsMathTestType data[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    {
      nsMat3T m = nsMat3::MakeFromColumnMajorArray(data);

      NS_TEST_BOOL(m.m_fElementsCM[0] == 1.0f && m.m_fElementsCM[1] == 2.0f && m.m_fElementsCM[2] == 3.0f && m.m_fElementsCM[3] == 4.0f &&
                   m.m_fElementsCM[4] == 5.0f && m.m_fElementsCM[5] == 6.0f && m.m_fElementsCM[6] == 7.0f && m.m_fElementsCM[7] == 8.0f &&
                   m.m_fElementsCM[8] == 9.0f);
    }

    {
      nsMat3T m = nsMat3::MakeFromRowMajorArray(data);

      NS_TEST_BOOL(m.m_fElementsCM[0] == 1.0f && m.m_fElementsCM[1] == 4.0f && m.m_fElementsCM[2] == 7.0f && m.m_fElementsCM[3] == 2.0f &&
                   m.m_fElementsCM[4] == 5.0f && m.m_fElementsCM[5] == 8.0f && m.m_fElementsCM[6] == 3.0f && m.m_fElementsCM[7] == 6.0f &&
                   m.m_fElementsCM[8] == 9.0f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetElements")
  {
    nsMat3T m = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);

    NS_TEST_FLOAT(m.Element(0, 0), 1, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 0), 2, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 0), 3, 0.00001f);
    NS_TEST_FLOAT(m.Element(0, 1), 4, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 1), 5, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 1), 6, 0.00001f);
    NS_TEST_FLOAT(m.Element(0, 2), 7, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 2), 8, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 2), 9, 0.00001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetAsArray")
  {
    nsMat3T m = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);

    nsMathTestType data[9];

    m.GetAsArray(data, nsMatrixLayout::ColumnMajor);
    NS_TEST_FLOAT(data[0], 1, 0.0001f);
    NS_TEST_FLOAT(data[1], 4, 0.0001f);
    NS_TEST_FLOAT(data[2], 7, 0.0001f);
    NS_TEST_FLOAT(data[3], 2, 0.0001f);
    NS_TEST_FLOAT(data[4], 5, 0.0001f);
    NS_TEST_FLOAT(data[5], 8, 0.0001f);
    NS_TEST_FLOAT(data[6], 3, 0.0001f);
    NS_TEST_FLOAT(data[7], 6, 0.0001f);
    NS_TEST_FLOAT(data[8], 9, 0.0001f);

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
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetZero")
  {
    nsMat3T m;
    m.SetZero();

    for (nsUInt32 i = 0; i < 9; ++i)
      NS_TEST_FLOAT(m.m_fElementsCM[i], 0.0f, 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetIdentity")
  {
    nsMat3T m;
    m.SetIdentity();

    NS_TEST_FLOAT(m.Element(0, 0), 1, 0);
    NS_TEST_FLOAT(m.Element(1, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 1), 1, 0);
    NS_TEST_FLOAT(m.Element(2, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 2), 1, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetScalingMatrix")
  {
    nsMat3T m = nsMat3::MakeScaling(nsVec3T(2, 3, 4));

    NS_TEST_FLOAT(m.Element(0, 0), 2, 0);
    NS_TEST_FLOAT(m.Element(1, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 1), 3, 0);
    NS_TEST_FLOAT(m.Element(2, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 2), 4, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetRotationMatrixX")
  {
    nsMat3T m;

    m = nsMat3::MakeRotationX(nsAngle::MakeFromDegree(90));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(1, -3, 2), 0.0001f));

    m = nsMat3::MakeRotationX(nsAngle::MakeFromDegree(180));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(1, -2, -3), 0.0001f));

    m = nsMat3::MakeRotationX(nsAngle::MakeFromDegree(270));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(1, 3, -2), 0.0001f));

    m = nsMat3::MakeRotationX(nsAngle::MakeFromDegree(360));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(1, 2, 3), 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetRotationMatrixY")
  {
    nsMat3T m;

    m = nsMat3::MakeRotationY(nsAngle::MakeFromDegree(90));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(3, 2, -1), 0.0001f));

    m = nsMat3::MakeRotationY(nsAngle::MakeFromDegree(180));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(-1, 2, -3), 0.0001f));

    m = nsMat3::MakeRotationY(nsAngle::MakeFromDegree(270));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(-3, 2, 1), 0.0001f));

    m = nsMat3::MakeRotationY(nsAngle::MakeFromDegree(360));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(1, 2, 3), 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetRotationMatrixZ")
  {
    nsMat3T m;

    m = nsMat3::MakeRotationZ(nsAngle::MakeFromDegree(90));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(-2, 1, 3), 0.0001f));

    m = nsMat3::MakeRotationZ(nsAngle::MakeFromDegree(180));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(-1, -2, 3), 0.0001f));

    m = nsMat3::MakeRotationZ(nsAngle::MakeFromDegree(270));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(2, -1, 3), 0.0001f));

    m = nsMat3::MakeRotationZ(nsAngle::MakeFromDegree(360));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(1, 2, 3), 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetRotationMatrix")
  {
    nsMat3T m;

    m = nsMat3::MakeAxisRotation(nsVec3T(1, 0, 0), nsAngle::MakeFromDegree(90));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(1, -3, 2), nsMath::LargeEpsilon<nsMathTestType>()));

    m = nsMat3::MakeAxisRotation(nsVec3T(1, 0, 0), nsAngle::MakeFromDegree(180));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(1, -2, -3), nsMath::LargeEpsilon<nsMathTestType>()));

    m = nsMat3::MakeAxisRotation(nsVec3T(1, 0, 0), nsAngle::MakeFromDegree(270));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(1, 3, -2), nsMath::LargeEpsilon<nsMathTestType>()));

    m = nsMat3::MakeAxisRotation(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(90));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(3, 2, -1), nsMath::LargeEpsilon<nsMathTestType>()));

    m = nsMat3::MakeAxisRotation(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(180));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(-1, 2, -3), nsMath::LargeEpsilon<nsMathTestType>()));

    m = nsMat3::MakeAxisRotation(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(270));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(-3, 2, 1), nsMath::LargeEpsilon<nsMathTestType>()));

    m = nsMat3::MakeAxisRotation(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(90));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(-2, 1, 3), nsMath::LargeEpsilon<nsMathTestType>()));

    m = nsMat3::MakeAxisRotation(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(180));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(-1, -2, 3), nsMath::LargeEpsilon<nsMathTestType>()));

    m = nsMat3::MakeAxisRotation(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(270));
    NS_TEST_BOOL((m * nsVec3T(1, 2, 3)).IsEqual(nsVec3T(2, -1, 3), nsMath::LargeEpsilon<nsMathTestType>()));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeIdentity")
  {
    nsMat3T m = nsMat3T::MakeIdentity();

    NS_TEST_FLOAT(m.Element(0, 0), 1, 0);
    NS_TEST_FLOAT(m.Element(1, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 1), 1, 0);
    NS_TEST_FLOAT(m.Element(2, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 2), 1, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeZero")
  {
    nsMat3T m = nsMat3T::MakeZero();

    NS_TEST_FLOAT(m.Element(0, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 0), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 1), 0, 0);
    NS_TEST_FLOAT(m.Element(0, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(1, 2), 0, 0);
    NS_TEST_FLOAT(m.Element(2, 2), 0, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Transpose")
  {
    nsMat3T m = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);

    m.Transpose();

    NS_TEST_FLOAT(m.Element(0, 0), 1, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 0), 4, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 0), 7, 0.00001f);
    NS_TEST_FLOAT(m.Element(0, 1), 2, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 1), 5, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 1), 8, 0.00001f);
    NS_TEST_FLOAT(m.Element(0, 2), 3, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 2), 6, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 2), 9, 0.00001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetTranspose")
  {
    nsMat3T m0 = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);

    nsMat3T m = m0.GetTranspose();

    NS_TEST_FLOAT(m.Element(0, 0), 1, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 0), 4, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 0), 7, 0.00001f);
    NS_TEST_FLOAT(m.Element(0, 1), 2, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 1), 5, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 1), 8, 0.00001f);
    NS_TEST_FLOAT(m.Element(0, 2), 3, 0.00001f);
    NS_TEST_FLOAT(m.Element(1, 2), 6, 0.00001f);
    NS_TEST_FLOAT(m.Element(2, 2), 9, 0.00001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Invert")
  {
    for (float x = 1.0f; x < 360.0f; x += 10.0f)
    {
      for (float y = 2.0f; y < 360.0f; y += 17.0f)
      {
        for (float z = 3.0f; z < 360.0f; z += 23.0f)
        {
          nsMat3T m, inv;
          m = nsMat3::MakeAxisRotation(nsVec3T(x, y, z).GetNormalized(), nsAngle::MakeFromDegree(19.0f));
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
          nsMat3T m, inv;
          m = nsMat3::MakeAxisRotation(nsVec3T(x, y, z).GetNormalized(), nsAngle::MakeFromDegree(83.0f));
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
    nsMat3T m;

    m.SetIdentity();
    NS_TEST_BOOL(!m.IsZero());

    m.SetZero();
    NS_TEST_BOOL(m.IsZero());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsIdentity")
  {
    nsMat3T m;

    m.SetIdentity();
    NS_TEST_BOOL(m.IsIdentity());

    m.SetZero();
    NS_TEST_BOOL(!m.IsIdentity());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsValid")
  {
    if (nsMath::SupportsNaN<nsMat3T::ComponentType>())
    {
      nsMat3T m;

      m.SetZero();
      NS_TEST_BOOL(m.IsValid());

      m.m_fElementsCM[0] = nsMath::NaN<nsMat3T::ComponentType>();
      NS_TEST_BOOL(!m.IsValid());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetRow")
  {
    nsMat3T m = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);

    NS_TEST_VEC3(m.GetRow(0), nsVec3T(1, 2, 3), 0.0f);
    NS_TEST_VEC3(m.GetRow(1), nsVec3T(4, 5, 6), 0.0f);
    NS_TEST_VEC3(m.GetRow(2), nsVec3T(7, 8, 9), 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetRow")
  {
    nsMat3T m;
    m.SetZero();

    m.SetRow(0, nsVec3T(1, 2, 3));
    NS_TEST_VEC3(m.GetRow(0), nsVec3T(1, 2, 3), 0.0f);

    m.SetRow(1, nsVec3T(4, 5, 6));
    NS_TEST_VEC3(m.GetRow(1), nsVec3T(4, 5, 6), 0.0f);

    m.SetRow(2, nsVec3T(7, 8, 9));
    NS_TEST_VEC3(m.GetRow(2), nsVec3T(7, 8, 9), 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetColumn")
  {
    nsMat3T m = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);

    NS_TEST_VEC3(m.GetColumn(0), nsVec3T(1, 4, 7), 0.0f);
    NS_TEST_VEC3(m.GetColumn(1), nsVec3T(2, 5, 8), 0.0f);
    NS_TEST_VEC3(m.GetColumn(2), nsVec3T(3, 6, 9), 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetColumn")
  {
    nsMat3T m;
    m.SetZero();

    m.SetColumn(0, nsVec3T(1, 2, 3));
    NS_TEST_VEC3(m.GetColumn(0), nsVec3T(1, 2, 3), 0.0f);

    m.SetColumn(1, nsVec3T(4, 5, 6));
    NS_TEST_VEC3(m.GetColumn(1), nsVec3T(4, 5, 6), 0.0f);

    m.SetColumn(2, nsVec3T(7, 8, 9));
    NS_TEST_VEC3(m.GetColumn(2), nsVec3T(7, 8, 9), 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetDiagonal")
  {
    nsMat3T m = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);

    NS_TEST_VEC3(m.GetDiagonal(), nsVec3T(1, 5, 9), 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetDiagonal")
  {
    nsMat3T m;
    m.SetZero();

    m.SetDiagonal(nsVec3T(1, 2, 3));
    NS_TEST_VEC3(m.GetColumn(0), nsVec3T(1, 0, 0), 0.0f);
    NS_TEST_VEC3(m.GetColumn(1), nsVec3T(0, 2, 0), 0.0f);
    NS_TEST_VEC3(m.GetColumn(2), nsVec3T(0, 0, 3), 0.0f);
  }


  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetScalingFactors")
  {
    nsMat3T m = nsMat3T::MakeFromValues(1, 2, 3, 5, 6, 7, 9, 10, 11);

    nsVec3T s = m.GetScalingFactors();
    NS_TEST_VEC3(s,
      nsVec3T(nsMath::Sqrt((nsMathTestType)(1 * 1 + 5 * 5 + 9 * 9)), nsMath::Sqrt((nsMathTestType)(2 * 2 + 6 * 6 + 10 * 10)),
        nsMath::Sqrt((nsMathTestType)(3 * 3 + 7 * 7 + 11 * 11))),
      0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetScalingFactors")
  {
    nsMat3T m = nsMat3T::MakeFromValues(1, 2, 3, 5, 6, 7, 9, 10, 11);

    NS_TEST_BOOL(m.SetScalingFactors(nsVec3T(1, 2, 3)) == NS_SUCCESS);

    nsVec3T s = m.GetScalingFactors();
    NS_TEST_VEC3(s, nsVec3T(1, 2, 3), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TransformDirection")
  {
    nsMat3T m = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);

    const nsVec3T r = m.TransformDirection(nsVec3T(1, 2, 3));

    NS_TEST_VEC3(r, nsVec3T(1 * 1 + 2 * 2 + 3 * 3, 1 * 4 + 2 * 5 + 3 * 6, 1 * 7 + 2 * 8 + 3 * 9), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator*=")
  {
    nsMat3T m = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);

    m *= 2.0f;

    NS_TEST_VEC3(m.GetRow(0), nsVec3T(2, 4, 6), 0.0001f);
    NS_TEST_VEC3(m.GetRow(1), nsVec3T(8, 10, 12), 0.0001f);
    NS_TEST_VEC3(m.GetRow(2), nsVec3T(14, 16, 18), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator/=")
  {
    nsMat3T m = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);

    m *= 4.0f;
    m /= 2.0f;

    NS_TEST_VEC3(m.GetRow(0), nsVec3T(2, 4, 6), 0.0001f);
    NS_TEST_VEC3(m.GetRow(1), nsVec3T(8, 10, 12), 0.0001f);
    NS_TEST_VEC3(m.GetRow(2), nsVec3T(14, 16, 18), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsIdentical")
  {
    nsMat3T m = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);

    nsMat3T m2 = m;

    NS_TEST_BOOL(m.IsIdentical(m2));

    m2.m_fElementsCM[0] += 0.00001f;
    NS_TEST_BOOL(!m.IsIdentical(m2));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqual")
  {
    nsMat3T m = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);

    nsMat3T m2 = m;

    NS_TEST_BOOL(m.IsEqual(m2, 0.0001f));

    m2.m_fElementsCM[0] += 0.00001f;
    NS_TEST_BOOL(m.IsEqual(m2, 0.0001f));
    NS_TEST_BOOL(!m.IsEqual(m2, 0.000001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator*(mat, mat)")
  {
    nsMat3T m1 = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);

    nsMat3T m2 = nsMat3T::MakeFromValues(-1, -2, -3, -4, -5, -6, -7, -8, -9);

    nsMat3T r = m1 * m2;

    NS_TEST_VEC3(r.GetColumn(0), nsVec3T(-1 * 1 + -4 * 2 + -7 * 3, -1 * 4 + -4 * 5 + -7 * 6, -1 * 7 + -4 * 8 + -7 * 9), 0.001f);
    NS_TEST_VEC3(r.GetColumn(1), nsVec3T(-2 * 1 + -5 * 2 + -8 * 3, -2 * 4 + -5 * 5 + -8 * 6, -2 * 7 + -5 * 8 + -8 * 9), 0.001f);
    NS_TEST_VEC3(r.GetColumn(2), nsVec3T(-3 * 1 + -6 * 2 + -9 * 3, -3 * 4 + -6 * 5 + -9 * 6, -3 * 7 + -6 * 8 + -9 * 9), 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator*(mat, vec)")
  {
    nsMat3T m = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);

    const nsVec3T r = m * (nsVec3T(1, 2, 3));

    NS_TEST_VEC3(r, nsVec3T(1 * 1 + 2 * 2 + 3 * 3, 1 * 4 + 2 * 5 + 3 * 6, 1 * 7 + 2 * 8 + 3 * 9), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator*(mat, float) | operator*(float, mat)")
  {
    nsMat3T m0 = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);

    nsMat3T m = m0 * (nsMathTestType)2;
    nsMat3T m2 = (nsMathTestType)2 * m0;

    NS_TEST_VEC3(m.GetRow(0), nsVec3T(2, 4, 6), 0.0001f);
    NS_TEST_VEC3(m.GetRow(1), nsVec3T(8, 10, 12), 0.0001f);
    NS_TEST_VEC3(m.GetRow(2), nsVec3T(14, 16, 18), 0.0001f);

    NS_TEST_VEC3(m2.GetRow(0), nsVec3T(2, 4, 6), 0.0001f);
    NS_TEST_VEC3(m2.GetRow(1), nsVec3T(8, 10, 12), 0.0001f);
    NS_TEST_VEC3(m2.GetRow(2), nsVec3T(14, 16, 18), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator/(mat, float)")
  {
    nsMat3T m0 = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);

    m0 *= 4.0f;

    nsMat3T m = m0 / (nsMathTestType)2;

    NS_TEST_VEC3(m.GetRow(0), nsVec3T(2, 4, 6), 0.0001f);
    NS_TEST_VEC3(m.GetRow(1), nsVec3T(8, 10, 12), 0.0001f);
    NS_TEST_VEC3(m.GetRow(2), nsVec3T(14, 16, 18), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator+(mat, mat) | operator-(mat, mat)")
  {
    nsMat3T m0 = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);

    nsMat3T m1 = nsMat3T::MakeFromValues(-1, -2, -3, -4, -5, -6, -7, -8, -9);

    NS_TEST_BOOL((m0 + m1).IsZero());
    NS_TEST_BOOL((m0 - m1).IsEqual(m0 * (nsMathTestType)2, 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator== (mat, mat) | operator!= (mat, mat)")
  {
    nsMat3T m = nsMat3T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);

    nsMat3T m2 = m;

    NS_TEST_BOOL(m == m2);

    m2.m_fElementsCM[0] += 0.00001f;

    NS_TEST_BOOL(m != m2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsNaN")
  {
    if (nsMath::SupportsNaN<nsMathTestType>())
    {
      nsMat3T m;

      m.SetIdentity();
      NS_TEST_BOOL(!m.IsNaN());

      for (nsUInt32 i = 0; i < 9; ++i)
      {
        m.SetIdentity();
        m.m_fElementsCM[i] = nsMath::NaN<nsMathTestType>();

        NS_TEST_BOOL(m.IsNaN());
      }
    }
  }
}
