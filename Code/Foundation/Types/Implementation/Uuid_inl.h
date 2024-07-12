
nsUuid::nsUuid()
  : m_uiHigh(0)
  , m_uiLow(0)
{
}

bool nsUuid::operator==(const nsUuid& other) const
{
  return m_uiHigh == other.m_uiHigh && m_uiLow == other.m_uiLow;
}

bool nsUuid::operator!=(const nsUuid& other) const
{
  return m_uiHigh != other.m_uiHigh || m_uiLow != other.m_uiLow;
}

bool nsUuid::operator<(const nsUuid& other) const
{
  if (m_uiHigh < other.m_uiHigh)
    return true;
  if (m_uiHigh > other.m_uiHigh)
    return false;

  return m_uiLow < other.m_uiLow;
}

bool nsUuid::IsValid() const
{
  return m_uiHigh != 0 || m_uiLow != 0;
}

void nsUuid::CombineWithSeed(const nsUuid& seed)
{
  m_uiHigh += seed.m_uiHigh;
  m_uiLow += seed.m_uiLow;
}

void nsUuid::RevertCombinationWithSeed(const nsUuid& seed)
{
  m_uiHigh -= seed.m_uiHigh;
  m_uiLow -= seed.m_uiLow;
}

void nsUuid::HashCombine(const nsUuid& guid)
{
  m_uiHigh = nsHashingUtils::xxHash64(&guid.m_uiHigh, sizeof(nsUInt64), m_uiHigh);
  m_uiLow = nsHashingUtils::xxHash64(&guid.m_uiLow, sizeof(nsUInt64), m_uiLow);
}

template <>
struct nsHashHelper<nsUuid>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(const nsUuid& value) { return nsHashingUtils::xxHash32(&value, sizeof(nsUuid)); }

  NS_ALWAYS_INLINE static bool Equal(const nsUuid& a, const nsUuid& b) { return a == b; }
};
