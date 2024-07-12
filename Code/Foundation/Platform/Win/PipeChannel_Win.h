#pragma once

#include <Foundation/FoundationInternal.h>
NS_FOUNDATION_INTERNAL_HEADER

#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)

#  include <Foundation/Basics.h>
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#  include <Foundation/Communication/IpcChannel.h>

struct IOContext
{
  OVERLAPPED Overlapped;  ///< Must be first field in class so we can do a reinterpret cast from *Overlapped to *IOContext.
  nsIpcChannel* pChannel; ///< Owner of this IOContext.
};

class NS_FOUNDATION_DLL nsPipeChannel_win : public nsIpcChannel
{
public:
  nsPipeChannel_win(nsStringView sAddress, Mode::Enum mode);
  ~nsPipeChannel_win();

private:
  friend class nsMessageLoop;
  friend class nsMessageLoop_win;

  bool CreatePipe(nsStringView sAddress);

  // All functions from here on down are run from worker thread only
  virtual void InternalConnect() override;
  virtual void InternalDisconnect() override;
  virtual void InternalSend() override;
  virtual bool NeedWakeup() const override;

  bool ProcessConnection();
  bool ProcessIncomingMessages(DWORD uiBytesRead);
  bool ProcessOutgoingMessages(DWORD uiBytesWritten);


protected:
  void OnIOCompleted(IOContext* pContext, DWORD uiBytesTransfered, DWORD uiError);

private:
  struct State
  {
    explicit State(nsPipeChannel_win* pChannel);
    ~State();
    IOContext Context;
    nsAtomicInteger32 IsPending = false; ///< Whether an async operation is in process.
  };

  enum Constants
  {
    BUFFER_SIZE = 4096,
  };

  // Shared data
  State m_InputState;
  State m_OutputState;

  // Setup in ctor
  HANDLE m_hPipeHandle = INVALID_HANDLE_VALUE;

  // Only accessed from worker thread
  nsUInt8 m_InputBuffer[BUFFER_SIZE];
};

#endif
