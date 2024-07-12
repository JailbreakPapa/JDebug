#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/FixedPoint.h>

NS_CREATE_SIMPLE_TEST(Math, FixedPoint)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor (int) / Conversion to Int")
  {
    // positive values
    for (nsInt32 i = 0; i < 1024; ++i)
    {
      nsFixedPoint<12> fp(i);
      NS_TEST_INT(fp.ToInt(), i);
    }

    // negative values
    for (nsInt32 i = 0; i < 1024; ++i)
    {
      nsFixedPoint<12> fp(-i);
      NS_TEST_INT(fp.ToInt(), -i);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor (float) / Conversion to Float")
  {
    // positive values
    for (float f = 0.0f; f < 100.0f; f += 0.01f)
    {
      nsFixedPoint<12> fp(f);

      NS_TEST_FLOAT(fp, f, 0.001f);
    }

    // negative values
    for (float f = 0.0f; f < 100.0f; f += 0.01f)
    {
      nsFixedPoint<12> fp(-f);

      NS_TEST_FLOAT(fp, -f, 0.001f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor (double) / Conversion to double")
  {
    // positive values
    for (double f = 0.0; f < 100.0; f += 0.01)
    {
      nsFixedPoint<12> fp(f);

      NS_TEST_DOUBLE(fp.ToDouble(), f, 0.001);
    }

    // negative values
    for (double f = 0.0; f < 100.0f; f += 0.01)
    {
      nsFixedPoint<12> fp(-f);

      NS_TEST_DOUBLE(fp.ToDouble(), -f, 0.001);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor (Other) / Assignment")
  {
    nsFixedPoint<12> fp1(2.4f);
    nsFixedPoint<12> fp2(fp1);
    nsFixedPoint<12> fp3;

    fp3 = fp1;

    NS_TEST_BOOL(fp1 == fp1);
    NS_TEST_BOOL(fp2 == fp2);
    NS_TEST_BOOL(fp3 == fp3);

    NS_TEST_BOOL(fp1 == fp2);
    NS_TEST_BOOL(fp1 == fp3);
    NS_TEST_BOOL(fp2 == fp3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Max Value")
  {
    nsFixedPoint<12> fp1((1 << 19) - 1);
    nsFixedPoint<12> fp2((1 << 19));
    nsFixedPoint<12> fp3(-(1 << 19)); // one more value available in the negative range
    nsFixedPoint<12> fp4(-(1 << 19) - 1);

    // 12 Bits for the fraction -> 19 Bits for the integral part and 1 'Sign Bit'
    NS_TEST_BOOL(fp1.ToInt() == (1 << 19) - 1); // This maximum value is still representable
    NS_TEST_BOOL(fp2.ToInt() != (1 << 19));     // The next value isn't representable anymore
    NS_TEST_BOOL(fp3.ToInt() == -(1 << 19));
    NS_TEST_BOOL(fp4.ToInt() != -(1 << 19) - 1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator*(fp, int)")
  {
    nsFixedPoint<12> fp(3.2f);
    fp = fp * 2;

    NS_TEST_FLOAT(fp.ToFloat(), 6.4f, 0.001f);

    fp = 3 * fp;

    NS_TEST_FLOAT(fp.ToFloat(), 19.2f, 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator/(fp, int)")
  {
    nsFixedPoint<12> fp(12.4f);
    fp = fp / 2;

    NS_TEST_FLOAT(fp.ToFloat(), 6.2f, 0.001f);

    fp = fp / 3;

    NS_TEST_FLOAT(fp.ToFloat(), 2.066f, 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator+(fp, fp)")
  {
    nsFixedPoint<12> fp(3.2f);
    fp = fp + nsFixedPoint<12>(2);

    NS_TEST_FLOAT(fp.ToFloat(), 5.2f, 0.001f);

    fp = nsFixedPoint<12>(3) + fp;

    NS_TEST_FLOAT(fp.ToFloat(), 8.2f, 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator-(fp, fp)")
  {
    nsFixedPoint<12> fp(3.2f);
    fp = fp - nsFixedPoint<12>(2);

    NS_TEST_FLOAT(fp.ToFloat(), 1.2f, 0.001f);

    fp = nsFixedPoint<12>(3) - fp;

    NS_TEST_FLOAT(fp.ToFloat(), 1.8f, 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator*(fp, fp)")
  {
    nsFixedPoint<12> fp(3.2f);

    fp = fp * nsFixedPoint<12>(2.5f);
    NS_TEST_FLOAT(fp.ToFloat(), 8.0f, 0.001f);

    fp = fp * nsFixedPoint<12>(-123.456f);
    NS_TEST_FLOAT(fp.ToFloat(), -987.648f, 0.1f);
  }

  // Disabled because MSVC 2017 has code generation issues in Release builds
  NS_TEST_BLOCK(nsTestBlock::Disabled, "operator/(fp, fp)")
  {
    nsFixedPoint<12> fp(100000.248f);

    fp = fp / nsFixedPoint<12>(-2);
    NS_TEST_FLOAT(fp.ToFloat(), -50000.124f, 0.001f);

    fp = fp / nsFixedPoint<12>(-4);
    NS_TEST_FLOAT(fp.ToFloat(), 12500.031f, 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Operator<,>,<=,>=,==,!=")
  {
    nsFixedPoint<12> fp1(1);
    nsFixedPoint<12> fp2(2.0f);
    nsFixedPoint<12> fp3(3);
    nsFixedPoint<12> fp3b(3.0f);

    NS_TEST_BOOL(fp1 < fp2);
    NS_TEST_BOOL(fp3 > fp2);
    NS_TEST_BOOL(fp3 <= fp3b);
    NS_TEST_BOOL(fp3 >= fp3b);
    NS_TEST_BOOL(fp1 != fp2);
    NS_TEST_BOOL(fp3 == fp3b);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Assignment Rounding")
  {
    nsFixedPoint<2> fp; // 2 Bits -> 4 fractional values

    fp = 1000.25f;
    NS_TEST_FLOAT(fp, 1000.25f, 0.01f);

    fp = 1000.75f;
    NS_TEST_FLOAT(fp, 1000.75f, 0.01f);



    fp = 1000.1f;
    NS_TEST_DOUBLE(fp.ToDouble(), 1000.0, 0.01);

    fp = 1000.2f;
    NS_TEST_DOUBLE(fp.ToDouble(), 1000.25, 0.01);

    fp = 1000.3f;
    NS_TEST_DOUBLE(fp.ToDouble(), 1000.25, 0.01);

    fp = 1000.4f;
    NS_TEST_DOUBLE(fp.ToDouble(), 1000.5, 0.01);

    fp = 1000.5f;
    NS_TEST_DOUBLE(fp.ToDouble(), 1000.5, 0.01);

    fp = 1000.6f;
    NS_TEST_DOUBLE(fp.ToDouble(), 1000.5, 0.01);

    fp = 1000.7f;
    NS_TEST_DOUBLE(fp.ToDouble(), 1000.75, 0.01);

    fp = 1000.8f;
    NS_TEST_DOUBLE(fp.ToDouble(), 1000.75, 0.01);

    fp = 1000.9f;
    NS_TEST_DOUBLE(fp.ToDouble(), 1001.0, 0.01);


    // negative
    fp = -1000.1;
    NS_TEST_FLOAT(fp.ToFloat(), -1000.0f, 0.01f);

    fp = -1000.2;
    NS_TEST_FLOAT(fp.ToFloat(), -1000.25f, 0.01f);

    fp = -1000.3;
    NS_TEST_FLOAT(fp.ToFloat(), -1000.25f, 0.01f);

    fp = -1000.4;
    NS_TEST_FLOAT(fp.ToFloat(), -1000.5f, 0.01f);

    fp = -1000.5;
    NS_TEST_FLOAT(fp.ToFloat(), -1000.5f, 0.01f);

    fp = -1000.6;
    NS_TEST_FLOAT(fp.ToFloat(), -1000.5f, 0.01f);

    fp = -1000.7;
    NS_TEST_FLOAT(fp.ToFloat(), -1000.75f, 0.01f);

    fp = -1000.8;
    NS_TEST_FLOAT(fp.ToFloat(), -1000.75f, 0.01f);

    fp = -1000.9;
    NS_TEST_FLOAT(fp.ToFloat(), -1001.0f, 0.01f);
  }

  // Disabled because MSVC 2017 has code generation issues in Release builds
  NS_TEST_BLOCK(nsTestBlock::Disabled, "Multiplication Rounding")
  {
    nsFixedPoint<2> fp;         // 2 Bits -> 4 fractional values

    fp = 0.25;
    fp *= nsFixedPoint<2>(1.5); // -> should be 0.375, which is not representable -> will be rounded up

    NS_TEST_FLOAT(fp, 0.5f, 0.01f);

    fp = -0.25;
    fp *= nsFixedPoint<2>(1.5); // -> should be -0.375, which is not representable -> will be rounded up (towards zero)

    NS_TEST_FLOAT(fp, -0.25f, 0.01f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Division Rounding")
  {
    nsFixedPoint<12> fp2(1000);
    NS_TEST_INT(fp2.GetRawValue(), 1000 << 12);

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), 500 << 12);

    fp2 += nsFixedPoint<12>(1);
    NS_TEST_INT(fp2.GetRawValue(), 501 << 12);

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (250 << 12) + (1 << 11));

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (125 << 12) + (1 << 10));

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (62 << 12) + (1 << 11) + (1 << 9));

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (31 << 12) + (1 << 10) + (1 << 8));

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (15 << 12) + (1 << 11) + (1 << 9) + (1 << 7));

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (7 << 12) + (1 << 11) + (1 << 10) + (1 << 8) + (1 << 6));

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (3 << 12) + (1 << 11) + (1 << 10) + (1 << 9) + (1 << 7) + (1 << 5));

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (1 << 12) + (1 << 11) + (1 << 10) + (1 << 9) + (1 << 8) + (1 << 6) + (1 << 4));

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (1 << 11) + (1 << 10) + (1 << 9) + (1 << 8) + (1 << 7) + (1 << 5) + (1 << 3));

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (1 << 10) + (1 << 9) + (1 << 8) + (1 << 7) + (1 << 6) + (1 << 4) + (1 << 2));

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (1 << 9) + (1 << 8) + (1 << 7) + (1 << 6) + (1 << 5) + (1 << 3) + (1 << 1));

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (1 << 8) + (1 << 7) + (1 << 6) + (1 << 5) + (1 << 4) + (1 << 2) + (1 << 0));

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (1 << 7) + (1 << 6) + (1 << 5) + (1 << 4) + (1 << 3) + (1 << 1) + (1 << 0)); // here we round up

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (1 << 6) + (1 << 5) + (1 << 4) + (1 << 3) + (1 << 2) + (1 << 1));

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (1 << 5) + (1 << 4) + (1 << 3) + (1 << 2) + (1 << 1) + (1 << 0));

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (1 << 4) + (1 << 3) + (1 << 2) + (1 << 1) + (1 << 0) + (1 << 0)); // here we round up again

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (1 << 3) + (1 << 2) + (1 << 1) + (1 << 0) + (1 << 0));            // here we round up again

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (1 << 2) + (1 << 1) + (1 << 1));                                  // here we round up again

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (1 << 1) + (1 << 1));                                             // here we round up again

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (1 << 1));

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (1 << 0));

    fp2 /= nsFixedPoint<12>(2);
    NS_TEST_INT(fp2.GetRawValue(), (1 << 0)); // we can never get lower than this by dividing by 2, as it will always get rounded up again

    fp2 /= nsFixedPoint<12>(2.01);
    NS_TEST_INT(fp2.GetRawValue(), 0);        // finally we round down
  }
}
