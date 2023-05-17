#ifdef WD_ATOMICUTLS_POSIX_INL_H_INCLUDED
#  error "This file must not be included twice."
#endif

#define WD_ATOMICUTLS_POSIX_INL_H_INCLUDED


#include <Foundation/Math/Math.h>

WD_ALWAYS_INLINE wdInt32 wdAtomicUtils::Read(volatile const wdInt32& src)
{
  return __sync_fetch_and_or(const_cast<volatile wdInt32*>(&src), 0);
}

WD_ALWAYS_INLINE wdInt64 wdAtomicUtils::Read(volatile const wdInt64& src)
{
  return __sync_fetch_and_or_8(const_cast<volatile wdInt64*>(&src), 0);
}

WD_ALWAYS_INLINE wdInt32 wdAtomicUtils::Increment(volatile wdInt32& dest)
{
  return __sync_add_and_fetch(&dest, 1);
}

WD_ALWAYS_INLINE wdInt64 wdAtomicUtils::Increment(volatile wdInt64& dest)
{
  return __sync_add_and_fetch_8(&dest, 1);
}


WD_ALWAYS_INLINE wdInt32 wdAtomicUtils::Decrement(volatile wdInt32& dest)
{
  return __sync_sub_and_fetch(&dest, 1);
}

WD_ALWAYS_INLINE wdInt64 wdAtomicUtils::Decrement(volatile wdInt64& dest)
{
  return __sync_sub_and_fetch_8(&dest, 1);
}

WD_ALWAYS_INLINE wdInt32 wdAtomicUtils::PostIncrement(volatile wdInt32& dest)
{
  return __sync_fetch_and_add(&dest, 1);
}

WD_ALWAYS_INLINE wdInt64 wdAtomicUtils::PostIncrement(volatile wdInt64& dest)
{
  return __sync_fetch_and_add_8(&dest, 1);
}


WD_ALWAYS_INLINE wdInt32 wdAtomicUtils::PostDecrement(volatile wdInt32& dest)
{
  return __sync_fetch_and_sub(&dest, 1);
}

WD_ALWAYS_INLINE wdInt64 wdAtomicUtils::PostDecrement(volatile wdInt64& dest)
{
  return __sync_fetch_and_sub_8(&dest, 1);
}

WD_ALWAYS_INLINE void wdAtomicUtils::Add(volatile wdInt32& dest, wdInt32 value)
{
  __sync_fetch_and_add(&dest, value);
}

WD_ALWAYS_INLINE void wdAtomicUtils::Add(volatile wdInt64& dest, wdInt64 value)
{
  __sync_fetch_and_add_8(&dest, value);
}


WD_ALWAYS_INLINE void wdAtomicUtils::And(volatile wdInt32& dest, wdInt32 value)
{
  __sync_fetch_and_and(&dest, value);
}

WD_ALWAYS_INLINE void wdAtomicUtils::And(volatile wdInt64& dest, wdInt64 value)
{
  __sync_fetch_and_and_8(&dest, value);
}


WD_ALWAYS_INLINE void wdAtomicUtils::Or(volatile wdInt32& dest, wdInt32 value)
{
  __sync_fetch_and_or(&dest, value);
}

WD_ALWAYS_INLINE void wdAtomicUtils::Or(volatile wdInt64& dest, wdInt64 value)
{
  __sync_fetch_and_or_8(&dest, value);
}


WD_ALWAYS_INLINE void wdAtomicUtils::Xor(volatile wdInt32& dest, wdInt32 value)
{
  __sync_fetch_and_xor(&dest, value);
}

WD_ALWAYS_INLINE void wdAtomicUtils::Xor(volatile wdInt64& dest, wdInt64 value)
{
  __sync_fetch_and_xor_8(&dest, value);
}


WD_FORCE_INLINE void wdAtomicUtils::Min(volatile wdInt32& dest, wdInt32 value)
{
  // tries to exchange dest with the new value as long as the oldValue is not what we expected
  while (true)
  {
    wdInt32 iOldValue = dest;
    wdInt32 iNewValue = wdMath::Min(iOldValue, value);

    if (__sync_bool_compare_and_swap(&dest, iOldValue, iNewValue))
      break;
  }
}

WD_FORCE_INLINE void wdAtomicUtils::Min(volatile wdInt64& dest, wdInt64 value)
{
  // tries to exchange dest with the new value as long as the oldValue is not what we expected
  while (true)
  {
    wdInt64 iOldValue = dest;
    wdInt64 iNewValue = wdMath::Min(iOldValue, value);

    if (__sync_bool_compare_and_swap_8(&dest, iOldValue, iNewValue))
      break;
  }
}


WD_FORCE_INLINE void wdAtomicUtils::Max(volatile wdInt32& dest, wdInt32 value)
{
  // tries to exchange dest with the new value as long as the oldValue is not what we expected
  while (true)
  {
    wdInt32 iOldValue = dest;
    wdInt32 iNewValue = wdMath::Max(iOldValue, value);

    if (__sync_bool_compare_and_swap(&dest, iOldValue, iNewValue))
      break;
  }
}

WD_FORCE_INLINE void wdAtomicUtils::Max(volatile wdInt64& dest, wdInt64 value)
{
  // tries to exchange dest with the new value as long as the oldValue is not what we expected
  while (true)
  {
    wdInt64 iOldValue = dest;
    wdInt64 iNewValue = wdMath::Max(iOldValue, value);

    if (__sync_bool_compare_and_swap_8(&dest, iOldValue, iNewValue))
      break;
  }
}


WD_ALWAYS_INLINE wdInt32 wdAtomicUtils::Set(volatile wdInt32& dest, wdInt32 value)
{
  return __sync_lock_test_and_set(&dest, value);
}

WD_ALWAYS_INLINE wdInt64 wdAtomicUtils::Set(volatile wdInt64& dest, wdInt64 value)
{
  return __sync_lock_test_and_set_8(&dest, value);
}


WD_ALWAYS_INLINE bool wdAtomicUtils::TestAndSet(volatile wdInt32& dest, wdInt32 expected, wdInt32 value)
{
  return __sync_bool_compare_and_swap(&dest, expected, value);
}

WD_ALWAYS_INLINE bool wdAtomicUtils::TestAndSet(volatile wdInt64& dest, wdInt64 expected, wdInt64 value)
{
  return __sync_bool_compare_and_swap_8(&dest, expected, value);
}

WD_ALWAYS_INLINE bool wdAtomicUtils::TestAndSet(void** volatile dest, void* expected, void* value)
{
#if WD_ENABLED(WD_PLATFORM_64BIT)
  wdUInt64* puiTemp = reinterpret_cast<wdUInt64*>(dest);
  return __sync_bool_compare_and_swap(puiTemp, reinterpret_cast<wdUInt64>(expected), reinterpret_cast<wdUInt64>(value));
#else
  wdUInt32* puiTemp = reinterpret_cast<wdUInt32*>(dest);
  return __sync_bool_compare_and_swap(puiTemp, reinterpret_cast<wdUInt32>(expected), reinterpret_cast<wdUInt32>(value));
#endif
}

WD_ALWAYS_INLINE wdInt32 wdAtomicUtils::CompareAndSwap(volatile wdInt32& dest, wdInt32 expected, wdInt32 value)
{
  return __sync_val_compare_and_swap(&dest, expected, value);
}

WD_ALWAYS_INLINE wdInt64 wdAtomicUtils::CompareAndSwap(volatile wdInt64& dest, wdInt64 expected, wdInt64 value)
{
  return __sync_val_compare_and_swap_8(&dest, expected, value);
}
