
#pragma once

/// \file

#include <Foundation/Basics/Platform/Win/MinWindows.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Memory/MemoryTracker.h>
#include <Foundation/Threading/Lock.h>
#include <Foundation/Threading/Mutex.h>

namespace nsApplicationDetails
{
  NS_FOUNDATION_DLL void SetConsoleCtrlHandler(nsMinWindows::BOOL(NS_WINDOWS_WINAPI* consoleHandler)(nsMinWindows::DWORD dwCtrlType));
  NS_FOUNDATION_DLL nsMutex& GetShutdownMutex();

  template <typename AppClass, typename... Args>
  int ConsoleEntry(int iArgc, const char** pArgv, Args&&... arguments)
  {
#if NS_ENABLED(NS_COMPILER_MSVC)             // Internal compiler error in MSVC. Can not align buffer otherwise the compiler will crash.
    static char appBuffer[sizeof(AppClass)]; // Not on the stack to cope with smaller stacks.
#else
    alignas(NS_ALIGNMENT_OF(AppClass)) static char appBuffer[sizeof(AppClass)]; // Not on the stack to cope with smaller stacks.
#endif

    // This mutex will prevent the console shutdown handler to return
    // as long as this entry point is not finished executing
    // (see consoleHandler below).
    NS_LOCK(GetShutdownMutex());

    static AppClass* pApp = new (appBuffer) AppClass(std::forward<Args>(arguments)...);
    pApp->SetCommandLineArguments((nsUInt32)iArgc, pArgv);

    // This handler overrides the default handler
    // (which would call ExitProcess, which leads to disorderly engine shutdowns)
    const auto consoleHandler = [](nsMinWindows::DWORD ctrlType) -> nsMinWindows::BOOL
    {
      // We have to wait until the application has shut down orderly
      // since Windows will kill everything after this handler returns
      pApp->SetReturnCode(ctrlType);
      pApp->RequestQuit();
      NS_LOCK(GetShutdownMutex());
      return 1; // returns TRUE, which deactivates the default console control handler
    };
    SetConsoleCtrlHandler(consoleHandler);

    nsRun(pApp); // Life cycle & run method calling

    const int iReturnCode = pApp->GetReturnCode();
    if (iReturnCode != 0)
    {
      std::string text = pApp->TranslateReturnCode();
      if (!text.empty())
        nsLog::Printf("Return Code: %i = '%s'\n", iReturnCode, text.c_str());
      else
        nsLog::Printf("Return Code: %i\n", iReturnCode, text.c_str());
    }

    const bool memLeaks = pApp->IsMemoryLeakReportingEnabled();
    pApp->~AppClass();
    memset((void*)pApp, 0, sizeof(AppClass));
    if (memLeaks)
      nsMemoryTracker::DumpMemoryLeaks();

    return iReturnCode;
  }

  template <typename AppClass, typename... Args>
  int ApplicationEntry(Args&&... arguments)
  {
#if NS_ENABLED(NS_COMPILER_MSVC)             // Internal compiler error in MSVC. Can not align buffer otherwise the compiler will crash.
    static char appBuffer[sizeof(AppClass)]; // Not on the stack to cope with smaller stacks.
#else
    alignas(NS_ALIGNMENT_OF(AppClass)) static char appBuffer[sizeof(AppClass)]; // Not on the stack to cope with smaller stacks.
#endif

    AppClass* pApp = new (appBuffer) AppClass(std::forward<Args>(arguments)...);
    pApp->SetCommandLineArguments((nsUInt32)__argc, const_cast<const char**>(__argv));
    nsRun(pApp); // Life cycle & run method calling

    const int iReturnCode = pApp->GetReturnCode();
    if (iReturnCode != 0)
    {
      std::string text = pApp->TranslateReturnCode();
      if (!text.empty())
        nsLog::Printf("Return Code: '%s'\n", text.c_str());
    }

    const bool memLeaks = pApp->IsMemoryLeakReportingEnabled();
    pApp->~AppClass();
    memset((void*)pApp, 0, sizeof(AppClass));
    if (memLeaks)
      nsMemoryTracker::DumpMemoryLeaks();

    return iReturnCode;
  }
} // namespace nsApplicationDetails

