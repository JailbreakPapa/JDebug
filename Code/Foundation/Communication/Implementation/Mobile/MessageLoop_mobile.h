#pragma once

#if WD_DISABLED(WD_PLATFORM_WINDOWS_DESKTOP)

#  include <Foundation/Basics.h>
#  include <Foundation/Communication/Implementation/MessageLoop.h>

class WD_FOUNDATION_DLL wdMessageLoop_mobile : public wdMessageLoop
{
public:
  wdMessageLoop_mobile();
  ~wdMessageLoop_mobile();

protected:
  virtual void WakeUp() override;
  virtual bool WaitForMessages(wdInt32 iTimeout, wdIpcChannel* pFilter) override;

private:
};

#endif
