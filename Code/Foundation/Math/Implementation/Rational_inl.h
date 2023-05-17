
WD_ALWAYS_INLINE wdRational::wdRational()
  : m_uiNumerator(0)
  , m_uiDenominator(1)
{}

WD_ALWAYS_INLINE wdRational::wdRational(wdUInt32 uiNumerator, wdUInt32 uiDenominator)
  : m_uiNumerator(uiNumerator)
  , m_uiDenominator(uiDenominator)
{
}

WD_ALWAYS_INLINE bool wdRational::IsIntegral() const
{
  if (m_uiNumerator == 0 && m_uiDenominator == 0)
    return true;

  return ((m_uiNumerator / m_uiDenominator) * m_uiDenominator) == m_uiNumerator;
}

WD_ALWAYS_INLINE bool wdRational::operator==(const wdRational& other) const
{
  return m_uiNumerator == other.m_uiNumerator && m_uiDenominator == other.m_uiDenominator;
}

WD_ALWAYS_INLINE bool wdRational::operator!=(const wdRational& other) const
{
  return m_uiNumerator != other.m_uiNumerator || m_uiDenominator != other.m_uiDenominator;
}

WD_ALWAYS_INLINE wdUInt32 wdRational::GetNumerator() const
{
  return m_uiNumerator;
}

WD_ALWAYS_INLINE wdUInt32 wdRational::GetDenominator() const
{
  return m_uiDenominator;
}

WD_ALWAYS_INLINE wdUInt32 wdRational::GetIntegralResult() const
{
  if (m_uiNumerator == 0 && m_uiDenominator == 0)
    return 0;

  return m_uiNumerator / m_uiDenominator;
}

WD_ALWAYS_INLINE double wdRational::GetFloatingPointResult() const
{
  if (m_uiNumerator == 0 && m_uiDenominator == 0)
    return 0.0;

  return static_cast<double>(m_uiNumerator) / static_cast<double>(m_uiDenominator);
}

WD_ALWAYS_INLINE bool wdRational::IsValid() const
{
  return m_uiDenominator != 0 || (m_uiNumerator == 0 && m_uiDenominator == 0);
}

WD_ALWAYS_INLINE wdRational wdRational::ReduceIntegralFraction() const
{
  WD_ASSERT_DEV(IsValid() && IsIntegral(), "ReduceIntegralFraction can only be called on valid, integral rational numbers");

  return wdRational(m_uiNumerator / m_uiDenominator, 1);
}
