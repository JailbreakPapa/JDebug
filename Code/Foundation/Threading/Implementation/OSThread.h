#pragma once

#include <Foundation/Basics.h>

#include <Foundation/Threading/Implementation/ThreadingDeclarations.h>

/// \brief Implementation of a thread.
///
/// Since the thread class needs a platform specific entry-point it is usually
/// recommended to use the wdThread class instead as the base for long running threads.
class WD_FOUNDATION_DLL wdOSThread
{
public:
  /// \brief Initializes the thread instance (e.g. thread creation etc.)
  ///
  /// Note that the thread won't start execution until Start() is called. Please note that szName must be valid until Start() has been
  /// called!
  wdOSThread(wdOSThreadEntryPoint threadEntryPoint, void* pUserData = nullptr, const char* szName = "wdOSThread", wdUInt32 uiStackSize = 128 * 1024);

  /// \brief Destructor.
  virtual ~wdOSThread();

  /// \brief Starts the thread
  void Start(); // [tested]

  /// \brief Waits in the calling thread until the thread has finished execution (e.g. returned from the thread function)
  void Join(); // [tested]

  /// \brief Returns the thread ID of the thread object, may be used in comparison operations with wdThreadUtils::GetCurrentThreadID() for
  /// example.
  const wdThreadID& GetThreadID() const { return m_ThreadID; }

  /// \brief Returns how many wdOSThreads are currently active.
  static wdInt32 GetThreadCount() { return s_iThreadCount; }

protected:
  wdThreadHandle m_hHandle;
  wdThreadID m_ThreadID;

  wdOSThreadEntryPoint m_EntryPoint;

  void* m_pUserData;

  const char* m_szName;

  wdUInt32 m_uiStackSize;


private:
  /// Stores how many wdOSThread are currently active.
  static wdAtomicInteger32 s_iThreadCount;

  WD_DISALLOW_COPY_AND_ASSIGN(wdOSThread);
};
