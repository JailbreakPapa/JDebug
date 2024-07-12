#pragma once

#include <Foundation/Basics.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Reflection/Reflection.h>

/// \todo Add move semantics for nsRemoteMessage

/// \brief Encapsulates all the data that is transmitted when sending or receiving a message with nsRemoteInterface
class NS_FOUNDATION_DLL nsRemoteMessage
{
public:
  nsRemoteMessage();
  nsRemoteMessage(nsUInt32 uiSystemID, nsUInt32 uiMessageID);
  nsRemoteMessage(const nsRemoteMessage& rhs);
  ~nsRemoteMessage();
  void operator=(const nsRemoteMessage& rhs);

  /// \name Sending
  ///@{

  /// \brief For setting the message IDs before sending it
  NS_ALWAYS_INLINE void SetMessageID(nsUInt32 uiSystemID, nsUInt32 uiMessageID)
  {
    m_uiSystemID = uiSystemID;
    m_uiMsgID = uiMessageID;
  }

  /// \brief Returns a stream writer to append data to the message
  NS_ALWAYS_INLINE nsStreamWriter& GetWriter() { return m_Writer; }


  ///@}

  /// \name Receiving
  ///@{

  /// \brief Returns a stream reader for reading the message data
  NS_ALWAYS_INLINE nsStreamReader& GetReader() { return m_Reader; }
  NS_ALWAYS_INLINE nsUInt32 GetApplicationID() const { return m_uiApplicationID; }
  NS_ALWAYS_INLINE nsUInt32 GetSystemID() const { return m_uiSystemID; }
  NS_ALWAYS_INLINE nsUInt32 GetMessageID() const { return m_uiMsgID; }
  NS_ALWAYS_INLINE nsArrayPtr<const nsUInt8> GetMessageData() const
  {
    return {m_Storage.GetData(), m_Storage.GetStorageSize32()};
  }

  ///@}

private:
  friend class nsRemoteInterface;

  nsUInt32 m_uiApplicationID = 0;
  nsUInt32 m_uiSystemID = 0;
  nsUInt32 m_uiMsgID = 0;

  nsContiguousMemoryStreamStorage m_Storage;
  nsMemoryStreamReader m_Reader;
  nsMemoryStreamWriter m_Writer;
};

/// \brief Base class for IPC messages transmitted by nsIpcChannel.
class NS_FOUNDATION_DLL nsProcessMessage : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsProcessMessage, nsReflectedClass);

public:
  nsProcessMessage() = default;
};
