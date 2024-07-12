#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)

#  include <Foundation/Basics/Platform/Win/HResultUtils.h>
#  include <Foundation/Strings/StringBuilder.h>
#  include <Foundation/Strings/StringConversion.h>

NS_FOUNDATION_DLL nsString nsHRESULTtoString(nsMinWindows::HRESULT result)
{
  wchar_t buffer[4096];
  if (::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,
        result,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
        buffer,
        NS_ARRAY_SIZE(buffer),
        nullptr) == 0)
  {
    return {};
  }

  // Com error tends to put /r/n at the end. Remove it.
  nsStringBuilder message(nsStringUtf8(&buffer[0]).GetData());
  message.ReplaceAll("\n", "");
  message.ReplaceAll("\r", "");

  return message;
}

#endif
