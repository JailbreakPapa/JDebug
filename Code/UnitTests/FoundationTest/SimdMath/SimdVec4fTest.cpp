#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/Vec4.h>
#include <Foundation/SimdMath/SimdVec4f.h>

namespace
{
  static bool AllCompSame(const nsSimdFloat& a)
  {
    // Make sure all components are the same
    nsSimdVec4f test;
    test.m_v = a.m_v;
    return test.x() == test.y() && test.x() == test.z() && test.x() == test.w();
  }

  template <nsMathAcc::Enum acc>
  static void TestLength(const nsSimdVec4f& a, float r[4], const nsSimdFloat& fEps)
  {
    nsSimdFloat l1 = a.GetLength<1, acc>();
    nsSimdFloat l2 = a.GetLength<2, acc>();
    nsSimdFloat l3 = a.GetLength<3, acc>();
    nsSimdFloat l4 = a.GetLength<4, acc>();
    NS_TEST_FLOAT(l1, r[0], fEps);
    NS_TEST_FLOAT(l2, r[1], fEps);
    NS_TEST_FLOAT(l3, r[2], fEps);
    NS_TEST_FLOAT(l4, r[3], fEps);
    NS_TEST_BOOL(AllCompSame(l1));
    NS_TEST_BOOL(AllCompSame(l2));
    NS_TEST_BOOL(AllCompSame(l3));
    NS_TEST_BOOL(AllCompSame(l4));
  }

  template <nsMathAcc::Enum acc>
  static void TestInvLength(const nsSimdVec4f& a, float r[4], const nsSimdFloat& fEps)
  {
    nsSimdFloat l1 = a.GetInvLength<1, acc>();
    nsSimdFloat l2 = a.GetInvLength<2, acc>();
    nsSimdFloat l3 = a.GetInvLength<3, acc>();
    nsSimdFloat l4 = a.GetInvLength<4, acc>();
    NS_TEST_FLOAT(l1, r[0], fEps);
    NS_TEST_FLOAT(l2, r[1], fEps);
    NS_TEST_FLOAT(l3, r[2], fEps);
    NS_TEST_FLOAT(l4, r[3], fEps);
    NS_TEST_BOOL(AllCompSame(l1));
    NS_TEST_BOOL(AllCompSame(l2));
    NS_TEST_BOOL(AllCompSame(l3));
    NS_TEST_BOOL(AllCompSame(l4));
  }

  template <nsMathAcc::Enum acc>
  static void TestNormalize(const nsSimdVec4f& a, nsSimdVec4f n[4], nsSimdFloat r[4], const nsSimdFloat& fEps)
  {
    nsSimdVec4f n1 = a.GetNormalized<1, acc>();
    nsSimdVec4f n2 = a.GetNormalized<2, acc>();
    nsSimdVec4f n3 = a.GetNormalized<3, acc>();
    nsSimdVec4f n4 = a.GetNormalized<4, acc>();
    NS_TEST_BOOL(n1.IsEqual(n[0], fEps).AllSet());
    NS_TEST_BOOL(n2.IsEqual(n[1], fEps).AllSet());
    NS_TEST_BOOL(n3.IsEqual(n[2], fEps).AllSet());
    NS_TEST_BOOL(n4.IsEqual(n[3], fEps).AllSet());

    nsSimdVec4f a1 = a;
    nsSimdVec4f a2 = a;
    nsSimdVec4f a3 = a;
    nsSimdVec4f a4 = a;

    nsSimdFloat l1 = a1.GetLengthAndNormalize<1, acc>();
    nsSimdFloat l2 = a2.GetLengthAndNormalize<2, acc>();
    nsSimdFloat l3 = a3.GetLengthAndNormalize<3, acc>();
    nsSimdFloat l4 = a4.GetLengthAndNormalize<4, acc>();
    NS_TEST_FLOAT(l1, r[0], fEps);
    NS_TEST_FLOAT(l2, r[1], fEps);
    NS_TEST_FLOAT(l3, r[2], fEps);
    NS_TEST_FLOAT(l4, r[3], fEps);
    NS_TEST_BOOL(AllCompSame(l1));
    NS_TEST_BOOL(AllCompSame(l2));
    NS_TEST_BOOL(AllCompSame(l3));
    NS_TEST_BOOL(AllCompSame(l4));

    NS_TEST_BOOL(a1.IsEqual(n[0], fEps).AllSet());
    NS_TEST_BOOL(a2.IsEqual(n[1], fEps).AllSet());
    NS_TEST_BOOL(a3.IsEqual(n[2], fEps).AllSet());
    NS_TEST_BOOL(a4.IsEqual(n[3], fEps).AllSet());

    NS_TEST_BOOL(a1.IsNormalized<1>(fEps));
    NS_TEST_BOOL(a2.IsNormalized<2>(fEps));
    NS_TEST_BOOL(a3.IsNormalized<3>(fEps));
    NS_TEST_BOOL(a4.IsNormalized<4>(fEps));
    NS_TEST_BOOL(!a1.IsNormalized<2>(fEps));
    NS_TEST_BOOL(!a2.IsNormalized<3>(fEps));
    NS_TEST_BOOL(!a3.IsNormalized<4>(fEps));

    a1 = a;
    a1.Normalize<1, acc>();
    a2 = a;
    a2.Normalize<2, acc>();
    a3 = a;
    a3.Normalize<3, acc>();
    a4 = a;
    a4.Normalize<4, acc>();
    NS_TEST_BOOL(a1.IsEqual(n[0], fEps).AllSet());
    NS_TEST_BOOL(a2.IsEqual(n[1], fEps).AllSet());
    NS_TEST_BOOL(a3.IsEqual(n[2], fEps).AllSet());
    NS_TEST_BOOL(a4.IsEqual(n[3], fEps).AllSet());
  }

