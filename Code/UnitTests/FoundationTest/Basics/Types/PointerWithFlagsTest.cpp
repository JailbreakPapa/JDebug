#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Types/PointerWithFlags.h>

NS_CREATE_SIMPLE_TEST(Basics, PointerWithFlags)
{
  struct Dummy
  {
    float a = 3.0f;
    int b = 7;
  };

  NS_TEST_BLOCK(nsTestBlock::Enabled, "General")
  {
    nsPointerWithFlags<Dummy, 2> ptr;

    NS_TEST_INT(ptr.GetFlags(), 0);
    ptr.SetFlags(3);
    NS_TEST_INT(ptr.GetFlags(), 3);

    NS_TEST_BOOL(ptr == nullptr);
    NS_TEST_BOOL(!ptr);

    NS_TEST_INT(ptr.GetFlags(), 3);
    ptr.SetFlags(2);
    NS_TEST_INT(ptr.GetFlags(), 2);

    Dummy d1, d2;
    ptr = &d1;
    d2.a = 4;
    d2.b = 8;

    NS_TEST_BOOL(ptr.GetPtr() == &d1);
    NS_TEST_BOOL(ptr.GetPtr() != &d2);

    NS_TEST_INT(ptr.GetFlags(), 2);
    ptr.SetFlags(1);
    NS_TEST_INT(ptr.GetFlags(), 1);

    NS_TEST_BOOL(ptr == &d1);
    NS_TEST_BOOL(ptr != &d2);
    NS_TEST_BOOL(ptr);


    NS_TEST_FLOAT(ptr->a, 3.0f, 0.0f);
    NS_TEST_INT(ptr->b, 7);

    ptr = &d2;

    NS_TEST_INT(ptr.GetFlags(), 1);
    ptr.SetFlags(3);
    NS_TEST_INT(ptr.GetFlags(), 3);

    NS_TEST_BOOL(ptr != &d1);
    NS_TEST_BOOL(ptr == &d2);
    NS_TEST_BOOL(ptr);

    ptr = nullptr;
    NS_TEST_BOOL(!ptr);
    NS_TEST_BOOL(ptr == nullptr);

    NS_TEST_INT(ptr.GetFlags(), 3);
    ptr.SetFlags(0);
    NS_TEST_INT(ptr.GetFlags(), 0);

    nsPointerWithFlags<Dummy, 2> ptr2 = ptr;
    NS_TEST_BOOL(ptr == ptr2);

    NS_TEST_BOOL(ptr2.GetPtr() == ptr.GetPtr());
    NS_TEST_BOOL(ptr2.GetFlags() == ptr.GetFlags());

    ptr2.SetFlags(3);
    NS_TEST_BOOL(ptr2.GetPtr() == ptr.GetPtr());
    NS_TEST_BOOL(ptr2.GetFlags() != ptr.GetFlags());

    // the two Ptrs still compare equal (pointer part is equal, even if flags are different)
    NS_TEST_BOOL(ptr == ptr2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Const ptr")
  {
    nsPointerWithFlags<const Dummy, 2> ptr;

    Dummy d1, d2;
    ptr = &d1;

    const Dummy* pD1 = &d1;
    const Dummy* pD2 = &d2;

    NS_TEST_BOOL(ptr.GetPtr() == pD1);
    NS_TEST_BOOL(ptr.GetPtr() != pD2);
  }
}
