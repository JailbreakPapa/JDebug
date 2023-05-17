#include <Foundation/FoundationPCH.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS)

#  include <Foundation/Basics/Platform/Win/HResultUtils.h>
#  include <Foundation/Strings/StringBuilder.h>
#  include <Foundation/Strings/StringConversion.h>

#  include <comdef.h>

WD_FOUNDATION_DLL wdString wdHRESULTtoString(wdMinWindows::HRESULT result)
{
  _com_error error(result, nullptr);
  const TCHAR* messageW = error.ErrorMessage();

  // Com error tends to put /r/n at the end. Remove it.
  wdStringBuilder message(wdStringUtf8(messageW).GetData());
  message.ReplaceAll("\n", "");
  message.ReplaceAll("\r", "");

  return message;
}

#endif



WD_STATICLINK_FILE(Foundation, Foundation_Basics_Platform_Win_HResultUtils);
