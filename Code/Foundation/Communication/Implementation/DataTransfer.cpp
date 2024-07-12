#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/DataTransfer.h>

bool nsDataTransfer::s_bInitialized = false;
nsSet<nsDataTransfer*> nsDataTransfer::s_AllTransfers;

nsDataTransferObject::nsDataTransferObject(nsDataTransfer& ref_belongsTo, nsStringView sObjectName, nsStringView sMimeType, nsStringView sFileExtension)
  : m_BelongsTo(ref_belongsTo)
{
  m_bHasBeenTransferred = false;

  m_Msg.SetMessageID('TRAN', 'DATA');
  m_Msg.GetWriter() << ref_belongsTo.m_sDataName;
  m_Msg.GetWriter() << sObjectName;
  m_Msg.GetWriter() << sMimeType;
  m_Msg.GetWriter() << sFileExtension;
}

nsDataTransferObject::~nsDataTransferObject()
{
  NS_ASSERT_DEV(m_bHasBeenTransferred, "The data transfer object has never been transmitted.");
}

void nsDataTransferObject::Transmit()
{
  NS_ASSERT_DEV(!m_bHasBeenTransferred, "The data transfer object has been transmitted already.");

  if (m_bHasBeenTransferred)
    return;

  m_bHasBeenTransferred = true;

  m_BelongsTo.Transfer(*this);
}

nsDataTransfer::nsDataTransfer()
{
  m_bTransferRequested = false;
  m_bEnabled = false;
}

nsDataTransfer::~nsDataTransfer()
{
  DisableDataTransfer();
}

void nsDataTransfer::SendStatus()
{
  if (!nsTelemetry::IsConnectedToClient())
    return;

  nsTelemetryMessage msg;
  msg.GetWriter() << m_sDataName;

  if (m_bEnabled)
  {
    msg.SetMessageID('TRAN', 'ENBL');
  }
  else
  {
    msg.SetMessageID('TRAN', 'DSBL');
  }

  nsTelemetry::Broadcast(nsTelemetry::Reliable, msg);
}

void nsDataTransfer::DisableDataTransfer()
{
  if (!m_bEnabled)
    return;

  nsDataTransfer::s_AllTransfers.Remove(this);

  m_bEnabled = false;
  SendStatus();

  m_bTransferRequested = false;
  m_sDataName.Clear();
}

void nsDataTransfer::EnableDataTransfer(nsStringView sDataName)
{
  if (m_bEnabled && m_sDataName == sDataName)
    return;

  DisableDataTransfer();

  Initialize();

  nsDataTransfer::s_AllTransfers.Insert(this);

  m_sDataName = sDataName;

  NS_ASSERT_DEV(!m_sDataName.IsEmpty(), "The name for the data transfer must not be empty.");

  m_bEnabled = true;
  SendStatus();
}

void nsDataTransfer::RequestDataTransfer()
{
  if (!m_bEnabled)
  {
    m_bTransferRequested = false;
    return;
  }

  nsLog::Dev("Data Transfer Request: {0}", m_sDataName);

  m_bTransferRequested = true;

  OnTransferRequest();
}

bool nsDataTransfer::IsTransferRequested(bool bReset)
{
  const bool bRes = m_bTransferRequested;

  if (bReset)
    m_bTransferRequested = false;

  return bRes;
}

void nsDataTransfer::Transfer(nsDataTransferObject& Object)
{
  if (!m_bEnabled)
    return;

  nsTelemetry::Broadcast(nsTelemetry::Reliable, Object.m_Msg);
}

void nsDataTransfer::Initialize()
{
  if (s_bInitialized)
    return;

  s_bInitialized = true;

  nsTelemetry::AddEventHandler(TelemetryEventsHandler);
  nsTelemetry::AcceptMessagesForSystem('DTRA', true, TelemetryMessage, nullptr);
}

void nsDataTransfer::TelemetryMessage(void* pPassThrough)
{
  nsTelemetryMessage Msg;

  while (nsTelemetry::RetrieveMessage('DTRA', Msg) == NS_SUCCESS)
  {
    if (Msg.GetMessageID() == ' REQ')
    {
      nsStringBuilder sName;
      Msg.GetReader() >> sName;

      nsLog::Dev("Requested data transfer '{0}'", sName);

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

void nsDataTransfer::TelemetryEventsHandler(const nsTelemetry::TelemetryEventData& e)
{
  if (!nsTelemetry::IsConnectedToClient())
    return;

  switch (e.m_EventType)
  {
    case nsTelemetry::TelemetryEventData::ConnectedToClient:
      SendAllDataTransfers();
      break;

    default:
      break;
  }
}

void nsDataTransfer::SendAllDataTransfers()
{
  nsTelemetryMessage msg;
  msg.SetMessageID('TRAN', ' CLR');
  nsTelemetry::Broadcast(nsTelemetry::Reliable, msg);

  for (auto it = s_AllTransfers.GetIterator(); it.IsValid(); ++it)
  {
    it.Key()->SendStatus();
  }
}
