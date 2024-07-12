#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Threading/Implementation/ThreadingDeclarations.h>

/// \brief Implementation of a thread.
///
/// Since the thread class needs a platform specific entry-point it is usually
/// recommended to use the nsThread class instead as the base for long running threads.
class NS_FOUNDATION_DLL nsOSThread
{
public:
  /// \brief Initializes the thread instance (e.g. thread creation etc.)
  ///
  /// Note that the thread won't start execution until Start() is called. Please note that szName must be valid until Start() has been
  /// called!
  nsOSThread(nsOSThreadEntryPoint threadEntryPoint, void* pUserData = nullptr, nsStringView sName = "nsOSThread", nsUInt32 uiStackSize = 128 * 1024);

  /// \brief Destructor.
  virtual ~nsOSThread();

  /// \brief Starts the thread
  void Start(); // [tested]

  /// \brief Waits in the calling thread until the thread has finished execution (e.g. returned from the thread function)
  void Join(); // [tested]

  /// \brief Returns the thread ID of the thread object, may be used in comparison operations with nsThreadUtils::GetCurrentThreadID() for
  /// example.
  const nsThreadID& GetThreadID() const { return m_ThreadID; }

  /// \brief Returns how many nsOSThreads are currently active.
  static nsInt32 GetThreadCount() { return s_iThreadCount; }

protected:
  nsThreadHandle m_hHandle;
  nsThreadID m_ThreadID;

  nsOSThreadEntryPoint m_EntryPoint;

  void* m_pUserData;

  nsString m_sName;

  nsUInt32 m_uiStackSize;


private:
  /// Stores how many nsOSThread are currently active.
  static nsAtomicInteger32 s_iThreadCount;

  NS_DISALLOW_COPY_AND_ASSIGN(nsOSThread);
};
