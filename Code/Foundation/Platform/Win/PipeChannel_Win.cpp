#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)

#  include <Foundation/Communication/Implementation/MessageLoop.h>
#  include <Foundation/Communication/RemoteMessage.h>
#  include <Foundation/Logging/Log.h>
#  include <Foundation/Platform/Win/MessageLoop_Win.h>
#  include <Foundation/Platform/Win/PipeChannel_Win.h>
#  include <Foundation/Serialization/ReflectionSerializer.h>

nsPipeChannel_win::State::State(nsPipeChannel_win* pChannel)
  : IsPending(false)
{
  memset(&Context.Overlapped, 0, sizeof(Context.Overlapped));
  Context.pChannel = pChannel;
  IsPending = false;
}

nsPipeChannel_win::State::~State() = default;

nsPipeChannel_win::nsPipeChannel_win(nsStringView sAddress, Mode::Enum mode)
  : nsIpcChannel(sAddress, mode)
  , m_InputState(this)
  , m_OutputState(this)
{
  m_pOwner->AddChannel(this);
}

nsPipeChannel_win::~nsPipeChannel_win()
{
  if (m_hPipeHandle != INVALID_HANDLE_VALUE)
  {
    Disconnect();
  }
  while (IsConnected())
  {
    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));
  }
  m_pOwner->RemoveChannel(this);
}

bool nsPipeChannel_win::CreatePipe(nsStringView sAddress)
{
  nsStringBuilder sPipename("\\\\.\\pipe\\", sAddress);

  if (m_Mode == Mode::Server)
  {
    SECURITY_ATTRIBUTES attributes = {0};
    attributes.nLength = sizeof(attributes);
    attributes.lpSecurityDescriptor = NULL;
    attributes.bInheritHandle = FALSE;

    m_hPipeHandle = CreateNamedPipeW(nsStringWChar(sPipename).GetData(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED | FILE_FLAG_FIRST_PIPE_INSTANCE,
      PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, 1, BUFFER_SIZE, BUFFER_SIZE, 5000, &attributes);
  }
  else
  {
    m_hPipeHandle = CreateFileW(nsStringWChar(sPipename).GetData(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
      SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION | FILE_FLAG_OVERLAPPED, NULL);
  }

  if (m_hPipeHandle == INVALID_HANDLE_VALUE)
  {
    nsLog::Error("Could not create named pipe: {0}", nsArgErrorCode(GetLastError()));
    return false;
  }

  if (m_hPipeHandle != INVALID_HANDLE_VALUE)
  {
    nsMessageLoop_win* pMsgLoopWin = static_cast<nsMessageLoop_win*>(m_pOwner);

    ULONG_PTR key = reinterpret_cast<ULONG_PTR>(this);
    HANDLE port = CreateIoCompletionPort(m_hPipeHandle, pMsgLoopWin->GetPort(), key, 1);
    NS_IGNORE_UNUSED(port);
    NS_ASSERT_DEBUG(pMsgLoopWin->GetPort() == port, "Failed to CreateIoCompletionPort: {0}", nsArgErrorCode(GetLastError()));
  }
  return true;
}

void nsPipeChannel_win::InternalConnect()
{
  if (GetConnectionState() != ConnectionState::Disconnected)
    return;

#  if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  if (m_ThreadId == 0)
    m_ThreadId = nsThreadUtils::GetCurrentThreadID();
#  endif

  if (!CreatePipe(m_sAddress))
    return;

  if (m_hPipeHandle == INVALID_HANDLE_VALUE)
    return;

  SetConnectionState(ConnectionState::Connecting);
  if (m_Mode == Mode::Server)
  {
    ProcessConnection();
  }
  else
  {
    // If CreatePipe succeeded, we are already connected.
    SetConnectionState(ConnectionState::Connected);
  }

  if (!m_InputState.IsPending)
  {
    OnIOCompleted(&m_InputState.Context, 0, 0);
  }

  if (IsConnected())
  {
    ProcessOutgoingMessages(0);
  }

  return;
}

void nsPipeChannel_win::InternalDisconnect()
{
  if (GetConnectionState() == ConnectionState::Disconnected)
    return;

#  if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  if (m_ThreadId != 0)
    NS_ASSERT_DEBUG(m_ThreadId == nsThreadUtils::GetCurrentThreadID(), "Function must be called from worker thread!");
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

  bool bWasConnected = false;
  {
    NS_LOCK(m_OutputQueueMutex);
    m_OutputQueue.Clear();
    bWasConnected = IsConnected();
  }

  if (bWasConnected)
  {
    SetConnectionState(ConnectionState::Disconnected);
    // Raise in case another thread is waiting for new messages (as we would sleep forever otherwise).
    m_IncomingMessages.RaiseSignal();
  }
}

void nsPipeChannel_win::InternalSend()
{
  if (!m_OutputState.IsPending && IsConnected())
  {
    ProcessOutgoingMessages(0);
  }
}


bool nsPipeChannel_win::NeedWakeup() const
{
  return m_OutputState.IsPending == 0;
}

bool nsPipeChannel_win::ProcessConnection()
{
  NS_ASSERT_DEBUG(m_ThreadId == nsThreadUtils::GetCurrentThreadID(), "Function must be called from worker thread!");
  if (m_InputState.IsPending)
    m_InputState.IsPending = false;

  BOOL res = ConnectNamedPipe(m_hPipeHandle, &m_InputState.Context.Overlapped);
  if (res)
  {
    // NS_REPORT_FAILURE
    return false;
  }

  nsUInt32 error = GetLastError();
  switch (error)
  {
    case ERROR_IO_PENDING:
      m_InputState.IsPending = true;
      break;
    case ERROR_PIPE_CONNECTED:
      SetConnectionState(ConnectionState::Connected);
      break;
    case ERROR_NO_DATA:
      return false;
    default:
      nsLog::Error("Could not connect to pipe (Error code: {0})", nsArgErrorCode(error));
      return false;
  }

  return true;
}

bool nsPipeChannel_win::ProcessIncomingMessages(DWORD uiBytesRead)
{
  NS_ASSERT_DEBUG(m_ThreadId == nsThreadUtils::GetCurrentThreadID(), "Function must be called from worker thread!");
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
        nsUInt32 error = GetLastError();
        if (error == ERROR_IO_PENDING)
        {
          m_InputState.IsPending = true;
          return true;
        }
        if (m_Mode == Mode::Server)
        {
          // only log when in server mode, otherwise this can result in an endless recursion
          nsLog::Error("Read from pipe failed: {0}", nsArgErrorCode(error));
        }
        return false;
      }
      m_InputState.IsPending = true;
      return true;
    }

    NS_ASSERT_DEBUG(uiBytesRead != 0, "We really should have data at this point.");
    ReceiveData(nsArrayPtr<nsUInt8>(m_InputBuffer, uiBytesRead));
    uiBytesRead = 0;
  }
  return true;
}

