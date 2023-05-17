#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Containers/Deque.h>
#include <Foundation/Threading/Mutex.h>
#include <Foundation/Threading/Thread.h>

class wdProcessMessage;
class wdIpcChannel;
class wdLoopThread;

/// \brief Internal sub-system used by wdIpcChannel.
///
/// This sub-system creates a background thread as soon as the first wdIpcChannel
/// is added to it. This class should never be needed to be accessed outside
/// of wdIpcChannel implementations.
class WD_FOUNDATION_DLL wdMessageLoop
{
  WD_DECLARE_SINGLETON(wdMessageLoop);

public:
  wdMessageLoop();
  virtual ~wdMessageLoop(){};

  /// \brief Needs to be called by newly created channels' constructors.
  void AddChannel(wdIpcChannel* pChannel);

  void RemoveChannel(wdIpcChannel* pChannel);

protected:
  WD_MAKE_SUBSYSTEM_STARTUP_FRIEND(Foundation, MessageLoop);
  friend class wdLoopThread;
  friend class wdIpcChannel;

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
  virtual bool WaitForMessages(wdInt32 iTimeout, wdIpcChannel* pFilter) = 0;

  wdThreadID m_ThreadId = 0;
  mutable wdMutex m_Mutex;
  bool m_bShouldQuit = false;
  bool m_bCallTickFunction = false;
  class wdLoopThread* m_pUpdateThread = nullptr;

  wdMutex m_TasksMutex;
  wdDynamicArray<wdIpcChannel*> m_ConnectQueue;
  wdDynamicArray<wdIpcChannel*> m_DisconnectQueue;
  wdDynamicArray<wdIpcChannel*> m_SendQueue;

  // Thread local copies of the different queues for the ProcessTasks method
  wdDynamicArray<wdIpcChannel*> m_ConnectQueueTask;
  wdDynamicArray<wdIpcChannel*> m_DisconnectQueueTask;
  wdDynamicArray<wdIpcChannel*> m_SendQueueTask;

  wdDynamicArray<wdIpcChannel*> m_AllAddedChannels;
};
