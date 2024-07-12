#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Types/Delegate.h>
#include <Foundation/Types/RefCounted.h>
#include <Foundation/Types/SharedPtr.h>

namespace
{
  struct TestType
  {
    TestType(){}; // NOLINT: Allow default construction

    nsInt32 MethodWithManyParams(nsInt32 a, nsInt32 b, nsInt32 c, nsInt32 d, nsInt32 e, nsInt32 f) { return m_iA + a + b + c + d + e + f; }

    nsInt32 Method(nsInt32 b) { return b + m_iA; }

    nsInt32 ConstMethod(nsInt32 b) const { return b + m_iA + 4; }

    virtual nsInt32 VirtualMethod(nsInt32 b) { return b; }

    mutable nsInt32 m_iA;
  };

  struct TestTypeDerived : public TestType
  {
    nsInt32 Method(nsInt32 b) { return b + 4; }

    virtual nsInt32 VirtualMethod(nsInt32 b) override { return b + 43; }
  };

  struct BaseA
  {
    virtual ~BaseA() = default;
    virtual void bar() {}

    int m_i1;
  };

  struct BaseB
  {
    virtual ~BaseB() = default;
    virtual void foo() {}
    int m_i2;
  };

  struct ComplexClass : public BaseA, public BaseB
  {
    ComplexClass() { m_ctorDel = nsMakeDelegate(&ComplexClass::nonVirtualFunc, this); }

    virtual ~ComplexClass()
    {
      m_dtorDel = nsMakeDelegate(&ComplexClass::nonVirtualFunc, this);
      NS_TEST_BOOL(m_ctorDel.IsEqualIfComparable(m_dtorDel));
    }
    virtual void bar() override {}
    virtual void foo() override {}



    void nonVirtualFunc()
    {
      m_i1 = 1;
      m_i2 = 2;
      m_i3 = 3;
    }

    int m_i3;

    nsDelegate<void()> m_ctorDel;
    nsDelegate<void()> m_dtorDel;
  };

  static nsInt32 Function(nsInt32 b)
  {
    return b + 2;
  }
} // namespace

