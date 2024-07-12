#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Communication/Message.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Time/Time.h>

/* Performance Statistics:

  AMD E-350 Processor 1.6 GHz ('Fusion'), 32 Bit, Debug Mode
    Virtual Function Calls:   ~60 ns
    Simple Function Calls:    ~27 ns
    Fastcall Function Calls:  ~27 ns
    Integer Division:         52 ns
    Integer Multiplication:   23 ns
    Float Division:           25 ns
    Float Multiplication:     25 ns

  AMD E-350 Processor 1.6 GHz ('Fusion'), 64 Bit, Debug Mode
    Virtual Function Calls:   ~80 ns
    Simple Function Calls:    ~55 ns
    Fastcall Function Calls:  ~55 ns
    Integer Division:         ~97 ns
    Integer Multiplication:   ~52 ns
    Float Division:           ~66 ns
    Float Multiplication:     ~58 ns

  AMD E-350 Processor 1.6 GHz ('Fusion'), 32 Bit, Release Mode
    Virtual Function Calls:   ~9 ns
    Simple Function Calls:    ~5 ns
    Fastcall Function Calls:  ~5 ns
    Integer Division:         35 ns
    Integer Multiplication:   3.78 ns
    Float Division:           10.7 ns
    Float Multiplication:     9.5 ns

  AMD E-350 Processor 1.6 GHz ('Fusion'), 64 Bit, Release Mode
    Virtual Function Calls:   ~10 ns
    Simple Function Calls:    ~5 ns
    Fastcall Function Calls:  ~5 ns
    Integer Division:         35 ns
    Integer Multiplication:   3.23 ns
    Float Division:           8.13 ns
    Float Multiplication:     4.13 ns

  Intel Core i7 3770 3.4 GHz, 64 Bit, Release Mode
    Virtual Function Calls:   ~3.8 ns
    Simple Function Calls:    ~4.4 ns
    Fastcall Function Calls:  ~4.0 ns
    Integer Division:         8.25 ns
    Integer Multiplication:   1.55 ns
    Float Division:           4.40 ns
    Float Multiplication:     1.87 ns

*/

NS_CREATE_SIMPLE_TEST_GROUP(Performance);

struct nsMsgTest : public nsMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgTest, nsMessage);
};

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgTest);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgTest, 1, nsRTTIDefaultAllocator<nsMsgTest>)
NS_END_DYNAMIC_REFLECTED_TYPE;


struct GetValueMessage : public nsMsgTest
{
  NS_DECLARE_MESSAGE_TYPE(GetValueMessage, nsMsgTest);

  nsInt32 m_iValue;
};
NS_IMPLEMENT_MESSAGE_TYPE(GetValueMessage);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(GetValueMessage, 1, nsRTTIDefaultAllocator<GetValueMessage>)
NS_END_DYNAMIC_REFLECTED_TYPE;



class Base : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(Base, nsReflectedClass);

