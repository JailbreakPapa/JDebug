#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Communication/Event.h>
#include <Foundation/Communication/RemoteMessage.h>
#include <Foundation/Containers/Deque.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Threading/Mutex.h>
#include <Foundation/Threading/Thread.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Types/Delegate.h>

/// \brief Whether the remote interface is configured as a server or a client
enum class wdRemoteMode
{
  None,   ///< Remote interface is shut down
  Server, ///< Remote interface acts as a server. Can connect with multiple clients
  Client  ///< Remote interface acts as a client. Can connect with exactly one server.
};

/// \brief Mode for transmitting messages
///
/// Depending on the remote interface implementation, Unreliable may not be supported and revert to Reliable.
enum class wdRemoteTransmitMode
{
  Reliable,   ///< Messages should definitely arrive at the target, if necessary they are send several times, until the target acknowledged it.
  Unreliable, ///< Messages are sent at most once, if they get lost, they are not resent. If it is known beforehand, that not receiver exists, they
              ///< are dropped without sending them at all.
};

/// \brief Event type for connections
struct WD_FOUNDATION_DLL wdRemoteEvent
{
  enum Type
  {
    ConnectedToClient,      ///< brief Sent whenever a new connection to a client has been established.
    ConnectedToServer,      ///< brief Sent whenever a connection to the server has been established.
    DisconnectedFromClient, ///< Sent every time the connection to a client is dropped
    DisconnectedFromServer, ///< Sent when the connection to the server has been lost
  };

  Type m_Type;
  wdUInt32 m_uiOtherAppID;
};

typedef wdDelegate<void(wdRemoteMessage&)> wdRemoteMessageHandler;

struct WD_FOUNDATION_DLL wdRemoteMessageQueue
{
  wdRemoteMessageHandler m_MessageHandler;
  /// \brief Messages are pushed into this container on arrival.
  wdDeque<wdRemoteMessage> m_MessageQueueIn;
  /// \brief To flush the message queue, m_MessageQueueIn and m_MessageQueueOut are swapped.
  /// Thus new messages can arrive while we execute the event handler for each element
  /// in this container and then clear it.
  wdDeque<wdRemoteMessage> m_MessageQueueOut;
};

class WD_FOUNDATION_DLL wdRemoteInterface
{
public:
  virtual ~wdRemoteInterface();

  /// \brief Exposes the mutex that is internally used to secure multi-threaded access
  wdMutex& GetMutex() const { return m_Mutex; }


  /// \name Connection
  ///@{

  /// \brief Starts the remote interface as a server.
  ///
  /// \param uiConnectionToken Should be a unique sequence (e.g. 'WDPZ') to identify the purpose of this connection.
  /// Only server and clients with the same token will accept connections.
  /// \param uiPort The port over which the connection should run.
  /// \param bStartUpdateThread If true, a thread is started that will regularly call UpdateNetwork() and UpdatePingToServer().
  /// If false, this has to be called manually in regular intervals.
  wdResult StartServer(wdUInt32 uiConnectionToken, const char* szAddress, bool bStartUpdateThread = true);

  /// \brief Starts the network interface as a client. Tries to connect to the given address.
  ///
  /// This function immediately returns and no connection is guaranteed.
  /// \param uiConnectionToken Same as for StartServer()
  /// \param szAddress Could be a network address "127.0.0.1" or "localhost" or some other name that identifies the target, e.g. a named pipe.
  /// \param bStartUpdateThread Same as for StartServer()
  ///
  /// If this function succeeds, it still might not be connected to a server.
  /// Use WaitForConnectionToServer() to enforce a connection.
  wdResult ConnectToServer(wdUInt32 uiConnectionToken, const char* szAddress, bool bStartUpdateThread = true);

  /// \brief Can only be called after ConnectToServer(). Updates the network in a loop until a connection is established, or the time has run out.
  ///
  /// A timeout of exactly zero means to wait indefinitely.
  wdResult WaitForConnectionToServer(wdTime timeout = wdTime::Seconds(10));

