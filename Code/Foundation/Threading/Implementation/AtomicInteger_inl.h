
template <typename T>
NS_ALWAYS_INLINE nsAtomicInteger<T>::nsAtomicInteger()
  : m_Value(0)
{
}

template <typename T>
NS_ALWAYS_INLINE nsAtomicInteger<T>::nsAtomicInteger(T value)
  : m_Value(static_cast<UnderlyingType>(value))
{
}

template <typename T>
NS_ALWAYS_INLINE nsAtomicInteger<T>::nsAtomicInteger(const nsAtomicInteger<T>& value)
  : m_Value(nsAtomicUtils::Read(value.m_Value))
{
}

template <typename T>
NS_ALWAYS_INLINE nsAtomicInteger<T>& nsAtomicInteger<T>::operator=(const T value)
{
  Set(value);
  return *this;
}

template <typename T>
NS_ALWAYS_INLINE nsAtomicInteger<T>& nsAtomicInteger<T>::operator=(const nsAtomicInteger<T>& value)
{
  Set(nsAtomicUtils::Read(value.m_Value));
  return *this;
}

template <typename T>
NS_ALWAYS_INLINE T nsAtomicInteger<T>::Increment()
{
  return static_cast<T>(nsAtomicUtils::Increment(m_Value));
}

template <typename T>
NS_ALWAYS_INLINE T nsAtomicInteger<T>::Decrement()
{
  return static_cast<T>(nsAtomicUtils::Decrement(m_Value));
}

template <typename T>
NS_ALWAYS_INLINE T nsAtomicInteger<T>::PostIncrement()
{
  return static_cast<T>(nsAtomicUtils::PostIncrement(m_Value));
}

template <typename T>
NS_ALWAYS_INLINE T nsAtomicInteger<T>::PostDecrement()
{
  return static_cast<T>(nsAtomicUtils::PostDecrement(m_Value));
}

template <typename T>
NS_ALWAYS_INLINE void nsAtomicInteger<T>::Add(T x)
{
  nsAtomicUtils::Add(m_Value, static_cast<UnderlyingType>(x));
}

template <typename T>
NS_ALWAYS_INLINE void nsAtomicInteger<T>::Subtract(T x)
{
  nsAtomicUtils::Add(m_Value, -static_cast<UnderlyingType>(x));
}

template <typename T>
NS_ALWAYS_INLINE void nsAtomicInteger<T>::And(T x)
{
  nsAtomicUtils::And(m_Value, static_cast<UnderlyingType>(x));
}

template <typename T>
NS_ALWAYS_INLINE void nsAtomicInteger<T>::Or(T x)
{
  nsAtomicUtils::Or(m_Value, static_cast<UnderlyingType>(x));
}

template <typename T>
NS_ALWAYS_INLINE void nsAtomicInteger<T>::Xor(T x)
{
  nsAtomicUtils::Xor(m_Value, static_cast<UnderlyingType>(x));
}

template <typename T>
NS_ALWAYS_INLINE void nsAtomicInteger<T>::Min(T x)
{
  nsAtomicUtils::Min(m_Value, static_cast<UnderlyingType>(x));
}

template <typename T>
NS_ALWAYS_INLINE void nsAtomicInteger<T>::Max(T x)
{
  nsAtomicUtils::Max(m_Value, static_cast<UnderlyingType>(x));
}

template <typename T>
NS_ALWAYS_INLINE T nsAtomicInteger<T>::Set(T x)
{
  return static_cast<T>(nsAtomicUtils::Set(m_Value, static_cast<UnderlyingType>(x)));
}

template <typename T>
NS_ALWAYS_INLINE bool nsAtomicInteger<T>::TestAndSet(T expected, T x)
{
  return nsAtomicUtils::TestAndSet(m_Value, static_cast<UnderlyingType>(expected), static_cast<UnderlyingType>(x));
}

template <typename T>
NS_ALWAYS_INLINE T nsAtomicInteger<T>::CompareAndSwap(T expected, T x)
{
  return static_cast<T>(nsAtomicUtils::CompareAndSwap(m_Value, static_cast<UnderlyingType>(expected), static_cast<UnderlyingType>(x)));
}

template <typename T>
NS_ALWAYS_INLINE nsAtomicInteger<T>::operator T() const
{
  return static_cast<T>(nsAtomicUtils::Read(m_Value));
}

//////////////////////////////////////////////////////////////////////////

NS_ALWAYS_INLINE nsAtomicBool::nsAtomicBool() = default;
NS_ALWAYS_INLINE nsAtomicBool::~nsAtomicBool() = default;

NS_ALWAYS_INLINE nsAtomicBool::nsAtomicBool(bool value)
{
  Set(value);
}

NS_ALWAYS_INLINE nsAtomicBool::nsAtomicBool(const nsAtomicBool& rhs)
{
  Set(static_cast<bool>(rhs));
}

NS_ALWAYS_INLINE bool nsAtomicBool::Set(bool value)
{
  return m_iAtomicInt.Set(value ? 1 : 0) != 0;
}

NS_ALWAYS_INLINE void nsAtomicBool::operator=(bool value)
{
  Set(value);
}

NS_ALWAYS_INLINE void nsAtomicBool::operator=(const nsAtomicBool& rhs)
{
  Set(static_cast<bool>(rhs));
}

NS_ALWAYS_INLINE nsAtomicBool::operator bool() const
{
  return static_cast<nsInt32>(m_iAtomicInt) != 0;
}

NS_ALWAYS_INLINE bool nsAtomicBool::TestAndSet(bool bExpected, bool bNewValue)
{
  return m_iAtomicInt.TestAndSet(bExpected ? 1 : 0, bNewValue ? 1 : 0) != 0;
}
