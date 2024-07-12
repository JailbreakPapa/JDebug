#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/SimdMath/SimdTransform.h>

NS_CREATE_SIMPLE_TEST(SimdMath, SimdMat4f)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeFromColumnMajorArray / MakeFromRowMajorArray")
  {
    const float data[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    {
      nsSimdMat4f m = nsSimdMat4f::MakeFromColumnMajorArray(data);

      NS_TEST_BOOL((m.m_col0 == nsSimdVec4f(1, 2, 3, 4)).AllSet());
      NS_TEST_BOOL((m.m_col1 == nsSimdVec4f(5, 6, 7, 8)).AllSet());
      NS_TEST_BOOL((m.m_col2 == nsSimdVec4f(9, 10, 11, 12)).AllSet());
      NS_TEST_BOOL((m.m_col3 == nsSimdVec4f(13, 14, 15, 16)).AllSet());
    }

    {
      nsSimdMat4f m = nsSimdMat4f::MakeFromRowMajorArray(data);

      NS_TEST_BOOL((m.m_col0 == nsSimdVec4f(1, 5, 9, 13)).AllSet());
      NS_TEST_BOOL((m.m_col1 == nsSimdVec4f(2, 6, 10, 14)).AllSet());
      NS_TEST_BOOL((m.m_col2 == nsSimdVec4f(3, 7, 11, 15)).AllSet());
      NS_TEST_BOOL((m.m_col3 == nsSimdVec4f(4, 8, 12, 16)).AllSet());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeFromColumns")
  {
    nsSimdVec4f c0(1, 2, 3, 4);
    nsSimdVec4f c1(5, 6, 7, 8);
    nsSimdVec4f c2(9, 10, 11, 12);
    nsSimdVec4f c3(13, 14, 15, 16);

    nsSimdMat4f m = nsSimdMat4f::MakeFromColumns(c0, c1, c2, c3);

    NS_TEST_BOOL((m.m_col0 == nsSimdVec4f(1, 2, 3, 4)).AllSet());
    NS_TEST_BOOL((m.m_col1 == nsSimdVec4f(5, 6, 7, 8)).AllSet());
    NS_TEST_BOOL((m.m_col2 == nsSimdVec4f(9, 10, 11, 12)).AllSet());
    NS_TEST_BOOL((m.m_col3 == nsSimdVec4f(13, 14, 15, 16)).AllSet());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetFromArray")
  {
    const float data[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    {
      nsSimdMat4f m = nsSimdMat4f::MakeFromColumnMajorArray(data);

      NS_TEST_BOOL((m.m_col0 == nsSimdVec4f(1, 2, 3, 4)).AllSet());
      NS_TEST_BOOL((m.m_col1 == nsSimdVec4f(5, 6, 7, 8)).AllSet());
      NS_TEST_BOOL((m.m_col2 == nsSimdVec4f(9, 10, 11, 12)).AllSet());
      NS_TEST_BOOL((m.m_col3 == nsSimdVec4f(13, 14, 15, 16)).AllSet());
    }

    {
      nsSimdMat4f m = nsSimdMat4f::MakeFromRowMajorArray(data);

      NS_TEST_BOOL((m.m_col0 == nsSimdVec4f(1, 5, 9, 13)).AllSet());
      NS_TEST_BOOL((m.m_col1 == nsSimdVec4f(2, 6, 10, 14)).AllSet());
      NS_TEST_BOOL((m.m_col2 == nsSimdVec4f(3, 7, 11, 15)).AllSet());
      NS_TEST_BOOL((m.m_col3 == nsSimdVec4f(4, 8, 12, 16)).AllSet());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetAsArray")
  {
    nsSimdMat4f m = nsSimdMat4f::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    float data[16];

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

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeIdentity")
  {
    nsSimdMat4f m = nsSimdMat4f::MakeIdentity();

    NS_TEST_BOOL((m.m_col0 == nsSimdVec4f(1, 0, 0, 0)).AllSet());
    NS_TEST_BOOL((m.m_col1 == nsSimdVec4f(0, 1, 0, 0)).AllSet());
    NS_TEST_BOOL((m.m_col2 == nsSimdVec4f(0, 0, 1, 0)).AllSet());
    NS_TEST_BOOL((m.m_col3 == nsSimdVec4f(0, 0, 0, 1)).AllSet());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeZero")
  {
    nsSimdMat4f m = nsSimdMat4f::MakeZero();

    NS_TEST_BOOL((m.m_col0 == nsSimdVec4f(0, 0, 0, 0)).AllSet());
    NS_TEST_BOOL((m.m_col1 == nsSimdVec4f(0, 0, 0, 0)).AllSet());
    NS_TEST_BOOL((m.m_col2 == nsSimdVec4f(0, 0, 0, 0)).AllSet());
    NS_TEST_BOOL((m.m_col3 == nsSimdVec4f(0, 0, 0, 0)).AllSet());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Transpose")
  {
    nsSimdMat4f m = nsSimdMat4f::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    m.Transpose();

    NS_TEST_BOOL((m.m_col0 == nsSimdVec4f(1, 2, 3, 4)).AllSet());
    NS_TEST_BOOL((m.m_col1 == nsSimdVec4f(5, 6, 7, 8)).AllSet());
    NS_TEST_BOOL((m.m_col2 == nsSimdVec4f(9, 10, 11, 12)).AllSet());
    NS_TEST_BOOL((m.m_col3 == nsSimdVec4f(13, 14, 15, 16)).AllSet());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetTranspose")
  {
    nsSimdMat4f m0 = nsSimdMat4f::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    nsSimdMat4f m = m0.GetTranspose();

    NS_TEST_BOOL((m.m_col0 == nsSimdVec4f(1, 2, 3, 4)).AllSet());
    NS_TEST_BOOL((m.m_col1 == nsSimdVec4f(5, 6, 7, 8)).AllSet());
    NS_TEST_BOOL((m.m_col2 == nsSimdVec4f(9, 10, 11, 12)).AllSet());
    NS_TEST_BOOL((m.m_col3 == nsSimdVec4f(13, 14, 15, 16)).AllSet());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Invert")
  {
    for (float x = 1.0f; x < 360.0f; x += 20.0f)
    {
      for (float y = 2.0f; y < 360.0f; y += 27.0f)
      {
        for (float z = 3.0f; z < 360.0f; z += 33.0f)
        {
          nsSimdQuat q = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(x, y, z).GetNormalized<3>(), nsAngle::MakeFromDegree(19.0f));

          nsSimdTransform t(q);

          nsSimdMat4f m, inv;
          m = t.GetAsMat4();
          inv = m;
          NS_TEST_BOOL(inv.Invert() == NS_SUCCESS);

          nsSimdVec4f v = m.TransformDirection(nsSimdVec4f(1, 3, -10));
          nsSimdVec4f vinv = inv.TransformDirection(v);

          NS_TEST_BOOL(vinv.IsEqual(nsSimdVec4f(1, 3, -10), nsMath::DefaultEpsilon<float>()).AllSet<3>());
        }
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetInverse")
  {
    for (float x = 1.0f; x < 360.0f; x += 19.0f)
    {
      for (float y = 2.0f; y < 360.0f; y += 29.0f)
      {
        for (float z = 3.0f; z < 360.0f; z += 31.0f)
        {
          nsSimdQuat q = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(x, y, z).GetNormalized<3>(), nsAngle::MakeFromDegree(83.0f));

          nsSimdTransform t(q);

          nsSimdMat4f m, inv;
          m = t.GetAsMat4();
          inv = m.GetInverse();

          nsSimdVec4f v = m.TransformDirection(nsSimdVec4f(1, 3, -10));
          nsSimdVec4f vinv = inv.TransformDirection(v);

          NS_TEST_BOOL(vinv.IsEqual(nsSimdVec4f(1, 3, -10), nsMath::DefaultEpsilon<float>()).AllSet<3>());
        }
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqual")
  {
    nsSimdMat4f m = nsSimdMat4f::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    nsSimdMat4f m2 = m;

    NS_TEST_BOOL(m.IsEqual(m2, 0.0001f));

    m2.m_col0 += nsSimdVec4f(0.00001f);
    NS_TEST_BOOL(m.IsEqual(m2, 0.0001f));
    NS_TEST_BOOL(!m.IsEqual(m2, 0.000001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsIdentity")
  {
    nsSimdMat4f m = nsSimdMat4f::MakeIdentity();

    NS_TEST_BOOL(m.IsIdentity());

    m.m_col0.SetZero();
    NS_TEST_BOOL(!m.IsIdentity());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsValid")
  {
    nsSimdMat4f m = nsSimdMat4f::MakeIdentity();

    NS_TEST_BOOL(m.IsValid());

    m.m_col0.SetX(nsMath::NaN<float>());
    NS_TEST_BOOL(!m.IsValid());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsNaN")
  {
    nsSimdMat4f m = nsSimdMat4f::MakeIdentity();

    NS_TEST_BOOL(!m.IsNaN());

    float data[16];

    for (nsUInt32 i = 0; i < 16; ++i)
    {
      m = nsSimdMat4f::MakeIdentity();
      m.GetAsArray(data, nsMatrixLayout::ColumnMajor);
      data[i] = nsMath::NaN<float>();
      m = nsSimdMat4f::MakeFromColumnMajorArray(data);

      NS_TEST_BOOL(m.IsNaN());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetRows")
  {
    nsSimdVec4f r0(1, 2, 3, 4);
    nsSimdVec4f r1(5, 6, 7, 8);
    nsSimdVec4f r2(9, 10, 11, 12);
    nsSimdVec4f r3(13, 14, 15, 16);

    nsSimdMat4f m;
    m.SetRows(r0, r1, r2, r3);

    NS_TEST_BOOL((m.m_col0 == nsSimdVec4f(1, 5, 9, 13)).AllSet());
    NS_TEST_BOOL((m.m_col1 == nsSimdVec4f(2, 6, 10, 14)).AllSet());
    NS_TEST_BOOL((m.m_col2 == nsSimdVec4f(3, 7, 11, 15)).AllSet());
    NS_TEST_BOOL((m.m_col3 == nsSimdVec4f(4, 8, 12, 16)).AllSet());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetRows")
  {
    nsSimdMat4f m = nsSimdMat4f::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    nsSimdVec4f r0, r1, r2, r3;
    m.GetRows(r0, r1, r2, r3);

    NS_TEST_BOOL((r0 == nsSimdVec4f(1, 2, 3, 4)).AllSet());
    NS_TEST_BOOL((r1 == nsSimdVec4f(5, 6, 7, 8)).AllSet());
    NS_TEST_BOOL((r2 == nsSimdVec4f(9, 10, 11, 12)).AllSet());
    NS_TEST_BOOL((r3 == nsSimdVec4f(13, 14, 15, 16)).AllSet());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TransformPosition")
  {
    nsSimdMat4f m = nsSimdMat4f::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    const nsSimdVec4f r = m.TransformPosition(nsSimdVec4f(1, 2, 3));

    NS_TEST_BOOL(r.IsEqual(nsSimdVec4f(1 * 1 + 2 * 2 + 3 * 3 + 4, 1 * 5 + 2 * 6 + 3 * 7 + 8, 1 * 9 + 2 * 10 + 3 * 11 + 12), 0.0001f).AllSet<3>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TransformDirection")
  {
    nsSimdMat4f m = nsSimdMat4f::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    const nsSimdVec4f r = m.TransformDirection(nsSimdVec4f(1, 2, 3));

    NS_TEST_BOOL(r.IsEqual(nsSimdVec4f(1 * 1 + 2 * 2 + 3 * 3, 1 * 5 + 2 * 6 + 3 * 7, 1 * 9 + 2 * 10 + 3 * 11), 0.0001f).AllSet<3>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator*(mat, mat)")
  {
    nsSimdMat4f m1 = nsSimdMat4f::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    nsSimdMat4f m2 = nsSimdMat4f::MakeFromValues(-1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16);

    nsSimdMat4f r = m1 * m2;

    NS_TEST_BOOL((r.m_col0 == nsSimdVec4f(-1 * 1 + -5 * 2 + -9 * 3 + -13 * 4, -1 * 5 + -5 * 6 + -9 * 7 + -13 * 8,
                                -1 * 9 + -5 * 10 + -9 * 11 + -13 * 12, -1 * 13 + -5 * 14 + -9 * 15 + -13 * 16))
                   .AllSet());
    NS_TEST_BOOL((r.m_col1 == nsSimdVec4f(-2 * 1 + -6 * 2 + -10 * 3 + -14 * 4, -2 * 5 + -6 * 6 + -10 * 7 + -14 * 8,
                                -2 * 9 + -6 * 10 + -10 * 11 + -14 * 12, -2 * 13 + -6 * 14 + -10 * 15 + -14 * 16))
                   .AllSet());
    NS_TEST_BOOL((r.m_col2 == nsSimdVec4f(-3 * 1 + -7 * 2 + -11 * 3 + -15 * 4, -3 * 5 + -7 * 6 + -11 * 7 + -15 * 8,
                                -3 * 9 + -7 * 10 + -11 * 11 + -15 * 12, -3 * 13 + -7 * 14 + -11 * 15 + -15 * 16))
                   .AllSet());
    NS_TEST_BOOL((r.m_col3 == nsSimdVec4f(-4 * 1 + -8 * 2 + -12 * 3 + -16 * 4, -4 * 5 + -8 * 6 + -12 * 7 + -16 * 8,
                                -4 * 9 + -8 * 10 + -12 * 11 + -16 * 12, -4 * 13 + -8 * 14 + -12 * 15 + -16 * 16))
                   .AllSet());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator== (mat, mat) | operator!= (mat, mat)")
  {
    nsSimdMat4f m = nsSimdMat4f::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    nsSimdMat4f m2 = m;

    NS_TEST_BOOL(m == m2);

    m2.m_col0 += nsSimdVec4f(0.00001f);

    NS_TEST_BOOL(m != m2);
  }
}
