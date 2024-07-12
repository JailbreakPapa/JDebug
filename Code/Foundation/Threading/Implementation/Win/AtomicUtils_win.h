#ifdef NS_ATOMICUTLS_WIN_INL_H_INCLUDED
#  error "This file must not be included twice."
#endif

#define NS_ATOMICUTLS_WIN_INL_H_INCLUDED

#include <intrin.h>

NS_ALWAYS_INLINE nsInt32 nsAtomicUtils::Read(const nsInt32& iSrc)
{
  return _InterlockedOr((long*)(&iSrc), 0);
}

NS_ALWAYS_INLINE nsInt64 nsAtomicUtils::Read(const nsInt64& iSrc)
{
#if NS_ENABLED(NS_PLATFORM_32BIT)
  nsInt64 old;
  do
  {
    old = iSrc;
  } while (_InterlockedCompareExchange64(const_cast<nsInt64*>(&iSrc), old, old) != old);
  return old;
#else
  return _InterlockedOr64(const_cast<nsInt64*>(&iSrc), 0);
#endif
}

NS_ALWAYS_INLINE nsInt32 nsAtomicUtils::Increment(nsInt32& ref_iDest)
{
  return _InterlockedIncrement(reinterpret_cast<long*>(&ref_iDest));
}

NS_ALWAYS_INLINE nsInt64 nsAtomicUtils::Increment(nsInt64& ref_iDest)
{
#if NS_ENABLED(NS_PLATFORM_32BIT)
  nsInt64 old;
  do
  {
    old = ref_iDest;
  } while (_InterlockedCompareExchange64(&ref_iDest, old + 1, old) != old);
  return old + 1;
#else
  return _InterlockedIncrement64(&ref_iDest);
#endif
}

NS_ALWAYS_INLINE nsInt32 nsAtomicUtils::Decrement(nsInt32& ref_iDest)
{
  return _InterlockedDecrement(reinterpret_cast<long*>(&ref_iDest));
}

NS_ALWAYS_INLINE nsInt64 nsAtomicUtils::Decrement(nsInt64& ref_iDest)
{
#if NS_ENABLED(NS_PLATFORM_32BIT)
  nsInt64 old;
  do
  {
    old = ref_iDest;
  } while (_InterlockedCompareExchange64(&ref_iDest, old - 1, old) != old);
  return old - 1;
#else
  return _InterlockedDecrement64(&ref_iDest);
#endif
}

NS_ALWAYS_INLINE nsInt32 nsAtomicUtils::PostIncrement(nsInt32& ref_iDest)
{
  return _InterlockedExchangeAdd(reinterpret_cast<long*>(&ref_iDest), 1);
}

NS_ALWAYS_INLINE nsInt64 nsAtomicUtils::PostIncrement(nsInt64& ref_iDest)
{
#if NS_ENABLED(NS_PLATFORM_32BIT)
  nsInt64 old;
  do
  {
    old = ref_iDest;
  } while (_InterlockedCompareExchange64(&ref_iDest, old + 1, old) != old);
  return old;
#else
  return _InterlockedExchangeAdd64(&ref_iDest, 1);
#endif
}

NS_ALWAYS_INLINE nsInt32 nsAtomicUtils::PostDecrement(nsInt32& ref_iDest)
{
  return _InterlockedExchangeAdd(reinterpret_cast<long*>(&ref_iDest), -1);
}

NS_ALWAYS_INLINE nsInt64 nsAtomicUtils::PostDecrement(nsInt64& ref_iDest)
{
#if NS_ENABLED(NS_PLATFORM_32BIT)
  nsInt64 old;
  do
  {
    old = ref_iDest;
  } while (_InterlockedCompareExchange64(&ref_iDest, old - 1, old) != old);
  return old;
#else
  return _InterlockedExchangeAdd64(&ref_iDest, -1);
#endif
}

NS_ALWAYS_INLINE void nsAtomicUtils::Add(nsInt32& ref_iDest, nsInt32 value)
{
  _InterlockedExchangeAdd(reinterpret_cast<long*>(&ref_iDest), value);
}

NS_ALWAYS_INLINE void nsAtomicUtils::Add(nsInt64& ref_iDest, nsInt64 value)
{
#if NS_ENABLED(NS_PLATFORM_32BIT)
  nsInt64 old;
  do
  {
    old = ref_iDest;
  } while (_InterlockedCompareExchange64(&ref_iDest, old + value, old) != old);
#else
  _InterlockedExchangeAdd64(&ref_iDest, value);
#endif
}


NS_ALWAYS_INLINE void nsAtomicUtils::And(nsInt32& ref_iDest, nsInt32 value)
{
  _InterlockedAnd(reinterpret_cast<long*>(&ref_iDest), value);
}

NS_ALWAYS_INLINE void nsAtomicUtils::And(nsInt64& ref_iDest, nsInt64 value)
{
#if NS_ENABLED(NS_PLATFORM_32BIT)
  nsInt64 old;
  do
  {
    old = ref_iDest;
  } while (_InterlockedCompareExchange64(&ref_iDest, old & value, old) != old);
#else
  _InterlockedAnd64(&ref_iDest, value);
#endif
}


NS_ALWAYS_INLINE void nsAtomicUtils::Or(nsInt32& ref_iDest, nsInt32 value)
{
  _InterlockedOr(reinterpret_cast<long*>(&ref_iDest), value);
}

