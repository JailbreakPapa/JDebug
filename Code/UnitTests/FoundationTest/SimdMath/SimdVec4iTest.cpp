#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/SimdMath/SimdVec4u.h>

NS_CREATE_SIMPLE_TEST(SimdMath, SimdVec4i)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
    // In debug the default constructor initializes everything with 0xCDCDCDCD.
    nsSimdVec4i vDefCtor;
    NS_TEST_BOOL(vDefCtor.x() == 0xCDCDCDCD && vDefCtor.y() == 0xCDCDCDCD && vDefCtor.z() == 0xCDCDCDCD && vDefCtor.w() == 0xCDCDCDCD);
#else
    // Placement new of the default constructor should not have any effect on the previous data.
    alignas(16) float testBlock[4] = {1, 2, 3, 4};
    nsSimdVec4i* pDefCtor = ::new ((void*)&testBlock[0]) nsSimdVec4i;
    NS_TEST_BOOL(testBlock[0] == 1 && testBlock[1] == 2 && testBlock[2] == 3 && testBlock[3] == 4);
#endif

    // Make sure the class didn't accidentally change in size.
#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
    NS_CHECK_AT_COMPILETIME(sizeof(nsSimdVec4i) == 16);
    NS_CHECK_AT_COMPILETIME(NS_ALIGNMENT_OF(nsSimdVec4i) == 16);
#endif

    nsSimdVec4i a(2);
    NS_TEST_BOOL(a.x() == 2 && a.y() == 2 && a.z() == 2 && a.w() == 2);

    nsSimdVec4i b(1, 2, 3, 4);
    NS_TEST_BOOL(b.x() == 1 && b.y() == 2 && b.z() == 3 && b.w() == 4);

    // Make sure all components have the correct values
#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE && NS_ENABLED(NS_COMPILER_MSVC)
    NS_TEST_BOOL(b.m_v.m128i_i32[0] == 1 && b.m_v.m128i_i32[1] == 2 && b.m_v.m128i_i32[2] == 3 && b.m_v.m128i_i32[3] == 4);
