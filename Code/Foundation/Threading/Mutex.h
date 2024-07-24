#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Threading/Implementation/ThreadingDeclarations.h>

/// \brief Provides a simple mechanism for mutual exclusion to prevent multiple threads from accessing a shared resource simultaneously.
///
/// This can be used to protect code that is not thread-safe against race conditions.
/// To ensure that mutexes are always properly released, use the nsLock class or NS_LOCK macro.
///
/// \sa nsSemaphore, nsConditionVariable
class NS_FOUNDATION_DLL nsMutex
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsMutex);

public:
  nsMutex();
  ~nsMutex();

  /// \brief Acquires an exclusive lock for this mutex object
  void Lock();

  /// \brief Attempts to acquire an exclusive lock for this mutex object. Returns true on success.
  ///
  /// If the mutex is already acquired by another thread, the function returns immediately and returns false.
  nsResult TryLock();

  /// \brief Releases a lock that has been previously acquired
  void Unlock();

  /// \brief Returns true, if the mutex is currently acquired. Can be used to assert that a lock was entered.
  ///
  /// Obviously, this check is not thread-safe and should not be used to check whether a mutex could be locked without blocking.
  /// Use TryLock for that instead.
  NS_ALWAYS_INLINE bool IsLocked() const { return m_iLockCount > 0; }

  nsMutexHandle& GetMutexHandle() { return m_hHandle; }

private:
  nsMutexHandle m_hHandle;
  nsInt32 m_iLockCount = 0;
};

/// \brief A dummy mutex that does no locking.
///
/// Used when a mutex object needs to be passed to some code (such as allocators), but thread-synchronization
/// is actually not necessary.
class NS_FOUNDATION_DLL nsNoMutex
{
public:
  /// \brief Implements the 'Acquire' interface function, but does nothing.
  NS_ALWAYS_INLINE void Lock() {}

  /// \brief Implements the 'TryLock' interface function, but does nothing.
  NS_ALWAYS_INLINE nsResult TryLock() { return NS_SUCCESS; }

  /// \brief Implements the 'Release' interface function, but does nothing.
  NS_ALWAYS_INLINE void Unlock() {}

  NS_ALWAYS_INLINE bool IsLocked() const { return false; }
};

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
#  include <Foundation/Threading/Implementation/Win/Mutex_win.h>
#elif NS_ENABLED(NS_PLATFORM_OSX) || NS_ENABLED(NS_PLATFORM_LINUX) || NS_ENABLED(NS_PLATFORM_ANDROID)
#  include <Foundation/Threading/Implementation/Posix/Mutex_posix.h>
#else
#  error "Mutex is not implemented on current platform"
#endif
