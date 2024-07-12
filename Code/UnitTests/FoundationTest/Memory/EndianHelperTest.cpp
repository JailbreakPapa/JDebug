#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Memory/EndianHelper.h>

namespace
{
  struct TempStruct
  {
    float fVal;
    nsUInt32 uiDVal;
    nsUInt16 uiWVal1;
    nsUInt16 uiWVal2;
    char pad[4];
  };

  struct FloatAndInt
  {
    union
    {
      float fVal;
      nsUInt32 uiVal;
    };
  };
} // namespace


NS_CREATE_SIMPLE_TEST(Memory, Endian)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Basics")
  {
// Test if the IsBigEndian() delivers the same result as the #define
#if NS_ENABLED(NS_PLATFORM_LITTLE_ENDIAN)
    NS_TEST_BOOL(!nsEndianHelper::IsBigEndian());
#elif NS_ENABLED(NS_PLATFORM_BIG_ENDIAN)
    NS_TEST_BOOL(nsEndianHelper::IsBigEndian());
#endif

    // Test conversion functions for single elements
    NS_TEST_BOOL(nsEndianHelper::Switch(static_cast<nsUInt16>(0x15FF)) == 0xFF15);
    NS_TEST_BOOL(nsEndianHelper::Switch(static_cast<nsUInt32>(0x34AA12FF)) == 0xFF12AA34);
    NS_TEST_BOOL(nsEndianHelper::Switch(static_cast<nsUInt64>(0x34AA12FFABC3421E)) == 0x1E42C3ABFF12AA34);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Switching Arrays")
  {
    nsArrayPtr<nsUInt16> p16BitArray = NS_DEFAULT_NEW_ARRAY(nsUInt16, 1024);
    nsArrayPtr<nsUInt16> p16BitArrayCopy = NS_DEFAULT_NEW_ARRAY(nsUInt16, 1024);

    nsArrayPtr<nsUInt32> p32BitArray = NS_DEFAULT_NEW_ARRAY(nsUInt32, 1024);
    nsArrayPtr<nsUInt32> p32BitArrayCopy = NS_DEFAULT_NEW_ARRAY(nsUInt32, 1024);

    nsArrayPtr<nsUInt64> p64BitArray = NS_DEFAULT_NEW_ARRAY(nsUInt64, 1024);
    nsArrayPtr<nsUInt64> p64BitArrayCopy = NS_DEFAULT_NEW_ARRAY(nsUInt64, 1024);

    for (nsUInt32 i = 0; i < 1024; i++)
    {
      nsInt32 iRand = rand();
      p16BitArray[i] = static_cast<nsUInt16>(iRand);
      p32BitArray[i] = static_cast<nsUInt32>(iRand);
      p64BitArray[i] = static_cast<nsUInt64>(iRand | static_cast<nsUInt64>((iRand % 3)) << 32);
    }

    p16BitArrayCopy.CopyFrom(p16BitArray);
    p32BitArrayCopy.CopyFrom(p32BitArray);
    p64BitArrayCopy.CopyFrom(p64BitArray);

    nsEndianHelper::SwitchWords(p16BitArray.GetPtr(), 1024);
    nsEndianHelper::SwitchDWords(p32BitArray.GetPtr(), 1024);
    nsEndianHelper::SwitchQWords(p64BitArray.GetPtr(), 1024);

    for (nsUInt32 i = 0; i < 1024; i++)
    {
      NS_TEST_BOOL(p16BitArray[i] == nsEndianHelper::Switch(p16BitArrayCopy[i]));
      NS_TEST_BOOL(p32BitArray[i] == nsEndianHelper::Switch(p32BitArrayCopy[i]));
      NS_TEST_BOOL(p64BitArray[i] == nsEndianHelper::Switch(p64BitArrayCopy[i]));

      // Test in place switcher
      nsEndianHelper::SwitchInPlace(&p16BitArrayCopy[i]);
      NS_TEST_BOOL(p16BitArray[i] == p16BitArrayCopy[i]);

      nsEndianHelper::SwitchInPlace(&p32BitArrayCopy[i]);
      NS_TEST_BOOL(p32BitArray[i] == p32BitArrayCopy[i]);

      nsEndianHelper::SwitchInPlace(&p64BitArrayCopy[i]);
      NS_TEST_BOOL(p64BitArray[i] == p64BitArrayCopy[i]);
    }


    NS_DEFAULT_DELETE_ARRAY(p16BitArray);
    NS_DEFAULT_DELETE_ARRAY(p16BitArrayCopy);

    NS_DEFAULT_DELETE_ARRAY(p32BitArray);
    NS_DEFAULT_DELETE_ARRAY(p32BitArrayCopy);

    NS_DEFAULT_DELETE_ARRAY(p64BitArray);
    NS_DEFAULT_DELETE_ARRAY(p64BitArrayCopy);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Switching Structs")
  {
    TempStruct instance = {42.0f, 0x34AA12FF, 0x15FF, 0x23FF, {'E', 'Z', 'F', 'T'}};

    nsEndianHelper::SwitchStruct(&instance, "ddwwcccc");

    nsIntFloatUnion floatHelper(42.0f);
    nsIntFloatUnion floatHelper2(instance.fVal);

    NS_TEST_BOOL(floatHelper2.i == nsEndianHelper::Switch(floatHelper.i));
    NS_TEST_BOOL(instance.uiDVal == nsEndianHelper::Switch(static_cast<nsUInt32>(0x34AA12FF)));
    NS_TEST_BOOL(instance.uiWVal1 == nsEndianHelper::Switch(static_cast<nsUInt16>(0x15FF)));
    NS_TEST_BOOL(instance.uiWVal2 == nsEndianHelper::Switch(static_cast<nsUInt16>(0x23FF)));
    NS_TEST_BOOL(instance.pad[0] == 'E');
    NS_TEST_BOOL(instance.pad[1] == 'Z');
    NS_TEST_BOOL(instance.pad[2] == 'F');
    NS_TEST_BOOL(instance.pad[3] == 'T');
  }
}