#endif

    nsSimdVec4i copy(b);
    NS_TEST_BOOL(copy.x() == 1 && copy.y() == 2 && copy.z() == 3 && copy.w() == 4);

    NS_TEST_BOOL(copy.GetComponent<0>() == 1 && copy.GetComponent<1>() == 2 && copy.GetComponent<2>() == 3 && copy.GetComponent<3>() == 4);

    nsSimdVec4i vZero = nsSimdVec4i::MakeZero();
    NS_TEST_BOOL(vZero.x() == 0 && vZero.y() == 0 && vZero.z() == 0 && vZero.w() == 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Setter")
  {
    nsSimdVec4i a;
    a.Set(2);
    NS_TEST_BOOL(a.x() == 2 && a.y() == 2 && a.z() == 2 && a.w() == 2);

    nsSimdVec4i b;
    b.Set(1, 2, 3, 4);
    NS_TEST_BOOL(b.x() == 1 && b.y() == 2 && b.z() == 3 && b.w() == 4);

    nsSimdVec4i vSetZero;
    vSetZero.SetZero();
    NS_TEST_BOOL(vSetZero.x() == 0 && vSetZero.y() == 0 && vSetZero.z() == 0 && vSetZero.w() == 0);

    {
      nsSimdVec4i z = nsSimdVec4i::MakeZero();
      NS_TEST_BOOL(z.x() == 0 && z.y() == 0 && z.z() == 0 && z.w() == 0);
    }

    {
      int testBlock[4] = {1, 2, 3, 4};
      nsSimdVec4i x;
      x.Load<1>(testBlock);
      NS_TEST_BOOL(x.x() == 1 && x.y() == 0 && x.z() == 0 && x.w() == 0);

      nsSimdVec4i xy;
      xy.Load<2>(testBlock);
      NS_TEST_BOOL(xy.x() == 1 && xy.y() == 2 && xy.z() == 0 && xy.w() == 0);

      nsSimdVec4i xyz;
      xyz.Load<3>(testBlock);
      NS_TEST_BOOL(xyz.x() == 1 && xyz.y() == 2 && xyz.z() == 3 && xyz.w() == 0);

      nsSimdVec4i xyzw;
      xyzw.Load<4>(testBlock);
      NS_TEST_BOOL(xyzw.x() == 1 && xyzw.y() == 2 && xyzw.z() == 3 && xyzw.w() == 4);

      NS_TEST_INT(xyzw.GetComponent<0>(), 1);
      NS_TEST_INT(xyzw.GetComponent<1>(), 2);
      NS_TEST_INT(xyzw.GetComponent<2>(), 3);
      NS_TEST_INT(xyzw.GetComponent<3>(), 4);

      // Make sure all components have the correct values
#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE && NS_ENABLED(NS_COMPILER_MSVC)
      NS_TEST_BOOL(xyzw.m_v.m128i_i32[0] == 1 && xyzw.m_v.m128i_i32[1] == 2 && xyzw.m_v.m128i_i32[2] == 3 && xyzw.m_v.m128i_i32[3] == 4);
#endif
    }

    {
      int testBlock[4] = {7, 7, 7, 7};
      int mem[4] = {};

      nsSimdVec4i b2(1, 2, 3, 4);

      memcpy(mem, testBlock, 16);
      b2.Store<1>(mem);
      NS_TEST_BOOL(mem[0] == 1 && mem[1] == 7 && mem[2] == 7 && mem[3] == 7);

      memcpy(mem, testBlock, 16);
      b2.Store<2>(mem);
      NS_TEST_BOOL(mem[0] == 1 && mem[1] == 2 && mem[2] == 7 && mem[3] == 7);

      memcpy(mem, testBlock, 16);
      b2.Store<3>(mem);
      NS_TEST_BOOL(mem[0] == 1 && mem[1] == 2 && mem[2] == 3 && mem[3] == 7);

      memcpy(mem, testBlock, 16);
      b2.Store<4>(mem);
      NS_TEST_BOOL(mem[0] == 1 && mem[1] == 2 && mem[2] == 3 && mem[3] == 4);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Conversion")
  {
    nsSimdVec4i ia(-3, 5, -7, 11);

    nsSimdVec4u ua(ia);
    NS_TEST_BOOL(ua.x() == -3 && ua.y() == 5 && ua.z() == -7 && ua.w() == 11);

    nsSimdVec4f fa = ia.ToFloat();
    NS_TEST_BOOL(fa.x() == -3.0f && fa.y() == 5.0f && fa.z() == -7.0f && fa.w() == 11.0f);

    fa = nsSimdVec4f(-2.3f, 5.7f, -2147483520.0f, 2147483520.0f);
    nsSimdVec4i b = nsSimdVec4i::Truncate(fa);
    NS_TEST_INT(b.x(), -2);
    NS_TEST_INT(b.y(), 5);
    NS_TEST_INT(b.z(), -2147483520);
    NS_TEST_INT(b.w(), 2147483520);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Swizzle")
  {
    nsSimdVec4i a(3, 5, 7, 9);

    nsSimdVec4i b = a.Get<nsSwizzle::XXXX>();
    NS_TEST_BOOL(b.x() == 3 && b.y() == 3 && b.z() == 3 && b.w() == 3);

    b = a.Get<nsSwizzle::YYYX>();
    NS_TEST_BOOL(b.x() == 5 && b.y() == 5 && b.z() == 5 && b.w() == 3);

    b = a.Get<nsSwizzle::ZZZX>();
    NS_TEST_BOOL(b.x() == 7 && b.y() == 7 && b.z() == 7 && b.w() == 3);

    b = a.Get<nsSwizzle::WWWX>();
    NS_TEST_BOOL(b.x() == 9 && b.y() == 9 && b.z() == 9 && b.w() == 3);

    b = a.Get<nsSwizzle::WZYX>();
    NS_TEST_BOOL(b.x() == 9 && b.y() == 7 && b.z() == 5 && b.w() == 3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Operators")
  {
    {
      nsSimdVec4i a(-3, 5, -7, 9);

      nsSimdVec4i b = -a;
      NS_TEST_BOOL(b.x() == 3 && b.y() == -5 && b.z() == 7 && b.w() == -9);

      b.Set(8, 6, 4, 2);
      nsSimdVec4i c;
      c = a + b;
      NS_TEST_BOOL(c.x() == 5 && c.y() == 11 && c.z() == -3 && c.w() == 11);

      c = a - b;
      NS_TEST_BOOL(c.x() == -11 && c.y() == -1 && c.z() == -11 && c.w() == 7);

      c = a.CompMul(b);
      NS_TEST_BOOL(c.x() == -24 && c.y() == 30 && c.z() == -28 && c.w() == 18);

      c = a.CompDiv(b);
      NS_TEST_BOOL(c.x() == 0 && c.y() == 0 && c.z() == -1 && c.w() == 4);
    }

    {
      nsSimdVec4i a(NS_BIT(1), NS_BIT(2), NS_BIT(3), NS_BIT(4));
      nsSimdVec4i b(NS_BIT(4), NS_BIT(3), NS_BIT(3), NS_BIT(5) - 1);
      nsSimdVec4i c;

      c = a | b;
      NS_TEST_BOOL(c.x() == (NS_BIT(1) | NS_BIT(4)) && c.y() == (NS_BIT(2) | NS_BIT(3)) && c.z() == NS_BIT(3) && c.w() == NS_BIT(5) - 1);

      c = a & b;
      NS_TEST_BOOL(c.x() == 0 && c.y() == 0 && c.z() == NS_BIT(3) && c.w() == NS_BIT(4));

      c = a ^ b;
      NS_TEST_BOOL(c.x() == (NS_BIT(1) | NS_BIT(4)) && c.y() == (NS_BIT(2) | NS_BIT(3)) && c.z() == 0 && c.w() == NS_BIT(4) - 1);

      c = ~a;
      NS_TEST_BOOL(c.x() == ~NS_BIT(1) && c.y() == ~NS_BIT(2) && c.z() == ~NS_BIT(3) && c.w() == ~NS_BIT(4));

      c = a << 3;
      NS_TEST_BOOL(c.x() == NS_BIT(4) && c.y() == NS_BIT(5) && c.z() == NS_BIT(6) && c.w() == NS_BIT(7));

      c = a >> 1;
      NS_TEST_BOOL(c.x() == NS_BIT(0) && c.y() == NS_BIT(1) && c.z() == NS_BIT(2) && c.w() == NS_BIT(3));

      nsSimdVec4i s(1, 2, 3, 4);
      c = a << s;
      NS_TEST_BOOL(c.x() == NS_BIT(2) && c.y() == NS_BIT(4) && c.z() == NS_BIT(6) && c.w() == NS_BIT(8));

      c = b >> s;
      NS_TEST_BOOL(c.x() == NS_BIT(3) && c.y() == NS_BIT(1) && c.z() == NS_BIT(0) && c.w() == NS_BIT(0));
    }

    {
      nsSimdVec4i a(-3, 5, -7, 9);
      nsSimdVec4i b(8, 6, 4, 2);

      nsSimdVec4i c = a;
      c += b;
      NS_TEST_BOOL(c.x() == 5 && c.y() == 11 && c.z() == -3 && c.w() == 11);

      c = a;
      c -= b;
      NS_TEST_BOOL(c.x() == -11 && c.y() == -1 && c.z() == -11 && c.w() == 7);
    }

    {
      nsSimdVec4i a(NS_BIT(1), NS_BIT(2), NS_BIT(3), NS_BIT(4));
      nsSimdVec4i b(NS_BIT(4), NS_BIT(3), NS_BIT(3), NS_BIT(5) - 1);

      nsSimdVec4i c = a;
      c |= b;
      NS_TEST_BOOL(c.x() == (NS_BIT(1) | NS_BIT(4)) && c.y() == (NS_BIT(2) | NS_BIT(3)) && c.z() == NS_BIT(3) && c.w() == NS_BIT(5) - 1);

      c = a;
      c &= b;
      NS_TEST_BOOL(c.x() == 0 && c.y() == 0 && c.z() == NS_BIT(3) && c.w() == NS_BIT(4));

      c = a;
      c ^= b;
      NS_TEST_BOOL(c.x() == (NS_BIT(1) | NS_BIT(4)) && c.y() == (NS_BIT(2) | NS_BIT(3)) && c.z() == 0 && c.w() == NS_BIT(4) - 1);

      c = a;
      c <<= 3;
      NS_TEST_BOOL(c.x() == NS_BIT(4) && c.y() == NS_BIT(5) && c.z() == NS_BIT(6) && c.w() == NS_BIT(7));

      c = a;
      c >>= 1;
      NS_TEST_BOOL(c.x() == NS_BIT(0) && c.y() == NS_BIT(1) && c.z() == NS_BIT(2) && c.w() == NS_BIT(3));

      c = nsSimdVec4i(-2, -4, -7, -8);
      c >>= 1;
      NS_TEST_BOOL(c.x() == -1 && c.y() == -2 && c.z() == -4 && c.w() == -4);
    }

    {
      nsSimdVec4i a(-3, 5, -7, 9);
      nsSimdVec4i b(8, 6, 4, 2);
      nsSimdVec4i c;

      c = a.CompMin(b);
      NS_TEST_BOOL(c.x() == -3 && c.y() == 5 && c.z() == -7 && c.w() == 2);

      c = a.CompMax(b);
      NS_TEST_BOOL(c.x() == 8 && c.y() == 6 && c.z() == 4 && c.w() == 9);

      c = a.Abs();
      NS_TEST_BOOL(c.x() == 3 && c.y() == 5 && c.z() == 7 && c.w() == 9);

      nsSimdVec4b cmp(false, true, false, true);
      c = nsSimdVec4i::Select(cmp, a, b);
      NS_TEST_BOOL(c.x() == 8 && c.y() == 5 && c.z() == 4 && c.w() == 9);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Comparison")
  {
    nsSimdVec4i a(-7, 5, 4, 3);
    nsSimdVec4i b(8, 6, 4, -2);
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
}
