#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/Float16.h>
#include <Foundation/Strings/String.h>

NS_CREATE_SIMPLE_TEST(Math, Float16)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "From float and back")
  {
    // default constructor
    NS_TEST_BOOL(static_cast<float>(nsFloat16()) == 0.0f);

    // Border cases - exact matching needed.
    NS_TEST_FLOAT(static_cast<float>(nsFloat16(1.0f)), 1.0f, 0);
    NS_TEST_FLOAT(static_cast<float>(nsFloat16(-1.0f)), -1.0f, 0);
    NS_TEST_FLOAT(static_cast<float>(nsFloat16(0.0f)), 0.0f, 0);
    NS_TEST_FLOAT(static_cast<float>(nsFloat16(-0.0f)), -0.0f, 0);
    NS_TEST_BOOL(static_cast<float>(nsFloat16(nsMath::Infinity<float>())) == nsMath::Infinity<float>());
    NS_TEST_BOOL(static_cast<float>(nsFloat16(-nsMath::Infinity<float>())) == -nsMath::Infinity<float>());
    NS_TEST_BOOL(nsMath::IsNaN(static_cast<float>(nsFloat16(nsMath::NaN<float>()))));

    // Some random values.
    NS_TEST_FLOAT(static_cast<float>(nsFloat16(42.0f)), 42.0f, nsMath::LargeEpsilon<float>());
    NS_TEST_FLOAT(static_cast<float>(nsFloat16(1.e3f)), 1.e3f, nsMath::LargeEpsilon<float>());
    NS_TEST_FLOAT(static_cast<float>(nsFloat16(-1230.0f)), -1230.0f, nsMath::LargeEpsilon<float>());
    NS_TEST_FLOAT(static_cast<float>(nsFloat16(nsMath::Pi<float>())), nsMath::Pi<float>(), nsMath::HugeEpsilon<float>());

    // Denormalized float.
    NS_TEST_FLOAT(static_cast<float>(nsFloat16(1.e-40f)), 0.0f, 0);
    NS_TEST_FLOAT(static_cast<float>(nsFloat16(1.e-44f)), 0.0f, 0);

    // Clamping of too large/small values
    // Half only supports 2^-14 to 2^14 (in 10^x this is roughly 4.51) (see Wikipedia)
    NS_TEST_FLOAT(static_cast<float>(nsFloat16(1.e-10f)), 0.0f, 0);
    NS_TEST_BOOL(static_cast<float>(nsFloat16(1.e5f)) == nsMath::Infinity<float>());
    NS_TEST_BOOL(static_cast<float>(nsFloat16(-1.e5f)) == -nsMath::Infinity<float>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator ==")
  {
    NS_TEST_BOOL(nsFloat16(1.0f) == nsFloat16(1.0f));
    NS_TEST_BOOL(nsFloat16(10000000.0f) == nsFloat16(10000000.0f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator !=")
  {
    NS_TEST_BOOL(nsFloat16(1.0f) != nsFloat16(-1.0f));
    NS_TEST_BOOL(nsFloat16(10000000.0f) != nsFloat16(10000.0f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetRawData / SetRawData")
  {
    nsFloat16 f;
    f.SetRawData(23);

    NS_TEST_INT(f.GetRawData(), 23);
  }
}
