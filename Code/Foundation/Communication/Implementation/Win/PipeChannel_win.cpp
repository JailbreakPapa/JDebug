#include <Foundation/FoundationPCH.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)

#  include <Foundation/Communication/Implementation/MessageLoop.h>
#  include <Foundation/Communication/Implementation/Win/MessageLoop_win.h>
#  include <Foundation/Communication/Implementation/Win/PipeChannel_win.h>
#  include <Foundation/Communication/RemoteMessage.h>
#  include <Foundation/Logging/Log.h>
#  include <Foundation/Serialization/ReflectionSerializer.h>

wdPipeChannel_win::State::State(wdPipeChannel_win* pChannel)
  : IsPending(false)
{
  memset(&Context.Overlapped, 0, sizeof(Context.Overlapped));
  Context.pChannel = pChannel;
  IsPending = false;
}

wdPipeChannel_win::State::~State() {}

wdPipeChannel_win::wdPipeChannel_win(const char* szAddress, Mode::Enum mode)
  : wdIpcChannel(szAddress, mode)
  , m_InputState(this)
  , m_OutputState(this)
{
  CreatePipe(szAddress);
  m_pOwner->AddChannel(this);
}

wdPipeChannel_win::~wdPipeChannel_win()
{
  if (m_hPipeHandle != INVALID_HANDLE_VALUE)
  {
    Disconnect();
  }
  while (m_bConnected)
  {
    wdThreadUtils::Sleep(wdTime::Milliseconds(10));
  }

  m_pOwner->RemoveChannel(this);
}