bool nsPipeChannel_win::ProcessOutgoingMessages(DWORD uiBytesWritten)
{
  NS_ASSERT_DEBUG(IsConnected(), "Must be connected to process outgoing messages.");
  NS_ASSERT_DEBUG(m_ThreadId == nsThreadUtils::GetCurrentThreadID(), "Function must be called from worker thread!");

  if (m_OutputState.IsPending)
  {
    if (uiBytesWritten == 0)
    {
      // Don't reset isPending right away as we want to use it to decide
      // whether we need to wake up the worker thread again.
      nsLog::Error("pipe error: {0}", nsArgErrorCode(GetLastError()));
      m_OutputState.IsPending = false;
      return false;
    }

    NS_LOCK(m_OutputQueueMutex);
    // message was send
    m_OutputQueue.PopFront();
  }

  if (m_hPipeHandle == INVALID_HANDLE_VALUE)
  {
    m_OutputState.IsPending = false;
    return false;
  }
  const nsMemoryStreamStorageInterface* storage = nullptr;
  {
    NS_LOCK(m_OutputQueueMutex);
    if (m_OutputQueue.IsEmpty())
    {
      m_OutputState.IsPending = false;
      return true;
    }
    storage = &m_OutputQueue.PeekFront();
  }

  nsUInt64 uiToWrite = storage->GetStorageSize64();
  nsUInt64 uiNextOffset = 0;
  while (uiToWrite > 0)
  {
    const nsArrayPtr<const nsUInt8> range = storage->GetContiguousMemoryRange(uiNextOffset);
    uiToWrite -= range.GetCount();

    BOOL res = WriteFile(m_hPipeHandle, range.GetPtr(), range.GetCount(), &uiBytesWritten, &m_OutputState.Context.Overlapped);

    if (!res)
    {
      nsUInt32 error = GetLastError();
      if (error == ERROR_IO_PENDING)
      {
        m_OutputState.IsPending = true;
        return true;
      }
      nsLog::Error("Write to pipe failed: {0}", nsArgErrorCode(error));
      return false;
    }

    uiNextOffset += range.GetCount();
  }


  m_OutputState.IsPending = true;
  return true;
}

void nsPipeChannel_win::OnIOCompleted(IOContext* pContext, DWORD uiBytesTransfered, DWORD uiError)
{
  NS_ASSERT_DEBUG(m_ThreadId == nsThreadUtils::GetCurrentThreadID(), "Function must be called from worker thread!");
  bool bRes = true;
  if (pContext == &m_InputState.Context)
  {
    if (!IsConnected())
    {
      if (!ProcessConnection())
        return;

      bool bHasOutput = false;
      {
        NS_LOCK(m_OutputQueueMutex);
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
    NS_ASSERT_DEBUG(pContext == &m_OutputState.Context, "");
    bRes = ProcessOutgoingMessages(uiBytesTransfered);
  }
  if (!bRes && m_hPipeHandle != INVALID_HANDLE_VALUE)
  {
    InternalDisconnect();
  }
}
#endif
