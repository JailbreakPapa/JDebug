#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/SimdMath/SimdFloat.h>

NS_CREATE_SIMPLE_TEST_GROUP(SimdMath);

NS_CREATE_SIMPLE_TEST(SimdMath, SimdFloat)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    // In debug the default constructor initializes everything with NaN.
    nsSimdFloat vDefCtor;
    NS_TEST_BOOL(nsMath::IsNaN((float)vDefCtor));
#else
// GCC assumes that the contents of the memory before calling the default constructor are irrelevant.
// So it optimizes away the 1,2,3,4 initializer completely.
#  if NS_DISABLED(NS_COMPILER_GCC)
    // Placement new of the default constructor should not have any effect on the previous data.
    alignas(16) float testBlock[4] = {1, 2, 3, 4};
    nsSimdFloat* pDefCtor = ::new ((void*)&testBlock[0]) nsSimdFloat;
    NS_TEST_BOOL_MSG((float)(*pDefCtor) == 1.0f, "Default constructed value is %f", (float)(*pDefCtor));
#  endif
#endif

    // Make sure the class didn't accidentally change in size.
#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
    NS_CHECK_AT_COMPILETIME(sizeof(nsSimdFloat) == 16);
    NS_CHECK_AT_COMPILETIME(NS_ALIGNMENT_OF(nsSimdFloat) == 16);
#endif

    nsSimdFloat vInit1F(2.0f);
    NS_TEST_BOOL(vInit1F == 2.0f);

    // Make sure all components are set to the same value
#if (NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE) && NS_ENABLED(NS_COMPILER_MSVC)
    NS_TEST_BOOL(
      vInit1F.m_v.m128_f32[0] == 2.0f && vInit1F.m_v.m128_f32[1] == 2.0f && vInit1F.m_v.m128_f32[2] == 2.0f && vInit1F.m_v.m128_f32[3] == 2.0f);
#endif

    nsSimdFloat vInit1I(1);
    NS_TEST_BOOL(vInit1I == 1.0f);

    // Make sure all components are set to the same value
#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE && NS_ENABLED(NS_COMPILER_MSVC)
    NS_TEST_BOOL(
      vInit1I.m_v.m128_f32[0] == 1.0f && vInit1I.m_v.m128_f32[1] == 1.0f && vInit1I.m_v.m128_f32[2] == 1.0f && vInit1I.m_v.m128_f32[3] == 1.0f);
#endif

    nsSimdFloat vInit1U(4553u);
    NS_TEST_BOOL(vInit1U == 4553.0f);

    // Make sure all components are set to the same value
#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE && NS_ENABLED(NS_COMPILER_MSVC)
    NS_TEST_BOOL(vInit1U.m_v.m128_f32[0] == 4553.0f && vInit1U.m_v.m128_f32[1] == 4553.0f && vInit1U.m_v.m128_f32[2] == 4553.0f &&
                 vInit1U.m_v.m128_f32[3] == 4553.0f);
#endif

    nsSimdFloat z = nsSimdFloat::MakeZero();
    NS_TEST_BOOL(z == 0.0f);

    // Make sure all components are set to the same value
#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE && NS_ENABLED(NS_COMPILER_MSVC)
    NS_TEST_BOOL(z.m_v.m128_f32[0] == 0.0f && z.m_v.m128_f32[1] == 0.0f && z.m_v.m128_f32[2] == 0.0f && z.m_v.m128_f32[3] == 0.0f);
