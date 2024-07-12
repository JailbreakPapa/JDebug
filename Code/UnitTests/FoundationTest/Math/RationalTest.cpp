#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/Rational.h>
#include <Foundation/Strings/StringBuilder.h>

NS_CREATE_SIMPLE_TEST(Math, Rational)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Rational")
  {
    nsRational r1(100, 1);

    NS_TEST_BOOL(r1.IsValid());
    NS_TEST_BOOL(r1.IsIntegral());

    nsRational r2(100, 0);
    NS_TEST_BOOL(!r2.IsValid());

    NS_TEST_BOOL(r1 != r2);

    nsRational r3(100, 1);
    NS_TEST_BOOL(r3 == r1);

    nsRational r4(0, 0);
    NS_TEST_BOOL(r4.IsValid());


    nsRational r5(30, 6);
    NS_TEST_BOOL(r5.IsIntegral());
    NS_TEST_INT(r5.GetIntegralResult(), 5);
    NS_TEST_FLOAT(r5.GetFloatingPointResult(), 5, nsMath::SmallEpsilon<double>());

    nsRational reducedTest(5, 1);
    NS_TEST_BOOL(r5.ReduceIntegralFraction() == reducedTest);

    nsRational r6(31, 6);
    NS_TEST_BOOL(!r6.IsIntegral());
    NS_TEST_FLOAT(r6.GetFloatingPointResult(), 5.16666666666, nsMath::SmallEpsilon<double>());


    NS_TEST_INT(r6.GetDenominator(), 6);
    NS_TEST_INT(r6.GetNumerator(), 31);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Rational String Formatting")
  {
    nsRational r1(50, 25);

    nsStringBuilder sb;
    sb.SetFormat("Rational: {}", r1);
    NS_TEST_STRING(sb, "Rational: 2");


    nsRational r2(233, 76);
    sb.SetFormat("Rational: {}", r2);
    NS_TEST_STRING(sb, "Rational: 233/76");
  }
}
