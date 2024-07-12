#pragma once

#include <Foundation/IO/MemoryStream.h>

class NS_FOUNDATION_DLL nsTelemetryMessage
{
public:
  nsTelemetryMessage();
  nsTelemetryMessage(const nsTelemetryMessage& rhs);
  ~nsTelemetryMessage();

  void operator=(const nsTelemetryMessage& rhs);

  NS_ALWAYS_INLINE nsStreamReader& GetReader() { return m_Reader; }
  NS_ALWAYS_INLINE nsStreamWriter& GetWriter() { return m_Writer; }

  NS_ALWAYS_INLINE nsUInt32 GetSystemID() const { return m_uiSystemID; }
  NS_ALWAYS_INLINE nsUInt32 GetMessageID() const { return m_uiMsgID; }

  NS_ALWAYS_INLINE void SetMessageID(nsUInt32 uiSystemID, nsUInt32 uiMessageID)
  {
    m_uiSystemID = uiSystemID;
    m_uiMsgID = uiMessageID;
  }

  // nsUInt64 GetMessageSize() const { return m_Storage.GetStorageSize64(); }

private:
  friend class nsTelemetry;

  nsUInt32 m_uiSystemID;
  nsUInt32 m_uiMsgID;

  nsContiguousMemoryStreamStorage m_Storage;
  nsMemoryStreamReader m_Reader;
  nsMemoryStreamWriter m_Writer;
};