NS_CREATE_SIMPLE_TEST(Basics, Delegate)
{
  using TestDelegate = nsDelegate<nsInt32(nsInt32)>;
  TestDelegate d;

#if NS_ENABLED(NS_PLATFORM_64BIT)
  NS_TEST_BOOL(sizeof(d) == 32);
#endif

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Method")
  {
    TestTypeDerived test;
    test.m_iA = 42;

    d = TestDelegate(&TestType::Method, &test);
    NS_TEST_BOOL(d.IsEqualIfComparable(TestDelegate(&TestType::Method, &test)));
    NS_TEST_BOOL(d.IsComparable());
    NS_TEST_INT(d(4), 46);

    d = TestDelegate(&TestTypeDerived::Method, &test);
    NS_TEST_BOOL(d.IsEqualIfComparable(TestDelegate(&TestTypeDerived::Method, &test)));
    NS_TEST_BOOL(d.IsComparable());
    NS_TEST_INT(d(4), 8);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Method With Many Params")
  {
    using TestDelegateMany = nsDelegate<nsInt32(nsInt32, nsInt32, nsInt32, nsInt32, nsInt32, nsInt32)>;
    TestDelegateMany many;

    TestType test;
    test.m_iA = 1000000;

    many = TestDelegateMany(&TestType::MethodWithManyParams, &test);
    NS_TEST_BOOL(many.IsEqualIfComparable(TestDelegateMany(&TestType::MethodWithManyParams, &test)));
    NS_TEST_BOOL(d.IsComparable());
    NS_TEST_INT(many(1, 10, 100, 1000, 10000, 100000), 1111111);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Complex Class")
  {
    NS_WARNING_PUSH()
    NS_WARNING_DISABLE_GCC("-Wfree-nonheap-object")

    ComplexClass* c = new ComplexClass();
    delete c;

    NS_WARNING_POP()
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Const Method")
  {
    const TestType constTest;
    constTest.m_iA = 35;

    d = TestDelegate(&TestType::ConstMethod, &constTest);
    NS_TEST_BOOL(d.IsEqualIfComparable(TestDelegate(&TestType::ConstMethod, &constTest)));
    NS_TEST_BOOL(d.IsComparable());
    NS_TEST_INT(d(4), 43);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Virtual Method")
  {
    TestTypeDerived test;

    d = TestDelegate(&TestType::VirtualMethod, &test);
    NS_TEST_BOOL(d.IsEqualIfComparable(TestDelegate(&TestType::VirtualMethod, &test)));
    NS_TEST_BOOL(d.IsComparable());
    NS_TEST_INT(d(4), 47);

    d = TestDelegate(&TestTypeDerived::VirtualMethod, &test);
    NS_TEST_BOOL(d.IsEqualIfComparable(TestDelegate(&TestTypeDerived::VirtualMethod, &test)));
    NS_TEST_BOOL(d.IsComparable());
    NS_TEST_INT(d(4), 47);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Function")
  {
    d = &Function;
    NS_TEST_BOOL(d.IsEqualIfComparable(&Function));
    NS_TEST_BOOL(d.IsComparable());
    NS_TEST_INT(d(4), 6);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Lambda - no capture")
  {
    d = [](nsInt32 i)
    { return i * 4; };
    NS_TEST_BOOL(d.IsComparable());
    NS_TEST_INT(d(2), 8);

    TestDelegate d2 = d;
    NS_TEST_BOOL(d2.IsEqualIfComparable(d));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Lambda - capture by value")
  {
    nsInt32 c = 20;
    d = [c](nsInt32)
    { return c; };
    NS_TEST_BOOL(!d.IsComparable());
    NS_TEST_INT(d(3), 20);
    c = 10;
    NS_TEST_INT(d(3), 20);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Lambda - capture by value, mutable")
  {
    nsInt32 c = 20;
    d = [c](nsInt32) mutable
    { return c; };
    NS_TEST_BOOL(!d.IsComparable());
    NS_TEST_INT(d(3), 20);
    c = 10;
    NS_TEST_INT(d(3), 20);

    d = [c](nsInt32 b) mutable -> decltype(b + c)
    {
      auto result = b + c;
      c = 1;
      return result;
    };
    NS_TEST_BOOL(!d.IsComparable());
    NS_TEST_INT(d(3), 13);
    NS_TEST_INT(d(3), 4);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Lambda - capture by reference")
  {
    nsInt32 c = 20;
    d = [&c](nsInt32 i) -> decltype(i)
    {
      c = 5;
      return i;
    };
    NS_TEST_BOOL(!d.IsComparable());
    NS_TEST_INT(d(3), 3);
    NS_TEST_INT(c, 5);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Lambda - capture by value of non-pod")
  {
    struct RefCountedInt : public nsRefCounted
    {
      RefCountedInt() = default;
      RefCountedInt(int i)
        : m_value(i)
      {
      }
      int m_value;
    };

    nsSharedPtr<RefCountedInt> shared = NS_DEFAULT_NEW(RefCountedInt, 1);
    NS_TEST_INT(shared->GetRefCount(), 1);
    {
      TestDelegate deleteMe = [shared](nsInt32 i) -> decltype(i)
      { return 0; };
      NS_TEST_BOOL(!deleteMe.IsComparable());
      NS_TEST_INT(shared->GetRefCount(), 2);
    }
    NS_TEST_INT(shared->GetRefCount(), 1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Lambda - capture lots of things")
  {
    nsInt64 a = 10;
    nsInt64 b = 20;
    nsInt64 c = 30;
    d = [a, b, c](nsInt32 i) -> nsInt32
    { return static_cast<nsInt32>(a + b + c + i); };
    NS_TEST_INT(d(6), 66);
    NS_TEST_BOOL(!d.IsComparable());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Lambda - capture lots of things - custom allocator")
  {
    nsInt64 a = 10;
    nsInt64 b = 20;
    nsInt64 c = 30;
    d = TestDelegate([a, b, c](nsInt32 i) -> nsInt32
      { return static_cast<nsInt32>(a + b + c + i); }, nsFoundation::GetAlignedAllocator());
    NS_TEST_INT(d(6), 66);
    NS_TEST_BOOL(!d.IsComparable());

    d.Invalidate();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Move semantics")
  {
    // Move pure function
    {
      d.Invalidate();
      TestDelegate d2 = &Function;
      d = std::move(d2);
      NS_TEST_BOOL(d.IsValid());
      NS_TEST_BOOL(!d2.IsValid());
      NS_TEST_BOOL(d.IsComparable());
      NS_TEST_INT(d(4), 6);
    }

    // Move delegate
    nsConstructionCounter::Reset();
    d.Invalidate();
    {
      nsConstructionCounter value;
      value.m_iData = 666;
      NS_TEST_INT(nsConstructionCounter::s_iConstructions, 1);
      NS_TEST_INT(nsConstructionCounter::s_iDestructions, 0);
      TestDelegate d2 = [value](nsInt32 i) -> nsInt32
      { return value.m_iData; };
      NS_TEST_INT(nsConstructionCounter::s_iConstructions, 3); // Capture plus moving the lambda.
      NS_TEST_INT(nsConstructionCounter::s_iDestructions, 1);  // Move of lambda
      d = std::move(d2);
      // Moving a construction counter also counts as construction
      NS_TEST_INT(nsConstructionCounter::s_iConstructions, 4);
      NS_TEST_INT(nsConstructionCounter::s_iDestructions, 1);
      NS_TEST_BOOL(d.IsValid());
      NS_TEST_BOOL(!d2.IsValid());
      NS_TEST_BOOL(!d.IsComparable());
      NS_TEST_INT(d(0), 666);
    }
    NS_TEST_INT(nsConstructionCounter::s_iDestructions, 2); // value out of scope
    NS_TEST_INT(nsConstructionCounter::s_iConstructions, 4);
    d.Invalidate();
    NS_TEST_INT(nsConstructionCounter::s_iDestructions, 3); // lambda destroyed.
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Lambda - Copy")
  {
    d.Invalidate();
    nsConstructionCounter::Reset();
    {
      nsConstructionCounter value;
      value.m_iData = 666;
      NS_TEST_INT(nsConstructionCounter::s_iConstructions, 1);
      NS_TEST_INT(nsConstructionCounter::s_iDestructions, 0);
      TestDelegate d2 = TestDelegate([value](nsInt32 i) -> nsInt32
        { return value.m_iData; }, nsFoundation::GetAlignedAllocator());
      NS_TEST_INT(nsConstructionCounter::s_iConstructions, 3); // Capture plus moving the lambda.
      NS_TEST_INT(nsConstructionCounter::s_iDestructions, 1);  // Move of lambda
      d = d2;
      NS_TEST_INT(nsConstructionCounter::s_iConstructions, 4); // Lambda Copy
      NS_TEST_INT(nsConstructionCounter::s_iDestructions, 1);
      NS_TEST_BOOL(d.IsValid());
      NS_TEST_BOOL(d2.IsValid());
      NS_TEST_BOOL(!d.IsComparable());
      NS_TEST_BOOL(!d2.IsComparable());
      NS_TEST_INT(d(0), 666);
      NS_TEST_INT(d2(0), 666);
    }
    NS_TEST_INT(nsConstructionCounter::s_iDestructions, 3); // value and lambda out of scope
    NS_TEST_INT(nsConstructionCounter::s_iConstructions, 4);
    d.Invalidate();
    NS_TEST_INT(nsConstructionCounter::s_iDestructions, 4); // lambda destroyed.
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Lambda - capture non-copyable type")
  {
    nsUniquePtr<nsConstructionCounter> data(NS_DEFAULT_NEW(nsConstructionCounter));
    data->m_iData = 666;
    TestDelegate d2 = [data = std::move(data)](nsInt32 i) -> nsInt32
    { return data->m_iData; };
    NS_TEST_INT(d2(0), 666);
    d = std::move(d2);
    NS_TEST_BOOL(d.IsValid());
    NS_TEST_BOOL(!d2.IsValid());
    NS_TEST_INT(d(0), 666);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsMakeDelegate")
  {
    auto d1 = nsMakeDelegate(&Function);
    NS_TEST_BOOL(d1.IsEqualIfComparable(nsMakeDelegate(&Function)));

    TestType instance;
    auto d2 = nsMakeDelegate(&TestType::Method, &instance);
    NS_TEST_BOOL(d2.IsEqualIfComparable(nsMakeDelegate(&TestType::Method, &instance)));
    auto d3 = nsMakeDelegate(&TestType::ConstMethod, &instance);
    NS_TEST_BOOL(d3.IsEqualIfComparable(nsMakeDelegate(&TestType::ConstMethod, &instance)));
    auto d4 = nsMakeDelegate(&TestType::VirtualMethod, &instance);
    NS_TEST_BOOL(d4.IsEqualIfComparable(nsMakeDelegate(&TestType::VirtualMethod, &instance)));

    TestType instance2;
    auto d2_2 = nsMakeDelegate(&TestType::Method, &instance2);
    NS_TEST_BOOL(!d2_2.IsEqualIfComparable(d2));

    NS_IGNORE_UNUSED(d1);
    NS_IGNORE_UNUSED(d2);
    NS_IGNORE_UNUSED(d2_2);
    NS_IGNORE_UNUSED(d3);
    NS_IGNORE_UNUSED(d4);
  }
}
