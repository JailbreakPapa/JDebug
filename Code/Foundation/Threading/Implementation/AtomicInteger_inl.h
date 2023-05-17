
template <typename T>
WD_ALWAYS_INLINE wdAtomicInteger<T>::wdAtomicInteger()
  : m_value(0)
{
}

template <typename T>
WD_ALWAYS_INLINE wdAtomicInteger<T>::wdAtomicInteger(T value)
  : m_value(value)
{
}

template <typename T>
WD_ALWAYS_INLINE wdAtomicInteger<T>::wdAtomicInteger(const wdAtomicInteger<T>& value)
  : m_value(value.m_value)
{
}

template <typename T>
WD_ALWAYS_INLINE wdAtomicInteger<T>& wdAtomicInteger<T>::operator=(const T value)
{
  m_value = value;
  return *this;
}

template <typename T>
WD_ALWAYS_INLINE wdAtomicInteger<T>& wdAtomicInteger<T>::operator=(const wdAtomicInteger<T>& value)
{
  m_value = value.m_value;
  return *this;
}

template <typename T>
WD_ALWAYS_INLINE T wdAtomicInteger<T>::Increment()
{
  return wdAtomicUtils::Increment(m_value);
}

template <typename T>
WD_ALWAYS_INLINE T wdAtomicInteger<T>::Decrement()
{
  return wdAtomicUtils::Decrement(m_value);
}

template <typename T>
WD_ALWAYS_INLINE T wdAtomicInteger<T>::PostIncrement()
{
  return wdAtomicUtils::PostIncrement(m_value);
}

template <typename T>
WD_ALWAYS_INLINE T wdAtomicInteger<T>::PostDecrement()
{
  return wdAtomicUtils::PostDecrement(m_value);
}

template <typename T>
WD_ALWAYS_INLINE void wdAtomicInteger<T>::Add(T x)
{
  wdAtomicUtils::Add(m_value, x);
}

template <typename T>
WD_ALWAYS_INLINE void wdAtomicInteger<T>::Subtract(T x)
{
  wdAtomicUtils::Add(m_value, -x);
}

template <typename T>
WD_ALWAYS_INLINE void wdAtomicInteger<T>::And(T x)
{
  wdAtomicUtils::And(m_value, x);
}

template <typename T>
WD_ALWAYS_INLINE void wdAtomicInteger<T>::Or(T x)
{
  wdAtomicUtils::Or(m_value, x);
}

template <typename T>
WD_ALWAYS_INLINE void wdAtomicInteger<T>::Xor(T x)
{
  wdAtomicUtils::Xor(m_value, x);
}

template <typename T>
WD_ALWAYS_INLINE void wdAtomicInteger<T>::Min(T x)
{
  wdAtomicUtils::Min(m_value, x);
}

template <typename T>
WD_ALWAYS_INLINE void wdAtomicInteger<T>::Max(T x)
{
  wdAtomicUtils::Max(m_value, x);
}

template <typename T>
WD_ALWAYS_INLINE T wdAtomicInteger<T>::Set(T x)
{
  return wdAtomicUtils::Set(m_value, x);
}

template <typename T>
WD_ALWAYS_INLINE bool wdAtomicInteger<T>::TestAndSet(T expected, T x)
{
  return wdAtomicUtils::TestAndSet(m_value, expected, x);
}

template <typename T>
WD_ALWAYS_INLINE T wdAtomicInteger<T>::CompareAndSwap(T expected, T x)
{
  return wdAtomicUtils::CompareAndSwap(m_value, expected, x);
}

template <typename T>
WD_ALWAYS_INLINE wdAtomicInteger<T>::operator T() const
{
  return wdAtomicUtils::Read(m_value);
}

//////////////////////////////////////////////////////////////////////////

WD_ALWAYS_INLINE wdAtomicBool::wdAtomicBool() = default;
WD_ALWAYS_INLINE wdAtomicBool::~wdAtomicBool() = default;

WD_ALWAYS_INLINE wdAtomicBool::wdAtomicBool(bool value)
{
  Set(value);
}

WD_ALWAYS_INLINE wdAtomicBool::wdAtomicBool(const wdAtomicBool& rhs)
{
  Set(static_cast<bool>(rhs));
}

WD_ALWAYS_INLINE bool wdAtomicBool::Set(bool value)
{
  return m_iAtomicInt.Set(value ? 1 : 0) != 0;
}

WD_ALWAYS_INLINE void wdAtomicBool::operator=(bool value)
{
  Set(value);
}

WD_ALWAYS_INLINE void wdAtomicBool::operator=(const wdAtomicBool& rhs)
{
  Set(static_cast<bool>(rhs));
}

WD_ALWAYS_INLINE wdAtomicBool::operator bool() const
{
  return static_cast<wdInt32>(m_iAtomicInt) != 0;
}

WD_ALWAYS_INLINE bool wdAtomicBool::TestAndSet(bool bExpected, bool bNewValue)
{
  return m_iAtomicInt.TestAndSet(bExpected ? 1 : 0, bNewValue ? 1 : 0) != 0;
}
