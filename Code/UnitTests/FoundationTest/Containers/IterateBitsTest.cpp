#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/IterateBits.h>

namespace
{
  template <typename T>
  void TestEmptyIntegerBitValues()
  {
    nsUInt32 uiNextBit = 1;
    for (auto bit : nsIterateBitValues(static_cast<T>(0)))
    {
      NS_TEST_BOOL_MSG(false, "No bit should be present");
    }
  }

  template <typename T>
  void TestFullIntegerBitValues()
  {
    constexpr nsUInt64 uiBitCount = sizeof(T) * 8;
    nsUInt64 uiNextBit = 1;
    nsUInt64 uiCount = 0;
    for (auto bit : nsIterateBitValues(nsMath::MaxValue<T>()))
    {
      NS_TEST_INT(bit, uiNextBit);
      uiNextBit *= 2;
      uiCount++;
    }
    NS_TEST_INT(uiBitCount, uiCount);
  }

  template <typename T>
  void TestEmptyIntegerBitIndices()
  {
    nsUInt32 uiNextBit = 1;
    for (auto bit : nsIterateBitIndices(static_cast<T>(0)))
    {
      NS_TEST_BOOL_MSG(false, "No bit should be present");
    }
  }

  template <typename T>
  void TestFullIntegerBitIndices()
  {
    constexpr nsUInt64 uiBitCount = sizeof(T) * 8;
    nsUInt64 uiNextBitIndex = 0;
    for (auto bit : nsIterateBitIndices(nsMath::MaxValue<T>()))
    {
      NS_TEST_INT(bit, uiNextBitIndex);
      ++uiNextBitIndex;
    }
    NS_TEST_INT(uiBitCount, uiNextBitIndex);
  }
} // namespace

NS_CREATE_SIMPLE_TEST(Containers, IterateBits)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsIterateBitValues")
  {
    {
      // Empty set
      TestEmptyIntegerBitValues<nsUInt8>();
      TestEmptyIntegerBitValues<nsUInt16>();
      TestEmptyIntegerBitValues<nsUInt32>();
      TestEmptyIntegerBitValues<nsUInt64>();
    }

    {
      // Full sets
      TestFullIntegerBitValues<nsUInt8>();
      TestFullIntegerBitValues<nsUInt16>();
      TestFullIntegerBitValues<nsUInt32>();
      TestFullIntegerBitValues<nsUInt64>();
    }

    {
      // Some bits set
      nsUInt64 uiBitMask = 0b1101;
      nsHybridArray<nsUInt64, 3> bits;
      bits.PushBack(0b0001);
      bits.PushBack(0b0100);
      bits.PushBack(0b1000);

      for (nsUInt64 bit : nsIterateBitValues(uiBitMask))
      {
        NS_TEST_INT(bit, bits[0]);
        bits.RemoveAtAndCopy(0);
      }
      NS_TEST_BOOL(bits.IsEmpty());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsIterateBitIndices")
  {
    {
      // Empty set
      TestEmptyIntegerBitIndices<nsUInt8>();
      TestEmptyIntegerBitIndices<nsUInt16>();
      TestEmptyIntegerBitIndices<nsUInt32>();
      TestEmptyIntegerBitIndices<nsUInt64>();
    }

    {
      // Full sets
      TestFullIntegerBitIndices<nsUInt8>();
      TestFullIntegerBitIndices<nsUInt16>();
      TestFullIntegerBitIndices<nsUInt32>();
      TestFullIntegerBitIndices<nsUInt64>();
    }

    {
      // Some bits set
      nsUInt64 uiBitMask = 0b1101;
      nsHybridArray<nsUInt64, 3> bits;
      bits.PushBack(0);
      bits.PushBack(2);
      bits.PushBack(3);

      for (nsUInt64 bit : nsIterateBitIndices(uiBitMask))
      {
        NS_TEST_INT(bit, bits[0]);
        bits.RemoveAtAndCopy(0);
      }
      NS_TEST_BOOL(bits.IsEmpty());
    }
  }
}