#endif
  }

  {
    nsSimdFloat z = nsSimdFloat::MakeZero();
    NS_TEST_BOOL(z == 0.0f);

    // Make sure all components are set to the same value
#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE && NS_ENABLED(NS_COMPILER_MSVC)
    NS_TEST_BOOL(z.m_v.m128_f32[0] == 0.0f && z.m_v.m128_f32[1] == 0.0f && z.m_v.m128_f32[2] == 0.0f && z.m_v.m128_f32[3] == 0.0f);
#endif
  }

  {
    nsSimdFloat z = nsSimdFloat::MakeNaN();

    // Make sure all components are set to the same value
#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE && NS_ENABLED(NS_COMPILER_MSVC)
    NS_TEST_BOOL(nsMath::IsNaN(z.m_v.m128_f32[0]));
    NS_TEST_BOOL(nsMath::IsNaN(z.m_v.m128_f32[1]));
    NS_TEST_BOOL(nsMath::IsNaN(z.m_v.m128_f32[2]));
    NS_TEST_BOOL(nsMath::IsNaN(z.m_v.m128_f32[3]));
#endif
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Operators")
  {
    nsSimdFloat a = 5.0f;
    nsSimdFloat b = 2.0f;

    NS_TEST_FLOAT(a + b, 7.0f, nsMath::SmallEpsilon<float>());
    NS_TEST_FLOAT(a - b, 3.0f, nsMath::SmallEpsilon<float>());
    NS_TEST_FLOAT(a * b, 10.0f, nsMath::SmallEpsilon<float>());
    NS_TEST_FLOAT(a / b, 2.5f, nsMath::SmallEpsilon<float>());

    nsSimdFloat c = 1.0f;
    c += a;
    NS_TEST_FLOAT(c, 6.0f, nsMath::SmallEpsilon<float>());

    c = 1.0f;
    c -= b;
    NS_TEST_FLOAT(c, -1.0f, nsMath::SmallEpsilon<float>());

    c = 1.0f;
    c *= a;
    NS_TEST_FLOAT(c, 5.0f, nsMath::SmallEpsilon<float>());

    c = 1.0f;
    c /= a;
    NS_TEST_FLOAT(c, 0.2f, nsMath::SmallEpsilon<float>());

    NS_TEST_BOOL(c.IsEqual(0.201f, nsMath::HugeEpsilon<float>()));
    NS_TEST_BOOL(c.IsEqual(0.199f, nsMath::HugeEpsilon<float>()));
    NS_TEST_BOOL(!c.IsEqual(0.202f, nsMath::HugeEpsilon<float>()));
    NS_TEST_BOOL(!c.IsEqual(0.198f, nsMath::HugeEpsilon<float>()));

    c = b;
    NS_TEST_BOOL(c == b);
    NS_TEST_BOOL(c != a);
    NS_TEST_BOOL(a > b);
    NS_TEST_BOOL(c >= b);
    NS_TEST_BOOL(b < a);
    NS_TEST_BOOL(b <= c);

    NS_TEST_BOOL(c == 2.0f);
    NS_TEST_BOOL(c != 5.0f);
    NS_TEST_BOOL(a > 2.0f);
    NS_TEST_BOOL(c >= 2.0f);
    NS_TEST_BOOL(b < 5.0f);
    NS_TEST_BOOL(b <= 2.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Misc")
  {
    nsSimdFloat a = 2.0f;

    NS_TEST_FLOAT(a.GetReciprocal(), 0.5f, nsMath::SmallEpsilon<float>());
    NS_TEST_FLOAT(a.GetReciprocal<nsMathAcc::FULL>(), 0.5f, nsMath::SmallEpsilon<float>());
    NS_TEST_FLOAT(a.GetReciprocal<nsMathAcc::BITS_23>(), 0.5f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(a.GetReciprocal<nsMathAcc::BITS_12>(), 0.5f, nsMath::HugeEpsilon<float>());

    NS_TEST_FLOAT(a.GetSqrt(), 1.41421356f, nsMath::SmallEpsilon<float>());
    NS_TEST_FLOAT(a.GetSqrt<nsMathAcc::FULL>(), 1.41421356f, nsMath::SmallEpsilon<float>());
    NS_TEST_FLOAT(a.GetSqrt<nsMathAcc::BITS_23>(), 1.41421356f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(a.GetSqrt<nsMathAcc::BITS_12>(), 1.41421356f, nsMath::HugeEpsilon<float>());

    NS_TEST_FLOAT(a.GetInvSqrt(), 0.70710678f, nsMath::SmallEpsilon<float>());
    NS_TEST_FLOAT(a.GetInvSqrt<nsMathAcc::FULL>(), 0.70710678f, nsMath::SmallEpsilon<float>());
    NS_TEST_FLOAT(a.GetInvSqrt<nsMathAcc::BITS_23>(), 0.70710678f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(a.GetInvSqrt<nsMathAcc::BITS_12>(), 0.70710678f, nsMath::HugeEpsilon<float>());

    nsSimdFloat b = 5.0f;
    NS_TEST_BOOL(a.Max(b) == b);
    NS_TEST_BOOL(a.Min(b) == a);

    nsSimdFloat c = -4.0f;
    NS_TEST_FLOAT(c.Abs(), 4.0f, nsMath::SmallEpsilon<float>());
  }
}
