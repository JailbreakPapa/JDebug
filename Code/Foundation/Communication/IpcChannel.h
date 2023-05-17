#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Communication/RemoteInterface.h>
#include <Foundation/Communication/RemoteMessage.h>
#include <Foundation/Threading/ThreadSignal.h>
#include <Foundation/Types/UniquePtr.h>

class wdIpcChannel;
class wdMessageLoop;

/// \brief Event data for wdIpcChannel::m_Events
struct WD_FOUNDATION_DLL wdIpcChannelEvent
{
  enum Type
  {
    ConnectedToClient,      ///< brief Sent whenever a new connection to a client has been established.
    ConnectedToServer,      ///< brief Sent whenever a connection to the server has been established.
    DisconnectedFromClient, ///< Sent every time the connection to a client is dropped
    DisconnectedFromServer, ///< Sent when the connection to the server has been lost
    NewMessages,            ///< Sent when a new message has been received.
  };

  wdIpcChannelEvent(Type type, wdIpcChannel* pChannel)
    : m_Type(type)
    , m_pChannel(pChannel)
  {
  }

  Type m_Type;
  wdIpcChannel* m_pChannel;
};

/// \brief Base class for a communication channel between processes.
///
///  Use wdIpcChannel:::CreatePipeChannel to create an IPC pipe instance.
class WD_FOUNDATION_DLL wdIpcChannel
{
public:
  struct Mode
  {
    enum Enum
    {
      Server,
      Client
    };
  };
  virtual ~wdIpcChannel();
  /// \brief Creates an IPC communication channel using pipes.
  /// \param szAddress Name of the pipe, must be unique on a system and less than 200 characters.
  /// \param mode Whether to run in client or server mode.
  static wdIpcChannel* CreatePipeChannel(const char* szAddress, Mode::Enum mode);

  static wdIpcChannel* CreateNetworkChannel(const char* szAddress, Mode::Enum mode);

  /// \brief Connects async. On success, m_Events will be broadcasted.
  void Connect();
  /// \brief Disconnect async. On completion, m_Events will be broadcasted.
  void Disconnect();
  /// \brief Returns whether we have a connection.
  bool IsConnected() const { return m_bConnected; }

  /// \brief Sends a message. pMsg can be destroyed after the call.
  bool Send(wdProcessMessage* pMsg);

  /// \brief Processes all pending messages by broadcasting m_MessageEvent. Not re-entrant.
  bool ProcessMessages();
  /// \brief Block and wait for new messages and call ProcessMessages.
  void WaitForMessages();
  /// \brief Block and wait for new messages and call ProcessMessages.
  wdResult WaitForMessages(wdTime timeout);

  wdEvent<const wdIpcChannelEvent&, wdMutex> m_Events; ///< Will be sent from any thread.
  wdEvent<const wdProcessMessage*> m_MessageEvent; ///< Will be sent from thread calling ProcessMessages or WaitForMessages.

protected:
  wdIpcChannel(const char* szAddress, Mode::Enum mode);

  /// \brief Called by AddChannel to do platform specific registration.
  virtual void AddToMessageLoop(wdMessageLoop* pMsgLoop) {}

  /// \brief Override this and return true, if the surrounding infrastructure should call the 'Tick()' function.
  virtual bool RequiresRegularTick() { return false; }
  /// \brief Can implement regular updates, e.g. for polling network state.
  virtual void Tick() {}

  /// \brief Called on worker thread after Connect was called.
  virtual void InternalConnect() = 0;
  /// \brief Called on worker thread after Disconnect was called.
  virtual void InternalDisconnect() = 0;
  /// \brief Called on worker thread to sent pending messages.
  virtual void InternalSend() = 0;
  /// \brief Called by Send to determine whether the message loop need to be woken up.
  virtual bool NeedWakeup() const = 0;

  /// \brief Implementation needs to call this when new data has been received.
  ///  data can be invalidated after the function.
  void ReceiveMessageData(wdArrayPtr<const wdUInt8> data);
  void FlushPendingOperations();

private:
  void EnqueueMessage(wdUniquePtr<wdProcessMessage>&& msg);
  void SwapWorkQueue(wdDeque<wdUniquePtr<wdProcessMessage>>& messages);

protected:
  enum Constants : wdUInt32
  {
    HEADER_SIZE = 8,                     ///< Magic value and size wdUint32
    MAGIC_VALUE = 'USED',                ///< Magic value
    MAX_MESSAGE_SIZE = 1024 * 1024 * 16, ///< Arbitrary message size limit
  };

  friend class wdMessageLoop;
  wdThreadID m_ThreadId = 0;
  wdAtomicBool m_bConnected = false;

  // Setup in ctor
  const Mode::Enum m_Mode;
  wdMessageLoop* m_pOwner = nullptr;

  // Mutex locked
  wdMutex m_OutputQueueMutex;
  wdDeque<wdContiguousMemoryStreamStorage> m_OutputQueue;

  // Only accessed from worker thread
  wdDynamicArray<wdUInt8> m_MessageAccumulator; ///< Message is assembled in here

  // Mutex locked
  wdMutex m_IncomingQueueMutex;
  wdDeque<wdUniquePtr<wdProcessMessage>> m_IncomingQueue;
  wdThreadSignal m_IncomingMessages;
};
