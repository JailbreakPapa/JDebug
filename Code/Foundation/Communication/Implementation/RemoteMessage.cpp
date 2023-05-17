#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/RemoteMessage.h>

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdProcessMessage, 1, wdRTTIDefaultAllocator<wdProcessMessage>)
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdRemoteMessage::wdRemoteMessage()
  : m_Reader(&m_Storage)
  , m_Writer(&m_Storage)
{
}

wdRemoteMessage::wdRemoteMessage(const wdRemoteMessage& rhs)
  : m_Storage(rhs.m_Storage)
  , m_Reader(&m_Storage)
  , m_Writer(&m_Storage)
{
  m_uiSystemID = rhs.m_uiSystemID;
  m_uiMsgID = rhs.m_uiMsgID;
}


wdRemoteMessage::wdRemoteMessage(wdUInt32 uiSystemID, wdUInt32 uiMessageID)
  : m_Reader(&m_Storage)
  , m_Writer(&m_Storage)
{
  m_uiSystemID = uiSystemID;
  m_uiMsgID = uiMessageID;
}

void wdRemoteMessage::operator=(const wdRemoteMessage& rhs)
{
  m_Storage = rhs.m_Storage;
  m_uiApplicationID = rhs.m_uiApplicationID;
  m_uiSystemID = rhs.m_uiSystemID;
  m_uiMsgID = rhs.m_uiMsgID;
  m_Reader.SetStorage(&m_Storage);
  m_Writer.SetStorage(&m_Storage);
}

wdRemoteMessage::~wdRemoteMessage()
{
  m_Reader.SetStorage(nullptr);
  m_Writer.SetStorage(nullptr);
}


WD_STATICLINK_FILE(Foundation, Foundation_Communication_Implementation_RemoteMessage);
