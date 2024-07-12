#pragma once

namespace nsMinWindows
{
  using BOOL = int;
  using DWORD = unsigned long;
  using UINT = unsigned int;
  using LPSTR = char*;
  struct nsHINSTANCE;
  using HINSTANCE = nsHINSTANCE*;
  using HMODULE = HINSTANCE;
  struct nsHWND;
  using HWND = nsHWND*;
  using HRESULT = long;
  using HANDLE = void*;

#if NS_ENABLED(NS_PLATFORM_64BIT)
  using WPARAM = nsUInt64;
  using LPARAM = nsUInt64;
#else
  using WPARAM = nsUInt32;
  using LPARAM = nsUInt32;
#endif

  template <typename T>
  struct ToNativeImpl
  {
  };

  template <typename T>
  struct FromNativeImpl
  {
  };

  /// Helper function to convert nsMinWindows types into native windows.h types.
  /// Include IncludeWindows.h before using it.
  template <typename T>
  NS_ALWAYS_INLINE typename ToNativeImpl<T>::type ToNative(T t)
  {
    return ToNativeImpl<T>::ToNative(t);
  }

  /// Helper function to native windows.h types to nsMinWindows types.
  /// Include IncludeWindows.h before using it.
  template <typename T>
  NS_ALWAYS_INLINE typename FromNativeImpl<T>::type FromNative(T t)
  {
    return FromNativeImpl<T>::FromNative(t);
  }
} // namespace nsMinWindows
#define NS_WINDOWS_CALLBACK __stdcall
#define NS_WINDOWS_WINAPI __stdcall
#define NS_WINDOWS_INVALID_HANDLE_VALUE ((void*)(long long)-1)