  template <nsMathAcc::Enum acc>
  static void TestNormalizeIfNotZero(const nsSimdVec4f& a, nsSimdVec4f n[4], const nsSimdFloat& fEps)
  {
    nsSimdVec4f a1 = a;
    a1.NormalizeIfNotZero<1>(fEps);
    nsSimdVec4f a2 = a;
    a2.NormalizeIfNotZero<2>(fEps);
    nsSimdVec4f a3 = a;
    a3.NormalizeIfNotZero<3>(fEps);
    nsSimdVec4f a4 = a;
    a4.NormalizeIfNotZero<4>(fEps);
    NS_TEST_BOOL(a1.IsEqual(n[0], fEps).AllSet());
    NS_TEST_BOOL(a2.IsEqual(n[1], fEps).AllSet());
    NS_TEST_BOOL(a3.IsEqual(n[2], fEps).AllSet());
    NS_TEST_BOOL(a4.IsEqual(n[3], fEps).AllSet());

    NS_TEST_BOOL(a1.IsNormalized<1>(fEps));
    NS_TEST_BOOL(a2.IsNormalized<2>(fEps));
    NS_TEST_BOOL(a3.IsNormalized<3>(fEps));
    NS_TEST_BOOL(a4.IsNormalized<4>(fEps));
    NS_TEST_BOOL(!a1.IsNormalized<2>(fEps));
    NS_TEST_BOOL(!a2.IsNormalized<3>(fEps));
    NS_TEST_BOOL(!a3.IsNormalized<4>(fEps));

    nsSimdVec4f b(fEps);
    b.NormalizeIfNotZero<4>(fEps);
    NS_TEST_BOOL(b.IsZero<4>());
  }
} // namespace

NS_CREATE_SIMPLE_TEST(SimdMath, SimdVec4f)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    // In debug the default constructor initializes everything with NaN.
    nsSimdVec4f vDefCtor;
    NS_TEST_BOOL(vDefCtor.IsNaN<4>());
#else
// GCC assumes that the contents of the memory prior to the placement constructor doesn't matter
// So it optimizes away the initialization.
#  if NS_DISABLED(NS_COMPILER_GCC)
    // Placement new of the default constructor should not have any effect on the previous data.
    alignas(16) float testBlock[4] = {1, 2, 3, 4};
    nsSimdVec4f* pDefCtor = ::new ((void*)&testBlock[0]) nsSimdVec4f;
    NS_TEST_BOOL(pDefCtor->x() == 1.0f && pDefCtor->y() == 2.0f && pDefCtor->z() == 3.0f && pDefCtor->w() == 4.0f);
#  endif
#endif

    // Make sure the class didn't accidentally change in size.
#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
    NS_CHECK_AT_COMPILETIME(sizeof(nsSimdVec4f) == 16);
    NS_CHECK_AT_COMPILETIME(NS_ALIGNMENT_OF(nsSimdVec4f) == 16);
#endif

    nsSimdVec4f vInit1F(2.0f);
    NS_TEST_BOOL(vInit1F.x() == 2.0f && vInit1F.y() == 2.0f && vInit1F.z() == 2.0f && vInit1F.w() == 2.0f);

    nsSimdFloat a(3.0f);
    nsSimdVec4f vInit1SF(a);
    NS_TEST_BOOL(vInit1SF.x() == 3.0f && vInit1SF.y() == 3.0f && vInit1SF.z() == 3.0f && vInit1SF.w() == 3.0f);

    nsSimdVec4f vInit4F(1.0f, 2.0f, 3.0f, 4.0f);
    NS_TEST_BOOL(vInit4F.x() == 1.0f && vInit4F.y() == 2.0f && vInit4F.z() == 3.0f && vInit4F.w() == 4.0f);

    // Make sure all components have the correct values
#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE && NS_ENABLED(NS_COMPILER_MSVC)
    NS_TEST_BOOL(
      vInit4F.m_v.m128_f32[0] == 1.0f && vInit4F.m_v.m128_f32[1] == 2.0f && vInit4F.m_v.m128_f32[2] == 3.0f && vInit4F.m_v.m128_f32[3] == 4.0f);