/// \brief Same as NS_APPLICATION_ENTRY_POINT but should be used for applications that shall always show a console window.
#define NS_CONSOLEAPP_ENTRY_POINT(AppClass, ...)                                                \
  /* Enables that on machines with multiple GPUs the NVIDIA / AMD GPU is preferred */           \
  extern "C"                                                                                    \
  {                                                                                             \
    _declspec(dllexport) nsMinWindows::DWORD NvOptimusEnablement = 0x00000001;                  \
    _declspec(dllexport) nsMinWindows::DWORD AmdPowerXpressRequestHighPerformance = 0x00000001; \
  }                                                                                             \
  NS_APPLICATION_ENTRY_POINT_CODE_INJECTION                                                     \
  int main(int argc, const char** argv)                                                         \
  {                                                                                             \
    return nsApplicationDetails::ConsoleEntry<AppClass>(argc, argv, __VA_ARGS__);               \
  }

// If windows.h is already included use the native types, otherwise use types from nsMinWindows
//
// In NS_APPLICATION_ENTRY_POINT we use macro magic to concatenate strings in such a way that depending on whether windows.h has
// been included in the mean time, either the macro is chosen which expands to the proper Windows.h type
// or the macro that expands to our nsMinWindows type.
// Unfortunately we cannot do the decision right here, as Windows.h may not yet be included, but may get included later.
#define _NS_APPLICATION_ENTRY_POINT_HINSTANCE HINSTANCE
#define _NS_APPLICATION_ENTRY_POINT_LPSTR LPSTR
#define _NS_APPLICATION_ENTRY_POINT_HINSTANCE_WINDOWS_ nsMinWindows::HINSTANCE
#define _NS_APPLICATION_ENTRY_POINT_LPSTR_WINDOWS_ nsMinWindows::LPSTR

#ifndef _In_
#  define UndefSAL
#  define _In_
#  define _In_opt_
#endif

/// \brief This macro allows for easy creation of application entry points (since they can't be placed in DLLs)
///
/// Just use the macro in a cpp file of your application and supply your app class (must be derived from nsApplication).
/// The additional (optional) parameters are passed to the constructor of your app class.
#define NS_APPLICATION_ENTRY_POINT(AppClass, ...)                                                                          \
  /* Enables that on machines with multiple GPUs the NVIDIA / AMD GPU is preferred */                                      \
  extern "C"                                                                                                               \
  {                                                                                                                        \
    _declspec(dllexport) nsMinWindows::DWORD NvOptimusEnablement = 0x00000001;                                             \
    _declspec(dllexport) nsMinWindows::DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;                            \
  }                                                                                                                        \
  NS_APPLICATION_ENTRY_POINT_CODE_INJECTION                                                                                \
  int NS_WINDOWS_CALLBACK WinMain(_In_ NS_CONCAT(_NS_, NS_CONCAT(APPLICATION_ENTRY_POINT_HINSTANCE, _WINDOWS_)) hInstance, \
    _In_opt_ NS_CONCAT(_NS_, NS_CONCAT(APPLICATION_ENTRY_POINT_HINSTANCE, _WINDOWS_)) hPrevInstance,                       \
    _In_ NS_CONCAT(_NS_, NS_CONCAT(APPLICATION_ENTRY_POINT_LPSTR, _WINDOWS_)) lpCmdLine, _In_ int nCmdShow)                \
  {                                                                                                                        \
    return nsApplicationDetails::ApplicationEntry<AppClass>(__VA_ARGS__);                                                  \
  }

#ifdef UndefSAL
#  undef _In_
#  undef _In_opt_
#endif
