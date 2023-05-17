
#pragma once

/// \file

#include <Foundation/Basics/Platform/Win/MinWindows.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Memory/MemoryTracker.h>
#include <Foundation/Threading/Lock.h>
#include <Foundation/Threading/Mutex.h>

namespace wdApplicationDetails
{
  WD_FOUNDATION_DLL void SetConsoleCtrlHandler(wdMinWindows::BOOL(WD_WINDOWS_WINAPI* consoleHandler)(wdMinWindows::DWORD dwCtrlType));
  WD_FOUNDATION_DLL wdMutex& GetShutdownMutex();

  template <typename AppClass, typename... Args>
  int ConsoleEntry(int iArgc, const char** pArgv, Args&&... arguments)
  {
#if WD_ENABLED(WD_COMPILER_MSVC)             // Internal compiler error in MSVC. Can not align buffer otherwise the compiler will crash.
    static char appBuffer[sizeof(AppClass)]; // Not on the stack to cope with smaller stacks.
#else
    alignas(WD_ALIGNMENT_OF(AppClass)) static char appBuffer[sizeof(AppClass)]; // Not on the stack to cope with smaller stacks.
#endif

    // This mutex will prevent the console shutdown handler to return
    // as long as this entry point is not finished executing
    // (see consoleHandler below).
    WD_LOCK(GetShutdownMutex());

    static AppClass* pApp = new (appBuffer) AppClass(std::forward<Args>(arguments)...);
    pApp->SetCommandLineArguments((wdUInt32)iArgc, pArgv);

    // This handler overrides the default handler
    // (which would call ExitProcess, which leads to disorderly engine shutdowns)
    const auto consoleHandler = [](wdMinWindows::DWORD ctrlType) -> wdMinWindows::BOOL {
      // We have to wait until the application has shut down orderly
      // since Windows will kill everything after this handler returns
      pApp->SetReturnCode(ctrlType);
      pApp->RequestQuit();
      WD_LOCK(GetShutdownMutex());
      return 1; // returns TRUE, which deactivates the default console control handler
    };
    SetConsoleCtrlHandler(consoleHandler);

    wdRun(pApp); // Life cycle & run method calling

    const int iReturnCode = pApp->GetReturnCode();
    if (iReturnCode != 0)
    {
      std::string text = pApp->TranslateReturnCode();
      if (!text.empty())
        wdLog::Printf("Return Code: %i = '%s'\n", iReturnCode, text.c_str());
      else
        wdLog::Printf("Return Code: %i\n", iReturnCode, text.c_str());
    }

    const bool memLeaks = pApp->IsMemoryLeakReportingEnabled();
    pApp->~AppClass();
    memset((void*)pApp, 0, sizeof(AppClass));
    if (memLeaks)
      wdMemoryTracker::DumpMemoryLeaks();

    return iReturnCode;
  }

  template <typename AppClass, typename... Args>
  int ApplicationEntry(Args&&... arguments)
  {
#if WD_ENABLED(WD_COMPILER_MSVC)             // Internal compiler error in MSVC. Can not align buffer otherwise the compiler will crash.
    static char appBuffer[sizeof(AppClass)]; // Not on the stack to cope with smaller stacks.
#else
    alignas(WD_ALIGNMENT_OF(AppClass)) static char appBuffer[sizeof(AppClass)]; // Not on the stack to cope with smaller stacks.
#endif

    AppClass* pApp = new (appBuffer) AppClass(std::forward<Args>(arguments)...);
    pApp->SetCommandLineArguments((wdUInt32)__argc, const_cast<const char**>(__argv));
    wdRun(pApp); // Life cycle & run method calling

    const int iReturnCode = pApp->GetReturnCode();
    if (iReturnCode != 0)
    {
      std::string text = pApp->TranslateReturnCode();
      if (!text.empty())
        wdLog::Printf("Return Code: '%s'\n", text.c_str());
    }

    const bool memLeaks = pApp->IsMemoryLeakReportingEnabled();
    pApp->~AppClass();
    memset((void*)pApp, 0, sizeof(AppClass));
    if (memLeaks)
      wdMemoryTracker::DumpMemoryLeaks();

    return iReturnCode;
  }
} // namespace wdApplicationDetails

