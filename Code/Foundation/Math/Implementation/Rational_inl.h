
NS_ALWAYS_INLINE nsRational::nsRational()

  = default;

NS_ALWAYS_INLINE nsRational::nsRational(nsUInt32 uiNumerator, nsUInt32 uiDenominator)
  : m_uiNumerator(uiNumerator)
  , m_uiDenominator(uiDenominator)
{
}

NS_ALWAYS_INLINE bool nsRational::IsIntegral() const
{
  if (m_uiNumerator == 0 && m_uiDenominator == 0)
    return true;

  return ((m_uiNumerator / m_uiDenominator) * m_uiDenominator) == m_uiNumerator;
}

NS_ALWAYS_INLINE bool nsRational::operator==(const nsRational& other) const
{
  return m_uiNumerator == other.m_uiNumerator && m_uiDenominator == other.m_uiDenominator;
}

NS_ALWAYS_INLINE bool nsRational::operator!=(const nsRational& other) const
{
  return m_uiNumerator != other.m_uiNumerator || m_uiDenominator != other.m_uiDenominator;
}

NS_ALWAYS_INLINE nsUInt32 nsRational::GetNumerator() const
{
  return m_uiNumerator;
}

NS_ALWAYS_INLINE nsUInt32 nsRational::GetDenominator() const
{
  return m_uiDenominator;
}

NS_ALWAYS_INLINE nsUInt32 nsRational::GetIntegralResult() const
{
  if (m_uiNumerator == 0 && m_uiDenominator == 0)
    return 0;

  return m_uiNumerator / m_uiDenominator;
}

NS_ALWAYS_INLINE double nsRational::GetFloatingPointResult() const
{
  if (m_uiNumerator == 0 && m_uiDenominator == 0)
    return 0.0;

  return static_cast<double>(m_uiNumerator) / static_cast<double>(m_uiDenominator);
}

NS_ALWAYS_INLINE bool nsRational::IsValid() const
{
  return m_uiDenominator != 0 || (m_uiNumerator == 0 && m_uiDenominator == 0);
}

NS_ALWAYS_INLINE nsRational nsRational::ReduceIntegralFraction() const
{
  NS_ASSERT_DEV(IsValid() && IsIntegral(), "ReduceIntegralFraction can only be called on valid, integral rational numbers");

  return nsRational(m_uiNumerator / m_uiDenominator, 1);
}
