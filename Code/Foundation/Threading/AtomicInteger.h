#pragma once

#include <Foundation/Types/TypeTraits.h>

#include <Foundation/Threading/AtomicUtils.h>

/// \brief Integer class that can be manipulated in an atomic (i.e. thread-safe) fashion.
template <typename T>
class wdAtomicInteger
{
public:
  WD_DECLARE_POD_TYPE();

  /// \brief Initializes the value to zero.
  wdAtomicInteger(); // [tested]

  /// \brief Initializes the object with a value
  wdAtomicInteger(const T value); // [tested]

  /// \brief Copy-constructor
  wdAtomicInteger(const wdAtomicInteger<T>& value); // [tested]

  /// \brief Assigns a new integer value to this object
  wdAtomicInteger& operator=(T value); // [tested]

  /// \brief Assignment operator
  wdAtomicInteger& operator=(const wdAtomicInteger& value); // [tested]

  /// \brief Increments the internal value and returns the incremented value
  T Increment(); // [tested]

  /// \brief Decrements the internal value and returns the decremented value
  T Decrement(); // [tested]

  /// \brief Increments the internal value and returns the value immediately before the increment
  T PostIncrement(); // [tested]

  /// \brief Decrements the internal value and returns the value immediately before the decrement
  T PostDecrement(); // [tested]

  void Add(T x);      // [tested]
  void Subtract(T x); // [tested]

  void And(T x); // [tested]
  void Or(T x);  // [tested]
  void Xor(T x); // [tested]

  void Min(T x); // [tested]
  void Max(T x); // [tested]

  /// \brief Sets the internal value to x and returns the original internal value.
  T Set(T x); // [tested]

  /// \brief Sets the internal value to x if the internal value is equal to expected and returns true, otherwise does nothing and returns false.
  bool TestAndSet(T expected, T x); // [tested]

  /// \brief If this is equal to *expected*, it is set to *value*. Otherwise it won't be modified. Always returns the previous value of this before
  /// the modification.
  T CompareAndSwap(T expected, T x); // [tested]

  operator T() const; // [tested]

private:
  volatile T m_value;
};

/// \brief An atomic boolean variable. This is just a wrapper around an atomic int32 for convenience.
class wdAtomicBool
{
public:
  /// \brief Initializes the bool to 'false'.
  wdAtomicBool(); // [tested]
  ~wdAtomicBool();

  /// \brief Initializes the object with a value
  wdAtomicBool(bool value); // [tested]

  /// \brief Copy-constructor
  wdAtomicBool(const wdAtomicBool& rhs);

  /// \brief Sets the bool to the given value and returns its previous value.
  bool Set(bool value); // [tested]

  /// \brief Sets the bool to the given value.
  void operator=(bool value); // [tested]

  /// \brief Sets the bool to the given value.
  void operator=(const wdAtomicBool& rhs);

  /// \brief Returns the current value.
  operator bool() const; // [tested]

  /// \brief Sets the internal value to \a newValue if the internal value is equal to \a expected and returns true, otherwise does nothing and returns
  /// false.
  bool TestAndSet(bool bExpected, bool bNewValue);

private:
  wdAtomicInteger<wdInt32> m_iAtomicInt;
};

// Include inline file
#include <Foundation/Threading/Implementation/AtomicInteger_inl.h>

using wdAtomicInteger32 = wdAtomicInteger<wdInt32>; // [tested]
using wdAtomicInteger64 = wdAtomicInteger<wdInt64>; // [tested]
