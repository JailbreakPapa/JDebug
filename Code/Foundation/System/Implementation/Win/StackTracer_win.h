#ifdef WD_STACKTRACER_WIN_INL_H_INCLUDED
#  error "This file must not be included twice."
#endif

#define WD_STACKTRACER_WIN_INL_H_INCLUDED

#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#define WD_MSVC_WARNING_NUMBER 4091
#include <Foundation/Basics/Compiler/MSVC/DisableWarning_MSVC.h>

#include <DbgHelp.h>

#include <Foundation/Basics/Compiler/MSVC/RestoreWarning_MSVC.h>

#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Math/Math.h>

#include <memory>

// Deactivate Doxygen document generation for the following block.
/// \cond

namespace
{
  typedef WORD(__stdcall* CaptureStackBackTraceFunc)(DWORD FramesToSkip, DWORD FramesToCapture, PVOID* BackTrace, PDWORD BackTraceHash);

  typedef BOOL(__stdcall* SymbolInitializeFunc)(HANDLE hProcess, PCWSTR UserSearchPath, BOOL fInvadeProcess);

  typedef DWORD64(__stdcall* SymbolLoadModuleFunc)(
    HANDLE hProcess, HANDLE hFile, PCWSTR ImageName, PCWSTR ModuleName, DWORD64 BaseOfDll, DWORD DllSize, PMODLOAD_DATA Data, DWORD Flags);

  typedef BOOL(__stdcall* SymbolGetModuleInfoFunc)(HANDLE hProcess, DWORD64 qwAddr, PIMAGEHLP_MODULEW64 ModuleInfo);

  typedef PVOID(__stdcall* SymbolFunctionTableAccess)(HANDLE hProcess, DWORD64 AddrBase);

  typedef DWORD64(__stdcall* SymbolGetModuleBaseFunc)(HANDLE hProcess, DWORD64 qwAddr);

  typedef BOOL(__stdcall* StackWalk)(DWORD MachineType, HANDLE hProcess, HANDLE hThread, LPSTACKFRAME64 StackFrame, PVOID ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine, PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine, PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress);

  typedef BOOL(__stdcall* SymbolFromAddressFunc)(HANDLE hProcess, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFOW Symbol);

  typedef BOOL(__stdcall* LineFromAddressFunc)(HANDLE hProcess, DWORD64 Address, PDWORD64 Displacement, PIMAGEHLP_LINEW64 Line);

  struct StackTracerImplementation
  {
    HMODULE kernel32Dll;
    HMODULE dbgHelpDll;

    CaptureStackBackTraceFunc captureStackBackTrace;
    SymbolInitializeFunc symbolInitialize;
    SymbolLoadModuleFunc symbolLoadModule;
    SymbolGetModuleInfoFunc getModuleInfo;
    SymbolFunctionTableAccess getFunctionTableAccess;
    SymbolGetModuleBaseFunc getModuleBase;
    StackWalk stackWalk;
    SymbolFromAddressFunc symbolFromAddress;
    LineFromAddressFunc lineFromAdress;
    bool m_bInitDbgHelp = false;

    StackTracerImplementation()
    {
      wdMemoryUtils::ZeroFill(this, 1);

      kernel32Dll = LoadLibraryW(L"kernel32.dll");
      WD_ASSERT_DEV(kernel32Dll != nullptr, "StackTracer could not load kernel32.dll");
      if (kernel32Dll != nullptr)
      {
        captureStackBackTrace = (CaptureStackBackTraceFunc)GetProcAddress(kernel32Dll, "RtlCaptureStackBackTrace");
      }

      dbgHelpDll = LoadLibraryW(L"dbghelp.dll");
      WD_ASSERT_DEV(dbgHelpDll != nullptr, "StackTracer could not load dbghelp.dll");
      if (dbgHelpDll != nullptr)
      {
        symbolInitialize = (SymbolInitializeFunc)GetProcAddress(dbgHelpDll, "SymInitializeW");
        symbolLoadModule = (SymbolLoadModuleFunc)GetProcAddress(dbgHelpDll, "SymLoadModuleExW");
        getModuleInfo = (SymbolGetModuleInfoFunc)GetProcAddress(dbgHelpDll, "SymGetModuleInfoW64");
        getFunctionTableAccess = (SymbolFunctionTableAccess)GetProcAddress(dbgHelpDll, "SymFunctionTableAccess64");
        getModuleBase = (SymbolGetModuleBaseFunc)GetProcAddress(dbgHelpDll, "SymGetModuleBase64");
        stackWalk = (StackWalk)GetProcAddress(dbgHelpDll, "StackWalk64");
        if (symbolInitialize == nullptr || symbolLoadModule == nullptr || getModuleInfo == nullptr || getFunctionTableAccess == nullptr ||
            getModuleBase == nullptr || stackWalk == nullptr)
          return;

        symbolFromAddress = (SymbolFromAddressFunc)GetProcAddress(dbgHelpDll, "SymFromAddrW");
        lineFromAdress = (LineFromAddressFunc)GetProcAddress(dbgHelpDll, "SymGetLineFromAddrW64");
      }
    }
  };

