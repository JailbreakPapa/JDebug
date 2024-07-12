#pragma once

#include <Foundation/Basics.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)

#  include <Foundation/FoundationInternal.h>
#  include <Foundation/Logging/Log.h>

NS_FOUNDATION_INTERNAL_HEADER

class nsETWProvider
{
public:
  nsETWProvider();
  ~nsETWProvider();

  void LogMessage(nsLogMsgType::Enum eventType, nsUInt8 uiIndentation, nsStringView sText);

  static nsETWProvider& GetInstance();
};

#endif
