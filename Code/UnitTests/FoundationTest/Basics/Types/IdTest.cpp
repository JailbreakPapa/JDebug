#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Types/Id.h>

NS_WARNING_PUSH()
NS_WARNING_DISABLE_MSVC(4463)
NS_WARNING_DISABLE_MSVC(4068)
NS_WARNING_DISABLE_GCC("-Wbitfield-constant-conversion")
NS_WARNING_DISABLE_GCC("-Woverflow")
NS_WARNING_DISABLE_CLANG("-Wbitfield-constant-conversion")

struct TestId
{
  using StorageType = nsUInt32;

  NS_DECLARE_ID_TYPE(TestId, 20, 6);

  NS_ALWAYS_INLINE TestId(StorageType instanceIndex, StorageType generation, StorageType systemIndex = 0)
  {
    m_Data = 0;
    m_InstanceIndex = instanceIndex;
    m_Generation = generation;
    m_SystemIndex = systemIndex;
  }

  union
  {
    StorageType m_Data;
    struct
    {
      StorageType m_InstanceIndex : 20;
      StorageType m_Generation : 6;
      StorageType m_SystemIndex : 6;
    };
  };
};

using LargeTestId = nsGenericId<32, 10>;

NS_CREATE_SIMPLE_TEST(Basics, Id)
{
  TestId id1;
  NS_TEST_INT(id1.m_InstanceIndex, TestId::INVALID_INSTANCE_INDEX);
  NS_TEST_INT(id1.m_Generation, 0);
  NS_TEST_INT(id1.m_SystemIndex, 0);

  TestId id2(1, 20, 15);
  TestId id3(1, 84, 79); // overflow
  NS_TEST_INT(id2.m_InstanceIndex, 1);
  NS_TEST_INT(id2.m_Generation, 20);
  NS_TEST_INT(id2.m_SystemIndex, 15);
  NS_TEST_BOOL(id2 == id3);

  id2.m_InstanceIndex = 2;
  NS_TEST_INT(id2.m_InstanceIndex, 2);
  NS_TEST_BOOL(id2 != id3);
  NS_TEST_BOOL(!id2.IsIndexAndGenerationEqual(id3));

  id2.m_InstanceIndex = 1;
  id2.m_SystemIndex = 16;
  NS_TEST_BOOL(id2 != id3);
  NS_TEST_BOOL(id2.IsIndexAndGenerationEqual(id3));

  id2.m_Generation = 94;    // overflow
  NS_TEST_INT(id2.m_Generation, 30);

  id2.m_SystemIndex = 94;   // overflow
  NS_TEST_INT(id2.m_SystemIndex, 30);

  LargeTestId id4(1, 1224); // overflow
  NS_TEST_INT(id4.m_InstanceIndex, 1);
  NS_TEST_INT(id4.m_Generation, 200);
}

NS_WARNING_POP()