  /// \brief Closes the connection in an orderly fashion
  void ShutdownConnection();

  /// \brief Whether the client is connected to a server
  bool IsConnectedToServer() const { return m_uiConnectedToServerWithID != 0; }

  /// \brief Whether the server is connected to any client
  bool IsConnectedToClients() const { return m_iConnectionsToClients > 0; }

  /// \brief Whether the client or server is connected its counterpart
  bool IsConnectedToOther() const { return IsConnectedToServer() || IsConnectedToClients(); }

  /// \brief Whether the remote interface is inactive, a client or a server
  wdRemoteMode GetRemoteMode() const { return m_RemoteMode; }

  /// \brief The address through which the connection was started
  const wdString& GetServerAddress() const { return m_sServerAddress; }

  /// \brief Returns the own (random) application ID used to identify this instance
  wdUInt32 GetApplicationID() const { return m_uiApplicationID; }

  /// \brief Returns the connection token used to identify compatible servers/clients
  wdUInt32 GetConnectionToken() const { return m_uiConnectionToken; }

  ///@}

  /// \name Server Information
  ///@{

  /// \brief For the client to display the name of the server
  // const wdString& GetServerInfoName() const { return m_ServerInfoName; }

  /// \brief For the client to display the IP of the server
  const wdString& GetServerInfoIP() const { return m_sServerInfoIP; }

  /// \brief Some random identifier, that allows to determine after a reconnect, whether the connected instance is still the same server
  wdUInt32 GetServerID() const { return m_uiConnectedToServerWithID; }

  /// \brief Returns the current ping to the server
  wdTime GetPingToServer() const { return m_PingToServer; }

  ///@}

  /// \name Updating the Remote Interface
  ///@{

  /// \brief If no update thread was spawned, this should be called to process messages
  void UpdateRemoteInterface();

  /// \brief If no update thread was spawned, this should be called by clients to determine the ping
  void UpdatePingToServer();

  ///@}

  /// \name Sending Messages
  ///@{

  /// \brief Sends a reliable message without any data.
  /// If it is a server, the message is broadcast to all clients.
  /// If it is a client, the message is only sent to the server.
  void Send(wdUInt32 uiSystemID, wdUInt32 uiMsgID);

  /// \brief Sends a message, appends the given array of data
  /// If it is a server, the message is broadcast to all clients.
  /// If it is a client, the message is only sent to the server.
  void Send(wdRemoteTransmitMode tm, wdUInt32 uiSystemID, wdUInt32 uiMsgID, const wdArrayPtr<const wdUInt8>& data);

  void Send(wdRemoteTransmitMode tm, wdUInt32 uiSystemID, wdUInt32 uiMsgID, const wdContiguousMemoryStreamStorage& data);

  /// \brief Sends a message, appends the given array of data
  /// If it is a server, the message is broadcast to all clients.
  /// If it is a client, the message is only sent to the server.
  void Send(wdRemoteTransmitMode tm, wdUInt32 uiSystemID, wdUInt32 uiMsgID, const void* pData = nullptr, wdUInt32 uiDataBytes = 0);

  /// \brief Sends an wdRemoteMessage
  /// If it is a server, the message is broadcast to all clients.
  /// If it is a client, the message is only sent to the server.
  void Send(wdRemoteTransmitMode tm, wdRemoteMessage& ref_msg);

  ///@}

  /// \name Message Handling
  ///@{

  /// \brief Registers a message handler that is executed for all incoming messages for the given system
  void SetMessageHandler(wdUInt32 uiSystemID, wdRemoteMessageHandler messageHandler);

  /// \brief Executes the message handler for all messages that have arrived for the given system
  wdUInt32 ExecuteMessageHandlers(wdUInt32 uiSystem);