  static StackTracerImplementation* s_pImplementation;

  static void Initialize()
  {
    if (s_pImplementation == nullptr)
    {
      alignas(WD_ALIGNMENT_OF(StackTracerImplementation)) static wdUInt8 ImplementationBuffer[sizeof(StackTracerImplementation)];
      s_pImplementation = new (ImplementationBuffer) StackTracerImplementation();
      WD_ASSERT_DEV(s_pImplementation != nullptr, "StackTracer initialization failed");
    }
  }

  static void SymbolInitialize()
  {
    if (!s_pImplementation->m_bInitDbgHelp)
    {
      s_pImplementation->m_bInitDbgHelp = true;
      if (!(*s_pImplementation->symbolInitialize)(GetCurrentProcess(), nullptr, TRUE))
      {
        wdLog::Error("StackTracer could not initialize symbols. Error-Code {0}", wdArgErrorCode(::GetLastError()));
      }
    }
  }
} // namespace

void wdStackTracer::OnPluginEvent(const wdPluginEvent& e)
{
  Initialize();

  if (s_pImplementation->symbolLoadModule == nullptr || s_pImplementation->getModuleInfo == nullptr)
    return;

  // Can't get dbghelp functions to work correctly. SymLoadModuleEx will fail on every dll after the first call.
  // However, SymInitialize works to load dynamic dlls if we postpone it until all dlls are loaded.
  // So we defer init until the first DLL is un-loaded or the first callstack is to be resolved.
  if (e.m_EventType == wdPluginEvent::BeforeUnloading)
  {
    SymbolInitialize();
  }

  if (false) // e.m_EventType == wdPluginEvent::AfterLoading)
  {
    char buffer[1024];
    strcpy_s(buffer, wdOSFile::GetApplicationDirectory());
    strcat_s(buffer, e.m_szPluginBinary);
    strcat_s(buffer, ".dll");

    wchar_t szPluginPath[1024];
    mbstowcs(szPluginPath, buffer, WD_ARRAY_SIZE(szPluginPath));

    wchar_t szPluginName[256];
    mbstowcs(szPluginName, e.m_szPluginBinary, WD_ARRAY_SIZE(szPluginName));

    HANDLE currentProcess = GetCurrentProcess();

    DWORD64 moduleAddress = (*s_pImplementation->symbolLoadModule)(currentProcess, nullptr, szPluginPath, szPluginName, 0, 0, nullptr, 0);
    if (moduleAddress == 0)
    {
      DWORD err = GetLastError();
      if (err != ERROR_SUCCESS)
      {
        wdLog::Error("StackTracer could not load symbols for '{0}'. Error-Code {1}", e.m_szPluginBinary, wdArgErrorCode(err));
      }

      return;
    }

    IMAGEHLP_MODULEW64 moduleInfo;
    wdMemoryUtils::ZeroFill(&moduleInfo, 1);
    moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULEW64);

    if (!(*s_pImplementation->getModuleInfo)(currentProcess, moduleAddress, &moduleInfo))
    {
      DWORD err = GetLastError();
      LPVOID lpMsgBuf = nullptr;

      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, err,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPTSTR)&lpMsgBuf, 0, nullptr);

      char errStr[1024];
      sprintf_s(errStr, "StackTracer could not get module info for '%s'. Error-Code %u (\"%s\")\n", e.m_szPluginBinary, err, static_cast<char*>(lpMsgBuf));
      wdLog::Print(errStr);

      LocalFree(lpMsgBuf);
    }
  }
}

