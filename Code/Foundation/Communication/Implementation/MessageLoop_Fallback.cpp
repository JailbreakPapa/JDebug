#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/Implementation/MessageLoop_Fallback.h>
#include <Foundation/Communication/IpcChannel.h>

nsMessageLoop_Fallback::nsMessageLoop_Fallback() = default;

nsMessageLoop_Fallback::~nsMessageLoop_Fallback()
{
  StopUpdateThread();
}

void nsMessageLoop_Fallback::WakeUp()
{
  // nothing to do
}

bool nsMessageLoop_Fallback::WaitForMessages(nsInt32 iTimeout, nsIpcChannel* pFilter)
{
  // nothing to do

  if (iTimeout < 0)
  {
    // if timeout is 'indefinite' wait a little
    nsThreadUtils::YieldTimeSlice();
  }

  return false;
}
