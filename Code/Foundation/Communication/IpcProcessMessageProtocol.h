#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Communication/RemoteInterface.h>
#include <Foundation/Communication/RemoteMessage.h>
#include <Foundation/Types/UniquePtr.h>

class nsIpcChannel;
class nsMessageLoop;


/// \brief A protocol around nsIpcChannel to send reflected messages instead of byte array messages between client and server.
///
/// This wrapper class hooks into an existing nsIpcChannel. The nsIpcChannel is still responsible for all connection logic. This class merely provides a high-level messaging protocol via reflected messages derived from nsProcessMessage.
/// Note that if this class is used, nsIpcChannel::Send must not be called manually anymore, only use nsIpcProcessMessageProtocol::Send.
/// Received messages are stored in a queue and must be flushed via calling ProcessMessages or WaitForMessages.
class NS_FOUNDATION_DLL nsIpcProcessMessageProtocol
{
public:
  nsIpcProcessMessageProtocol(nsIpcChannel* pChannel);
  ~nsIpcProcessMessageProtocol();

  /// \brief Sends a message. pMsg can be destroyed after the call.
  bool Send(nsProcessMessage* pMsg);


  /// \brief Processes all pending messages by broadcasting m_MessageEvent. Not re-entrant.
  bool ProcessMessages();
  /// \brief Block and wait for new messages and call ProcessMessages.
  nsResult WaitForMessages(nsTime timeout = nsTime::MakeZero());

public:
  nsEvent<const nsProcessMessage*> m_MessageEvent; ///< Will be sent from thread calling ProcessMessages or WaitForMessages.

private:
  void EnqueueMessage(nsUniquePtr<nsProcessMessage>&& msg);
  void SwapWorkQueue(nsDeque<nsUniquePtr<nsProcessMessage>>& messages);
  void ReceiveMessageData(nsArrayPtr<const nsUInt8> data);

private:
  nsIpcChannel* m_pChannel = nullptr;

  nsMutex m_IncomingQueueMutex;
  nsDeque<nsUniquePtr<nsProcessMessage>> m_IncomingQueue;
};
