#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Types/RefCounted.h>

class RefCountedTestClass : public nsRefCounted
{
public:
  nsUInt32 m_uiDummyMember = 0x42u;
};

NS_CREATE_SIMPLE_TEST(Basics, RefCounted)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Ref Counting")
  {
    RefCountedTestClass Instance;

    NS_TEST_BOOL(Instance.GetRefCount() == 0);
    NS_TEST_BOOL(!Instance.IsReferenced());

    Instance.AddRef();

    NS_TEST_BOOL(Instance.GetRefCount() == 1);
    NS_TEST_BOOL(Instance.IsReferenced());

    /// Test scoped ref pointer
    {
      nsScopedRefPointer<RefCountedTestClass> ScopeTester(&Instance);

      NS_TEST_BOOL(Instance.GetRefCount() == 2);
      NS_TEST_BOOL(Instance.IsReferenced());
    }

    /// Test assignment of scoped ref pointer
    {
      nsScopedRefPointer<RefCountedTestClass> ScopeTester;

      ScopeTester = &Instance;

      NS_TEST_BOOL(Instance.GetRefCount() == 2);
      NS_TEST_BOOL(Instance.IsReferenced());

      nsScopedRefPointer<RefCountedTestClass> ScopeTester2;

      ScopeTester2 = ScopeTester;

      NS_TEST_BOOL(Instance.GetRefCount() == 3);
      NS_TEST_BOOL(Instance.IsReferenced());

      nsScopedRefPointer<RefCountedTestClass> ScopeTester3(ScopeTester);

      NS_TEST_BOOL(Instance.GetRefCount() == 4);
      NS_TEST_BOOL(Instance.IsReferenced());
    }

    /// Test copy constructor for nsRefCounted
    {
      RefCountedTestClass inst2(Instance);
      RefCountedTestClass inst3;
      inst3 = Instance;

      NS_TEST_BOOL(Instance.GetRefCount() == 1);
      NS_TEST_BOOL(Instance.IsReferenced());

      NS_TEST_BOOL(inst2.GetRefCount() == 0);
      NS_TEST_BOOL(!inst2.IsReferenced());

      NS_TEST_BOOL(inst3.GetRefCount() == 0);
      NS_TEST_BOOL(!inst3.IsReferenced());
    }

    NS_TEST_BOOL(Instance.GetRefCount() == 1);
    NS_TEST_BOOL(Instance.IsReferenced());

    Instance.ReleaseRef();

    NS_TEST_BOOL(Instance.GetRefCount() == 0);
    NS_TEST_BOOL(!Instance.IsReferenced());
  }
}
