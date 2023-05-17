#pragma once

#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)

#  include <Foundation/Basics.h>
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#  include <Foundation/Communication/IpcChannel.h>

struct IOContext
{
  OVERLAPPED Overlapped;  ///< Must be first field in class so we can do a reinterpret cast from *Overlapped to *IOContext.
  wdIpcChannel* pChannel; ///< Owner of this IOContext.
};

class WD_FOUNDATION_DLL wdPipeChannel_win : public wdIpcChannel
{
public:
  wdPipeChannel_win(const char* szAddress, Mode::Enum mode);
  ~wdPipeChannel_win();

private:
  friend class wdMessageLoop;
  friend class wdMessageLoop_win;

  bool CreatePipe(const char* szAddress);

  virtual void AddToMessageLoop(wdMessageLoop* pMsgLoop) override;

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
    explicit State(wdPipeChannel_win* pChannel);
    ~State();
    IOContext Context;
    wdAtomicInteger32 IsPending = false; ///< Whether an async operation is in process.
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
  wdUInt8 m_InputBuffer[BUFFER_SIZE];
};

#endif
