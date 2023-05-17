#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/DataTransfer.h>

bool wdDataTransfer::s_bInitialized = false;
wdSet<wdDataTransfer*> wdDataTransfer::s_AllTransfers;

wdDataTransferObject::wdDataTransferObject(wdDataTransfer& ref_belongsTo, const char* szObjectName, const char* szMimeType, const char* szFileExtension)
  : m_BelongsTo(ref_belongsTo)
{
  m_bHasBeenTransferred = false;

  m_Msg.SetMessageID('TRAN', 'DATA');
  m_Msg.GetWriter() << ref_belongsTo.m_sDataName;
  m_Msg.GetWriter() << szObjectName;
  m_Msg.GetWriter() << szMimeType;
  m_Msg.GetWriter() << szFileExtension;
}

wdDataTransferObject::~wdDataTransferObject()
{
  WD_ASSERT_DEV(m_bHasBeenTransferred, "The data transfer object has never been transmitted.");
}

void wdDataTransferObject::Transmit()
{
  WD_ASSERT_DEV(!m_bHasBeenTransferred, "The data transfer object has been transmitted already.");

  if (m_bHasBeenTransferred)
    return;

  m_bHasBeenTransferred = true;

  m_BelongsTo.Transfer(*this);
}

wdDataTransfer::wdDataTransfer()
{
  m_bTransferRequested = false;
  m_bEnabled = false;
}

wdDataTransfer::~wdDataTransfer()
{
  DisableDataTransfer();
}

void wdDataTransfer::SendStatus()
{
  if (!wdTelemetry::IsConnectedToClient())
    return;

  wdTelemetryMessage msg;
  msg.GetWriter() << m_sDataName;

  if (m_bEnabled)
  {
    msg.SetMessageID('TRAN', 'ENBL');
  }
  else
  {
    msg.SetMessageID('TRAN', 'DSBL');
  }

  wdTelemetry::Broadcast(wdTelemetry::Reliable, msg);
}

void wdDataTransfer::DisableDataTransfer()
{
  if (!m_bEnabled)
    return;

  wdDataTransfer::s_AllTransfers.Remove(this);

  m_bEnabled = false;
  SendStatus();

  m_bTransferRequested = false;
  m_sDataName.Clear();
}

void wdDataTransfer::EnableDataTransfer(const char* szDataName)
{
  if (m_bEnabled && m_sDataName == szDataName)
    return;

  DisableDataTransfer();

  Initialize();

  wdDataTransfer::s_AllTransfers.Insert(this);

  m_sDataName = szDataName;

  WD_ASSERT_DEV(!m_sDataName.IsEmpty(), "The name for the data transfer must not be empty.");

  m_bEnabled = true;
  SendStatus();
}

void wdDataTransfer::RequestDataTransfer()
{
  if (!m_bEnabled)
  {
    m_bTransferRequested = false;
    return;
  }

  wdLog::Dev("Data Transfer Request: {0}", m_sDataName);

  m_bTransferRequested = true;

  OnTransferRequest();
}

bool wdDataTransfer::IsTransferRequested(bool bReset)
{
  const bool bRes = m_bTransferRequested;

  if (bReset)
    m_bTransferRequested = false;

  return bRes;
}

void wdDataTransfer::Transfer(wdDataTransferObject& Object)
{
  if (!m_bEnabled)
    return;

  wdTelemetry::Broadcast(wdTelemetry::Reliable, Object.m_Msg);
}

void wdDataTransfer::Initialize()
{
  if (s_bInitialized)
    return;

  s_bInitialized = true;

  wdTelemetry::AddEventHandler(TelemetryEventsHandler);
  wdTelemetry::AcceptMessagesForSystem('DTRA', true, TelemetryMessage, nullptr);
}

void wdDataTransfer::TelemetryMessage(void* pPassThrough)
{
  wdTelemetryMessage Msg;

  while (wdTelemetry::RetrieveMessage('DTRA', Msg) == WD_SUCCESS)
  {
    if (Msg.GetMessageID() == 'REQ')
    {
      wdStringBuilder sName;
      Msg.GetReader() >> sName;

      wdLog::Dev("Requested data transfer '{0}'", sName);

      for (auto it = s_AllTransfers.GetIterator(); it.IsValid(); ++it)
      {
        if (it.Key()->m_sDataName == sName)
        {
          it.Key()->RequestDataTransfer();
          break;
        }
      }
    }
  }
}

void wdDataTransfer::TelemetryEventsHandler(const wdTelemetry::TelemetryEventData& e)
{
  if (!wdTelemetry::IsConnectedToClient())
    return;

  switch (e.m_EventType)
  {
    case wdTelemetry::TelemetryEventData::ConnectedToClient:
      SendAllDataTransfers();
      break;

    default:
      break;
  }
}

void wdDataTransfer::SendAllDataTransfers()
{
  wdTelemetryMessage msg;
  msg.SetMessageID('TRAN', ' CLR');
  wdTelemetry::Broadcast(wdTelemetry::Reliable, msg);

  for (auto it = s_AllTransfers.GetIterator(); it.IsValid(); ++it)
  {
    it.Key()->SendStatus();
  }
}



WD_STATICLINK_FILE(Foundation, Foundation_Communication_Implementation_DataTransfer);
