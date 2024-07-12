#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/SimdMath/SimdVec4b.h>

NS_CREATE_SIMPLE_TEST(SimdMath, SimdVec4b)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
#if NS_DISABLED(NS_COMPILER_GCC) && NS_DISABLED(NS_COMPILE_FOR_DEBUG)
    // Placement new of the default constructor should not have any effect on the previous data.
    alignas(16) float testBlock[4] = {1, 2, 3, 4};
    nsSimdVec4b* pDefCtor = ::new ((void*)&testBlock[0]) nsSimdVec4b;
    NS_TEST_BOOL(testBlock[0] == 1.0f && testBlock[1] == 2.0f && testBlock[2] == 3.0f && testBlock[3] == 4.0f);
#endif

    // Make sure the class didn't accidentally change in size.
#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
    NS_CHECK_AT_COMPILETIME(sizeof(nsSimdVec4b) == 16);
    NS_CHECK_AT_COMPILETIME(NS_ALIGNMENT_OF(nsSimdVec4b) == 16);
#endif

    nsSimdVec4b vInit1B(true);
    NS_TEST_BOOL(vInit1B.x() == true && vInit1B.y() == true && vInit1B.z() == true && vInit1B.w() == true);

    // Make sure all components have the correct value
#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE && NS_ENABLED(NS_COMPILER_MSVC)
    NS_TEST_BOOL(vInit1B.m_v.m128_u32[0] == 0xFFFFFFFF && vInit1B.m_v.m128_u32[1] == 0xFFFFFFFF && vInit1B.m_v.m128_u32[2] == 0xFFFFFFFF &&
                 vInit1B.m_v.m128_u32[3] == 0xFFFFFFFF);
#endif

    nsSimdVec4b vInit4B(false, true, false, true);
    NS_TEST_BOOL(vInit4B.x() == false && vInit4B.y() == true && vInit4B.z() == false && vInit4B.w() == true);

    // Make sure all components have the correct value
#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE && NS_ENABLED(NS_COMPILER_MSVC)
    NS_TEST_BOOL(
      vInit4B.m_v.m128_u32[0] == 0 && vInit4B.m_v.m128_u32[1] == 0xFFFFFFFF && vInit4B.m_v.m128_u32[2] == 0 && vInit4B.m_v.m128_u32[3] == 0xFFFFFFFF);
#endif

    nsSimdVec4b vCopy(vInit4B);
    NS_TEST_BOOL(vCopy.x() == false && vCopy.y() == true && vCopy.z() == false && vCopy.w() == true);

    NS_TEST_BOOL(
      vCopy.GetComponent<0>() == false && vCopy.GetComponent<1>() == true && vCopy.GetComponent<2>() == false && vCopy.GetComponent<3>() == true);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Swizzle")
  {
    nsSimdVec4b a(true, false, true, false);

    nsSimdVec4b b = a.Get<nsSwizzle::XXXX>();
    NS_TEST_BOOL(b.x() && b.y() && b.z() && b.w());

    b = a.Get<nsSwizzle::YYYX>();
    NS_TEST_BOOL(!b.x() && !b.y() && !b.z() && b.w());

    b = a.Get<nsSwizzle::ZZZX>();
    NS_TEST_BOOL(b.x() && b.y() && b.z() && b.w());

    b = a.Get<nsSwizzle::WWWX>();
    NS_TEST_BOOL(!b.x() && !b.y() && !b.z() && b.w());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Operators")
  {
    nsSimdVec4b a(true, false, true, false);
    nsSimdVec4b b(false, true, true, false);

    nsSimdVec4b c = a && b;
    NS_TEST_BOOL(!c.x() && !c.y() && c.z() && !c.w());

    c = a || b;
    NS_TEST_BOOL(c.x() && c.y() && c.z() && !c.w());

    c = !a;
    NS_TEST_BOOL(!c.x() && c.y() && !c.z() && c.w());
    NS_TEST_BOOL(c.AnySet<2>());
    NS_TEST_BOOL(!c.AllSet<4>());
    NS_TEST_BOOL(!c.NoneSet<4>());

    c = c || a;
    NS_TEST_BOOL(c.AnySet<4>());
    NS_TEST_BOOL(c.AllSet<4>());
    NS_TEST_BOOL(!c.NoneSet<4>());

    c = !c;
    NS_TEST_BOOL(!c.AnySet<4>());
    NS_TEST_BOOL(!c.AllSet<4>());
    NS_TEST_BOOL(c.NoneSet<4>());

    c = a == b;
    NS_TEST_BOOL(!c.x() && !c.y() && c.z() && c.w());

    c = a != b;
    NS_TEST_BOOL(c.x() && c.y() && !c.z() && !c.w());

    NS_TEST_BOOL(a.AllSet<1>());
    NS_TEST_BOOL(b.NoneSet<1>());

    nsSimdVec4b cmp(false, true, false, true);
    c = nsSimdVec4b::Select(cmp, a, b);
    NS_TEST_BOOL(!c.x() && !c.y() && c.z() && !c.w());
  }
}
