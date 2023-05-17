#include <Foundation/FoundationPCH.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#  include <Foundation/Basics/Platform/Win/MinWindows.h>
#  include <type_traits>

template <typename wdType, typename WindowsType, bool mustBeConvertible>
void wdVerifyWindowsType()
{
  static_assert(sizeof(wdType) == sizeof(WindowsType), "wd <=> windows.h size mismatch");
  static_assert(alignof(wdType) == alignof(WindowsType), "wd <=> windows.h alignment mismatch");
  static_assert(std::is_pointer<wdType>::value == std::is_pointer<WindowsType>::value, "wd <=> windows.h pointer type mismatch");
  static_assert(!mustBeConvertible || wdConversionTest<wdType, WindowsType>::exists == 1, "wd <=> windows.h conversion failure");
  static_assert(!mustBeConvertible || wdConversionTest<WindowsType, wdType>::exists == 1, "windows.h <=> wd conversion failure");
};

void CALLBACK WindowsCallbackTest1();
void WD_WINDOWS_CALLBACK WindowsCallbackTest2();
void WINAPI WindowsWinapiTest1();
void WD_WINDOWS_WINAPI WindowsWinapiTest2();

// Will never be called and thus removed by the linker
void wdCheckWindowsTypeSizes()
{
  wdVerifyWindowsType<wdMinWindows::DWORD, DWORD, true>();
  wdVerifyWindowsType<wdMinWindows::UINT, UINT, true>();
  wdVerifyWindowsType<wdMinWindows::BOOL, BOOL, true>();
  wdVerifyWindowsType<wdMinWindows::LPARAM, LPARAM, true>();
  wdVerifyWindowsType<wdMinWindows::WPARAM, WPARAM, true>();
  wdVerifyWindowsType<wdMinWindows::HINSTANCE, HINSTANCE, false>();
  wdVerifyWindowsType<wdMinWindows::HMODULE, HMODULE, false>();
  wdVerifyWindowsType<wdMinWindows::LPSTR, LPSTR, true>();
  wdVerifyWindowsType<wdMinWindows::HWND, HWND, false>();
  wdVerifyWindowsType<wdMinWindows::HRESULT, HRESULT, true>();

  static_assert(std::is_same<decltype(&WindowsCallbackTest1), decltype(&WindowsCallbackTest2)>::value, "WD_WINDOWS_CALLBACK does not match CALLBACK");
  static_assert(std::is_same<decltype(&WindowsWinapiTest1), decltype(&WindowsWinapiTest2)>::value, "WD_WINDOWS_WINAPI does not match WINAPI");

  // Clang doesn't allow us to do this check at compile time
#  if WD_DISABLED(WD_COMPILER_CLANG)
  static_assert(WD_WINDOWS_INVALID_HANDLE_VALUE == INVALID_HANDLE_VALUE, "WD_WINDOWS_INVALID_HANDLE_VALUE does not match INVALID_HANDLE_VALUE");
#  endif
}
#endif


WD_STATICLINK_FILE(Foundation, Foundation_Basics_Platform_Win_MinWindows);
