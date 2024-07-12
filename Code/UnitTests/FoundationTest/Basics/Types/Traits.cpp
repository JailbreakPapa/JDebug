#include <FoundationTest/FoundationTestPCH.h>

// This test does not actually run, it tests compile time stuff

namespace
{
  struct AggregatePod
  {
    int m_1;
    float m_2;

    NS_DETECT_TYPE_CLASS(int, float);
  };

  struct AggregatePod2
  {
    int m_1;
    float m_2;
    AggregatePod m_3;

    NS_DETECT_TYPE_CLASS(int, float, AggregatePod);
  };

  struct MemRelocateable
  {
    NS_DECLARE_MEM_RELOCATABLE_TYPE();
  };

  struct AggregateMemRelocateable
  {
    int m_1;
    float m_2;
    AggregatePod m_3;
    MemRelocateable m_4;

    NS_DETECT_TYPE_CLASS(int, float, AggregatePod, MemRelocateable);
  };

  class ClassType
  {
  };

  struct AggregateClass
  {
    int m_1;
    float m_2;
    AggregatePod m_3;
    MemRelocateable m_4;
    ClassType m_5;

    NS_DETECT_TYPE_CLASS(int, float, AggregatePod, MemRelocateable, ClassType);
  };

  NS_CHECK_AT_COMPILETIME(nsGetTypeClass<AggregatePod>::value == nsTypeIsPod::value);
  NS_CHECK_AT_COMPILETIME(nsGetTypeClass<AggregatePod2>::value == nsTypeIsPod::value);
  NS_CHECK_AT_COMPILETIME(nsGetTypeClass<MemRelocateable>::value == nsTypeIsMemRelocatable::value);
  NS_CHECK_AT_COMPILETIME(nsGetTypeClass<AggregateMemRelocateable>::value == nsTypeIsMemRelocatable::value);
  NS_CHECK_AT_COMPILETIME(nsGetTypeClass<ClassType>::value == nsTypeIsClass::value);
  NS_CHECK_AT_COMPILETIME(nsGetTypeClass<AggregateClass>::value == nsTypeIsClass::value);
} // namespace
