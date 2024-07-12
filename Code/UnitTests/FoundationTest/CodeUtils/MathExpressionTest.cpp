#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/CodeUtils/MathExpression.h>

NS_CREATE_SIMPLE_TEST(CodeUtils, MathExpression)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Basics")
  {
    {
      nsMathExpression expr("");
      NS_TEST_BOOL(!expr.IsValid());

      expr.Reset("");
      NS_TEST_BOOL(!expr.IsValid());
    }
    {
      nsMathExpression expr(nullptr);
      NS_TEST_BOOL(!expr.IsValid());

      expr.Reset(nullptr);
      NS_TEST_BOOL(!expr.IsValid());
    }
    {
      nsMathExpression expr("1.5 + 2.5");
      NS_TEST_BOOL(expr.IsValid());
      NS_TEST_DOUBLE(expr.Evaluate(), 4.0, 0.0);
    }
    {
      nsMathExpression expr("1- 2");
      NS_TEST_BOOL(expr.IsValid());
      NS_TEST_DOUBLE(expr.Evaluate(), -1.0, 0.0);
    }
    {
      nsMathExpression expr("1 *2");
      NS_TEST_BOOL(expr.IsValid());
      NS_TEST_DOUBLE(expr.Evaluate(), 2.0, 0.0);
    }
    {
      nsMathExpression expr(" 1.0/2 ");
      NS_TEST_BOOL(expr.IsValid());
      NS_TEST_DOUBLE(expr.Evaluate(), 0.5, 0.0);
    }
    {
      nsMathExpression expr("1 - -1");
      NS_TEST_BOOL(expr.IsValid());
      NS_TEST_DOUBLE(expr.Evaluate(), 2.0, 0.0);
    }
    {
      nsMathExpression expr("abs(-3)");
      NS_TEST_BOOL(expr.IsValid());
      NS_TEST_DOUBLE(expr.Evaluate(), 3.0, 0.0);
    }
    {
      nsMathExpression expr("sqrt(4)");
      NS_TEST_BOOL(expr.IsValid());
      NS_TEST_DOUBLE(expr.Evaluate(), 2.0, 0.0);
    }
    {
      nsMathExpression expr("saturate(4)");
      NS_TEST_BOOL(expr.IsValid());
      NS_TEST_DOUBLE(expr.Evaluate(), 1.0, 0.0);
    }
    {
      nsMathExpression expr("min(3, 4)");
      NS_TEST_BOOL(expr.IsValid());
      NS_TEST_DOUBLE(expr.Evaluate(), 3.0, 0.0);
    }
    {
      nsMathExpression expr("max(3, 4)");
      NS_TEST_BOOL(expr.IsValid());
      NS_TEST_DOUBLE(expr.Evaluate(), 4.0, 0.0);
    }
    {
      nsMathExpression expr("clamp(2, 3, 4)");
      NS_TEST_BOOL(expr.IsValid());
      NS_TEST_DOUBLE(expr.Evaluate(), 3.0, 0.0);
    }
    {
      nsMathExpression expr("clamp(5, 3, 4)");
      NS_TEST_BOOL(expr.IsValid());
      NS_TEST_DOUBLE(expr.Evaluate(), 4.0, 0.0);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Operator Priority")
  {
    {
      nsMathExpression expr("1 - 2 * 4");
      NS_TEST_BOOL(expr.IsValid());
      NS_TEST_DOUBLE(expr.Evaluate(), -7.0, 0.0);
    }
    {
      nsMathExpression expr("-1 - 2 * 4");
      NS_TEST_BOOL(expr.IsValid());
      NS_TEST_DOUBLE(expr.Evaluate(), -9.0, 0.0);
    }
    {
      nsMathExpression expr("1 - 2.0 / 4");
      NS_TEST_BOOL(expr.IsValid());
      NS_TEST_DOUBLE(expr.Evaluate(), 0.5, 0.0);
    }
    {
      nsMathExpression expr("abs (-4 + 2)");
      NS_TEST_BOOL(expr.IsValid());
      NS_TEST_DOUBLE(expr.Evaluate(), 2.0, 0.0);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Braces")
  {
    {
      nsMathExpression expr("(1 - 2) * 4");
      NS_TEST_BOOL(expr.IsValid());
      NS_TEST_DOUBLE(expr.Evaluate(), -4.0, 0.0);
    }
    {
      nsMathExpression expr("(((((0)))))");
      NS_TEST_BOOL(expr.IsValid());
      NS_TEST_DOUBLE(expr.Evaluate(), 0.0, 0.0);
    }
    {
      nsMathExpression expr("(1 + 2) * (3 - 2)");
      NS_TEST_BOOL(expr.IsValid());
      NS_TEST_DOUBLE(expr.Evaluate(), 3.0, 0.0);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Variables")
  {
    nsHybridArray<nsMathExpression::Input, 4> inputs;
    inputs.SetCount(4);

    {
      nsMathExpression expr("_var1 + v2Ar");
      NS_TEST_BOOL(expr.IsValid());

      inputs[0] = {nsMakeHashedString("_var1"), 1.0};
      inputs[1] = {nsMakeHashedString("v2Ar"), 2.0};

      double result = expr.Evaluate(inputs);
      NS_TEST_DOUBLE(result, 3.0, 0.0);

      inputs[0].m_fValue = 2.0;
      inputs[1].m_fValue = 0.5;

      result = expr.Evaluate(inputs);
      NS_TEST_DOUBLE(result, 2.5, 0.0);
    }

    // Make sure we got the spaces right and don't count it as part of the variable.
    {
      nsMathExpression expr("  a +  b /c*d");
      NS_TEST_BOOL(expr.IsValid());

      inputs[0] = {nsMakeHashedString("a"), 1.0};
      inputs[1] = {nsMakeHashedString("b"), 4.0};
      inputs[2] = {nsMakeHashedString("c"), 2.0};
      inputs[3] = {nsMakeHashedString("d"), 3.0};

      double result = expr.Evaluate(inputs);
      NS_TEST_DOUBLE(result, 7.0, 0.0);
    }
  }


  NS_TEST_BLOCK(nsTestBlock::Enabled, "Invalid Expressions")
  {
    nsMuteLog logErrorSink;
    nsLogSystemScope ls(&logErrorSink);

    {
      nsMathExpression expr("1+");
      NS_TEST_BOOL(!expr.IsValid());
    }
    {
      nsMathExpression expr("1+/1");
      NS_TEST_BOOL(!expr.IsValid());
    }
    {
      nsMathExpression expr("(((((0))))");
      NS_TEST_BOOL(!expr.IsValid());
    }
    {
      nsMathExpression expr("_va£r + asdf");
      NS_TEST_BOOL(!expr.IsValid());
    }
    {
      nsMathExpression expr("sqrt(2, 4)");
      NS_TEST_BOOL(!expr.IsValid());
    }
  }
}