  /// \brief Executes all message handlers for all received messages
  wdUInt32 ExecuteAllMessageHandlers();

  ///@}

  /// \name Events
  ///@{

  /// \brief Broadcasts events about connections
  wdEvent<const wdRemoteEvent&> m_RemoteEvents;

  ///@}

protected:
  /// \name Implementation Details
  ///@{

  /// \brief Derived classes have to implement this to start a network connection
  virtual wdResult InternalCreateConnection(wdRemoteMode mode, const char* szServerAddress) = 0;

  /// \brief Derived classes have to implement this to shutdown a network connection
  virtual void InternalShutdownConnection() = 0;

  /// \brief Derived classes have to implement this to update
  virtual void InternalUpdateRemoteInterface() = 0;

  /// \brief Derived classes have to implement this to get the ping to the server (client mode only)
  virtual wdTime InternalGetPingToServer() = 0;

  /// \brief Derived classes have to implement this to deliver messages to the server or client
  virtual wdResult InternalTransmit(wdRemoteTransmitMode tm, const wdArrayPtr<const wdUInt8>& data) = 0;

  /// \brief Derived classes can override this to interpret an address differently
  virtual wdResult DetermineTargetAddress(const char* szConnectTo, wdUInt32& out_IP, wdUInt16& out_Port);

  /// Derived classes should update this when the information is available
  // wdString m_ServerInfoName;
  /// Derived classes should update this when the information is available
  wdString m_sServerInfoIP;

  /// \brief Should be called by the implementation, when a server connection has been established
  void ReportConnectionToServer(wdUInt32 uiServerID);
  /// \brief Should be called by the implementation, when a client connection has been established
  void ReportConnectionToClient(wdUInt32 uiApplicationID);
  /// \brief Should be called by the implementation, when a server connection has been lost
  void ReportDisconnectedFromServer();
  /// \brief Should be called by the implementation, when a client connection has been lost
  void ReportDisconnectedFromClient(wdUInt32 uiApplicationID);
  /// \brief Should be called by the implementation, when a message has arrived
  void ReportMessage(wdUInt32 uiApplicationID, wdUInt32 uiSystemID, wdUInt32 uiMsgID, const wdArrayPtr<const wdUInt8>& data);

  ///@}


private:
  void StartUpdateThread();
  void StopUpdateThread();
  wdResult Transmit(wdRemoteTransmitMode tm, const wdArrayPtr<const wdUInt8>& data);
  wdResult CreateConnection(wdUInt32 uiConnectionToken, wdRemoteMode mode, const char* szServerAddress, bool bStartUpdateThread);
  wdUInt32 ExecuteMessageHandlersForQueue(wdRemoteMessageQueue& queue);

  mutable wdMutex m_Mutex;
  class wdRemoteThread* m_pUpdateThread = nullptr;
  wdRemoteMode m_RemoteMode = wdRemoteMode::None;
  wdString m_sServerAddress;
  wdTime m_PingToServer;
  wdUInt32 m_uiApplicationID = 0; // sent when connecting to identify the sending instance
  wdUInt32 m_uiConnectionToken = 0;
  wdUInt32 m_uiConnectedToServerWithID = 0;
  wdInt32 m_iConnectionsToClients = 0;
  wdDynamicArray<wdUInt8> m_TempSendBuffer;
  wdHashTable<wdUInt32, wdRemoteMessageQueue> m_MessageQueues;
};

/// \brief The remote interface thread updates in regular intervals to keep the connection alive.
///
/// The thread does NOT call wdRemoteInterface::ExecuteAllMessageHandlers(), so by default no message handlers are executed.
/// This has to be done manually by the application elsewhere.
class WD_FOUNDATION_DLL wdRemoteThread : public wdThread
{
public:
  wdRemoteThread();

  wdRemoteInterface* m_pRemoteInterface = nullptr;
  volatile bool m_bKeepRunning = true;

private:
  virtual wdUInt32 Run();
};