#endif

    nsSimdVec4f vCopy(vInit4F);
    NS_TEST_BOOL(vCopy.x() == 1.0f && vCopy.y() == 2.0f && vCopy.z() == 3.0f && vCopy.w() == 4.0f);

    nsSimdVec4f vZero = nsSimdVec4f::MakeZero();
    NS_TEST_BOOL(vZero.x() == 0.0f && vZero.y() == 0.0f && vZero.z() == 0.0f && vZero.w() == 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Setter")
  {
    nsSimdVec4f a;
    a.Set(2.0f);
    NS_TEST_BOOL(a.x() == 2.0f && a.y() == 2.0f && a.z() == 2.0f && a.w() == 2.0f);

    nsSimdVec4f b;
    b.Set(1.0f, 2.0f, 3.0f, 4.0f);
    NS_TEST_BOOL(b.x() == 1.0f && b.y() == 2.0f && b.z() == 3.0f && b.w() == 4.0f);

    b.SetX(5.0f);
    NS_TEST_BOOL(b.x() == 5.0f && b.y() == 2.0f && b.z() == 3.0f && b.w() == 4.0f);

    b.SetY(6.0f);
    NS_TEST_BOOL(b.x() == 5.0f && b.y() == 6.0f && b.z() == 3.0f && b.w() == 4.0f);

    b.SetZ(7.0f);
    NS_TEST_BOOL(b.x() == 5.0f && b.y() == 6.0f && b.z() == 7.0f && b.w() == 4.0f);

    b.SetW(8.0f);
    NS_TEST_BOOL(b.x() == 5.0f && b.y() == 6.0f && b.z() == 7.0f && b.w() == 8.0f);

    nsSimdVec4f c;
    c.SetZero();
    NS_TEST_BOOL(c.x() == 0.0f && c.y() == 0.0f && c.z() == 0.0f && c.w() == 0.0f);

    {
      nsSimdVec4f z = nsSimdVec4f::MakeZero();
      NS_TEST_BOOL(z.x() == 0.0f && z.y() == 0.0f && z.z() == 0.0f && z.w() == 0.0f);
    }

    {
      nsSimdVec4f z = nsSimdVec4f::MakeNaN();
      NS_TEST_BOOL(nsMath::IsNaN((float)z.x()));
      NS_TEST_BOOL(nsMath::IsNaN((float)z.y()));
      NS_TEST_BOOL(nsMath::IsNaN((float)z.z()));
      NS_TEST_BOOL(nsMath::IsNaN((float)z.w()));
    }

    {
      float testBlock[4] = {1, 2, 3, 4};
      nsSimdVec4f x;
      x.Load<1>(testBlock);
      NS_TEST_BOOL(x.x() == 1.0f && x.y() == 0.0f && x.z() == 0.0f && x.w() == 0.0f);

      nsSimdVec4f xy;
      xy.Load<2>(testBlock);
      NS_TEST_BOOL(xy.x() == 1.0f && xy.y() == 2.0f && xy.z() == 0.0f && xy.w() == 0.0f);

      nsSimdVec4f xyz;
      xyz.Load<3>(testBlock);
      NS_TEST_BOOL(xyz.x() == 1.0f && xyz.y() == 2.0f && xyz.z() == 3.0f && xyz.w() == 0.0f);

      nsSimdVec4f xyzw;
      xyzw.Load<4>(testBlock);
      NS_TEST_BOOL(xyzw.x() == 1.0f && xyzw.y() == 2.0f && xyzw.z() == 3.0f && xyzw.w() == 4.0f);

      NS_TEST_BOOL(xyzw.GetComponent(0) == 1.0f);
      NS_TEST_BOOL(xyzw.GetComponent(1) == 2.0f);
      NS_TEST_BOOL(xyzw.GetComponent(2) == 3.0f);
      NS_TEST_BOOL(xyzw.GetComponent(3) == 4.0f);
      NS_TEST_BOOL(xyzw.GetComponent(4) == 4.0f);

      // Make sure all components have the correct values
#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE && NS_ENABLED(NS_COMPILER_MSVC)
      NS_TEST_BOOL(xyzw.m_v.m128_f32[0] == 1.0f && xyzw.m_v.m128_f32[1] == 2.0f && xyzw.m_v.m128_f32[2] == 3.0f && xyzw.m_v.m128_f32[3] == 4.0f);
#endif
    }

    {
      float testBlock[4] = {7, 7, 7, 7};
      float mem[4] = {};

      nsSimdVec4f b2(1, 2, 3, 4);

      memcpy(mem, testBlock, 16);
      b2.Store<1>(mem);
      NS_TEST_BOOL(mem[0] == 1.0f && mem[1] == 7.0f && mem[2] == 7.0f && mem[3] == 7.0f);

      memcpy(mem, testBlock, 16);
      b2.Store<2>(mem);
      NS_TEST_BOOL(mem[0] == 1.0f && mem[1] == 2.0f && mem[2] == 7.0f && mem[3] == 7.0f);

      memcpy(mem, testBlock, 16);
      b2.Store<3>(mem);
      NS_TEST_BOOL(mem[0] == 1.0f && mem[1] == 2.0f && mem[2] == 3.0f && mem[3] == 7.0f);

      memcpy(mem, testBlock, 16);
      b2.Store<4>(mem);
      NS_TEST_BOOL(mem[0] == 1.0f && mem[1] == 2.0f && mem[2] == 3.0f && mem[3] == 4.0f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Functions")
  {
    {
      nsSimdVec4f a(1.0f, 2.0f, 4.0f, 8.0f);
      nsSimdVec4f b(1.0f, 0.5f, 0.25f, 0.125f);

      NS_TEST_BOOL(a.GetReciprocal().IsEqual(b, nsMath::SmallEpsilon<float>()).AllSet());
      NS_TEST_BOOL(a.GetReciprocal<nsMathAcc::FULL>().IsEqual(b, nsMath::SmallEpsilon<float>()).AllSet());
      NS_TEST_BOOL(a.GetReciprocal<nsMathAcc::BITS_23>().IsEqual(b, nsMath::DefaultEpsilon<float>()).AllSet());
      NS_TEST_BOOL(a.GetReciprocal<nsMathAcc::BITS_12>().IsEqual(b, nsMath::HugeEpsilon<float>()).AllSet());
    }

    {
      nsSimdVec4f a(1.0f, 2.0f, 4.0f, 8.0f);
      nsSimdVec4f b(1.0f, nsMath::Sqrt(2.0f), nsMath::Sqrt(4.0f), nsMath::Sqrt(8.0f));

      NS_TEST_BOOL(a.GetSqrt().IsEqual(b, nsMath::SmallEpsilon<float>()).AllSet());
      NS_TEST_BOOL(a.GetSqrt<nsMathAcc::FULL>().IsEqual(b, nsMath::SmallEpsilon<float>()).AllSet());
      NS_TEST_BOOL(a.GetSqrt<nsMathAcc::BITS_23>().IsEqual(b, nsMath::DefaultEpsilon<float>()).AllSet());
      NS_TEST_BOOL(a.GetSqrt<nsMathAcc::BITS_12>().IsEqual(b, nsMath::HugeEpsilon<float>()).AllSet());
    }

    {
      nsSimdVec4f a(1.0f, 2.0f, 4.0f, 8.0f);
      nsSimdVec4f b(1.0f, 1.0f / nsMath::Sqrt(2.0f), 1.0f / nsMath::Sqrt(4.0f), 1.0f / nsMath::Sqrt(8.0f));

      NS_TEST_BOOL(a.GetInvSqrt().IsEqual(b, nsMath::SmallEpsilon<float>()).AllSet());
      NS_TEST_BOOL(a.GetInvSqrt<nsMathAcc::FULL>().IsEqual(b, nsMath::SmallEpsilon<float>()).AllSet());
      NS_TEST_BOOL(a.GetInvSqrt<nsMathAcc::BITS_23>().IsEqual(b, nsMath::DefaultEpsilon<float>()).AllSet());
      NS_TEST_BOOL(a.GetInvSqrt<nsMathAcc::BITS_12>().IsEqual(b, nsMath::HugeEpsilon<float>()).AllSet());
    }

    {
      nsSimdVec4f a(2.0f, -2.0f, 4.0f, -8.0f);
      float r[4];
      r[0] = 2.0f;
      r[1] = nsVec2(a.x(), a.y()).GetLength();
      r[2] = nsVec3(a.x(), a.y(), a.z()).GetLength();
      r[3] = nsVec4(a.x(), a.y(), a.z(), a.w()).GetLength();

      NS_TEST_FLOAT(a.GetLength<1>(), r[0], nsMath::SmallEpsilon<float>());
      NS_TEST_FLOAT(a.GetLength<2>(), r[1], nsMath::SmallEpsilon<float>());
      NS_TEST_FLOAT(a.GetLength<3>(), r[2], nsMath::SmallEpsilon<float>());
      NS_TEST_FLOAT(a.GetLength<4>(), r[3], nsMath::SmallEpsilon<float>());

      TestLength<nsMathAcc::FULL>(a, r, nsMath::SmallEpsilon<float>());
      TestLength<nsMathAcc::BITS_23>(a, r, nsMath::DefaultEpsilon<float>());
      TestLength<nsMathAcc::BITS_12>(a, r, 0.01f);
    }

    {
      nsSimdVec4f a(2.0f, -2.0f, 4.0f, -8.0f);
      float r[4];
      r[0] = 0.5f;
      r[1] = 1.0f / nsVec2(a.x(), a.y()).GetLength();
      r[2] = 1.0f / nsVec3(a.x(), a.y(), a.z()).GetLength();
      r[3] = 1.0f / nsVec4(a.x(), a.y(), a.z(), a.w()).GetLength();

      NS_TEST_FLOAT(a.GetInvLength<1>(), r[0], nsMath::SmallEpsilon<float>());
      NS_TEST_FLOAT(a.GetInvLength<2>(), r[1], nsMath::SmallEpsilon<float>());
      NS_TEST_FLOAT(a.GetInvLength<3>(), r[2], nsMath::SmallEpsilon<float>());
      NS_TEST_FLOAT(a.GetInvLength<4>(), r[3], nsMath::SmallEpsilon<float>());

      TestInvLength<nsMathAcc::FULL>(a, r, nsMath::SmallEpsilon<float>());
      TestInvLength<nsMathAcc::BITS_23>(a, r, nsMath::DefaultEpsilon<float>());
      TestInvLength<nsMathAcc::BITS_12>(a, r, nsMath::HugeEpsilon<float>());
    }

    {
      nsSimdVec4f a(2.0f, -2.0f, 4.0f, -8.0f);
      float r[4];
      r[0] = 2.0f * 2.0f;
      r[1] = nsVec2(a.x(), a.y()).GetLengthSquared();
      r[2] = nsVec3(a.x(), a.y(), a.z()).GetLengthSquared();
      r[3] = nsVec4(a.x(), a.y(), a.z(), a.w()).GetLengthSquared();

      NS_TEST_FLOAT(a.GetLengthSquared<1>(), r[0], nsMath::SmallEpsilon<float>());
      NS_TEST_FLOAT(a.GetLengthSquared<2>(), r[1], nsMath::SmallEpsilon<float>());
      NS_TEST_FLOAT(a.GetLengthSquared<3>(), r[2], nsMath::SmallEpsilon<float>());
      NS_TEST_FLOAT(a.GetLengthSquared<4>(), r[3], nsMath::SmallEpsilon<float>());
    }

    {
      nsSimdVec4f a(2.0f, -2.0f, 4.0f, -8.0f);
      nsSimdFloat r[4];
      r[0] = 2.0f;
      r[1] = nsVec2(a.x(), a.y()).GetLength();
      r[2] = nsVec3(a.x(), a.y(), a.z()).GetLength();
      r[3] = nsVec4(a.x(), a.y(), a.z(), a.w()).GetLength();

      nsSimdVec4f n[4];
      n[0] = a / r[0];
      n[1] = a / r[1];
      n[2] = a / r[2];
      n[3] = a / r[3];

      TestNormalize<nsMathAcc::FULL>(a, n, r, nsMath::SmallEpsilon<float>());
      TestNormalize<nsMathAcc::BITS_23>(a, n, r, nsMath::DefaultEpsilon<float>());
      TestNormalize<nsMathAcc::BITS_12>(a, n, r, 0.01f);
    }

    {
      nsSimdVec4f a(2.0f, -2.0f, 4.0f, -8.0f);
      nsSimdVec4f n[4];
      n[0] = a / 2.0f;
      n[1] = a / nsVec2(a.x(), a.y()).GetLength();
      n[2] = a / nsVec3(a.x(), a.y(), a.z()).GetLength();
      n[3] = a / nsVec4(a.x(), a.y(), a.z(), a.w()).GetLength();

      TestNormalizeIfNotZero<nsMathAcc::FULL>(a, n, nsMath::SmallEpsilon<float>());
      TestNormalizeIfNotZero<nsMathAcc::BITS_23>(a, n, nsMath::DefaultEpsilon<float>());
      TestNormalizeIfNotZero<nsMathAcc::BITS_12>(a, n, nsMath::HugeEpsilon<float>());
    }

    {
      nsSimdVec4f a;

      a.Set(0.0f, 2.0f, 0.0f, 0.0f);
      NS_TEST_BOOL(a.IsZero<1>());
      NS_TEST_BOOL(!a.IsZero<2>());

      a.Set(0.0f, 0.0f, 3.0f, 0.0f);
      NS_TEST_BOOL(a.IsZero<2>());
      NS_TEST_BOOL(!a.IsZero<3>());

      a.Set(0.0f, 0.0f, 0.0f, 4.0f);
      NS_TEST_BOOL(a.IsZero<3>());
      NS_TEST_BOOL(!a.IsZero<4>());

      float smallEps = nsMath::SmallEpsilon<float>();
      a.Set(smallEps, 2.0f, smallEps, smallEps);
      NS_TEST_BOOL(a.IsZero<1>(nsMath::DefaultEpsilon<float>()));
      NS_TEST_BOOL(!a.IsZero<2>(nsMath::DefaultEpsilon<float>()));

      a.Set(smallEps, smallEps, 3.0f, smallEps);
      NS_TEST_BOOL(a.IsZero<2>(nsMath::DefaultEpsilon<float>()));
      NS_TEST_BOOL(!a.IsZero<3>(nsMath::DefaultEpsilon<float>()));

      a.Set(smallEps, smallEps, smallEps, 4.0f);
      NS_TEST_BOOL(a.IsZero<3>(nsMath::DefaultEpsilon<float>()));
      NS_TEST_BOOL(!a.IsZero<4>(nsMath::DefaultEpsilon<float>()));
    }

    {
      nsSimdVec4f a;

      float NaN = nsMath::NaN<float>();
      float Inf = nsMath::Infinity<float>();

      a.Set(NaN, 1.0f, NaN, NaN);
      NS_TEST_BOOL(a.IsNaN<1>());
      NS_TEST_BOOL(a.IsNaN<2>());
      NS_TEST_BOOL(!a.IsValid<2>());

      a.Set(Inf, 1.0f, NaN, NaN);
      NS_TEST_BOOL(!a.IsNaN<1>());
      NS_TEST_BOOL(!a.IsNaN<2>());
      NS_TEST_BOOL(!a.IsValid<2>());

      a.Set(1.0f, 2.0f, Inf, NaN);
      NS_TEST_BOOL(a.IsNaN<4>());
      NS_TEST_BOOL(!a.IsNaN<3>());
      NS_TEST_BOOL(a.IsValid<2>());
      NS_TEST_BOOL(!a.IsValid<3>());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Swizzle")
  {
    nsSimdVec4f a(3.0f, 5.0f, 7.0f, 9.0f);

    nsSimdVec4f b = a.Get<nsSwizzle::XXXX>();
    NS_TEST_BOOL(b.x() == 3.0f && b.y() == 3.0f && b.z() == 3.0f && b.w() == 3.0f);

    b = a.Get<nsSwizzle::YYYX>();
    NS_TEST_BOOL(b.x() == 5.0f && b.y() == 5.0f && b.z() == 5.0f && b.w() == 3.0f);

    b = a.Get<nsSwizzle::ZZZX>();
    NS_TEST_BOOL(b.x() == 7.0f && b.y() == 7.0f && b.z() == 7.0f && b.w() == 3.0f);

    b = a.Get<nsSwizzle::WWWX>();
    NS_TEST_BOOL(b.x() == 9.0f && b.y() == 9.0f && b.z() == 9.0f && b.w() == 3.0f);

    b = a.Get<nsSwizzle::WZYX>();
    NS_TEST_BOOL(b.x() == 9.0f && b.y() == 7.0f && b.z() == 5.0f && b.w() == 3.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Operators")
  {
    {
      nsSimdVec4f a(-3.0f, 5.0f, -7.0f, 9.0f);

      nsSimdVec4f b = -a;
      NS_TEST_BOOL(b.x() == 3.0f && b.y() == -5.0f && b.z() == 7.0f && b.w() == -9.0f);

      b.Set(8.0f, 6.0f, 4.0f, 2.0f);
      nsSimdVec4f c;
      c = a + b;
      NS_TEST_BOOL(c.x() == 5.0f && c.y() == 11.0f && c.z() == -3.0f && c.w() == 11.0f);

      c = a - b;
      NS_TEST_BOOL(c.x() == -11.0f && c.y() == -1.0f && c.z() == -11.0f && c.w() == 7.0f);

      c = a * nsSimdFloat(3.0f);
      NS_TEST_BOOL(c.x() == -9.0f && c.y() == 15.0f && c.z() == -21.0f && c.w() == 27.0f);

      c = a / nsSimdFloat(2.0f);
      NS_TEST_BOOL(c.x() == -1.5f && c.y() == 2.5f && c.z() == -3.5f && c.w() == 4.5f);

      c = a.CompMul(b);
      NS_TEST_BOOL(c.x() == -24.0f && c.y() == 30.0f && c.z() == -28.0f && c.w() == 18.0f);

      nsSimdVec4f divRes(-0.375f, 5.0f / 6.0f, -1.75f, 4.5f);
      nsSimdVec4f d1 = a.CompDiv(b);
      nsSimdVec4f d2 = a.CompDiv<nsMathAcc::FULL>(b);
      nsSimdVec4f d3 = a.CompDiv<nsMathAcc::BITS_23>(b);
      nsSimdVec4f d4 = a.CompDiv<nsMathAcc::BITS_12>(b);

      NS_TEST_BOOL(d1.IsEqual(divRes, nsMath::SmallEpsilon<float>()).AllSet());
      NS_TEST_BOOL(d2.IsEqual(divRes, nsMath::SmallEpsilon<float>()).AllSet());
      NS_TEST_BOOL(d3.IsEqual(divRes, nsMath::DefaultEpsilon<float>()).AllSet());
      NS_TEST_BOOL(d4.IsEqual(divRes, 0.01f).AllSet());
    }

    {
      nsSimdVec4f a(-3.4f, 5.4f, -7.6f, 9.6f);
      nsSimdVec4f b(8.0f, 6.0f, 4.0f, 2.0f);
      nsSimdVec4f c;

      c = a.CompMin(b);
      NS_TEST_BOOL(c.x() == -3.4f && c.y() == 5.4f && c.z() == -7.6f && c.w() == 2.0f);

      c = a.CompMax(b);
      NS_TEST_BOOL(c.x() == 8.0f && c.y() == 6.0f && c.z() == 4.0f && c.w() == 9.6f);

      c = a.Abs();
      NS_TEST_BOOL(c.x() == 3.4f && c.y() == 5.4f && c.z() == 7.6f && c.w() == 9.6f);

      c = a.Round();
      NS_TEST_BOOL(c.x() == -3.0f && c.y() == 5.0f && c.z() == -8.0f && c.w() == 10.0f);

      c = a.Floor();
      NS_TEST_BOOL(c.x() == -4.0f && c.y() == 5.0f && c.z() == -8.0f && c.w() == 9.0f);

      c = a.Ceil();
      NS_TEST_BOOL(c.x() == -3.0f && c.y() == 6.0f && c.z() == -7.0f && c.w() == 10.0f);

      c = a.Trunc();
      NS_TEST_BOOL(c.x() == -3.0f && c.y() == 5.0f && c.z() == -7.0f && c.w() == 9.0f);

      c = a.Fraction();
      NS_TEST_BOOL(c.IsEqual(nsSimdVec4f(-0.4f, 0.4f, -0.6f, 0.6f), nsMath::SmallEpsilon<float>()).AllSet());
    }

    {
      nsSimdVec4f a(-3.0f, 5.0f, -7.0f, 9.0f);
      nsSimdVec4f b(8.0f, 6.0f, 4.0f, 2.0f);

      nsSimdVec4b cmp(true, false, false, true);
      nsSimdVec4f c;

      c = a.FlipSign(cmp);
      NS_TEST_BOOL(c.x() == 3.0f && c.y() == 5.0f && c.z() == -7.0f && c.w() == -9.0f);

      c = nsSimdVec4f::Select(cmp, b, a);
      NS_TEST_BOOL(c.x() == 8.0f && c.y() == 5.0f && c.z() == -7.0f && c.w() == 2.0f);

      c = nsSimdVec4f::Select(cmp, a, b);
      NS_TEST_BOOL(c.x() == -3.0f && c.y() == 6.0f && c.z() == 4.0f && c.w() == 9.0f);
    }

    {
      nsSimdVec4f a(-3.0f, 5.0f, -7.0f, 9.0f);
      nsSimdVec4f b(8.0f, 6.0f, 4.0f, 2.0f);

      nsSimdVec4f c = a;
      c += b;
      NS_TEST_BOOL(c.x() == 5.0f && c.y() == 11.0f && c.z() == -3.0f && c.w() == 11.0f);

      c = a;
      c -= b;
      NS_TEST_BOOL(c.x() == -11.0f && c.y() == -1.0f && c.z() == -11.0f && c.w() == 7.0f);

      c = a;
      c *= nsSimdFloat(3.0f);
      NS_TEST_BOOL(c.x() == -9.0f && c.y() == 15.0f && c.z() == -21.0f && c.w() == 27.0f);

      c = a;
      c /= nsSimdFloat(2.0f);
      NS_TEST_BOOL(c.x() == -1.5f && c.y() == 2.5f && c.z() == -3.5f && c.w() == 4.5f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Comparison")
  {
    nsSimdVec4f a(7.0f, 5.0f, 4.0f, 3.0f);
    nsSimdVec4f b(8.0f, 6.0f, 4.0f, 2.0f);
    nsSimdVec4b cmp;

    cmp = a == b;
    NS_TEST_BOOL(!cmp.x() && !cmp.y() && cmp.z() && !cmp.w());

    cmp = a != b;
    NS_TEST_BOOL(cmp.x() && cmp.y() && !cmp.z() && cmp.w());

    cmp = a <= b;
    NS_TEST_BOOL(cmp.x() && cmp.y() && cmp.z() && !cmp.w());

    cmp = a < b;
    NS_TEST_BOOL(cmp.x() && cmp.y() && !cmp.z() && !cmp.w());

    cmp = a >= b;
    NS_TEST_BOOL(!cmp.x() && !cmp.y() && cmp.z() && cmp.w());

    cmp = a > b;
    NS_TEST_BOOL(!cmp.x() && !cmp.y() && !cmp.z() && cmp.w());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Advanced Operators")
  {
    {
      nsSimdVec4f a(-3.0f, 5.0f, -7.0f, 9.0f);

      NS_TEST_FLOAT(a.HorizontalSum<1>(), -3.0f, 0.0f);
      NS_TEST_FLOAT(a.HorizontalSum<2>(), 2.0f, 0.0f);
      NS_TEST_FLOAT(a.HorizontalSum<3>(), -5.0f, 0.0f);
      NS_TEST_FLOAT(a.HorizontalSum<4>(), 4.0f, 0.0f);
      NS_TEST_BOOL(AllCompSame(a.HorizontalSum<1>()));
      NS_TEST_BOOL(AllCompSame(a.HorizontalSum<2>()));
      NS_TEST_BOOL(AllCompSame(a.HorizontalSum<3>()));
      NS_TEST_BOOL(AllCompSame(a.HorizontalSum<4>()));

      NS_TEST_FLOAT(a.HorizontalMin<1>(), -3.0f, 0.0f);
      NS_TEST_FLOAT(a.HorizontalMin<2>(), -3.0f, 0.0f);
      NS_TEST_FLOAT(a.HorizontalMin<3>(), -7.0f, 0.0f);
      NS_TEST_FLOAT(a.HorizontalMin<4>(), -7.0f, 0.0f);
      NS_TEST_BOOL(AllCompSame(a.HorizontalMin<1>()));
      NS_TEST_BOOL(AllCompSame(a.HorizontalMin<2>()));
      NS_TEST_BOOL(AllCompSame(a.HorizontalMin<3>()));
      NS_TEST_BOOL(AllCompSame(a.HorizontalMin<4>()));

      NS_TEST_FLOAT(a.HorizontalMax<1>(), -3.0f, 0.0f);
      NS_TEST_FLOAT(a.HorizontalMax<2>(), 5.0f, 0.0f);
      NS_TEST_FLOAT(a.HorizontalMax<3>(), 5.0f, 0.0f);
      NS_TEST_FLOAT(a.HorizontalMax<4>(), 9.0f, 0.0f);
      NS_TEST_BOOL(AllCompSame(a.HorizontalMax<1>()));
      NS_TEST_BOOL(AllCompSame(a.HorizontalMax<2>()));
      NS_TEST_BOOL(AllCompSame(a.HorizontalMax<3>()));
      NS_TEST_BOOL(AllCompSame(a.HorizontalMax<4>()));
    }

    {
      nsSimdVec4f a(-3.0f, 5.0f, -7.0f, 9.0f);
      nsSimdVec4f b(8.0f, 6.0f, 4.0f, 2.0f);

      NS_TEST_FLOAT(a.Dot<1>(b), -24.0f, 0.0f);
      NS_TEST_FLOAT(a.Dot<2>(b), 6.0f, 0.0f);
      NS_TEST_FLOAT(a.Dot<3>(b), -22.0f, 0.0f);
      NS_TEST_FLOAT(a.Dot<4>(b), -4.0f, 0.0f);
      NS_TEST_BOOL(AllCompSame(a.Dot<1>(b)));
      NS_TEST_BOOL(AllCompSame(a.Dot<2>(b)));
      NS_TEST_BOOL(AllCompSame(a.Dot<3>(b)));
      NS_TEST_BOOL(AllCompSame(a.Dot<4>(b)));
    }

    {
      nsSimdVec4f a(1.0f, 2.0f, 3.0f, 0.0f);
      nsSimdVec4f b(2.0f, -4.0f, 6.0f, 8.0f);

      nsVec3 res = nsVec3(a.x(), a.y(), a.z()).CrossRH(nsVec3(b.x(), b.y(), b.z()));

      nsSimdVec4f c = a.CrossRH(b);
      NS_TEST_BOOL(c.x() == res.x);
      NS_TEST_BOOL(c.y() == res.y);
      NS_TEST_BOOL(c.z() == res.z);
    }

    {
      nsSimdVec4f a(1.0f, 2.0f, 3.0f, 0.0f);
      nsSimdVec4f b(2.0f, -4.0f, 6.0f, 0.0f);

      nsVec3 res = nsVec3(a.x(), a.y(), a.z()).CrossRH(nsVec3(b.x(), b.y(), b.z()));

      nsSimdVec4f c = a.CrossRH(b);
      NS_TEST_BOOL(c.x() == res.x);
      NS_TEST_BOOL(c.y() == res.y);
      NS_TEST_BOOL(c.z() == res.z);
    }

    {
      nsSimdVec4f a(-3.0f, 5.0f, -7.0f, 0.0f);
      nsSimdVec4f b = a.GetOrthogonalVector();

      NS_TEST_BOOL(!b.IsZero<3>());
      NS_TEST_FLOAT(a.Dot<3>(b), 0.0f, 0.0f);
    }

    {
      nsSimdVec4f a(-3.0f, 5.0f, -7.0f, 9.0f);
      nsSimdVec4f b(8.0f, 6.0f, 4.0f, 2.0f);
      nsSimdVec4f c(1.0f, 2.0f, 3.0f, 4.0f);
      nsSimdVec4f d;

      d = nsSimdVec4f::MulAdd(a, b, c);
      NS_TEST_BOOL(d.x() == -23.0f && d.y() == 32.0f && d.z() == -25.0f && d.w() == 22.0f);

      d = nsSimdVec4f::MulAdd(a, nsSimdFloat(3.0f), c);
      NS_TEST_BOOL(d.x() == -8.0f && d.y() == 17.0f && d.z() == -18.0f && d.w() == 31.0f);

      d = nsSimdVec4f::MulSub(a, b, c);
      NS_TEST_BOOL(d.x() == -25.0f && d.y() == 28.0f && d.z() == -31.0f && d.w() == 14.0f);

      d = nsSimdVec4f::MulSub(a, nsSimdFloat(3.0f), c);
      NS_TEST_BOOL(d.x() == -10.0f && d.y() == 13.0f && d.z() == -24.0f && d.w() == 23.0f);

      d = nsSimdVec4f::CopySign(b, a);
      NS_TEST_BOOL(d.x() == -8.0f && d.y() == 6.0f && d.z() == -4.0f && d.w() == 2.0f);
    }
  }
}
