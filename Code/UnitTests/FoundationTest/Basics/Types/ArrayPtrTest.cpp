#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/DynamicArray.h>

template <typename T>
static void testArrayPtr(nsArrayPtr<T> arrayPtr, typename nsArrayPtr<T>::PointerType extectedPtr, nsUInt32 uiExpectedCount)
{
  NS_TEST_BOOL(arrayPtr.GetPtr() == extectedPtr);
  NS_TEST_INT(arrayPtr.GetCount(), uiExpectedCount);
}

// static void TakeConstArrayPtr(nsArrayPtr<const int> cint)
//{
//}
//
// static void TakeConstArrayPtr2(nsArrayPtr<const int*> cint, nsArrayPtr<const int* const> cintc)
//{
//}

NS_CREATE_SIMPLE_TEST(Basics, ArrayPtr)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Empty Constructor")
  {
    nsArrayPtr<nsInt32> Empty;

    NS_TEST_BOOL(Empty.GetPtr() == nullptr);
    NS_TEST_BOOL(Empty.GetCount() == 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    nsInt32 pIntData[] = {1, 2, 3, 4, 5};

    nsArrayPtr<nsInt32> ap(pIntData, 3);
    NS_TEST_BOOL(ap.GetPtr() == pIntData);
    NS_TEST_BOOL(ap.GetCount() == 3);

    nsArrayPtr<nsInt32> ap2(pIntData, 0u);
    NS_TEST_BOOL(ap2.GetPtr() == nullptr);
    NS_TEST_BOOL(ap2.GetCount() == 0);

    nsArrayPtr<nsInt32> ap3(pIntData);
    NS_TEST_BOOL(ap3.GetPtr() == pIntData);
    NS_TEST_BOOL(ap3.GetCount() == 5);

    nsArrayPtr<nsInt32> ap4(ap);
    NS_TEST_BOOL(ap4.GetPtr() == pIntData);
    NS_TEST_BOOL(ap4.GetCount() == 3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsMakeArrayPtr")
  {
    nsInt32 pIntData[] = {1, 2, 3, 4, 5};

    testArrayPtr(nsMakeArrayPtr(pIntData, 3), pIntData, 3);
    testArrayPtr(nsMakeArrayPtr(pIntData, 0), nullptr, 0);
    testArrayPtr(nsMakeArrayPtr(pIntData), pIntData, 5);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator=")
  {
    nsInt32 pIntData[] = {1, 2, 3, 4, 5};

    nsArrayPtr<nsInt32> ap(pIntData, 3);
    NS_TEST_BOOL(ap.GetPtr() == pIntData);
    NS_TEST_BOOL(ap.GetCount() == 3);

    nsArrayPtr<nsInt32> ap2;
    ap2 = ap;

    NS_TEST_BOOL(ap2.GetPtr() == pIntData);
    NS_TEST_BOOL(ap2.GetCount() == 3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Clear")
  {
    nsInt32 pIntData[] = {1, 2, 3, 4, 5};

    nsArrayPtr<nsInt32> ap(pIntData, 3);
    NS_TEST_BOOL(ap.GetPtr() == pIntData);
    NS_TEST_BOOL(ap.GetCount() == 3);

    ap.Clear();

    NS_TEST_BOOL(ap.GetPtr() == nullptr);
    NS_TEST_BOOL(ap.GetCount() == 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator== / operator!= / operator<")
  {
    nsInt32 pIntData[] = {1, 2, 3, 4, 5};

    nsArrayPtr<nsInt32> ap1(pIntData, 3);
    nsArrayPtr<nsInt32> ap2(pIntData, 3);
    nsArrayPtr<nsInt32> ap3(pIntData, 4);
    nsArrayPtr<nsInt32> ap4(pIntData + 1, 3);

    NS_TEST_BOOL(ap1 == ap2);
    NS_TEST_BOOL(ap1 != ap3);
    NS_TEST_BOOL(ap1 != ap4);

    NS_TEST_BOOL(ap1 < ap3);
    nsInt32 pIntData2[] = {1, 2, 4};
    nsArrayPtr<nsInt32> ap5(pIntData2, 3);
    NS_TEST_BOOL(ap1 < ap5);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator[]")
  {
    nsInt32 pIntData[] = {1, 2, 3, 4, 5};

    nsArrayPtr<nsInt32> ap(pIntData + 1, 3);
    NS_TEST_INT(ap[0], 2);
    NS_TEST_INT(ap[1], 3);
    NS_TEST_INT(ap[2], 4);
    ap[2] = 10;
    NS_TEST_INT(ap[2], 10);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "const operator[]")
  {
    nsInt32 pIntData[] = {1, 2, 3, 4, 5};

    const nsArrayPtr<nsInt32> ap(pIntData + 1, 3);
    NS_TEST_INT(ap[0], 2);
    NS_TEST_INT(ap[1], 3);
    NS_TEST_INT(ap[2], 4);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CopyFrom")
  {
    nsInt32 pIntData1[] = {1, 2, 3, 4, 5};
    nsInt32 pIntData2[] = {6, 7, 8, 9, 0};

    nsArrayPtr<nsInt32> ap1(pIntData1 + 1, 3);
    nsArrayPtr<nsInt32> ap2(pIntData2 + 2, 3);

    ap1.CopyFrom(ap2);

    NS_TEST_INT(pIntData1[0], 1);
    NS_TEST_INT(pIntData1[1], 8);
    NS_TEST_INT(pIntData1[2], 9);
    NS_TEST_INT(pIntData1[3], 0);
    NS_TEST_INT(pIntData1[4], 5);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetSubArray")
  {
    nsInt32 pIntData1[] = {1, 2, 3, 4, 5};

    nsArrayPtr<nsInt32> ap1(pIntData1, 5);
    nsArrayPtr<nsInt32> ap2 = ap1.GetSubArray(2, 3);

    NS_TEST_BOOL(ap2.GetPtr() == &pIntData1[2]);
    NS_TEST_BOOL(ap2.GetCount() == 3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Const Conversions")
  {
    nsInt32 pIntData1[] = {1, 2, 3, 4, 5};
    nsArrayPtr<nsInt32> ap1(pIntData1);
    nsArrayPtr<const nsInt32> ap2(ap1);
    nsArrayPtr<const nsInt32> ap3(pIntData1);
    ap2 = ap1; // non const to const assign
    ap3 = ap2; // const to const assign
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Empty Constructor (const)")
  {
    nsArrayPtr<const nsInt32> Empty;

    NS_TEST_BOOL(Empty.GetPtr() == nullptr);
    NS_TEST_BOOL(Empty.GetCount() == 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor (const)")
  {
    const nsInt32 pIntData[] = {1, 2, 3, 4, 5};

    nsArrayPtr<const nsInt32> ap(pIntData, 3);
    NS_TEST_BOOL(ap.GetPtr() == pIntData);
    NS_TEST_BOOL(ap.GetCount() == 3);

    nsArrayPtr<const nsInt32> ap2(pIntData, 0u);
    NS_TEST_BOOL(ap2.GetPtr() == nullptr);
    NS_TEST_BOOL(ap2.GetCount() == 0);

    nsArrayPtr<const nsInt32> ap3(pIntData);
    NS_TEST_BOOL(ap3.GetPtr() == pIntData);
    NS_TEST_BOOL(ap3.GetCount() == 5);

    nsArrayPtr<const nsInt32> ap4(ap);
    NS_TEST_BOOL(ap4.GetPtr() == pIntData);
    NS_TEST_BOOL(ap4.GetCount() == 3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator=  (const)")
  {
    const nsInt32 pIntData[] = {1, 2, 3, 4, 5};

    nsArrayPtr<const nsInt32> ap(pIntData, 3);
    NS_TEST_BOOL(ap.GetPtr() == pIntData);
    NS_TEST_BOOL(ap.GetCount() == 3);

    nsArrayPtr<const nsInt32> ap2;
    ap2 = ap;

    NS_TEST_BOOL(ap2.GetPtr() == pIntData);
    NS_TEST_BOOL(ap2.GetCount() == 3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Clear (const)")
  {
    const nsInt32 pIntData[] = {1, 2, 3, 4, 5};

    nsArrayPtr<const nsInt32> ap(pIntData, 3);
    NS_TEST_BOOL(ap.GetPtr() == pIntData);
    NS_TEST_BOOL(ap.GetCount() == 3);

    ap.Clear();

    NS_TEST_BOOL(ap.GetPtr() == nullptr);
    NS_TEST_BOOL(ap.GetCount() == 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator== / operator!=  (const)")
  {
    nsInt32 pIntData[] = {1, 2, 3, 4, 5};

    nsArrayPtr<nsInt32> ap1(pIntData, 3);
    nsArrayPtr<const nsInt32> ap2(pIntData, 3);
    nsArrayPtr<const nsInt32> ap3(pIntData, 4);
    nsArrayPtr<const nsInt32> ap4(pIntData + 1, 3);

    NS_TEST_BOOL(ap1 == ap2);
    NS_TEST_BOOL(ap3 != ap1);
    NS_TEST_BOOL(ap1 != ap4);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator[]  (const)")
  {
    const nsInt32 pIntData[] = {1, 2, 3, 4, 5};

    nsArrayPtr<const nsInt32> ap(pIntData + 1, 3);
    NS_TEST_INT(ap[0], 2);
    NS_TEST_INT(ap[1], 3);
    NS_TEST_INT(ap[2], 4);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "const operator[] (const)")
  {
    const nsInt32 pIntData[] = {1, 2, 3, 4, 5};

    const nsArrayPtr<const nsInt32> ap(pIntData + 1, 3);
    NS_TEST_INT(ap[0], 2);
    NS_TEST_INT(ap[1], 3);
    NS_TEST_INT(ap[2], 4);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetSubArray (const)")
  {
    const nsInt32 pIntData1[] = {1, 2, 3, 4, 5};

    nsArrayPtr<const nsInt32> ap1(pIntData1, 5);
    nsArrayPtr<const nsInt32> ap2 = ap1.GetSubArray(2, 3);

    NS_TEST_BOOL(ap2.GetPtr() == &pIntData1[2]);
    NS_TEST_BOOL(ap2.GetCount() == 3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "STL Iterator")
  {
    nsDynamicArray<nsInt32> a1;

    for (nsInt32 i = 0; i < 1000; ++i)
      a1.PushBack(1000 - i - 1);

    nsArrayPtr<nsInt32> ptr1 = a1;

    // STL sort
    std::sort(begin(ptr1), end(ptr1));

    for (nsInt32 i = 1; i < 1000; ++i)
    {
      NS_TEST_BOOL(ptr1[i - 1] <= ptr1[i]);
    }

    // foreach
    nsUInt32 prev = 0;
    for (nsUInt32 val : ptr1)
    {
      NS_TEST_BOOL(prev <= val);
      prev = val;
    }

    // const array
    const nsDynamicArray<nsInt32>& a2 = a1;

    const nsArrayPtr<const nsInt32> ptr2 = a2;

    // STL lower bound
    auto lb = std::lower_bound(begin(ptr2), end(ptr2), 400);
    NS_TEST_BOOL(*lb == ptr2[400]);
  }


  NS_TEST_BLOCK(nsTestBlock::Enabled, "STL Reverse Iterator")
  {
    nsDynamicArray<nsInt32> a1;

    for (nsInt32 i = 0; i < 1000; ++i)
      a1.PushBack(1000 - i - 1);

    nsArrayPtr<nsInt32> ptr1 = a1;

    // STL sort
    std::sort(rbegin(ptr1), rend(ptr1));

    for (nsInt32 i = 1; i < 1000; ++i)
    {
      NS_TEST_BOOL(ptr1[i - 1] >= ptr1[i]);
    }

    // foreach
    nsUInt32 prev = 1000;
    for (nsUInt32 val : ptr1)
    {
      NS_TEST_BOOL(prev >= val);
      prev = val;
    }

    // const array
    const nsDynamicArray<nsInt32>& a2 = a1;

    const nsArrayPtr<const nsInt32> ptr2 = a2;

    // STL lower bound
    auto lb = std::lower_bound(rbegin(ptr2), rend(ptr2), 400);
    NS_TEST_BOOL(*lb == ptr2[1000 - 400 - 1]);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains / IndexOf / LastIndexOf")
  {
    nsDynamicArray<nsInt32> a0;
    nsArrayPtr<nsInt32> a1 = a0;

    for (nsInt32 i = -100; i < 100; ++i)
      NS_TEST_BOOL(!a1.Contains(i));

    for (nsInt32 i = 0; i < 100; ++i)
      a0.PushBack(i);
    for (nsInt32 i = 0; i < 100; ++i)
      a0.PushBack(i);

    a1 = a0;

    for (nsInt32 i = 0; i < 100; ++i)
    {
      NS_TEST_BOOL(a1.Contains(i));
      NS_TEST_INT(a1.IndexOf(i), i);
      NS_TEST_INT(a1.IndexOf(i, 100), i + 100);
      NS_TEST_INT(a1.LastIndexOf(i), i + 100);
      NS_TEST_INT(a1.LastIndexOf(i, 100), i);
    }
  }

  // "Implicit Conversions"
  //{
  //  {
  //    nsHybridArray<int, 4> data;
  //    TakeConstArrayPtr(data);
  //    TakeConstArrayPtr(data.GetArrayPtr());
  //  }
  //  {
  //    nsHybridArray<int*, 4> data;
  //    //TakeConstArrayPtr2(data, data); // does not compile
  //    TakeConstArrayPtr2(data.GetArrayPtr(), data.GetArrayPtr());
  //  }
  //}
}