public:
  virtual ~Base() = default;

  virtual nsInt32 Virtual() = 0;
};

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(Base, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
#  define NS_FASTCALL __fastcall
#  define NS_NO_INLINE __declspec(noinline)
#elif NS_ENABLED(NS_PLATFORM_OSX) || NS_ENABLED(NS_PLATFORM_LINUX) || NS_ENABLED(NS_PLATFORM_ANDROID)
#  if NS_ENABLED(NS_PLATFORM_ARCH_X86) && NS_ENABLED(NS_PLATFORM_32BIT)
#    define NS_FASTCALL __attribute((fastcall)) // Fastcall only relevant on x86-32 and would otherwise generate warnings
#  else
#    define NS_FASTCALL
#  endif
#  define NS_NO_INLINE __attribute__((noinline))
#else
#  warning Unknown Platform.
#  define NS_FASTCALL
#  define NS_NO_INLINE __attribute__((noinline)) /* should work on GCC */
#endif

class Derived1 : public Base
{
  NS_ADD_DYNAMIC_REFLECTION(Derived1, Base);

public:
  NS_NO_INLINE nsInt32 NS_FASTCALL FastCall() { return 1; }
  NS_NO_INLINE nsInt32 NonVirtual() { return 1; }
  NS_NO_INLINE virtual nsInt32 Virtual() override { return 1; }
  NS_NO_INLINE void OnGetValueMessage(GetValueMessage& ref_msg) { ref_msg.m_iValue = 1; }
};

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(Derived1, 1, nsRTTINoAllocator)
{
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(GetValueMessage, OnGetValueMessage),
  }
  NS_END_MESSAGEHANDLERS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

class Derived2 : public Base
{
  NS_ADD_DYNAMIC_REFLECTION(Derived2, Base);

public:
  NS_NO_INLINE nsInt32 NS_FASTCALL FastCall() { return 2; }
  NS_NO_INLINE nsInt32 NonVirtual() { return 2; }
  NS_NO_INLINE virtual nsInt32 Virtual() override { return 2; }
  NS_NO_INLINE void OnGetValueMessage(GetValueMessage& ref_msg) { ref_msg.m_iValue = 2; }
};

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(Derived2, 1, nsRTTINoAllocator)
{
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(GetValueMessage, OnGetValueMessage),
  }
  NS_END_MESSAGEHANDLERS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

NS_CREATE_SIMPLE_TEST(Performance, Basics)
{
  const nsInt32 iNumObjects = 1000000;
  const float fNumObjects = (float)iNumObjects;

  nsDynamicArray<Derived1> Der1;
  Der1.SetCount(iNumObjects / 2);

  nsDynamicArray<Derived2> Der2;
  Der2.SetCount(iNumObjects / 2);

  nsDynamicArray<Base*> Objects;
  Objects.SetCount(iNumObjects);

  for (nsInt32 i = 0; i < iNumObjects; i += 2)
  {
    Objects[i] = &Der1[i / 2];
    Objects[i + 1] = &Der2[i / 2];
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Dispatch Message")
  {
    nsInt32 iResult = 0;

    // warm up
    for (nsUInt32 i = 0; i < iNumObjects; ++i)
    {
      GetValueMessage msg;
      Objects[i]->GetDynamicRTTI()->DispatchMessage(Objects[i], msg);
      iResult += msg.m_iValue;
    }

    nsTime t0 = nsTime::Now();

    for (nsUInt32 i = 0; i < iNumObjects; ++i)
    {
      GetValueMessage msg;
      Objects[i]->GetDynamicRTTI()->DispatchMessage(Objects[i], msg);
      iResult += msg.m_iValue;
    }

    nsTime t1 = nsTime::Now();

    NS_TEST_INT(iResult, iNumObjects * 1 + iNumObjects * 2);

    nsTime tdiff = t1 - t0;
    double tFC = tdiff.GetNanoseconds() / (double)iNumObjects;

    nsLog::Info("[test]Dispatch Message: {0}ns", nsArgF(tFC, 2), iResult);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Virtual")
  {
    nsInt32 iResult = 0;

    // warm up
    for (nsUInt32 i = 0; i < iNumObjects; ++i)
      iResult += Objects[i]->Virtual();

    nsTime t0 = nsTime::Now();

    for (nsUInt32 i = 0; i < iNumObjects; ++i)
      iResult += Objects[i]->Virtual();

    nsTime t1 = nsTime::Now();

    NS_TEST_INT(iResult, iNumObjects * 1 + iNumObjects * 2);

    nsTime tdiff = t1 - t0;
    double tFC = tdiff.GetNanoseconds() / (double)iNumObjects;

    nsLog::Info("[test]Virtual Function Calls: {0}ns", nsArgF(tFC, 2), iResult);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "NonVirtual")
  {
    nsInt32 iResult = 0;

    // warm up
    for (nsUInt32 i = 0; i < iNumObjects; i += 2)
    {
      iResult += ((Derived1*)Objects[i])->NonVirtual();
      iResult += ((Derived2*)Objects[i])->NonVirtual();
    }

    nsTime t0 = nsTime::Now();

    for (nsUInt32 i = 0; i < iNumObjects; i += 2)
    {
      iResult += ((Derived1*)Objects[i])->NonVirtual();
      iResult += ((Derived2*)Objects[i])->NonVirtual();
    }

    nsTime t1 = nsTime::Now();

    NS_TEST_INT(iResult, iNumObjects * 1 + iNumObjects * 2);

    nsTime tdiff = t1 - t0;
    double tFC = tdiff.GetNanoseconds() / (double)iNumObjects;

    nsLog::Info("[test]Non-Virtual Function Calls: {0}ns", nsArgF(tFC, 2), iResult);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FastCall")
  {
    nsInt32 iResult = 0;

    // warm up
    for (nsUInt32 i = 0; i < iNumObjects; i += 2)
    {
      iResult += ((Derived1*)Objects[i])->FastCall();
      iResult += ((Derived2*)Objects[i])->FastCall();
    }

    nsTime t0 = nsTime::Now();

    for (nsUInt32 i = 0; i < iNumObjects; i += 2)
    {
      iResult += ((Derived1*)Objects[i])->FastCall();
      iResult += ((Derived2*)Objects[i])->FastCall();
    }

    nsTime t1 = nsTime::Now();

    NS_TEST_INT(iResult, iNumObjects * 1 + iNumObjects * 2);

    nsTime tdiff = t1 - t0;
    double tFC = tdiff.GetNanoseconds() / (double)iNumObjects;

    nsLog::Info("[test]FastCall Function Calls: {0}ns", nsArgF(tFC, 2), iResult);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "32 Bit Integer Division")
  {
    nsDynamicArray<nsInt32> Ints;
    Ints.SetCountUninitialized(iNumObjects);

    for (nsInt32 i = 0; i < iNumObjects; i += 1)
      Ints[i] = i * 100;

    nsTime t0 = nsTime::Now();

    nsInt32 iResult = 0;

    for (nsInt32 i = 1; i < iNumObjects; i += 1)
      iResult += Ints[i] / i;

    nsTime t1 = nsTime::Now();

    nsTime tdiff = t1 - t0;
    double t = tdiff.GetNanoseconds() / (double)(iNumObjects - 1);

    nsLog::Info("[test]32 Bit Integer Division: {0}ns", nsArgF(t, 2), iResult);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "32 Bit Integer Multiplication")
  {
    nsDynamicArray<nsInt32> Ints;
    Ints.SetCountUninitialized(iNumObjects);

    for (nsInt32 i = 0; i < iNumObjects; i += 1)
      Ints[i] = iNumObjects - i;

    nsTime t0 = nsTime::Now();

    nsInt32 iResult = 0;

    for (nsInt32 i = 0; i < iNumObjects; i += 1)
      iResult += Ints[i] * i;

    nsTime t1 = nsTime::Now();

    nsTime tdiff = t1 - t0;
    double t = tdiff.GetNanoseconds() / (double)(iNumObjects);

    nsLog::Info("[test]32 Bit Integer Multiplication: {0}ns", nsArgF(t, 2), iResult);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "64 Bit Integer Division")
  {
    nsDynamicArray<nsInt64> Ints;
    Ints.SetCountUninitialized(iNumObjects);

    for (nsInt32 i = 0; i < iNumObjects; i += 1)
      Ints[i] = (nsInt64)i * (nsInt64)100;

    nsTime t0 = nsTime::Now();

    nsInt64 iResult = 0;

    for (nsInt32 i = 1; i < iNumObjects; i += 1)
      iResult += Ints[i] / (nsInt64)i;

    nsTime t1 = nsTime::Now();

    nsTime tdiff = t1 - t0;
    double t = tdiff.GetNanoseconds() / (double)(iNumObjects - 1);

    nsLog::Info("[test]64 Bit Integer Division: {0}ns", nsArgF(t, 2), iResult);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "64 Bit Integer Multiplication")
  {
    nsDynamicArray<nsInt64> Ints;
    Ints.SetCountUninitialized(iNumObjects);

    for (nsInt32 i = 0; i < iNumObjects; i += 1)
      Ints[i] = iNumObjects - i;

    nsTime t0 = nsTime::Now();

    nsInt64 iResult = 0;

    for (nsInt32 i = 0; i < iNumObjects; i += 1)
      iResult += Ints[i] * (nsInt64)i;

    nsTime t1 = nsTime::Now();

    nsTime tdiff = t1 - t0;
    double t = tdiff.GetNanoseconds() / (double)(iNumObjects);

    nsLog::Info("[test]64 Bit Integer Multiplication: {0}ns", nsArgF(t, 2), iResult);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "32 Bit Float Division")
  {
    nsDynamicArray<float> Ints;
    Ints.SetCountUninitialized(iNumObjects);

    for (nsInt32 i = 0; i < iNumObjects; i += 1)
      Ints[i] = i * 100.0f;

    nsTime t0 = nsTime::Now();

    float fResult = 0;

    float d = 1.0f;
    for (nsInt32 i = 0; i < iNumObjects; i++, d += 1.0f)
      fResult += Ints[i] / d;

    nsTime t1 = nsTime::Now();

    nsTime tdiff = t1 - t0;
    double t = tdiff.GetNanoseconds() / (double)(iNumObjects);

    nsLog::Info("[test]32 Bit Float Division: {0}ns", nsArgF(t, 2), fResult);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "32 Bit Float Multiplication")
  {
    nsDynamicArray<float> Ints;
    Ints.SetCountUninitialized(iNumObjects);

    for (nsInt32 i = 0; i < iNumObjects; i++)
      Ints[i] = (float)(fNumObjects) - (float)(i);

    nsTime t0 = nsTime::Now();

    float iResult = 0;

    float d = 1.0f;
    for (nsInt32 i = 0; i < iNumObjects; i++, d += 1.0f)
      iResult += Ints[i] * d;

    nsTime t1 = nsTime::Now();

    nsTime tdiff = t1 - t0;
    double t = tdiff.GetNanoseconds() / (double)(iNumObjects);

    nsLog::Info("[test]32 Bit Float Multiplication: {0}ns", nsArgF(t, 2), iResult);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "64 Bit Double Division")
  {
    nsDynamicArray<double> Ints;
    Ints.SetCountUninitialized(iNumObjects);

    for (nsInt32 i = 0; i < iNumObjects; i += 1)
      Ints[i] = i * 100.0;

    nsTime t0 = nsTime::Now();

    double fResult = 0;

    double d = 1.0;
    for (nsInt32 i = 0; i < iNumObjects; i++, d += 1.0f)
      fResult += Ints[i] / d;

    nsTime t1 = nsTime::Now();

    nsTime tdiff = t1 - t0;
    double t = tdiff.GetNanoseconds() / (double)(iNumObjects);

    nsLog::Info("[test]64 Bit Double Division: {0}ns", nsArgF(t, 2), fResult);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "64 Bit Double Multiplication")
  {
    nsDynamicArray<double> Ints;
    Ints.SetCountUninitialized(iNumObjects);

    for (nsInt32 i = 0; i < iNumObjects; i++)
      Ints[i] = (double)(fNumObjects) - (double)(i);

    nsTime t0 = nsTime::Now();

    double iResult = 0;

    double d = 1.0;
    for (nsInt32 i = 0; i < iNumObjects; i++, d += 1.0)
      iResult += Ints[i] * d;

    nsTime t1 = nsTime::Now();

    nsTime tdiff = t1 - t0;
    double t = tdiff.GetNanoseconds() / (double)(iNumObjects);

    nsLog::Info("[test]64 Bit Double Multiplication: {0}ns", nsArgF(t, 2), iResult);
  }
}
