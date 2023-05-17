
wdUuid::wdUuid()
  : m_uiHigh(0)
  , m_uiLow(0)
{
}

void wdUuid::SetInvalid()
{
  m_uiHigh = 0;
  m_uiLow = 0;
}

wdUuid wdUuid::CreateUuid()
{
  wdUuid guid;
  guid.CreateNewUuid();
  return guid;
}

bool wdUuid::operator==(const wdUuid& other) const
{
  return m_uiHigh == other.m_uiHigh && m_uiLow == other.m_uiLow;
}

bool wdUuid::operator!=(const wdUuid& other) const
{
  return m_uiHigh != other.m_uiHigh || m_uiLow != other.m_uiLow;
}

bool wdUuid::operator<(const wdUuid& other) const
{
  if (m_uiHigh < other.m_uiHigh)
    return true;
  if (m_uiHigh > other.m_uiHigh)
    return false;

  return m_uiLow < other.m_uiLow;
}

bool wdUuid::IsValid() const
{
  return m_uiHigh != 0 || m_uiLow != 0;
}

void wdUuid::CombineWithSeed(const wdUuid& seed)
{
  m_uiHigh += seed.m_uiHigh;
  m_uiLow += seed.m_uiLow;
}

void wdUuid::RevertCombinationWithSeed(const wdUuid& seed)
{
  m_uiHigh -= seed.m_uiHigh;
  m_uiLow -= seed.m_uiLow;
}

void wdUuid::HashCombine(const wdUuid& guid)
{
  m_uiHigh = wdHashingUtils::xxHash64(&guid.m_uiHigh, sizeof(wdUInt64), m_uiHigh);
  m_uiLow = wdHashingUtils::xxHash64(&guid.m_uiLow, sizeof(wdUInt64), m_uiLow);
}

template <>
struct wdHashHelper<wdUuid>
{
  WD_ALWAYS_INLINE static wdUInt32 Hash(const wdUuid& value) { return wdHashingUtils::xxHash32(&value, sizeof(wdUuid)); }

  WD_ALWAYS_INLINE static bool Equal(const wdUuid& a, const wdUuid& b) { return a == b; }
};
