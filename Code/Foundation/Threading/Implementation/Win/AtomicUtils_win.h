#ifdef WD_ATOMICUTLS_WIN_INL_H_INCLUDED
#  error "This file must not be included twice."
#endif

#define WD_ATOMICUTLS_WIN_INL_H_INCLUDED

#include <intrin.h>

WD_ALWAYS_INLINE wdInt32 wdAtomicUtils::Read(volatile const wdInt32& iSrc)
{
  return _InterlockedOr((volatile long*)(&iSrc), 0);
}

WD_ALWAYS_INLINE wdInt64 wdAtomicUtils::Read(volatile const wdInt64& iSrc)
{
#if WD_ENABLED(WD_PLATFORM_32BIT)
  wdInt64 old;
  do
  {
    old = src;
  } while (_InterlockedCompareExchange64(const_cast<volatile wdInt64*>(&src), old, old) != old);
  return old;
#else
  return _InterlockedOr64(const_cast<volatile wdInt64*>(&iSrc), 0);
#endif
}

WD_ALWAYS_INLINE wdInt32 wdAtomicUtils::Increment(volatile wdInt32& ref_iDest)
{
  return _InterlockedIncrement(reinterpret_cast<volatile long*>(&ref_iDest));
}

WD_ALWAYS_INLINE wdInt64 wdAtomicUtils::Increment(volatile wdInt64& ref_iDest)
{
#if WD_ENABLED(WD_PLATFORM_32BIT)
  wdInt64 old;
  do
  {
    old = dest;
  } while (_InterlockedCompareExchange64(&dest, old + 1, old) != old);
  return old + 1;
#else
  return _InterlockedIncrement64(&ref_iDest);
#endif
}

WD_ALWAYS_INLINE wdInt32 wdAtomicUtils::Decrement(volatile wdInt32& ref_iDest)
{
  return _InterlockedDecrement(reinterpret_cast<volatile long*>(&ref_iDest));
}

WD_ALWAYS_INLINE wdInt64 wdAtomicUtils::Decrement(volatile wdInt64& ref_iDest)
{
#if WD_ENABLED(WD_PLATFORM_32BIT)
  wdInt64 old;
  do
  {
    old = dest;
  } while (_InterlockedCompareExchange64(&dest, old - 1, old) != old);
  return old - 1;
#else
  return _InterlockedDecrement64(&ref_iDest);
#endif
}

WD_ALWAYS_INLINE wdInt32 wdAtomicUtils::PostIncrement(volatile wdInt32& ref_iDest)
{
  return _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&ref_iDest), 1);
}

WD_ALWAYS_INLINE wdInt64 wdAtomicUtils::PostIncrement(volatile wdInt64& ref_iDest)
{
#if WD_ENABLED(WD_PLATFORM_32BIT)
  wdInt64 old;
  do
  {
    old = dest;
  } while (_InterlockedCompareExchange64(&dest, old + 1, old) != old);
  return old;
#else
  return _InterlockedExchangeAdd64(&ref_iDest, 1);
#endif
}

WD_ALWAYS_INLINE wdInt32 wdAtomicUtils::PostDecrement(volatile wdInt32& ref_iDest)
{
  return _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&ref_iDest), -1);
}

WD_ALWAYS_INLINE wdInt64 wdAtomicUtils::PostDecrement(volatile wdInt64& ref_iDest)
{
#if WD_ENABLED(WD_PLATFORM_32BIT)
  wdInt64 old;
  do
  {
    old = dest;
  } while (_InterlockedCompareExchange64(&dest, old - 1, old) != old);
  return old;
#else
  return _InterlockedExchangeAdd64(&ref_iDest, -1);
#endif
}

WD_ALWAYS_INLINE void wdAtomicUtils::Add(volatile wdInt32& ref_iDest, wdInt32 value)
{
  _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&ref_iDest), value);
}

WD_ALWAYS_INLINE void wdAtomicUtils::Add(volatile wdInt64& ref_iDest, wdInt64 value)
{
#if WD_ENABLED(WD_PLATFORM_32BIT)
  wdInt64 old;
  do
  {
    old = dest;
  } while (_InterlockedCompareExchange64(&dest, old + value, old) != old);
#else
  _InterlockedExchangeAdd64(&ref_iDest, value);
#endif
}


WD_ALWAYS_INLINE void wdAtomicUtils::And(volatile wdInt32& ref_iDest, wdInt32 value)
{
  _InterlockedAnd(reinterpret_cast<volatile long*>(&ref_iDest), value);
}

WD_ALWAYS_INLINE void wdAtomicUtils::And(volatile wdInt64& ref_iDest, wdInt64 value)
{
#if WD_ENABLED(WD_PLATFORM_32BIT)
  wdInt64 old;
  do
  {
    old = dest;
  } while (_InterlockedCompareExchange64(&dest, old & value, old) != old);
#else
  _InterlockedAnd64(&ref_iDest, value);
#endif
}


WD_ALWAYS_INLINE void wdAtomicUtils::Or(volatile wdInt32& ref_iDest, wdInt32 value)
{
  _InterlockedOr(reinterpret_cast<volatile long*>(&ref_iDest), value);
}

