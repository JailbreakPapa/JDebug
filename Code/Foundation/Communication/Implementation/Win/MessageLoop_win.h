#pragma once

#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)

#  include <Foundation/Basics.h>
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#  include <Foundation/Communication/Implementation/MessageLoop.h>

class wdIpcChannel;
struct IOContext;

class WD_FOUNDATION_DLL wdMessageLoop_win : public wdMessageLoop
{
public:
  struct IOItem
  {
    WD_DECLARE_POD_TYPE();

    wdIpcChannel* pChannel;
    IOContext* pContext;
    DWORD uiBytesTransfered;
    DWORD uiError;
  };

public:
  wdMessageLoop_win();
  ~wdMessageLoop_win();

  HANDLE GetPort() const { return m_hPort; }

protected:
  virtual void WakeUp() override;
  virtual bool WaitForMessages(wdInt32 iTimeout, wdIpcChannel* pFilter) override;

  bool GetIOItem(wdInt32 iTimeout, IOItem* pItem);
  bool ProcessInternalIOItem(const IOItem& item);
  bool MatchCompletedIOItem(wdIpcChannel* pFilter, IOItem* pItem);

private:
  wdDynamicArray<IOItem> m_CompletedIO;
  LONG m_iHaveWork = 0;
  HANDLE m_hPort = INVALID_HANDLE_VALUE;
};

#endif
