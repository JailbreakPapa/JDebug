#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Containers/Deque.h>
#include <Foundation/Threading/Mutex.h>
#include <Foundation/Threading/Thread.h>

class nsProcessMessage;
class nsIpcChannel;
class nsLoopThread;

/// \brief Internal sub-system used by nsIpcChannel.
///
/// This sub-system creates a background thread as soon as the first nsIpcChannel
/// is added to it. This class should never be needed to be accessed outside
/// of nsIpcChannel implementations.
class NS_FOUNDATION_DLL nsMessageLoop
{
  NS_DECLARE_SINGLETON(nsMessageLoop);

public:
  nsMessageLoop();
  virtual ~nsMessageLoop() = default;
  ;

  /// \brief Needs to be called by newly created channels' constructors.
  void AddChannel(nsIpcChannel* pChannel);

  void RemoveChannel(nsIpcChannel* pChannel);

protected:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(Foundation, MessageLoop);
  friend class nsLoopThread;
  friend class nsIpcChannel;

  void StartUpdateThread();
  void StopUpdateThread();
  void RunLoop();
  bool ProcessTasks();
  void Quit();

  /// \brief Wake up the message loop when new work comes in.
  virtual void WakeUp() = 0;
  /// \brief Waits until a new message has been processed (sent, received).
  /// \param timeout If negative, wait indefinitely.
  /// \param pFilter If not null, wait for a message for the specific channel.
  /// \return Returns whether a message was received or the timeout was reached.
  virtual bool WaitForMessages(nsInt32 iTimeout, nsIpcChannel* pFilter) = 0;

  nsThreadID m_ThreadId = 0;
  mutable nsMutex m_Mutex;
  bool m_bShouldQuit = false;
  bool m_bCallTickFunction = false;
  class nsLoopThread* m_pUpdateThread = nullptr;

  nsMutex m_TasksMutex;
  nsDynamicArray<nsIpcChannel*> m_ConnectQueue;
  nsDynamicArray<nsIpcChannel*> m_DisconnectQueue;
  nsDynamicArray<nsIpcChannel*> m_SendQueue;

  // Thread local copies of the different queues for the ProcessTasks method
  nsDynamicArray<nsIpcChannel*> m_ConnectQueueTask;
  nsDynamicArray<nsIpcChannel*> m_DisconnectQueueTask;
  nsDynamicArray<nsIpcChannel*> m_SendQueueTask;

  nsDynamicArray<nsIpcChannel*> m_AllAddedChannels;
};
