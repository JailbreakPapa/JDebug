#pragma once

#include <Foundation/FoundationInternal.h>
NS_FOUNDATION_INTERNAL_HEADER

#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)

#  include <Foundation/Basics.h>
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#  include <Foundation/Communication/Implementation/MessageLoop.h>

class nsIpcChannel;
struct IOContext;

class NS_FOUNDATION_DLL nsMessageLoop_win : public nsMessageLoop
{
public:
  struct IOItem
  {
    NS_DECLARE_POD_TYPE();

    nsIpcChannel* pChannel;
    IOContext* pContext;
    DWORD uiBytesTransfered;
    DWORD uiError;
  };

public:
  nsMessageLoop_win();
  ~nsMessageLoop_win();

  HANDLE GetPort() const { return m_hPort; }

protected:
  virtual void WakeUp() override;
  virtual bool WaitForMessages(nsInt32 iTimeout, nsIpcChannel* pFilter) override;

  bool GetIOItem(nsInt32 iTimeout, IOItem* pItem);
  bool ProcessInternalIOItem(const IOItem& item);
  bool MatchCompletedIOItem(nsIpcChannel* pFilter, IOItem* pItem);

private:
  nsDynamicArray<IOItem> m_CompletedIO;
  LONG m_iHaveWork = 0;
  HANDLE m_hPort = INVALID_HANDLE_VALUE;
};

#endif
