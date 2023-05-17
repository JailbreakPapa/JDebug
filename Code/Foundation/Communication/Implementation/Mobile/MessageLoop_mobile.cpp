#include <Foundation/FoundationPCH.h>

#if WD_DISABLED(WD_PLATFORM_WINDOWS_DESKTOP)

#  include <Foundation/Communication/Implementation/Mobile/MessageLoop_mobile.h>
#  include <Foundation/Communication/IpcChannel.h>

wdMessageLoop_mobile::wdMessageLoop_mobile() {}

wdMessageLoop_mobile::~wdMessageLoop_mobile()
{
  StopUpdateThread();
}

void wdMessageLoop_mobile::WakeUp()
{
  // nothing to do
}

bool wdMessageLoop_mobile::WaitForMessages(wdInt32 iTimeout, wdIpcChannel* pFilter)
{
  // nothing to do

  if (iTimeout < 0)
  {
    // if timeout is 'indefinite' wait a little
    wdThreadUtils::YieldTimeSlice();
  }

  return false;
}

#endif



WD_STATICLINK_FILE(Foundation, Foundation_Communication_Implementation_Mobile_MessageLoop_mobile);
