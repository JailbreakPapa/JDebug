#pragma once

#include <Foundation/Algorithm/HashingUtils.h>
#include <TestFramework/TestFrameworkDLL.h>

struct nsConstructionCounter
{
  /// Dummy m_iData, such that one can test the constructor with initialization
  nsInt32 m_iData;
  bool m_valid;

  /// Default Constructor
  nsConstructionCounter()
    : m_iData(0)
    , m_valid(true)
  {
    ++s_iConstructions;
  }

  /// Constructor with initialization
  nsConstructionCounter(nsInt32 d)
    : m_iData(d)
    , m_valid(true)
  {
    ++s_iConstructions;
  }

  /// Copy Constructor
  nsConstructionCounter(const nsConstructionCounter& cc)
    : m_iData(cc.m_iData)
    , m_valid(true)
  {
    ++s_iConstructions;
  }

  /// Move construction counts as a construction as well.
  nsConstructionCounter(nsConstructionCounter&& cc) noexcept
    : m_iData(cc.m_iData)
    , m_valid(true)
  {
    cc.m_iData = 0; // data has been moved, so "destroy" it.
    ++s_iConstructions;
  }

  /// Destructor
  ~nsConstructionCounter()
  {
    NS_ASSERT_ALWAYS(m_valid, "Destroying object twice");
    m_valid = false;
    ++s_iDestructions;
  }

  /// Assignment does not change the construction counter, because it is only executed on already constructed objects.
  void operator=(const nsConstructionCounter& cc) { m_iData = cc.m_iData; }
  /// Move assignment does not change the construction counter, because it is only executed on already constructed objects.
  void operator=(const nsConstructionCounter&& cc) noexcept { m_iData = cc.m_iData; }

  bool operator==(const nsConstructionCounter& cc) const { return m_iData == cc.m_iData; }
  NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(const nsConstructionCounter&);

  bool operator<(const nsConstructionCounter& rhs) const { return m_iData < rhs.m_iData; }

  /// Checks whether n constructions have been done since the last check.
  static bool HasConstructed(nsInt32 iCons)
  {
    const bool b = s_iConstructions == s_iConstructionsLast + iCons;
    s_iConstructionsLast = s_iConstructions;
    s_iDestructionsLast = s_iDestructions;

    if (!b)
      PrintStats();

    return (b);
  }

  /// Checks whether n destructions have been done since the last check.
  static bool HasDestructed(nsInt32 iCons)
  {
    const bool b = s_iDestructions == s_iDestructionsLast + iCons;
    s_iConstructionsLast = s_iConstructions;
    s_iDestructionsLast = s_iDestructions;

    if (!b)
      PrintStats();

    return (b);
  }

  /// Checks whether n constructions and destructions have been done since the last check.
  static bool HasDone(nsInt32 iCons, nsInt32 iDes)
  {
    const bool bc = (s_iConstructions == (s_iConstructionsLast + iCons));
    const bool bd = (s_iDestructions == (s_iDestructionsLast + iDes));

    if (!(bc && bd))
      PrintStats();

    s_iConstructionsLast = s_iConstructions;
    s_iDestructionsLast = s_iDestructions;

    return (bc && bd);
  }

  /// For debugging and getting tests right: Prints out the current number of constructions and destructions
  static void PrintStats()
  {
    printf("Constructions: %d (New: %i), Destructions: %d (New: %i) \n", s_iConstructions, s_iConstructions - s_iConstructionsLast, s_iDestructions,
      s_iDestructions - s_iDestructionsLast);
  }

  /// Checks that all instances have been destructed.
  static bool HasAllDestructed()
  {
    if (s_iConstructions != s_iDestructions)
      PrintStats();

    s_iConstructionsLast = s_iConstructions;
    s_iDestructionsLast = s_iDestructions;

    return (s_iConstructions == s_iDestructions);
  }

  static void Reset()
  {
    s_iConstructions = 0;
    s_iConstructionsLast = 0;
    s_iDestructions = 0;
    s_iDestructionsLast = 0;
  }

  static nsInt32 s_iConstructions;
  static nsInt32 s_iConstructionsLast;
  static nsInt32 s_iDestructions;
  static nsInt32 s_iDestructionsLast;
};

struct nsConstructionCounterRelocatable
{
  NS_DECLARE_MEM_RELOCATABLE_TYPE();

  /// Dummy m_iData, such that one can test the constructor with initialization
  nsInt32 m_iData;

  /// Bool to track if the element was default constructed or received valid data.
  bool m_valid = false;

  nsConstructionCounterRelocatable() = default;

  nsConstructionCounterRelocatable(nsInt32 d)
    : m_iData(d)
    , m_valid(true)
  {
    s_iConstructions++;
  }

  nsConstructionCounterRelocatable(const nsConstructionCounterRelocatable& other) = delete;

  nsConstructionCounterRelocatable(nsConstructionCounterRelocatable&& other) noexcept
  {
    m_iData = other.m_iData;
    m_valid = other.m_valid;

    other.m_valid = false;
  }

  ~nsConstructionCounterRelocatable()
  {
    if (m_valid)
      s_iDestructions++;
  }

  void operator=(nsConstructionCounterRelocatable&& other) noexcept
  {
    m_iData = other.m_iData;
    m_valid = other.m_valid;

    other.m_valid = false;
    ;
  }

  /// For debugging and getting tests right: Prints out the current number of constructions and destructions
  static void PrintStats()
  {
    printf("Constructions: %d (New: %i), Destructions: %d (New: %i) \n", s_iConstructions, s_iConstructions - s_iConstructionsLast, s_iDestructions,
      s_iDestructions - s_iDestructionsLast);
  }

  /// Checks whether n constructions and destructions have been done since the last check.
  static bool HasDone(nsInt32 iCons, nsInt32 iDes)
  {
    const bool bc = (s_iConstructions == (s_iConstructionsLast + iCons));
    const bool bd = (s_iDestructions == (s_iDestructionsLast + iDes));

    if (!(bc && bd))
      PrintStats();

    s_iConstructionsLast = s_iConstructions;
    s_iDestructionsLast = s_iDestructions;

    return (bc && bd);
  }

  /// Checks that all instances have been destructed.
  static bool HasAllDestructed()
  {
    if (s_iConstructions != s_iDestructions)
      PrintStats();

    s_iConstructionsLast = s_iConstructions;
    s_iDestructionsLast = s_iDestructions;

    return (s_iConstructions == s_iDestructions);
  }

  static void Reset()
  {
    s_iConstructions = 0;
    s_iConstructionsLast = 0;
    s_iDestructions = 0;
    s_iDestructionsLast = 0;
  }

  static nsInt32 s_iConstructions;
  static nsInt32 s_iConstructionsLast;
  static nsInt32 s_iDestructions;
  static nsInt32 s_iDestructionsLast;
};

template <>
struct nsHashHelper<nsConstructionCounter>
{
  static nsUInt32 Hash(const nsConstructionCounter& value) { return nsHashHelper<nsInt32>::Hash(value.m_iData); }

  NS_ALWAYS_INLINE static bool Equal(const nsConstructionCounter& a, const nsConstructionCounter& b) { return a == b; }
};
