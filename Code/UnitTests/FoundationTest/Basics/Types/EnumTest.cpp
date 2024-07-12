#include <FoundationTest/FoundationTestPCH.h>

//////////////////////////////////////////////////////////////////////
// Start of the definition of a example Enum
// It takes quite some lines of code to define a enum,
// but it could be encapsulated into an preprocessor macro if wanted
struct nsTestEnumBase
{
  using StorageType = nsUInt8; // The storage type for the enum

  enum Enum
  {
    No = 0,
    Yes = 1,
    Default = No // Default initialization
  };
};

using nsTestEnum = nsEnum<nsTestEnumBase>; // The name of the final enum
// End of the definition of a example enum
///////////////////////////////////////////////////////////////////////

struct nsTestEnum2Base
{
  using StorageType = nsUInt16;

  enum Enum
  {
    Bit1 = NS_BIT(0),
    Bit2 = NS_BIT(1),
    Default = Bit1
  };
};

using nsTestEnum2 = nsEnum<nsTestEnum2Base>;

// Test if the type actually has the requested size
NS_CHECK_AT_COMPILETIME(sizeof(nsTestEnum) == sizeof(nsUInt8));
NS_CHECK_AT_COMPILETIME(sizeof(nsTestEnum2) == sizeof(nsUInt16));

NS_CREATE_SIMPLE_TEST_GROUP(Basics);

// This takes a c++ enum. Tests the implict conversion
void TakeEnum1(nsTestEnum::Enum value) {}

// This takes our own enum type
void TakeEnum2(nsTestEnum value) {}

NS_CREATE_SIMPLE_TEST(Basics, Enum)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Default initialized enum")
  {
    nsTestEnum e1;
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Enum with explicit initialization")
  {
    nsTestEnum e2(nsTestEnum::Yes);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "This tests if the default initialization works and if the implicit conversion works")
  {
    nsTestEnum e1;
    nsTestEnum e2(nsTestEnum::Yes);

    NS_TEST_BOOL(e1 == nsTestEnum::No);
    NS_TEST_BOOL(e2 == nsTestEnum::Yes);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Function call tests")
  {
    nsTestEnum e1;

    TakeEnum1(e1);
    TakeEnum2(e1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetValue and SetValue")
  {
    nsTestEnum e1;
    NS_TEST_INT(e1.GetValue(), 0);
    e1.SetValue(17);
    NS_TEST_INT(e1.GetValue(), 17);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Assignment of different values")
  {
    nsTestEnum e1, e2;

    e1 = nsTestEnum::Yes;
    e2 = nsTestEnum::No;
    NS_TEST_BOOL(e1 == nsTestEnum::Yes);
    NS_TEST_BOOL(e2 == nsTestEnum::No);

    e1 = e2;
    NS_TEST_BOOL(e1 == nsTestEnum::No);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Test the | operator")
  {
    nsTestEnum2 e3(nsTestEnum2::Bit1);
    nsTestEnum2 e4(nsTestEnum2::Bit2);
    nsUInt16 uiBits = (e3 | e4).GetValue();
    NS_TEST_BOOL(uiBits == (nsTestEnum2::Bit1 | nsTestEnum2::Bit2));
  }


  NS_TEST_BLOCK(nsTestBlock::Enabled, "Test the & operator")
  {
    nsTestEnum2 e3(nsTestEnum2::Bit1);
    nsTestEnum2 e4(nsTestEnum2::Bit2);
    nsUInt16 uiBits = ((e3 | e4) & e4).GetValue();
    NS_TEST_BOOL(uiBits == nsTestEnum2::Bit2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Test conversion to int")
  {
    nsTestEnum e1;
    int iTest = e1.GetValue();
    NS_TEST_BOOL(iTest == nsTestEnum::No);
  }
}
