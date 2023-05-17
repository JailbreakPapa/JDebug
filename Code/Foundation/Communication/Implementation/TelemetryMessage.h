#pragma once

#include <Foundation/IO/MemoryStream.h>

class WD_FOUNDATION_DLL wdTelemetryMessage
{
public:
  wdTelemetryMessage();
  wdTelemetryMessage(const wdTelemetryMessage& rhs);
  ~wdTelemetryMessage();

  void operator=(const wdTelemetryMessage& rhs);

  WD_ALWAYS_INLINE wdStreamReader& GetReader() { return m_Reader; }
  WD_ALWAYS_INLINE wdStreamWriter& GetWriter() { return m_Writer; }

  WD_ALWAYS_INLINE wdUInt32 GetSystemID() const { return m_uiSystemID; }
  WD_ALWAYS_INLINE wdUInt32 GetMessageID() const { return m_uiMsgID; }

  WD_ALWAYS_INLINE void SetMessageID(wdUInt32 uiSystemID, wdUInt32 uiMessageID)
  {
    m_uiSystemID = uiSystemID;
    m_uiMsgID = uiMessageID;
  }

  //wdUInt64 GetMessageSize() const { return m_Storage.GetStorageSize64(); }

private:
  friend class wdTelemetry;

  wdUInt32 m_uiSystemID;
  wdUInt32 m_uiMsgID;

  wdContiguousMemoryStreamStorage m_Storage;
  wdMemoryStreamReader m_Reader;
  wdMemoryStreamWriter m_Writer;
};
