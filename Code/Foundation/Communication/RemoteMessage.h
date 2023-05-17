#pragma once

#include <Foundation/Basics.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Reflection/Reflection.h>

/// \todo Add move semantics for wdRemoteMessage

/// \brief Encapsulates all the data that is transmitted when sending or receiving a message with wdRemoteInterface
class WD_FOUNDATION_DLL wdRemoteMessage
{
public:
  wdRemoteMessage();
  wdRemoteMessage(wdUInt32 uiSystemID, wdUInt32 uiMessageID);
  wdRemoteMessage(const wdRemoteMessage& rhs);
  ~wdRemoteMessage();
  void operator=(const wdRemoteMessage& rhs);

  /// \name Sending
  ///@{

  /// \brief For setting the message IDs before sending it
  WD_ALWAYS_INLINE void SetMessageID(wdUInt32 uiSystemID, wdUInt32 uiMessageID)
  {
    m_uiSystemID = uiSystemID;
    m_uiMsgID = uiMessageID;
  }

  /// \brief Returns a stream writer to append data to the message
  WD_ALWAYS_INLINE wdStreamWriter& GetWriter() { return m_Writer; }


  ///@}

  /// \name Receiving
  ///@{

  /// \brief Returns a stream reader for reading the message data
  WD_ALWAYS_INLINE wdStreamReader& GetReader() { return m_Reader; }
  WD_ALWAYS_INLINE wdUInt32 GetApplicationID() const { return m_uiApplicationID; }
  WD_ALWAYS_INLINE wdUInt32 GetSystemID() const { return m_uiSystemID; }
  WD_ALWAYS_INLINE wdUInt32 GetMessageID() const { return m_uiMsgID; }
  WD_ALWAYS_INLINE wdArrayPtr<const wdUInt8> GetMessageData() const
  {
    return {m_Storage.GetData(), m_Storage.GetStorageSize32()};
  }

  ///@}

private:
  friend class wdRemoteInterface;

  wdUInt32 m_uiApplicationID = 0;
  wdUInt32 m_uiSystemID = 0;
  wdUInt32 m_uiMsgID = 0;

  wdContiguousMemoryStreamStorage m_Storage;
  wdMemoryStreamReader m_Reader;
  wdMemoryStreamWriter m_Writer;
};

/// \brief Base class for IPC messages transmitted by wdIpcChannel.
class WD_FOUNDATION_DLL wdProcessMessage : public wdReflectedClass
{
  WD_ADD_DYNAMIC_REFLECTION(wdProcessMessage, wdReflectedClass);

public:
  wdProcessMessage() {}
};
