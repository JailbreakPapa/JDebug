#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Configuration/Singleton.h>

class TestSingleton
{
  NS_DECLARE_SINGLETON(TestSingleton);

public:
  TestSingleton()
    : m_SingletonRegistrar(this)
  {
  }

  nsInt32 m_iValue = 41;
};

NS_IMPLEMENT_SINGLETON(TestSingleton);

class SingletonInterface
{
public:
  virtual nsInt32 GetValue() = 0;
};

class TestSingletonOfInterface : public SingletonInterface
{
  NS_DECLARE_SINGLETON_OF_INTERFACE(TestSingletonOfInterface, SingletonInterface);

public:
  TestSingletonOfInterface()
    : m_SingletonRegistrar(this)
  {
  }

  virtual nsInt32 GetValue() { return 23; }
};

NS_IMPLEMENT_SINGLETON(TestSingletonOfInterface);


NS_CREATE_SIMPLE_TEST(Configuration, Singleton)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Singleton Registration")
  {
    {
      TestSingleton* pSingleton = nsSingletonRegistry::GetSingletonInstance<TestSingleton>();
      NS_TEST_BOOL(pSingleton == nullptr);
    }

    {
      TestSingleton g_Singleton;

      {
        TestSingleton* pSingleton = nsSingletonRegistry::GetSingletonInstance<TestSingleton>();
        NS_TEST_BOOL(pSingleton == &g_Singleton);
        NS_TEST_INT(pSingleton->m_iValue, 41);
      }
    }

    {
      TestSingleton* pSingleton = nsSingletonRegistry::GetSingletonInstance<TestSingleton>();
      NS_TEST_BOOL(pSingleton == nullptr);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Singleton of Interface")
  {
    {
      SingletonInterface* pSingleton = nsSingletonRegistry::GetSingletonInstance<SingletonInterface>();
      NS_TEST_BOOL(pSingleton == nullptr);
    }

    {
      TestSingletonOfInterface g_Singleton;

      {
        SingletonInterface* pSingleton = nsSingletonRegistry::GetSingletonInstance<SingletonInterface>();
        NS_TEST_BOOL(pSingleton == &g_Singleton);
        NS_TEST_INT(pSingleton->GetValue(), 23);
      }

      {
        TestSingletonOfInterface* pSingleton = nsSingletonRegistry::GetSingletonInstance<TestSingletonOfInterface>();
        NS_TEST_BOOL(pSingleton == &g_Singleton);
        NS_TEST_INT(pSingleton->GetValue(), 23);
      }

      {
        SingletonInterface* pSingleton = nsSingletonRegistry::GetRequiredSingletonInstance<SingletonInterface>();
        NS_TEST_BOOL(pSingleton == &g_Singleton);
        NS_TEST_INT(pSingleton->GetValue(), 23);
      }

      {
        TestSingletonOfInterface* pSingleton = nsSingletonRegistry::GetRequiredSingletonInstance<TestSingletonOfInterface>();
        NS_TEST_BOOL(pSingleton == &g_Singleton);
        NS_TEST_INT(pSingleton->GetValue(), 23);
      }
    }

    {
      SingletonInterface* pSingleton = nsSingletonRegistry::GetSingletonInstance<SingletonInterface>();
      NS_TEST_BOOL(pSingleton == nullptr);
    }

    {
      TestSingletonOfInterface* pSingleton = nsSingletonRegistry::GetSingletonInstance<TestSingletonOfInterface>();
      NS_TEST_BOOL(pSingleton == nullptr);
    }
  }
}
