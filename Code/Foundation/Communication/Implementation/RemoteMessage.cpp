#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/RemoteMessage.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsProcessMessage, 1, nsRTTIDefaultAllocator<nsProcessMessage>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsRemoteMessage::nsRemoteMessage()
  : m_Reader(&m_Storage)
  , m_Writer(&m_Storage)
{
}

nsRemoteMessage::nsRemoteMessage(const nsRemoteMessage& rhs)
  : m_Storage(rhs.m_Storage)
  , m_Reader(&m_Storage)
  , m_Writer(&m_Storage)
{
  m_uiSystemID = rhs.m_uiSystemID;
  m_uiMsgID = rhs.m_uiMsgID;
}


nsRemoteMessage::nsRemoteMessage(nsUInt32 uiSystemID, nsUInt32 uiMessageID)
  : m_Reader(&m_Storage)
  , m_Writer(&m_Storage)
{
  m_uiSystemID = uiSystemID;
  m_uiMsgID = uiMessageID;
}

void nsRemoteMessage::operator=(const nsRemoteMessage& rhs)
{
  m_Storage = rhs.m_Storage;
  m_uiApplicationID = rhs.m_uiApplicationID;
  m_uiSystemID = rhs.m_uiSystemID;
  m_uiMsgID = rhs.m_uiMsgID;
  m_Reader.SetStorage(&m_Storage);
  m_Writer.SetStorage(&m_Storage);
}

nsRemoteMessage::~nsRemoteMessage()
{
  m_Reader.SetStorage(nullptr);
  m_Writer.SetStorage(nullptr);
}


NS_STATICLINK_FILE(Foundation, Foundation_Communication_Implementation_RemoteMessage);
