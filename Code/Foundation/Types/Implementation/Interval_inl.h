
template <class Type>
constexpr nsInterval<Type>::nsInterval(Type startAndEndValue)
  : m_StartValue(startAndEndValue)
  , m_EndValue(startAndEndValue)
{
}

template <class Type>
constexpr nsInterval<Type>::nsInterval(Type start, Type end)
  : m_StartValue(start)
  , m_EndValue(nsMath::Max(start, end))
{
}

template <class Type>
void nsInterval<Type>::SetStartAdjustEnd(Type value)
{
  m_StartValue = value;
  m_EndValue = nsMath::Max(m_EndValue, m_StartValue);
}

template <class Type>
void nsInterval<Type>::SetEndAdjustStart(Type value)
{
  m_EndValue = value;
  m_StartValue = nsMath::Min(m_StartValue, m_EndValue);
}

template <class Type>
void nsInterval<Type>::ClampToIntervalAdjustEnd(Type minValue, Type maxValue, Type minimumSeparation /*= Type()*/)
{
  // clamp the start value to the valid range, leave minimumSeparation at the end
  m_StartValue = nsMath::Clamp(m_StartValue, minValue, maxValue - minimumSeparation);

  // clamp the start value to the remaining range
  m_EndValue = nsMath::Clamp(m_EndValue, m_StartValue, maxValue);
}

template <class Type>
void nsInterval<Type>::ClampToIntervalAdjustStart(Type minValue, Type maxValue, Type minimumSeparation /*= Type()*/)
{
  // clamp the end value to the valid range, leave minimumSeparation at the start
  m_EndValue = nsMath::Clamp(m_EndValue, minValue + minimumSeparation, maxValue);

  // clamp the start value to the remaining range
  m_StartValue = nsMath::Clamp(m_StartValue, minValue, m_EndValue);
}

template <class Type>
Type nsInterval<Type>::GetSeparation() const
{
  return m_EndValue - m_StartValue;
}

template <class Type>
bool nsInterval<Type>::operator==(const nsInterval<Type>& rhs) const
{
  return m_StartValue == rhs.m_StartValue && m_EndValue == rhs.m_EndValue;
}

template <class Type>
bool nsInterval<Type>::operator!=(const nsInterval<Type>& rhs) const
{
  return !operator==(rhs);
}
