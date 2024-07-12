#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Types/Tag.h>
#include <Foundation/Types/TagRegistry.h>
#include <Foundation/Types/TagSet.h>

static_assert(sizeof(nsTagSet) == 16);

#if NS_ENABLED(NS_PLATFORM_64BIT)
static_assert(sizeof(nsTag) == 16);
#else
static_assert(sizeof(nsTag) == 12);
#endif

NS_CREATE_SIMPLE_TEST(Basics, TagSet)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Basic Tag Tests")
  {
    nsTagRegistry TempTestRegistry;

    {
      nsTag TestTag;
      NS_TEST_BOOL(!TestTag.IsValid());
    }

    nsHashedString TagName;
    TagName.Assign("BASIC_TAG_TEST");

    const nsTag& SecondInstance = TempTestRegistry.RegisterTag(TagName);
    NS_TEST_BOOL(SecondInstance.IsValid());

    const nsTag* SecondInstance2 = TempTestRegistry.GetTagByName("BASIC_TAG_TEST");

    NS_TEST_BOOL(SecondInstance2 != nullptr);
    NS_TEST_BOOL(SecondInstance2->IsValid());

    NS_TEST_BOOL(&SecondInstance == SecondInstance2);

    NS_TEST_STRING(SecondInstance2->GetTagString(), "BASIC_TAG_TEST");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Basic Tag Registration")
  {
    nsTagRegistry TempTestRegistry;

    nsTag TestTag;

    NS_TEST_BOOL(!TestTag.IsValid());

    NS_TEST_BOOL(TempTestRegistry.GetTagByName("TEST_TAG1") == nullptr);

    TestTag = TempTestRegistry.RegisterTag("TEST_TAG1");

    NS_TEST_BOOL(TestTag.IsValid());

    NS_TEST_BOOL(TempTestRegistry.GetTagByName("TEST_TAG1") != nullptr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Basic Tag Work")
  {
    nsTagRegistry TempTestRegistry;

    TempTestRegistry.RegisterTag("TEST_TAG1");

    const nsTag* TestTag1 = TempTestRegistry.GetTagByName("TEST_TAG1");
    NS_TEST_BOOL(TestTag1 != nullptr);

    const nsTag& TestTag2 = TempTestRegistry.RegisterTag("TEST_TAG2");

    NS_TEST_BOOL(TestTag2.IsValid());

    nsTagSet tagSet;

    NS_TEST_BOOL(tagSet.IsSet(*TestTag1) == false);
    NS_TEST_BOOL(tagSet.IsSet(TestTag2) == false);

    tagSet.Set(TestTag2);

    NS_TEST_BOOL(tagSet.IsSet(*TestTag1) == false);
    NS_TEST_BOOL(tagSet.IsSet(TestTag2) == true);
    NS_TEST_INT(tagSet.GetNumTagsSet(), 1);

    tagSet.Set(*TestTag1);

    NS_TEST_BOOL(tagSet.IsSet(*TestTag1) == true);
    NS_TEST_BOOL(tagSet.IsSet(TestTag2) == true);
    NS_TEST_INT(tagSet.GetNumTagsSet(), 2);

    tagSet.Remove(*TestTag1);

    NS_TEST_BOOL(tagSet.IsSet(*TestTag1) == false);
    NS_TEST_BOOL(tagSet.IsSet(TestTag2) == true);
    NS_TEST_INT(tagSet.GetNumTagsSet(), 1);

    nsTagSet tagSet2 = tagSet;
    NS_TEST_BOOL(tagSet2.IsSet(*TestTag1) == false);
    NS_TEST_BOOL(tagSet2.IsSet(TestTag2) == true);
    NS_TEST_INT(tagSet2.GetNumTagsSet(), 1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Many Tags")
  {
    nsTagRegistry TempTestRegistry;

    // TagSets have local storage for 1 block (64 tags)
    // Allocate enough tags so the storage overflows (or doesn't start at block 0)
    // for these tests

    nsTag RegisteredTags[250];

    // Pre register some tags
    TempTestRegistry.RegisterTag("TEST_TAG1");
    TempTestRegistry.RegisterTag("TEST_TAG2");

    for (nsUInt32 i = 0; i < 250; ++i)
    {
      nsStringBuilder TagName;
      TagName.SetFormat("TEST_TAG{0}", i);

      RegisteredTags[i] = TempTestRegistry.RegisterTag(TagName.GetData());

      NS_TEST_BOOL(RegisteredTags[i].IsValid());
    }

    NS_TEST_INT(TempTestRegistry.GetNumTags(), 250);

    // Set all tags
    nsTagSet BigTagSet;

    BigTagSet.Set(RegisteredTags[128]);
    BigTagSet.Set(RegisteredTags[64]);
    BigTagSet.Set(RegisteredTags[0]);

    NS_TEST_BOOL(BigTagSet.IsSet(RegisteredTags[0]));
    NS_TEST_BOOL(BigTagSet.IsSet(RegisteredTags[64]));
    NS_TEST_BOOL(BigTagSet.IsSet(RegisteredTags[128]));

    for (nsUInt32 i = 0; i < 250; ++i)
    {
      BigTagSet.Set(RegisteredTags[i]);
    }

    for (nsUInt32 i = 0; i < 250; ++i)
    {
      NS_TEST_BOOL(BigTagSet.IsSet(RegisteredTags[i]));
    }

    for (nsUInt32 i = 10; i < 60; ++i)
    {
      BigTagSet.Remove(RegisteredTags[i]);
    }

    for (nsUInt32 i = 0; i < 10; ++i)
    {
      NS_TEST_BOOL(BigTagSet.IsSet(RegisteredTags[i]));
    }

    for (nsUInt32 i = 10; i < 60; ++i)
    {
      NS_TEST_BOOL(!BigTagSet.IsSet(RegisteredTags[i]));
    }

    for (nsUInt32 i = 60; i < 250; ++i)
    {
      NS_TEST_BOOL(BigTagSet.IsSet(RegisteredTags[i]));
    }

    // Set tags, but starting outside block 0. This should do no allocation
    nsTagSet Non0BlockStartSet;
    Non0BlockStartSet.Set(RegisteredTags[100]);
    NS_TEST_BOOL(Non0BlockStartSet.IsSet(RegisteredTags[100]));
    NS_TEST_BOOL(!Non0BlockStartSet.IsSet(RegisteredTags[0]));

    nsTagSet Non0BlockStartSet2 = Non0BlockStartSet;
    NS_TEST_BOOL(Non0BlockStartSet2.IsSet(RegisteredTags[100]));
    NS_TEST_INT(Non0BlockStartSet2.GetNumTagsSet(), Non0BlockStartSet.GetNumTagsSet());

    // Also test allocating a tag in an earlier block than the first tag allocated in the set
    Non0BlockStartSet.Set(RegisteredTags[0]);
    NS_TEST_BOOL(Non0BlockStartSet.IsSet(RegisteredTags[100]));
    NS_TEST_BOOL(Non0BlockStartSet.IsSet(RegisteredTags[0]));

    // Copying a tag set should work as well
    nsTagSet SecondTagSet = BigTagSet;

    for (nsUInt32 i = 60; i < 250; ++i)
    {
      NS_TEST_BOOL(SecondTagSet.IsSet(RegisteredTags[i]));
    }

    for (nsUInt32 i = 10; i < 60; ++i)
    {
      NS_TEST_BOOL(!SecondTagSet.IsSet(RegisteredTags[i]));
    }

    NS_TEST_INT(SecondTagSet.GetNumTagsSet(), BigTagSet.GetNumTagsSet());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsAnySet")
  {
    nsTagRegistry TempTestRegistry;

    // TagSets have local storage for 1 block (64 tags)
    // Allocate enough tags so the storage overflows (or doesn't start at block 0)
    // for these tests

    nsTag RegisteredTags[250];

    for (nsUInt32 i = 0; i < 250; ++i)
    {
      nsStringBuilder TagName;
      TagName.SetFormat("TEST_TAG{0}", i);

      RegisteredTags[i] = TempTestRegistry.RegisterTag(TagName.GetData());

      NS_TEST_BOOL(RegisteredTags[i].IsValid());
    }

    nsTagSet EmptyTagSet;
    nsTagSet SecondEmptyTagSet;

    NS_TEST_BOOL(!EmptyTagSet.IsAnySet(SecondEmptyTagSet));
    NS_TEST_BOOL(!SecondEmptyTagSet.IsAnySet(EmptyTagSet));


    nsTagSet SimpleSingleTagBlock0;
    SimpleSingleTagBlock0.Set(RegisteredTags[0]);

    nsTagSet SimpleSingleTagBlock1;
    SimpleSingleTagBlock1.Set(RegisteredTags[0]);

    NS_TEST_BOOL(!SecondEmptyTagSet.IsAnySet(SimpleSingleTagBlock0));

    NS_TEST_BOOL(SimpleSingleTagBlock0.IsAnySet(SimpleSingleTagBlock0));
    NS_TEST_BOOL(SimpleSingleTagBlock0.IsAnySet(SimpleSingleTagBlock1));

    SimpleSingleTagBlock1.Remove(RegisteredTags[0]);
    NS_TEST_BOOL(!SimpleSingleTagBlock1.IsAnySet(SimpleSingleTagBlock0));

    // Try with different block sizes/offsets (but same bit index)
    SimpleSingleTagBlock1.Set(RegisteredTags[64]);

    NS_TEST_BOOL(!SimpleSingleTagBlock1.IsAnySet(SimpleSingleTagBlock0));
    NS_TEST_BOOL(!SimpleSingleTagBlock0.IsAnySet(SimpleSingleTagBlock1));

    SimpleSingleTagBlock0.Set(RegisteredTags[65]);
    NS_TEST_BOOL(!SimpleSingleTagBlock1.IsAnySet(SimpleSingleTagBlock0));
    NS_TEST_BOOL(!SimpleSingleTagBlock0.IsAnySet(SimpleSingleTagBlock1));

    SimpleSingleTagBlock0.Set(RegisteredTags[64]);
    NS_TEST_BOOL(SimpleSingleTagBlock1.IsAnySet(SimpleSingleTagBlock0));
    NS_TEST_BOOL(SimpleSingleTagBlock0.IsAnySet(SimpleSingleTagBlock1));

    nsTagSet OffsetBlock;
    OffsetBlock.Set(RegisteredTags[65]);
    NS_TEST_BOOL(OffsetBlock.IsAnySet(SimpleSingleTagBlock0));
    NS_TEST_BOOL(SimpleSingleTagBlock0.IsAnySet(OffsetBlock));

    nsTagSet OffsetBlock2;
    OffsetBlock2.Set(RegisteredTags[66]);
    NS_TEST_BOOL(!OffsetBlock.IsAnySet(OffsetBlock2));
    NS_TEST_BOOL(!OffsetBlock2.IsAnySet(OffsetBlock));

    OffsetBlock2.Set(RegisteredTags[65]);
    NS_TEST_BOOL(OffsetBlock.IsAnySet(OffsetBlock2));
    NS_TEST_BOOL(OffsetBlock2.IsAnySet(OffsetBlock));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Add / Remove / IsEmpty / Clear")
  {
    nsTagRegistry TempTestRegistry;

    TempTestRegistry.RegisterTag("TEST_TAG1");

    const nsTag* TestTag1 = TempTestRegistry.GetTagByName("TEST_TAG1");
    NS_TEST_BOOL(TestTag1 != nullptr);

    const nsTag& TestTag2 = TempTestRegistry.RegisterTag("TEST_TAG2");

    NS_TEST_BOOL(TestTag2.IsValid());

    nsTagSet tagSet;

    NS_TEST_BOOL(tagSet.IsEmpty());
    NS_TEST_BOOL(tagSet.IsSet(*TestTag1) == false);
    NS_TEST_BOOL(tagSet.IsSet(TestTag2) == false);

    tagSet.Clear();

    NS_TEST_BOOL(tagSet.IsEmpty());
    NS_TEST_BOOL(tagSet.IsSet(*TestTag1) == false);
    NS_TEST_BOOL(tagSet.IsSet(TestTag2) == false);

    tagSet.Set(TestTag2);

    NS_TEST_BOOL(!tagSet.IsEmpty());
    NS_TEST_BOOL(tagSet.IsSet(*TestTag1) == false);
    NS_TEST_BOOL(tagSet.IsSet(TestTag2) == true);

    tagSet.Remove(TestTag2);

    NS_TEST_BOOL(tagSet.IsEmpty());
    NS_TEST_BOOL(tagSet.IsSet(*TestTag1) == false);
    NS_TEST_BOOL(tagSet.IsSet(TestTag2) == false);

    tagSet.Set(*TestTag1);
    tagSet.Set(TestTag2);

    NS_TEST_BOOL(!tagSet.IsEmpty());
    NS_TEST_BOOL(tagSet.IsSet(*TestTag1) == true);
    NS_TEST_BOOL(tagSet.IsSet(TestTag2) == true);

    tagSet.Remove(*TestTag1);
    tagSet.Remove(TestTag2);

    NS_TEST_BOOL(tagSet.IsEmpty());
    NS_TEST_BOOL(tagSet.IsSet(*TestTag1) == false);
    NS_TEST_BOOL(tagSet.IsSet(TestTag2) == false);

    tagSet.Set(*TestTag1);
    tagSet.Set(TestTag2);

    NS_TEST_BOOL(!tagSet.IsEmpty());
    NS_TEST_BOOL(tagSet.IsSet(*TestTag1) == true);
    NS_TEST_BOOL(tagSet.IsSet(TestTag2) == true);

    tagSet.Clear();

    NS_TEST_BOOL(tagSet.IsEmpty());
    NS_TEST_BOOL(tagSet.IsSet(*TestTag1) == false);
    NS_TEST_BOOL(tagSet.IsSet(TestTag2) == false);
  }
}
