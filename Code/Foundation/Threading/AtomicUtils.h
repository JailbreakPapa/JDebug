#pragma once

#include <Foundation/Basics.h>

/// \brief This class provides functions to do atomic operations.
///
/// Atomic operations are generally faster than mutexes, and should therefore be preferred whenever possible.
/// However only the operations in themselves are atomic, once you execute several of them in sequence,
/// the sequence will not be atomic.
/// Also atomic operations are a lot slower than non-atomic operations, thus you should not use them in code that
/// does not need to be thread-safe.
/// wdAtomicInteger is built on top of wdAtomicUtils and provides a more convenient interface to use atomic
/// integer instructions.
struct WD_FOUNDATION_DLL wdAtomicUtils
{
  /// \brief Returns src as an atomic operation and returns its value.
  static wdInt32 Read(volatile const wdInt32& iSrc); // [tested]

  /// \brief Returns src as an atomic operation and returns its value.
  static wdInt64 Read(volatile const wdInt64& iSrc); // [tested]

  /// \brief Increments dest as an atomic operation and returns the new value.
  static wdInt32 Increment(volatile wdInt32& ref_iDest); // [tested]

  /// \brief Increments dest as an atomic operation and returns the new value.
  static wdInt64 Increment(volatile wdInt64& ref_iDest); // [tested]

  /// \brief Decrements dest as an atomic operation and returns the new value.
  static wdInt32 Decrement(volatile wdInt32& ref_iDest); // [tested]

  /// \brief Decrements dest as an atomic operation and returns the new value.
  static wdInt64 Decrement(volatile wdInt64& ref_iDest); // [tested]

  /// \brief Increments dest as an atomic operation and returns the old value.
  static wdInt32 PostIncrement(volatile wdInt32& ref_iDest); // [tested]

  /// \brief Increments dest as an atomic operation and returns the old value.
  static wdInt64 PostIncrement(volatile wdInt64& ref_iDest); // [tested]

  /// \brief Decrements dest as an atomic operation and returns the old value.
  static wdInt32 PostDecrement(volatile wdInt32& ref_iDest); // [tested]

  /// \brief Decrements dest as an atomic operation and returns the old value.
  static wdInt64 PostDecrement(volatile wdInt64& ref_iDest); // [tested]

  /// \brief Adds value to dest as an atomic operation.
  static void Add(volatile wdInt32& ref_iDest, wdInt32 value); // [tested]

  /// \brief Adds value to dest as an atomic operation.
  static void Add(volatile wdInt64& ref_iDest, wdInt64 value); // [tested]

  /// \brief Performs an atomic bitwise AND on dest using value.
  static void And(volatile wdInt32& ref_iDest, wdInt32 value); // [tested]

  /// \brief Performs an atomic bitwise AND on dest using value.
  static void And(volatile wdInt64& ref_iDest, wdInt64 value); // [tested]

  /// \brief Performs an atomic bitwise OR on dest using value.
  static void Or(volatile wdInt32& ref_iDest, wdInt32 value); // [tested]

  /// \brief Performs an atomic bitwise OR on dest using value.
  static void Or(volatile wdInt64& ref_iDest, wdInt64 value); // [tested]

  /// \brief Performs an atomic bitwise XOR on dest using value.
  static void Xor(volatile wdInt32& ref_iDest, wdInt32 value); // [tested]

  /// \brief Performs an atomic bitwise XOR on dest using value.
  static void Xor(volatile wdInt64& ref_iDest, wdInt64 value); // [tested]

  /// \brief Performs an atomic min operation on dest using value.
  static void Min(volatile wdInt32& ref_iDest, wdInt32 value); // [tested]

  /// \brief Performs an atomic min operation on dest using value.
  static void Min(volatile wdInt64& ref_iDest, wdInt64 value); // [tested]

  /// \brief Performs an atomic max operation on dest using value.
  static void Max(volatile wdInt32& ref_iDest, wdInt32 value); // [tested]

  /// \brief Performs an atomic max operation on dest using value.
  static void Max(volatile wdInt64& ref_iDest, wdInt64 value); // [tested]

  /// \brief Sets dest to value as an atomic operation and returns the original value of dest.
  static wdInt32 Set(volatile wdInt32& ref_iDest, wdInt32 value); // [tested]

  /// \brief Sets dest to value as an atomic operation and returns the original value of dest.
  static wdInt64 Set(volatile wdInt64& ref_iDest, wdInt64 value); // [tested]

  /// \brief If *dest* is equal to *expected*, this function sets *dest* to *value* and returns true. Otherwise *dest* will not be modified and the
  /// function returns false.
  static bool TestAndSet(volatile wdInt32& ref_iDest, wdInt32 iExpected, wdInt32 value); // [tested]

  /// \brief If *dest* is equal to *expected*, this function sets *dest* to *value* and returns true. Otherwise *dest* will not be modified and the
  /// function returns false.
  static bool TestAndSet(volatile wdInt64& ref_iDest, wdInt64 iExpected, wdInt64 value); // [tested]

  /// \brief If *dest* is equal to *expected*, this function sets *dest* to *value* and returns true. Otherwise *dest* will not be modified and the
  /// function returns false.
  static bool TestAndSet(void** volatile pDest, void* pExpected, void* value); // [tested]

  /// \brief If *dest* is equal to *expected*, this function sets *dest* to *value*. Otherwise *dest* will not be modified. Always returns the value
  /// of *dest* before the modification.
  static wdInt32 CompareAndSwap(volatile wdInt32& ref_iDest, wdInt32 iExpected, wdInt32 value); // [tested]

  /// \brief If *dest* is equal to *expected*, this function sets *dest* to *value*. Otherwise *dest* will not be modified. Always returns the value
  /// of *dest* before the modification.
  static wdInt64 CompareAndSwap(volatile wdInt64& ref_iDest, wdInt64 iExpected, wdInt64 value); // [tested]
};

// Include inline file
#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Threading/Implementation/Win/AtomicUtils_win.h>
#elif WD_ENABLED(WD_PLATFORM_OSX) || WD_ENABLED(WD_PLATFORM_LINUX) || WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <Foundation/Threading/Implementation/Posix/AtomicUtils_posix.h>
#else
#  error "Atomics are not implemented on current platform"
#endif