// static
wdUInt32 wdStackTracer::GetStackTrace(wdArrayPtr<void*>& ref_trace, void* pContext)
{
  Initialize();

  if (pContext && s_pImplementation->stackWalk)
  {
    // We need dbghelp init for stackWalk call.
    SymbolInitialize();
    // in order not to destroy the pContext handed in we need to make a copy of it
    // see StackWalk/StackWalk64 docs https://docs.microsoft.com/windows/win32/api/dbghelp/nf-dbghelp-stackwalk
    PCONTEXT originalContext = static_cast<PCONTEXT>(pContext);
    PCONTEXT copiedContext = nullptr;

    DWORD contextSize = 0;
    // get size needed for buffer and allocate buffer of that size
    InitializeContext(nullptr, originalContext->ContextFlags, &copiedContext, &contextSize);
    unsigned char* rawBuffer = new (std::nothrow) unsigned char[contextSize];
    if (rawBuffer == nullptr)
    {
      return 0;
    }
    auto pBuffer = std::unique_ptr<unsigned char[]>(rawBuffer);

    BOOL contextInitalized = InitializeContext(static_cast<void*>(pBuffer.get()), originalContext->ContextFlags, &copiedContext, &contextSize);
    if (!contextInitalized)
    {
      return 0;
    }

    BOOL contextCopied = CopyContext(copiedContext, originalContext->ContextFlags, originalContext);
    if (!contextCopied)
    {
      return 0;
    }

    CONTEXT& context = *static_cast<PCONTEXT>(copiedContext);
    DWORD machine_type;
    STACKFRAME64 frame;
    ZeroMemory(&frame, sizeof(frame));
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Mode = AddrModeFlat;
#ifdef _M_X64
    frame.AddrPC.Offset = context.Rip;
    frame.AddrFrame.Offset = context.Rbp;
    frame.AddrStack.Offset = context.Rsp;
    machine_type = IMAGE_FILE_MACHINE_AMD64;
#else
    frame.AddrPC.Offset = context.Eip;
    frame.AddrFrame.Offset = context.Ebp;
    frame.AddrStack.Offset = context.Esp;
    machine_type = IMAGE_FILE_MACHINE_I386;
#endif
    for (wdInt32 i = 0; i < (wdInt32)ref_trace.GetCount(); i++)
    {
      if (s_pImplementation->stackWalk(machine_type, GetCurrentProcess(), GetCurrentThread(), &frame, &context, NULL,
            s_pImplementation->getFunctionTableAccess, s_pImplementation->getModuleBase, NULL))
      {
        ref_trace[i] = reinterpret_cast<void*>(frame.AddrPC.Offset);
      }
      else
      {
        // skip the last three stack-frames since they are useless
        return wdMath::Max(i - 4, 0);
      }
    }
  }
  else if (s_pImplementation->captureStackBackTrace != nullptr)
  {
    const wdUInt32 uiSkip = 1;
    const wdUInt32 uiMaxNumTrace = wdMath::Min(62U, ref_trace.GetCount());
    wdInt32 iNumTraces = (*s_pImplementation->captureStackBackTrace)(uiSkip, uiMaxNumTrace, ref_trace.GetPtr(), nullptr);

    // skip the last three stack-frames since they are useless
    return wdMath::Max(iNumTraces - 3, 0);
  }

  return 0;
}

// static
void wdStackTracer::ResolveStackTrace(const wdArrayPtr<void*>& trace, PrintFunc printFunc)
{
  Initialize();
  SymbolInitialize();

  if (s_pImplementation->symbolFromAddress != nullptr && s_pImplementation->lineFromAdress != nullptr)
  {
    char buffer[1024];
    HANDLE currentProcess = GetCurrentProcess();

    const wdUInt32 uiNumTraceEntries = trace.GetCount();
    for (wdUInt32 i = 0; i < uiNumTraceEntries; i++)
    {
      DWORD64 pSymbolAddress = reinterpret_cast<UINT_PTR>(trace[i]);

      _SYMBOL_INFOW& symbolInfo = *(_SYMBOL_INFOW*)buffer;
      wdMemoryUtils::ZeroFill(&symbolInfo, 1);
      symbolInfo.SizeOfStruct = sizeof(_SYMBOL_INFOW);
      symbolInfo.MaxNameLen = (WD_ARRAY_SIZE(buffer) - symbolInfo.SizeOfStruct) / sizeof(WCHAR);

      DWORD64 displacement = 0;
      BOOL result = (*s_pImplementation->symbolFromAddress)(currentProcess, pSymbolAddress, &displacement, &symbolInfo);
      if (!result)
      {
        wcscpy_s(symbolInfo.Name, symbolInfo.MaxNameLen, L"<Unknown>");
      }

      IMAGEHLP_LINEW64 lineInfo;
      memset(&lineInfo, 0, sizeof(lineInfo));
      lineInfo.SizeOfStruct = sizeof(lineInfo);
      (*s_pImplementation->lineFromAdress)(currentProcess, pSymbolAddress, &displacement, &lineInfo);

      wchar_t str[1024];
      swprintf_s(str, L"%s(%u):'%s'\n", lineInfo.FileName, lineInfo.LineNumber, symbolInfo.Name);

      char finalStr[1024];
      wcstombs(finalStr, str, WD_ARRAY_SIZE(finalStr));

      printFunc(finalStr);
    }
  }
}


/// \endcond
