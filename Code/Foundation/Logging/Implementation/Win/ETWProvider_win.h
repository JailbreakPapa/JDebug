#pragma once

#include <Foundation/Basics.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS)

#  include <Foundation/FoundationInternal.h>
#  include <Foundation/Logging/Log.h>

WD_FOUNDATION_INTERNAL_HEADER

class wdETWProvider
{
public:
  wdETWProvider();
  ~wdETWProvider();

  void LogMessge(wdLogMsgType::Enum eventType, wdUInt8 uiIndentation, wdStringView sText);

  static wdETWProvider& GetInstance();
};
#endif