WD_ALWAYS_INLINE void wdAtomicUtils::Or(volatile wdInt64& ref_iDest, wdInt64 value)
{
#if WD_ENABLED(WD_PLATFORM_32BIT)
  wdInt64 old;
  do
  {
    old = dest;
  } while (_InterlockedCompareExchange64(&dest, old | value, old) != old);
#else
  _InterlockedOr64(&ref_iDest, value);
#endif
}


WD_ALWAYS_INLINE void wdAtomicUtils::Xor(volatile wdInt32& ref_iDest, wdInt32 value)
{
  _InterlockedXor(reinterpret_cast<volatile long*>(&ref_iDest), value);
}

WD_ALWAYS_INLINE void wdAtomicUtils::Xor(volatile wdInt64& ref_iDest, wdInt64 value)
{
#if WD_ENABLED(WD_PLATFORM_32BIT)
  wdInt64 old;
  do
  {
    old = dest;
  } while (_InterlockedCompareExchange64(&dest, old ^ value, old) != old);
#else
  _InterlockedXor64(&ref_iDest, value);
#endif
}


inline void wdAtomicUtils::Min(volatile wdInt32& ref_iDest, wdInt32 value)
{
  // tries to exchange dest with the new value as long as the oldValue is not what we expected
  while (true)
  {
    wdInt32 iOldValue = ref_iDest;
    wdInt32 iNewValue = value < iOldValue ? value : iOldValue; // do Min manually here, to break #include cycles

    if (_InterlockedCompareExchange(reinterpret_cast<volatile long*>(&ref_iDest), iNewValue, iOldValue) == iOldValue)
      break;
  }
}

inline void wdAtomicUtils::Min(volatile wdInt64& ref_iDest, wdInt64 value)
{
  // tries to exchange dest with the new value as long as the oldValue is not what we expected
  while (true)
  {
    wdInt64 iOldValue = ref_iDest;
    wdInt64 iNewValue = value < iOldValue ? value : iOldValue; // do Min manually here, to break #include cycles

    if (_InterlockedCompareExchange64(&ref_iDest, iNewValue, iOldValue) == iOldValue)
      break;
  }
}

inline void wdAtomicUtils::Max(volatile wdInt32& ref_iDest, wdInt32 value)
{
  // tries to exchange dest with the new value as long as the oldValue is not what we expected
  while (true)
  {
    wdInt32 iOldValue = ref_iDest;
    wdInt32 iNewValue = iOldValue < value ? value : iOldValue; // do Max manually here, to break #include cycles

    if (_InterlockedCompareExchange(reinterpret_cast<volatile long*>(&ref_iDest), iNewValue, iOldValue) == iOldValue)
      break;
  }
}

inline void wdAtomicUtils::Max(volatile wdInt64& ref_iDest, wdInt64 value)
{
  // tries to exchange dest with the new value as long as the oldValue is not what we expected
  while (true)
  {
    wdInt64 iOldValue = ref_iDest;
    wdInt64 iNewValue = iOldValue < value ? value : iOldValue; // do Max manually here, to break #include cycles

    if (_InterlockedCompareExchange64(&ref_iDest, iNewValue, iOldValue) == iOldValue)
      break;
  }
}


inline wdInt32 wdAtomicUtils::Set(volatile wdInt32& ref_iDest, wdInt32 value)
{
  return _InterlockedExchange(reinterpret_cast<volatile long*>(&ref_iDest), value);
}

WD_ALWAYS_INLINE wdInt64 wdAtomicUtils::Set(volatile wdInt64& ref_iDest, wdInt64 value)
{
#if WD_ENABLED(WD_PLATFORM_32BIT)
  wdInt64 old;
  do
  {
    old = dest;
  } while (_InterlockedCompareExchange64(&dest, value, old) != old);
  return old;
#else
  return _InterlockedExchange64(&ref_iDest, value);
#endif
}


WD_ALWAYS_INLINE bool wdAtomicUtils::TestAndSet(volatile wdInt32& ref_iDest, wdInt32 iExpected, wdInt32 value)
{
  return _InterlockedCompareExchange(reinterpret_cast<volatile long*>(&ref_iDest), value, iExpected) == iExpected;
}

WD_ALWAYS_INLINE bool wdAtomicUtils::TestAndSet(volatile wdInt64& ref_iDest, wdInt64 iExpected, wdInt64 value)
{
  return _InterlockedCompareExchange64(&ref_iDest, value, iExpected) == iExpected;
}

WD_ALWAYS_INLINE bool wdAtomicUtils::TestAndSet(void** volatile pDest, void* pExpected, void* value)
{
  return _InterlockedCompareExchangePointer(pDest, value, pExpected) == pExpected;
}

WD_ALWAYS_INLINE wdInt32 wdAtomicUtils::CompareAndSwap(volatile wdInt32& ref_iDest, wdInt32 iExpected, wdInt32 value)
{
  return _InterlockedCompareExchange(reinterpret_cast<volatile long*>(&ref_iDest), value, iExpected);
}

WD_ALWAYS_INLINE wdInt64 wdAtomicUtils::CompareAndSwap(volatile wdInt64& ref_iDest, wdInt64 iExpected, wdInt64 value)
{
  return _InterlockedCompareExchange64(&ref_iDest, value, iExpected);
}
