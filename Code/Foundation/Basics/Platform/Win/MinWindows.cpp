#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#  include <Foundation/Basics/Platform/Win/MinWindows.h>
#  include <type_traits>

template <typename nsType, typename WindowsType, bool mustBeConvertible>
void nsVerifyWindowsType()
{
  static_assert(sizeof(nsType) == sizeof(WindowsType), "ns <=> windows.h size mismatch");
  static_assert(alignof(nsType) == alignof(WindowsType), "ns <=> windows.h alignment mismatch");
  static_assert(std::is_pointer<nsType>::value == std::is_pointer<WindowsType>::value, "ns <=> windows.h pointer type mismatch");
  static_assert(!mustBeConvertible || nsConversionTest<nsType, WindowsType>::exists == 1, "ns <=> windows.h conversion failure");
  static_assert(!mustBeConvertible || nsConversionTest<WindowsType, nsType>::exists == 1, "windows.h <=> ns conversion failure");
};

void CALLBACK WindowsCallbackTest1();
void NS_WINDOWS_CALLBACK WindowsCallbackTest2();
void WINAPI WindowsWinapiTest1();
void NS_WINDOWS_WINAPI WindowsWinapiTest2();

// Will never be called and thus removed by the linker
void nsCheckWindowsTypeSizes()
{
  nsVerifyWindowsType<nsMinWindows::DWORD, DWORD, true>();
  nsVerifyWindowsType<nsMinWindows::UINT, UINT, true>();
  nsVerifyWindowsType<nsMinWindows::BOOL, BOOL, true>();
  nsVerifyWindowsType<nsMinWindows::LPARAM, LPARAM, true>();
  nsVerifyWindowsType<nsMinWindows::WPARAM, WPARAM, true>();
  nsVerifyWindowsType<nsMinWindows::HINSTANCE, HINSTANCE, false>();
  nsVerifyWindowsType<nsMinWindows::HMODULE, HMODULE, false>();
  nsVerifyWindowsType<nsMinWindows::LPSTR, LPSTR, true>();
  nsVerifyWindowsType<nsMinWindows::HWND, HWND, false>();
  nsVerifyWindowsType<nsMinWindows::HRESULT, HRESULT, true>();

  static_assert(std::is_same<decltype(&WindowsCallbackTest1), decltype(&WindowsCallbackTest2)>::value, "NS_WINDOWS_CALLBACK does not match CALLBACK");
  static_assert(std::is_same<decltype(&WindowsWinapiTest1), decltype(&WindowsWinapiTest2)>::value, "NS_WINDOWS_WINAPI does not match WINAPI");

  // Clang doesn't allow us to do this check at compile time
#  if NS_DISABLED(NS_COMPILER_CLANG)
  static_assert(NS_WINDOWS_INVALID_HANDLE_VALUE == INVALID_HANDLE_VALUE, "NS_WINDOWS_INVALID_HANDLE_VALUE does not match INVALID_HANDLE_VALUE");
#  endif
}
#endif