bool wdPipeChannel_win::CreatePipe(const char* szAddress)
{
  wdStringBuilder sPipename("\\\\.\\pipe\\", szAddress);

  if (m_Mode == Mode::Server)
  {
    SECURITY_ATTRIBUTES attributes = {0};
    attributes.nLength = sizeof(attributes);
    attributes.lpSecurityDescriptor = NULL;
    attributes.bInheritHandle = FALSE;

    m_hPipeHandle = CreateNamedPipeW(wdStringWChar(sPipename).GetData(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED | FILE_FLAG_FIRST_PIPE_INSTANCE,
      PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, 1, BUFFER_SIZE, BUFFER_SIZE, 5000, &attributes);
  }
  else
  {
    m_hPipeHandle = CreateFileW(wdStringWChar(sPipename).GetData(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
      SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION | FILE_FLAG_OVERLAPPED, NULL);
  }

  if (m_hPipeHandle == INVALID_HANDLE_VALUE)
  {
    wdLog::Error("Could not create named pipe: {0}", wdArgErrorCode(GetLastError()));
    return false;
  }

  return true;
}

void wdPipeChannel_win::AddToMessageLoop(wdMessageLoop* pMsgLoop)
{
  if (m_hPipeHandle != INVALID_HANDLE_VALUE)
  {
    wdMessageLoop_win* pMsgLoopWin = static_cast<wdMessageLoop_win*>(pMsgLoop);

    ULONG_PTR key = reinterpret_cast<ULONG_PTR>(this);
    HANDLE port = CreateIoCompletionPort(m_hPipeHandle, pMsgLoopWin->GetPort(), key, 1);
    WD_ASSERT_DEBUG(pMsgLoopWin->GetPort() == port, "Failed to CreateIoCompletionPort: {0}", wdArgErrorCode(GetLastError()));
  }
}

void wdPipeChannel_win::InternalConnect()
{
  if (m_hPipeHandle == INVALID_HANDLE_VALUE)
    return;
  if (m_bConnected)
    return;
#  if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  if (m_ThreadId == 0)
    m_ThreadId = wdThreadUtils::GetCurrentThreadID();
#  endif

  if (m_Mode == Mode::Server)
  {
    ProcessConnection();
  }
  else
  {
    m_bConnected = true;
  }

  if (!m_InputState.IsPending)
  {
    OnIOCompleted(&m_InputState.Context, 0, 0);
  }

  if (m_bConnected)
  {
    ProcessOutgoingMessages(0);
    m_Events.Broadcast(wdIpcChannelEvent(m_Mode == Mode::Client ? wdIpcChannelEvent::ConnectedToServer : wdIpcChannelEvent::ConnectedToClient, this));
  }

  return;
}

void wdPipeChannel_win::InternalDisconnect()
{
#  if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  if (m_ThreadId != 0)
    WD_ASSERT_DEBUG(m_ThreadId == wdThreadUtils::GetCurrentThreadID(), "Function must be called from worker thread!");
#  endif
  if (m_InputState.IsPending || m_OutputState.IsPending)
  {
    CancelIo(m_hPipeHandle);
  }

  if (m_hPipeHandle != INVALID_HANDLE_VALUE)
  {
    CloseHandle(m_hPipeHandle);
    m_hPipeHandle = INVALID_HANDLE_VALUE;
  }

  while (m_InputState.IsPending || m_OutputState.IsPending)
  {
    FlushPendingOperations();
  }

  {
    WD_LOCK(m_OutputQueueMutex);
    m_OutputQueue.Clear();
    m_bConnected = false;
  }

  m_Events.Broadcast(
    wdIpcChannelEvent(m_Mode == Mode::Client ? wdIpcChannelEvent::DisconnectedFromServer : wdIpcChannelEvent::DisconnectedFromClient, this));
  // Raise in case another thread is waiting for new messages (as we would sleep forever otherwise).
  m_IncomingMessages.RaiseSignal();
}

void wdPipeChannel_win::InternalSend()
{
  if (!m_OutputState.IsPending && m_bConnected)
  {
    ProcessOutgoingMessages(0);
  }
}


bool wdPipeChannel_win::NeedWakeup() const
{
  return m_OutputState.IsPending == 0;
}

bool wdPipeChannel_win::ProcessConnection()
{
  WD_ASSERT_DEBUG(m_ThreadId == wdThreadUtils::GetCurrentThreadID(), "Function must be called from worker thread!");
  if (m_InputState.IsPending)
    m_InputState.IsPending = false;

  BOOL res = ConnectNamedPipe(m_hPipeHandle, &m_InputState.Context.Overlapped);
  if (res)
  {
    // WD_REPORT_FAILURE
    return false;
  }

  wdUInt32 error = GetLastError();
  switch (error)
  {
    case ERROR_IO_PENDING:
      m_InputState.IsPending = true;
      break;
    case ERROR_PIPE_CONNECTED:
      m_bConnected = true;
      m_Events.Broadcast(wdIpcChannelEvent(m_Mode == Mode::Client ? wdIpcChannelEvent::ConnectedToServer : wdIpcChannelEvent::ConnectedToClient, this));
      break;
    case ERROR_NO_DATA:
      return false;
    default:
      wdLog::Error("Could not connect to pipe (Error code: {0})", wdArgErrorCode(error));
      return false;
  }

  return true;
}

bool wdPipeChannel_win::ProcessIncomingMessages(DWORD uiBytesRead)
{
  WD_ASSERT_DEBUG(m_ThreadId == wdThreadUtils::GetCurrentThreadID(), "Function must be called from worker thread!");
  if (m_InputState.IsPending)
  {
    m_InputState.IsPending = false;
    if (uiBytesRead == 0)
      return false;
  }

  while (true)
  {
    if (uiBytesRead == 0)
    {
      if (m_hPipeHandle == INVALID_HANDLE_VALUE)
        return false;

      BOOL res = ReadFile(m_hPipeHandle, m_InputBuffer, BUFFER_SIZE, &uiBytesRead, &m_InputState.Context.Overlapped);

      if (!res)
      {
        wdUInt32 error = GetLastError();
        if (error == ERROR_IO_PENDING)
        {
          m_InputState.IsPending = true;
          return true;
        }
        if (m_Mode == Mode::Server)
        {
          // only log when in server mode, otherwise this can result in an endless recursion
          wdLog::Error("Read from pipe failed: {0}", wdArgErrorCode(error));
        }
        return false;
      }
      m_InputState.IsPending = true;
      return true;
    }

    WD_ASSERT_DEBUG(uiBytesRead != 0, "We really should have data at this point.");
    ReceiveMessageData(wdArrayPtr<wdUInt8>(m_InputBuffer, uiBytesRead));
    uiBytesRead = 0;
  }
  return true;
}

bool wdPipeChannel_win::ProcessOutgoingMessages(DWORD uiBytesWritten)
{
  WD_ASSERT_DEBUG(m_bConnected, "Must be connected to process outgoing messages.");
  WD_ASSERT_DEBUG(m_ThreadId == wdThreadUtils::GetCurrentThreadID(), "Function must be called from worker thread!");

  if (m_OutputState.IsPending)
  {
    if (uiBytesWritten == 0)
    {
      // Don't reset isPending right away as we want to use it to decide
      // whether we need to wake up the worker thread again.
      wdLog::Error("pipe error: {0}", wdArgErrorCode(GetLastError()));
      m_OutputState.IsPending = false;
      return false;
    }

    WD_LOCK(m_OutputQueueMutex);
    // message was send
    m_OutputQueue.PopFront();
  }

  if (m_hPipeHandle == INVALID_HANDLE_VALUE)
  {
    m_OutputState.IsPending = false;
    return false;
  }
  const wdMemoryStreamStorageInterface* storage = nullptr;
  {
    WD_LOCK(m_OutputQueueMutex);
    if (m_OutputQueue.IsEmpty())
    {
      m_OutputState.IsPending = false;
      return true;
    }
    storage = &m_OutputQueue.PeekFront();
  }

  wdUInt64 uiToWrite = storage->GetStorageSize64();
  wdUInt64 uiNextOffset = 0;
  while (uiToWrite > 0)
  {
    const wdArrayPtr<const wdUInt8> range = storage->GetContiguousMemoryRange(uiNextOffset);
    uiToWrite -= range.GetCount();

    BOOL res = WriteFile(m_hPipeHandle, range.GetPtr(), range.GetCount(), &uiBytesWritten, &m_OutputState.Context.Overlapped);

    if (!res)
    {
      wdUInt32 error = GetLastError();
      if (error == ERROR_IO_PENDING)
      {
        m_OutputState.IsPending = true;
        return true;
      }
      wdLog::Error("Write to pipe failed: {0}", wdArgErrorCode(error));
      return false;
    }

    uiNextOffset += range.GetCount();
  }


  m_OutputState.IsPending = true;
  return true;
}

void wdPipeChannel_win::OnIOCompleted(IOContext* pContext, DWORD uiBytesTransfered, DWORD uiError)
{
  WD_ASSERT_DEBUG(m_ThreadId == wdThreadUtils::GetCurrentThreadID(), "Function must be called from worker thread!");
  bool bRes = true;
  if (pContext == &m_InputState.Context)
  {
    if (!m_bConnected)
    {
      if (!ProcessConnection())
        return;

      bool bHasOutput = false;
      {
        WD_LOCK(m_OutputQueueMutex);
        bHasOutput = !m_OutputQueue.IsEmpty();
      }

      if (bHasOutput && m_OutputState.IsPending == 0)
        ProcessOutgoingMessages(0);
      if (m_InputState.IsPending)
        return;
    }
    bRes = ProcessIncomingMessages(uiBytesTransfered);
  }
  else
  {
    WD_ASSERT_DEBUG(pContext == &m_OutputState.Context, "");
    bRes = ProcessOutgoingMessages(uiBytesTransfered);
  }
  if (!bRes && m_hPipeHandle != INVALID_HANDLE_VALUE)
  {
    InternalDisconnect();
  }
}
#endif

WD_STATICLINK_FILE(Foundation, Foundation_Communication_Implementation_Win_PipeChannel_win);
