#pragma once

namespace wdMinWindows
{
  using BOOL = int;
  using DWORD = unsigned long;
  using UINT = unsigned int;
  using LPSTR = char*;
  struct wdHINSTANCE;
  using HINSTANCE = wdHINSTANCE*;
  using HMODULE = HINSTANCE;
  struct wdHWND;
  using HWND = wdHWND*;
  using HRESULT = long;
  using HANDLE = void*;

#if WD_ENABLED(WD_PLATFORM_64BIT)
  using WPARAM = wdUInt64;
  using LPARAM = wdUInt64;
#else
  typedef wdUInt32 WPARAM;
  typedef wdUInt32 LPARAM;
#endif

  template <typename T>
  struct ToNativeImpl
  {
  };

  template <typename T>
  struct FromNativeImpl
  {
  };

  /// Helper function to convert wdMinWindows types into native windows.h types.
  /// Include IncludeWindows.h before using it.
  template <typename T>
  WD_ALWAYS_INLINE typename ToNativeImpl<T>::type ToNative(T t)
  {
    return ToNativeImpl<T>::ToNative(t);
  }

  /// Helper function to native windows.h types to wdMinWindows types.
  /// Include IncludeWindows.h before using it.
  template <typename T>
  WD_ALWAYS_INLINE typename FromNativeImpl<T>::type FromNative(T t)
  {
    return FromNativeImpl<T>::FromNative(t);
  }
} // namespace wdMinWindows
#define WD_WINDOWS_CALLBACK __stdcall
#define WD_WINDOWS_WINAPI __stdcall
#define WD_WINDOWS_INVALID_HANDLE_VALUE ((void*)(long long)-1)