/// \brief Same as WD_APPLICATION_ENTRY_POINT but should be used for applications that shall always show a console window.
#define WD_CONSOLEAPP_ENTRY_POINT(AppClass, ...)                                                \
  /* Enables that on machines with multiple GPUs the NVIDIA / AMD GPU is preferred */           \
  extern "C"                                                                                    \
  {                                                                                             \
    _declspec(dllexport) wdMinWindows::DWORD NvOptimusEnablement = 0x00000001;                  \
    _declspec(dllexport) wdMinWindows::DWORD AmdPowerXpressRequestHighPerformance = 0x00000001; \
  }                                                                                             \
  WD_APPLICATION_ENTRY_POINT_CODE_INJECTION                                     \
  int main(int argc, const char** argv) { return wdApplicationDetails::ConsoleEntry<AppClass>(argc, argv, __VA_ARGS__); }

// If windows.h is already included use the native types, otherwise use types from wdMinWindows
//
// In WD_APPLICATION_ENTRY_POINT we use macro magic to concatenate strings in such a way that depending on whether windows.h has
// been included in the mean time, either the macro is chosen which expands to the proper Windows.h type
// or the macro that expands to our wdMinWindows type.
// Unfortunately we cannot do the decision right here, as Windows.h may not yet be included, but may get included later.
#define _WD_APPLICATION_ENTRY_POINT_HINSTANCE HINSTANCE
#define _WD_APPLICATION_ENTRY_POINT_LPSTR LPSTR
#define _WD_APPLICATION_ENTRY_POINT_HINSTANCE_WINDOWS_ wdMinWindows::HINSTANCE
#define _WD_APPLICATION_ENTRY_POINT_LPSTR_WINDOWS_ wdMinWindows::LPSTR

#ifndef _In_
#  define UndefSAL
#  define _In_
#  define _In_opt_
#endif

/// \brief This macro allows for easy creation of application entry points (since they can't be placed in DLLs)
///
/// Just use the macro in a cpp file of your application and supply your app class (must be derived from wdApplication).
/// The additional (optional) parameters are passed to the constructor of your app class.
#define WD_APPLICATION_ENTRY_POINT(AppClass, ...)                                                                          \
  /* Enables that on machines with multiple GPUs the NVIDIA / AMD GPU is preferred */                                      \
  extern "C"                                                                                                               \
  {                                                                                                                        \
    _declspec(dllexport) wdMinWindows::DWORD NvOptimusEnablement = 0x00000001;                                             \
    _declspec(dllexport) wdMinWindows::DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;                            \
  }                                                                                                                        \
  WD_APPLICATION_ENTRY_POINT_CODE_INJECTION                                                                                \
  int WD_WINDOWS_CALLBACK WinMain(_In_ WD_CONCAT(_WD_, WD_CONCAT(APPLICATION_ENTRY_POINT_HINSTANCE, _WINDOWS_)) hInstance, \
    _In_opt_ WD_CONCAT(_WD_, WD_CONCAT(APPLICATION_ENTRY_POINT_HINSTANCE, _WINDOWS_)) hPrevInstance,                       \
    _In_ WD_CONCAT(_WD_, WD_CONCAT(APPLICATION_ENTRY_POINT_LPSTR, _WINDOWS_)) lpCmdLine, _In_ int nCmdShow)                \
  {                                                                                                                        \
    return wdApplicationDetails::ApplicationEntry<AppClass>(__VA_ARGS__);                                                  \
  }

#ifdef UndefSAL
#  undef _In_
#  undef _In_opt_
#endif
