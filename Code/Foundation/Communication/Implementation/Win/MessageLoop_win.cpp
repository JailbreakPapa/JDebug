#include <Foundation/FoundationPCH.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)

#  include <Foundation/Communication/Implementation/Win/MessageLoop_win.h>
#  include <Foundation/Communication/Implementation/Win/PipeChannel_win.h>
#  include <Foundation/Communication/IpcChannel.h>

wdMessageLoop_win::wdMessageLoop_win()
{
  m_hPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
  WD_ASSERT_DEBUG(m_hPort != INVALID_HANDLE_VALUE, "Failed to create IO completion port!");
}

wdMessageLoop_win::~wdMessageLoop_win()
{
  StopUpdateThread();
  CloseHandle(m_hPort);
}


bool wdMessageLoop_win::WaitForMessages(wdInt32 iTimeout, wdIpcChannel* pFilter)
{
  if (iTimeout < 0)
    iTimeout = INFINITE;

  IOItem item;
  if (!MatchCompletedIOItem(pFilter, &item))
  {
    if (!GetIOItem(iTimeout, &item))
      return false;

    if (ProcessInternalIOItem(item))
      return true;
  }

  if (item.pContext->pChannel != NULL)
  {
    if (pFilter != NULL && item.pChannel != pFilter)
    {
      m_CompletedIO.PushBack(item);
    }
    else
    {
      WD_ASSERT_DEBUG(item.pContext->pChannel == item.pChannel, "");
      static_cast<wdPipeChannel_win*>(item.pChannel)->OnIOCompleted(item.pContext, item.uiBytesTransfered, item.uiError);
    }
  }
  return true;
}

bool wdMessageLoop_win::GetIOItem(wdInt32 iTimeout, IOItem* pItem)
{
  memset(pItem, 0, sizeof(*pItem));
  ULONG_PTR key = 0;
  OVERLAPPED* overlapped = NULL;
  if (!GetQueuedCompletionStatus(m_hPort, &pItem->uiBytesTransfered, &key, &overlapped, iTimeout))
  {
    // nothing queued
    if (overlapped == NULL)
      return false;

    pItem->uiError = GetLastError();
    pItem->uiBytesTransfered = 0;
  }

  pItem->pChannel = reinterpret_cast<wdIpcChannel*>(key);
  pItem->pContext = reinterpret_cast<IOContext*>(overlapped);
  return true;
}

bool wdMessageLoop_win::ProcessInternalIOItem(const IOItem& item)
{
  if (reinterpret_cast<wdMessageLoop_win*>(item.pContext) == this && reinterpret_cast<wdMessageLoop_win*>(item.pChannel) == this)
  {
    // internal notification
    WD_ASSERT_DEBUG(item.uiBytesTransfered == 0, "");
    InterlockedExchange(&m_iHaveWork, 0);
    return true;
  }
  return false;
}

bool wdMessageLoop_win::MatchCompletedIOItem(wdIpcChannel* pFilter, IOItem* pItem)
{
  for (wdUInt32 i = 0; i < m_CompletedIO.GetCount(); i++)
  {
    if (pFilter == NULL || m_CompletedIO[i].pChannel == pFilter)
    {
      *pItem = m_CompletedIO[i];
      m_CompletedIO.RemoveAtAndCopy(i);
      return true;
    }
  }
  return false;
}

void wdMessageLoop_win::WakeUp()
{
  if (InterlockedExchange(&m_iHaveWork, 1))
  {
    // already running
    return;
  }
  // wake up the loop
  BOOL res = PostQueuedCompletionStatus(m_hPort, 0, reinterpret_cast<ULONG_PTR>(this), reinterpret_cast<OVERLAPPED*>(this));
  WD_ASSERT_DEBUG(res, "Could not PostQueuedCompletionStatus: {0}", wdArgErrorCode(GetLastError()));
}

#endif

WD_STATICLINK_FILE(Foundation, Foundation_Communication_Implementation_Win_MessageLoop_win);
