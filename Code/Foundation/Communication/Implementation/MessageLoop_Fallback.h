#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Communication/Implementation/MessageLoop.h>

class NS_FOUNDATION_DLL nsMessageLoop_Fallback : public nsMessageLoop
{
public:
  nsMessageLoop_Fallback();
  ~nsMessageLoop_Fallback();

protected:
  virtual void WakeUp() override;
  virtual bool WaitForMessages(nsInt32 iTimeout, nsIpcChannel* pFilter) override;

private:
};
