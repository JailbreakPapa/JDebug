#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/HybridArray.h>

static nsInt32 iCallPodConstructor = 0;
static nsInt32 iCallPodDestructor = 0;
static nsInt32 iCallNonPodConstructor = 0;
static nsInt32 iCallNonPodDestructor = 0;

struct nsConstructTest
{
public:
  static nsHybridArray<void*, 10> s_dtorList;

  nsConstructTest() { m_iData = 42; }

  ~nsConstructTest() { s_dtorList.PushBack(this); }

  nsInt32 m_iData;
};
nsHybridArray<void*, 10> nsConstructTest::s_dtorList;

NS_CHECK_AT_COMPILETIME(sizeof(nsConstructTest) == 4);


struct PODTest
{
  NS_DECLARE_POD_TYPE();

  PODTest() { m_iData = -1; }

  nsInt32 m_iData;
};

static const nsUInt32 s_uiSize = sizeof(nsConstructTest);

NS_CREATE_SIMPLE_TEST(Memory, MemoryUtils)
{
  nsConstructTest::s_dtorList.Clear();

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Construct")
  {
    nsUInt8 uiRawData[s_uiSize * 5] = {0};
    nsConstructTest* pTest = (nsConstructTest*)(uiRawData);

    nsMemoryUtils::Construct<SkipTrivialTypes, nsConstructTest>(pTest + 1, 2);

    NS_TEST_INT(pTest[0].m_iData, 0);
    NS_TEST_INT(pTest[1].m_iData, 42);
    NS_TEST_INT(pTest[2].m_iData, 42);
    NS_TEST_INT(pTest[3].m_iData, 0);
    NS_TEST_INT(pTest[4].m_iData, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeConstructorFunction")
  {
    nsMemoryUtils::ConstructorFunction func = nsMemoryUtils::MakeConstructorFunction<SkipTrivialTypes, nsConstructTest>();
    NS_TEST_BOOL(func != nullptr);

    nsUInt8 uiRawData[s_uiSize] = {0};
    nsConstructTest* pTest = (nsConstructTest*)(uiRawData);

    (*func)(pTest);

    NS_TEST_INT(pTest->m_iData, 42);

    func = nsMemoryUtils::MakeConstructorFunction<SkipTrivialTypes, PODTest>();
    NS_TEST_BOOL(func != nullptr);

    func = nsMemoryUtils::MakeConstructorFunction<SkipTrivialTypes, nsInt32>();
    NS_TEST_BOOL(func == nullptr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "DefaultConstruct")
  {
    nsUInt32 uiRawData[5]; // not initialized here

    nsMemoryUtils::Construct<ConstructAll>(uiRawData + 1, 2);

    NS_TEST_INT(uiRawData[1], 0);
    NS_TEST_INT(uiRawData[2], 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Construct Copy(Array)")
  {
    nsUInt8 uiRawData[s_uiSize * 5] = {0};
    nsConstructTest* pTest = (nsConstructTest*)(uiRawData);

    nsConstructTest copy[2];
    copy[0].m_iData = 43;
    copy[1].m_iData = 44;

    nsMemoryUtils::CopyConstructArray<nsConstructTest>(pTest + 1, copy, 2);

    NS_TEST_INT(pTest[0].m_iData, 0);
    NS_TEST_INT(pTest[1].m_iData, 43);
    NS_TEST_INT(pTest[2].m_iData, 44);
    NS_TEST_INT(pTest[3].m_iData, 0);
    NS_TEST_INT(pTest[4].m_iData, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Construct Copy(Element)")
  {
    nsUInt8 uiRawData[s_uiSize * 5] = {0};
    nsConstructTest* pTest = (nsConstructTest*)(uiRawData);

    nsConstructTest copy;
    copy.m_iData = 43;

    nsMemoryUtils::CopyConstruct<nsConstructTest>(pTest + 1, copy, 2);

    NS_TEST_INT(pTest[0].m_iData, 0);
    NS_TEST_INT(pTest[1].m_iData, 43);
    NS_TEST_INT(pTest[2].m_iData, 43);
    NS_TEST_INT(pTest[3].m_iData, 0);
    NS_TEST_INT(pTest[4].m_iData, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeCopyConstructorFunction")
  {
    nsMemoryUtils::CopyConstructorFunction func = nsMemoryUtils::MakeCopyConstructorFunction<nsConstructTest>();
    NS_TEST_BOOL(func != nullptr);

    nsUInt8 uiRawData[s_uiSize] = {0};
    nsConstructTest* pTest = (nsConstructTest*)(uiRawData);

    nsConstructTest copy;
    copy.m_iData = 43;

    (*func)(pTest, &copy);

    NS_TEST_INT(pTest->m_iData, 43);

    func = nsMemoryUtils::MakeCopyConstructorFunction<PODTest>();
    NS_TEST_BOOL(func != nullptr);

    func = nsMemoryUtils::MakeCopyConstructorFunction<nsInt32>();
    NS_TEST_BOOL(func != nullptr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Destruct")
  {
    nsUInt8 uiRawData[s_uiSize * 5] = {0};
    nsConstructTest* pTest = (nsConstructTest*)(uiRawData);

    nsMemoryUtils::Construct<SkipTrivialTypes, nsConstructTest>(pTest + 1, 2);

    NS_TEST_INT(pTest[0].m_iData, 0);
    NS_TEST_INT(pTest[1].m_iData, 42);
    NS_TEST_INT(pTest[2].m_iData, 42);
    NS_TEST_INT(pTest[3].m_iData, 0);
    NS_TEST_INT(pTest[4].m_iData, 0);

    nsConstructTest::s_dtorList.Clear();
    nsMemoryUtils::Destruct<nsConstructTest>(pTest, 4);
    NS_TEST_INT(4, nsConstructTest::s_dtorList.GetCount());

    if (nsConstructTest::s_dtorList.GetCount() == 4)
    {
      NS_TEST_BOOL(nsConstructTest::s_dtorList[0] == &pTest[0]);
      NS_TEST_BOOL(nsConstructTest::s_dtorList[1] == &pTest[1]);
      NS_TEST_BOOL(nsConstructTest::s_dtorList[2] == &pTest[2]);
      NS_TEST_BOOL(nsConstructTest::s_dtorList[3] == &pTest[3]);
      NS_TEST_INT(pTest[4].m_iData, 0);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeDestructorFunction")
  {
    nsMemoryUtils::DestructorFunction func = nsMemoryUtils::MakeDestructorFunction<nsConstructTest>();
    NS_TEST_BOOL(func != nullptr);

    nsUInt8 uiRawData[s_uiSize] = {0};
    nsConstructTest* pTest = (nsConstructTest*)(uiRawData);

    nsMemoryUtils::Construct<SkipTrivialTypes>(pTest, 1);
    NS_TEST_INT(pTest->m_iData, 42);

    nsConstructTest::s_dtorList.Clear();
    (*func)(pTest);
    NS_TEST_INT(1, nsConstructTest::s_dtorList.GetCount());

    if (nsConstructTest::s_dtorList.GetCount() == 1)
    {
      NS_TEST_BOOL(nsConstructTest::s_dtorList[0] == pTest);
    }

    func = nsMemoryUtils::MakeDestructorFunction<PODTest>();
    NS_TEST_BOOL(func == nullptr);

    func = nsMemoryUtils::MakeDestructorFunction<nsInt32>();
    NS_TEST_BOOL(func == nullptr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Copy")
  {
    nsUInt8 uiRawData[5] = {1, 2, 3, 4, 5};
    nsUInt8 uiRawData2[5] = {6, 7, 8, 9, 0};

    NS_TEST_INT(uiRawData[0], 1);
    NS_TEST_INT(uiRawData[1], 2);
    NS_TEST_INT(uiRawData[2], 3);
    NS_TEST_INT(uiRawData[3], 4);
    NS_TEST_INT(uiRawData[4], 5);

    nsMemoryUtils::Copy(uiRawData + 1, uiRawData2 + 2, 3);

    NS_TEST_INT(uiRawData[0], 1);
    NS_TEST_INT(uiRawData[1], 8);
    NS_TEST_INT(uiRawData[2], 9);
    NS_TEST_INT(uiRawData[3], 0);
    NS_TEST_INT(uiRawData[4], 5);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Move")
  {
    nsUInt8 uiRawData[5] = {1, 2, 3, 4, 5};

    NS_TEST_INT(uiRawData[0], 1);
    NS_TEST_INT(uiRawData[1], 2);
    NS_TEST_INT(uiRawData[2], 3);
    NS_TEST_INT(uiRawData[3], 4);
    NS_TEST_INT(uiRawData[4], 5);

    nsMemoryUtils::CopyOverlapped(uiRawData + 1, uiRawData + 3, 2);

    NS_TEST_INT(uiRawData[0], 1);
    NS_TEST_INT(uiRawData[1], 4);
    NS_TEST_INT(uiRawData[2], 5);
    NS_TEST_INT(uiRawData[3], 4);
    NS_TEST_INT(uiRawData[4], 5);

    nsMemoryUtils::CopyOverlapped(uiRawData + 1, uiRawData, 4);

    NS_TEST_INT(uiRawData[0], 1);
    NS_TEST_INT(uiRawData[1], 1);
    NS_TEST_INT(uiRawData[2], 4);
    NS_TEST_INT(uiRawData[3], 5);
    NS_TEST_INT(uiRawData[4], 4);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqual")
  {
    nsUInt8 uiRawData1[5] = {1, 2, 3, 4, 5};
    nsUInt8 uiRawData2[5] = {1, 2, 3, 4, 5};
    nsUInt8 uiRawData3[5] = {1, 2, 3, 4, 6};

    NS_TEST_BOOL(nsMemoryUtils::IsEqual(uiRawData1, uiRawData2, 5));
    NS_TEST_BOOL(!nsMemoryUtils::IsEqual(uiRawData1, uiRawData3, 5));
    NS_TEST_BOOL(nsMemoryUtils::IsEqual(uiRawData1, uiRawData3, 4));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ZeroFill")
  {
    nsUInt8 uiRawData[5] = {1, 2, 3, 4, 5};

    NS_TEST_INT(uiRawData[0], 1);
    NS_TEST_INT(uiRawData[1], 2);
    NS_TEST_INT(uiRawData[2], 3);
    NS_TEST_INT(uiRawData[3], 4);
    NS_TEST_INT(uiRawData[4], 5);

    // T*, size_t N overload
    nsMemoryUtils::ZeroFill(uiRawData + 1, 3);

    NS_TEST_INT(uiRawData[0], 1);
    NS_TEST_INT(uiRawData[1], 0);
    NS_TEST_INT(uiRawData[2], 0);
    NS_TEST_INT(uiRawData[3], 0);
    NS_TEST_INT(uiRawData[4], 5);

    // T[N] overload
    nsMemoryUtils::ZeroFillArray(uiRawData);

    NS_TEST_INT(uiRawData[0], 0);
    NS_TEST_INT(uiRawData[1], 0);
    NS_TEST_INT(uiRawData[2], 0);
    NS_TEST_INT(uiRawData[3], 0);
    NS_TEST_INT(uiRawData[4], 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "PatternFill")
  {
    nsUInt8 uiRawData[5] = {1, 2, 3, 4, 5};

    NS_TEST_INT(uiRawData[0], 1);
    NS_TEST_INT(uiRawData[1], 2);
    NS_TEST_INT(uiRawData[2], 3);
    NS_TEST_INT(uiRawData[3], 4);
    NS_TEST_INT(uiRawData[4], 5);

    // T*, size_t N overload
    nsMemoryUtils::PatternFill(uiRawData + 1, 0xAB, 3);

    NS_TEST_INT(uiRawData[0], 1);
    NS_TEST_INT(uiRawData[1], 0xAB);
    NS_TEST_INT(uiRawData[2], 0xAB);
    NS_TEST_INT(uiRawData[3], 0xAB);
    NS_TEST_INT(uiRawData[4], 5);

    // T[N] overload
    nsMemoryUtils::PatternFillArray(uiRawData, 0xCD);

    NS_TEST_INT(uiRawData[0], 0xCD);
    NS_TEST_INT(uiRawData[1], 0xCD);
    NS_TEST_INT(uiRawData[2], 0xCD);
    NS_TEST_INT(uiRawData[3], 0xCD);
    NS_TEST_INT(uiRawData[4], 0xCD);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Compare")
  {
    nsUInt32 uiRawDataA[3] = {1, 2, 3};
    nsUInt32 uiRawDataB[3] = {3, 4, 5};

    NS_TEST_INT(uiRawDataA[0], 1);
    NS_TEST_INT(uiRawDataA[1], 2);
    NS_TEST_INT(uiRawDataA[2], 3);
    NS_TEST_INT(uiRawDataB[0], 3);
    NS_TEST_INT(uiRawDataB[1], 4);
    NS_TEST_INT(uiRawDataB[2], 5);

    NS_TEST_BOOL(nsMemoryUtils::Compare(uiRawDataA, uiRawDataB, 3) < 0);
    NS_TEST_BOOL(nsMemoryUtils::Compare(uiRawDataA + 2, uiRawDataB, 1) == 0);
    NS_TEST_BOOL(nsMemoryUtils::Compare(uiRawDataB, uiRawDataA, 3) > 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddByteOffset")
  {
    nsInt32* pData1 = nullptr;
    pData1 = nsMemoryUtils::AddByteOffset(pData1, 13);
    NS_TEST_BOOL(pData1 == reinterpret_cast<nsInt32*>(13));

    const nsInt32* pData2 = nullptr;
    const nsInt32* pData3 = nsMemoryUtils::AddByteOffset(pData2, 17);
    NS_TEST_BOOL(pData3 == reinterpret_cast<nsInt32*>(17));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Align / IsAligned")
  {
    {
      nsInt32* pData = (nsInt32*)1;
      NS_TEST_BOOL(!nsMemoryUtils::IsAligned(pData, 4));
      pData = nsMemoryUtils::AlignBackwards(pData, 4);
      NS_TEST_BOOL(pData == reinterpret_cast<nsInt32*>(0));
      NS_TEST_BOOL(nsMemoryUtils::IsAligned(pData, 4));
    }
    {
      nsInt32* pData = (nsInt32*)2;
      NS_TEST_BOOL(!nsMemoryUtils::IsAligned(pData, 4));
      pData = nsMemoryUtils::AlignBackwards(pData, 4);
      NS_TEST_BOOL(pData == reinterpret_cast<nsInt32*>(0));
      NS_TEST_BOOL(nsMemoryUtils::IsAligned(pData, 4));
    }
    {
      nsInt32* pData = (nsInt32*)3;
      NS_TEST_BOOL(!nsMemoryUtils::IsAligned(pData, 4));
      pData = nsMemoryUtils::AlignBackwards(pData, 4);
      NS_TEST_BOOL(pData == reinterpret_cast<nsInt32*>(0));
      NS_TEST_BOOL(nsMemoryUtils::IsAligned(pData, 4));
    }
    {
      nsInt32* pData = (nsInt32*)4;
      NS_TEST_BOOL(nsMemoryUtils::IsAligned(pData, 4));
      pData = nsMemoryUtils::AlignBackwards(pData, 4);
      NS_TEST_BOOL(pData == reinterpret_cast<nsInt32*>(4));
      NS_TEST_BOOL(nsMemoryUtils::IsAligned(pData, 4));
    }

    {
      nsInt32* pData = (nsInt32*)1;
      NS_TEST_BOOL(!nsMemoryUtils::IsAligned(pData, 4));
      pData = nsMemoryUtils::AlignForwards(pData, 4);
      NS_TEST_BOOL(pData == reinterpret_cast<nsInt32*>(4));
      NS_TEST_BOOL(nsMemoryUtils::IsAligned(pData, 4));
    }
    {
      nsInt32* pData = (nsInt32*)2;
      NS_TEST_BOOL(!nsMemoryUtils::IsAligned(pData, 4));
      pData = nsMemoryUtils::AlignForwards(pData, 4);
      NS_TEST_BOOL(pData == reinterpret_cast<nsInt32*>(4));
      NS_TEST_BOOL(nsMemoryUtils::IsAligned(pData, 4));
    }
    {
      nsInt32* pData = (nsInt32*)3;
      NS_TEST_BOOL(!nsMemoryUtils::IsAligned(pData, 4));
      pData = nsMemoryUtils::AlignForwards(pData, 4);
      NS_TEST_BOOL(pData == reinterpret_cast<nsInt32*>(4));
      NS_TEST_BOOL(nsMemoryUtils::IsAligned(pData, 4));
    }
    {
      nsInt32* pData = (nsInt32*)4;
      NS_TEST_BOOL(nsMemoryUtils::IsAligned(pData, 4));
      pData = nsMemoryUtils::AlignForwards(pData, 4);
      NS_TEST_BOOL(pData == reinterpret_cast<nsInt32*>(4));
      NS_TEST_BOOL(nsMemoryUtils::IsAligned(pData, 4));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "POD")
  {
    struct Trivial
    {
      NS_DECLARE_POD_TYPE();

      ~Trivial() = default;

      nsUInt32 a;
      nsUInt32 b;
    };

    static_assert(std::is_trivial<Trivial>::value != 0);
    static_assert(nsIsPodType<Trivial>::value == 1);
    static_assert(std::is_trivially_destructible<Trivial>::value != 0);

    struct POD
    {
      NS_DECLARE_POD_TYPE();

      nsUInt32 a = 2;
      nsUInt32 b = 4;

      POD()
      {
        iCallPodConstructor++;
      }

      // this isn't allowed anymore in types that use NS_DECLARE_POD_TYPE
      // unfortunately that means we can't do this kind of check either
      //~POD()
      //{
      //  iCallPodDestructor++;
      //}
    };

    static_assert(std::is_trivial<POD>::value == 0);
    static_assert(nsIsPodType<POD>::value == 1);

    struct NonPOD
    {
      nsUInt32 a = 3;
      nsUInt32 b = 5;

      NonPOD()
      {
        iCallNonPodConstructor++;
      }

      ~NonPOD()
      {
        iCallNonPodDestructor++;
      }
    };

    static_assert(std::is_trivial<NonPOD>::value == 0);
    static_assert(nsIsPodType<NonPOD>::value == 0);

    struct NonPOD2
    {
      nsUInt32 a;
      nsUInt32 b;

      ~NonPOD2()
      {
        iCallNonPodDestructor++;
      }
    };

    static_assert(std::is_trivial<NonPOD2>::value == 0); // destructor makes it non-trivial
    static_assert(nsIsPodType<NonPOD2>::value == 0);
    static_assert(std::is_trivially_destructible<NonPOD2>::value == 0);

    // check that nsMemoryUtils::Construct and nsMemoryUtils::Destruct ignore POD types
    {
      nsUInt8 mem[sizeof(POD) * 2];

      NS_TEST_INT(iCallPodConstructor, 0);
      NS_TEST_INT(iCallPodDestructor, 0);

      nsMemoryUtils::Construct<SkipTrivialTypes, POD>((POD*)mem, 1);

      NS_TEST_INT(iCallPodConstructor, 1);
      NS_TEST_INT(iCallPodDestructor, 0);

      nsMemoryUtils::Destruct<POD>((POD*)mem, 1);
      NS_TEST_INT(iCallPodConstructor, 1);
      NS_TEST_INT(iCallPodDestructor, 0);

      iCallPodConstructor = 0;
    }

    // check that nsMemoryUtils::Destruct calls the destructor of a non-trivial type
    {
      nsUInt8 mem[sizeof(NonPOD2) * 2];

      NS_TEST_INT(iCallNonPodDestructor, 0);
      nsMemoryUtils::Destruct<NonPOD2>((NonPOD2*)mem, 1);

      NS_TEST_INT(iCallNonPodDestructor, 1);

      iCallNonPodDestructor = 0;
    }

    {
      // make sure nsMemoryUtils::Construct and nsMemoryUtils::Destruct don't touch built-in types

      nsInt32 a = 42;
      nsMemoryUtils::Construct<SkipTrivialTypes, nsInt32>(&a, 1);
      NS_TEST_INT(a, 42);
      nsMemoryUtils::Destruct<nsInt32>(&a, 1);
      NS_TEST_INT(a, 42);
    }
  }
}
