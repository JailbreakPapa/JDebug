#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Types/Uuid.h>


NS_CREATE_SIMPLE_TEST(Basics, Uuid)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Uuid Generation")
  {
    nsUuid ShouldBeInvalid;

    NS_TEST_BOOL(ShouldBeInvalid.IsValid() == false);

    nsUuid FirstGenerated = nsUuid::MakeUuid();
    NS_TEST_BOOL(FirstGenerated.IsValid());

    nsUuid SecondGenerated = nsUuid::MakeUuid();
    NS_TEST_BOOL(SecondGenerated.IsValid());

    NS_TEST_BOOL(!(FirstGenerated == SecondGenerated));
    NS_TEST_BOOL(FirstGenerated != SecondGenerated);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Uuid Serialization")
  {
    nsUuid Uuid;
    NS_TEST_BOOL(Uuid.IsValid() == false);

    Uuid = nsUuid::MakeUuid();
    NS_TEST_BOOL(Uuid.IsValid());

    nsDefaultMemoryStreamStorage StreamStorage;

    // Create reader
    nsMemoryStreamReader StreamReader(&StreamStorage);

    // Create writer
    nsMemoryStreamWriter StreamWriter(&StreamStorage);

    StreamWriter << Uuid;

    nsUuid ReadBack;
    NS_TEST_BOOL(ReadBack.IsValid() == false);

    StreamReader >> ReadBack;

    NS_TEST_BOOL(ReadBack == Uuid);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Stable Uuid From String")
  {
    nsUuid uuid1 = nsUuid::MakeStableUuidFromString("TEST 1");
    nsUuid uuid2 = nsUuid::MakeStableUuidFromString("TEST 2");
    nsUuid uuid3 = nsUuid::MakeStableUuidFromString("TEST 1");

    NS_TEST_BOOL(uuid1 == uuid3);
    NS_TEST_BOOL(uuid1 != uuid2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Uuid Combine")
  {
    nsUuid uuid1 = nsUuid::MakeUuid();
    nsUuid uuid2 = nsUuid::MakeUuid();
    nsUuid combined = uuid1;
    combined.CombineWithSeed(uuid2);
    NS_TEST_BOOL(combined != uuid1);
    NS_TEST_BOOL(combined != uuid2);
    combined.RevertCombinationWithSeed(uuid2);
    NS_TEST_BOOL(combined == uuid1);

    nsUuid hashA = uuid1;
    hashA.HashCombine(uuid2);
    nsUuid hashB = uuid2;
    hashA.HashCombine(uuid1);
    NS_TEST_BOOL(hashA != hashB);
  }
}