NS_ALWAYS_INLINE void nsAtomicUtils::Or(nsInt64& ref_iDest, nsInt64 value)
{
#if NS_ENABLED(NS_PLATFORM_32BIT)
  nsInt64 old;
  do
  {
    old = ref_iDest;
  } while (_InterlockedCompareExchange64(&ref_iDest, old | value, old) != old);
#else
  _InterlockedOr64(&ref_iDest, value);
#endif
}


NS_ALWAYS_INLINE void nsAtomicUtils::Xor(nsInt32& ref_iDest, nsInt32 value)
{
  _InterlockedXor(reinterpret_cast<long*>(&ref_iDest), value);
}

NS_ALWAYS_INLINE void nsAtomicUtils::Xor(nsInt64& ref_iDest, nsInt64 value)
{
#if NS_ENABLED(NS_PLATFORM_32BIT)
  nsInt64 old;
  do
  {
    old = ref_iDest;
  } while (_InterlockedCompareExchange64(&ref_iDest, old ^ value, old) != old);
#else
  _InterlockedXor64(&ref_iDest, value);
#endif
}


inline void nsAtomicUtils::Min(nsInt32& ref_iDest, nsInt32 value)
{
  // tries to exchange dest with the new value as long as the oldValue is not what we expected
  while (true)
  {
    nsInt32 iOldValue = ref_iDest;
    nsInt32 iNewValue = value < iOldValue ? value : iOldValue; // do Min manually here, to break #include cycles

    if (_InterlockedCompareExchange(reinterpret_cast<long*>(&ref_iDest), iNewValue, iOldValue) == iOldValue)
      break;
  }
}

inline void nsAtomicUtils::Min(nsInt64& ref_iDest, nsInt64 value)
{
  // tries to exchange dest with the new value as long as the oldValue is not what we expected
  while (true)
  {
    nsInt64 iOldValue = ref_iDest;
    nsInt64 iNewValue = value < iOldValue ? value : iOldValue; // do Min manually here, to break #include cycles

    if (_InterlockedCompareExchange64(&ref_iDest, iNewValue, iOldValue) == iOldValue)
      break;
  }
}

inline void nsAtomicUtils::Max(nsInt32& ref_iDest, nsInt32 value)
{
  // tries to exchange dest with the new value as long as the oldValue is not what we expected
  while (true)
  {
    nsInt32 iOldValue = ref_iDest;
    nsInt32 iNewValue = iOldValue < value ? value : iOldValue; // do Max manually here, to break #include cycles

    if (_InterlockedCompareExchange(reinterpret_cast<long*>(&ref_iDest), iNewValue, iOldValue) == iOldValue)
      break;
  }
}

inline void nsAtomicUtils::Max(nsInt64& ref_iDest, nsInt64 value)
{
  // tries to exchange dest with the new value as long as the oldValue is not what we expected
  while (true)
  {
    nsInt64 iOldValue = ref_iDest;
    nsInt64 iNewValue = iOldValue < value ? value : iOldValue; // do Max manually here, to break #include cycles

    if (_InterlockedCompareExchange64(&ref_iDest, iNewValue, iOldValue) == iOldValue)
      break;
  }
}


inline nsInt32 nsAtomicUtils::Set(nsInt32& ref_iDest, nsInt32 value)
{
  return _InterlockedExchange(reinterpret_cast<long*>(&ref_iDest), value);
}

NS_ALWAYS_INLINE nsInt64 nsAtomicUtils::Set(nsInt64& ref_iDest, nsInt64 value)
{
#if NS_ENABLED(NS_PLATFORM_32BIT)
  nsInt64 old;
  do
  {
    old = ref_iDest;
  } while (_InterlockedCompareExchange64(&ref_iDest, value, old) != old);
  return old;
#else
  return _InterlockedExchange64(&ref_iDest, value);
#endif
}


NS_ALWAYS_INLINE bool nsAtomicUtils::TestAndSet(nsInt32& ref_iDest, nsInt32 iExpected, nsInt32 value)
{
  return _InterlockedCompareExchange(reinterpret_cast<long*>(&ref_iDest), value, iExpected) == iExpected;
}

NS_ALWAYS_INLINE bool nsAtomicUtils::TestAndSet(nsInt64& ref_iDest, nsInt64 iExpected, nsInt64 value)
{
  return _InterlockedCompareExchange64(&ref_iDest, value, iExpected) == iExpected;
}

NS_ALWAYS_INLINE bool nsAtomicUtils::TestAndSet(void** pDest, void* pExpected, void* value)
{
  return _InterlockedCompareExchangePointer(pDest, value, pExpected) == pExpected;
}

NS_ALWAYS_INLINE nsInt32 nsAtomicUtils::CompareAndSwap(nsInt32& ref_iDest, nsInt32 iExpected, nsInt32 value)
{
  return _InterlockedCompareExchange(reinterpret_cast<long*>(&ref_iDest), value, iExpected);
}

NS_ALWAYS_INLINE nsInt64 nsAtomicUtils::CompareAndSwap(nsInt64& ref_iDest, nsInt64 iExpected, nsInt64 value)
{
  return _InterlockedCompareExchange64(&ref_iDest, value, iExpected);
}
