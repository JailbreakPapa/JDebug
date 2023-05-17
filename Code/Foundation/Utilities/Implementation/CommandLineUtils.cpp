#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/OSFile.h>
#include <Foundation/Utilities/CommandLineUtils.h>
#include <Foundation/Utilities/ConversionUtils.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#  include <shellapi.h>
#endif

static wdCommandLineUtils g_pCmdLineInstance;

wdCommandLineUtils* wdCommandLineUtils::GetGlobalInstance()
{
  return &g_pCmdLineInstance;
}

void wdCommandLineUtils::SplitCommandLineString(const char* szCommandString, bool bAddExecutableDir, wdDynamicArray<wdString>& out_args, wdDynamicArray<const char*>& out_argsV)
{
  // Add application dir as first argument as customary on other platforms.
  if (bAddExecutableDir)
  {
#if WD_ENABLED(WD_PLATFORM_WINDOWS)
    wchar_t moduleFilename[256];
    GetModuleFileNameW(nullptr, moduleFilename, 256);
    out_args.PushBack(wdStringUtf8(moduleFilename).GetData());
#else
    WD_ASSERT_NOT_IMPLEMENTED;
#endif
  }

  // Simple args splitting. Not as powerful as Win32's CommandLineToArgvW.
  const char* currentChar = szCommandString;
  const char* lastEnd = currentChar;
  bool inQuotes = false;
  while (*currentChar != '\0')
  {
    if (*currentChar == '\"')
      inQuotes = !inQuotes;
    else if (*currentChar == ' ' && !inQuotes)
    {
      wdStringBuilder path = wdStringView(lastEnd, currentChar);
      path.Trim(" \"");
      out_args.PushBack(path);
      lastEnd = currentChar + 1;
    }
    wdUnicodeUtils::MoveToNextUtf8(currentChar);
  }

  out_argsV.Reserve(out_argsV.GetCount());
  for (wdString& str : out_args)
    out_argsV.PushBack(str.GetData());
}

void wdCommandLineUtils::SetCommandLine(wdUInt32 uiArgc, const char** pArgv, ArgMode mode /*= UseArgcArgv*/)
{
#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)
  if (mode == ArgMode::PreferOsArgs)
  {
    SetCommandLine();
    return;
  }
#endif

  m_Commands.Clear();
  m_Commands.Reserve(uiArgc);

  for (wdUInt32 i = 0; i < uiArgc; ++i)
    m_Commands.PushBack(pArgv[i]);
}

void wdCommandLineUtils::SetCommandLine(wdArrayPtr<wdString> commands)
{
  m_Commands = commands;
}

#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)

void wdCommandLineUtils::SetCommandLine()
{
  int argc = 0;

  LPWSTR* argvw = CommandLineToArgvW(::GetCommandLineW(), &argc);

  WD_ASSERT_RELEASE(argvw != nullptr, "CommandLineToArgvW failed");

  wdArrayPtr<wdStringUtf8> ArgvUtf8 = WD_DEFAULT_NEW_ARRAY(wdStringUtf8, argc);
  wdArrayPtr<const char*> argv = WD_DEFAULT_NEW_ARRAY(const char*, argc);

  for (wdInt32 i = 0; i < argc; ++i)
  {
    ArgvUtf8[i] = argvw[i];
    argv[i] = ArgvUtf8[i].GetData();
  }

  SetCommandLine(argc, argv.GetPtr(), ArgMode::UseArgcArgv);


  WD_DEFAULT_DELETE_ARRAY(ArgvUtf8);
  WD_DEFAULT_DELETE_ARRAY(argv);
  LocalFree(argvw);
}

#elif WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
// Not implemented on Windows UWP.
#elif WD_ENABLED(WD_PLATFORM_OSX)
// Not implemented on OSX.
#elif WD_ENABLED(WD_PLATFORM_LINUX)
// Not implemented on Linux.
#elif WD_ENABLED(WD_PLATFORM_ANDROID)
// Not implemented on Android.
#else
#  error "wdCommandLineUtils::SetCommandLine(): Abstraction missing."
#endif

const wdDynamicArray<wdString>& wdCommandLineUtils::GetCommandLineArray() const
{
  return m_Commands;
}

wdString wdCommandLineUtils::GetCommandLineString() const
{
  wdStringBuilder commandLine;
  for (const wdString& command : m_Commands)
  {
    if (commandLine.IsEmpty())
    {
      commandLine.Append(command.GetView());
    }
    else
    {
      commandLine.Append(" ", command);
    }
  }
  return commandLine;
}

wdUInt32 wdCommandLineUtils::GetParameterCount() const
{
  return m_Commands.GetCount();
}

const char* wdCommandLineUtils::GetParameter(wdUInt32 uiParam) const
{
  return m_Commands[uiParam].GetData();
}

wdInt32 wdCommandLineUtils::GetOptionIndex(const char* szOption, bool bCaseSensitive) const
{
  WD_ASSERT_DEV(wdStringUtils::StartsWith(szOption, "-"), "All command line option names must start with a hyphen (e.g. -file)");

  for (wdUInt32 i = 0; i < m_Commands.GetCount(); ++i)
  {
    if ((bCaseSensitive && m_Commands[i].IsEqual(szOption)) || (!bCaseSensitive && m_Commands[i].IsEqual_NoCase(szOption)))
      return i;
  }

  return -1;
}

bool wdCommandLineUtils::HasOption(const char* szOption, bool bCaseSensitive /*= false*/) const
{
  return GetOptionIndex(szOption, bCaseSensitive) >= 0;
}

wdUInt32 wdCommandLineUtils::GetStringOptionArguments(const char* szOption, bool bCaseSensitive) const
{
  const wdInt32 iIndex = GetOptionIndex(szOption, bCaseSensitive);

  // not found -> no parameters
  if (iIndex < 0)
    return 0;

  wdUInt32 uiParamCount = 0;

  for (wdUInt32 uiParam = iIndex + 1; uiParam < m_Commands.GetCount(); ++uiParam)
  {
    if (m_Commands[uiParam].StartsWith("-")) // next command is the next option -> no parameters
      break;

    ++uiParamCount;
  }

  return uiParamCount;
}

const char* wdCommandLineUtils::GetStringOption(const char* szOption, wdUInt32 uiArgument, const char* szDefault, bool bCaseSensitive) const
{
  const wdInt32 iIndex = GetOptionIndex(szOption, bCaseSensitive);

  // not found -> no parameters
  if (iIndex < 0)
    return szDefault;

  wdUInt32 uiParamCount = 0;

  for (wdUInt32 uiParam = iIndex + 1; uiParam < m_Commands.GetCount(); ++uiParam)
  {
    if (m_Commands[uiParam].StartsWith("-")) // next command is the next option -> not enough parameters
      return szDefault;

    // found the right one, return it
    if (uiParamCount == uiArgument)
      return m_Commands[uiParam].GetData();

    ++uiParamCount;
  }

  return szDefault;
}

const wdString wdCommandLineUtils::GetAbsolutePathOption(const char* szOption, wdUInt32 uiArgument /*= 0*/, const char* szDefault /*= ""*/, bool bCaseSensitive /*= false*/) const
{
  const char* szPath = GetStringOption(szOption, uiArgument, szDefault, bCaseSensitive);

  if (wdStringUtils::IsNullOrEmpty(szPath))
    return szPath;

  return wdOSFile::MakePathAbsoluteWithCWD(szPath);
}

bool wdCommandLineUtils::GetBoolOption(const char* szOption, bool bDefault, bool bCaseSensitive) const
{
  const wdInt32 iIndex = GetOptionIndex(szOption, bCaseSensitive);

  if (iIndex < 0)
    return bDefault;

  if (iIndex + 1 == m_Commands.GetCount()) // last command, treat this as 'on'
    return true;

  if (m_Commands[iIndex + 1].StartsWith("-")) // next command is the next option -> treat this as 'on' as well
    return true;

  // otherwise try to convert the next option to a boolean
  bool bRes = bDefault;
  wdConversionUtils::StringToBool(m_Commands[iIndex + 1].GetData(), bRes).IgnoreResult();

  return bRes;
}

wdInt32 wdCommandLineUtils::GetIntOption(const char* szOption, wdInt32 iDefault, bool bCaseSensitive) const
{
  const wdInt32 iIndex = GetOptionIndex(szOption, bCaseSensitive);

  if (iIndex < 0)
    return iDefault;

  if (iIndex + 1 == m_Commands.GetCount()) // last command
    return iDefault;

  // try to convert the next option to a number
  wdInt32 iRes = iDefault;
  wdConversionUtils::StringToInt(m_Commands[iIndex + 1].GetData(), iRes).IgnoreResult();

  return iRes;
}

wdUInt32 wdCommandLineUtils::GetUIntOption(const char* szOption, wdUInt32 uiDefault, bool bCaseSensitive) const
{
  const wdInt32 iIndex = GetOptionIndex(szOption, bCaseSensitive);

  if (iIndex < 0)
    return uiDefault;

  if (iIndex + 1 == m_Commands.GetCount()) // last command
    return uiDefault;

  // try to convert the next option to a number
  wdUInt32 uiRes = uiDefault;
  wdConversionUtils::StringToUInt(m_Commands[iIndex + 1].GetData(), uiRes).IgnoreResult();

  return uiRes;
}

double wdCommandLineUtils::GetFloatOption(const char* szOption, double fDefault, bool bCaseSensitive) const
{
  const wdInt32 iIndex = GetOptionIndex(szOption, bCaseSensitive);

  if (iIndex < 0)
    return fDefault;

  if (iIndex + 1 == m_Commands.GetCount()) // last command
    return fDefault;

  // try to convert the next option to a number
  double fRes = fDefault;
  wdConversionUtils::StringToFloat(m_Commands[iIndex + 1].GetData(), fRes).IgnoreResult();

  return fRes;
}

void wdCommandLineUtils::InjectCustomArgument(const char* szArgument)
{
  m_Commands.PushBack(szArgument);
}

WD_STATICLINK_FILE(Foundation, Foundation_Utilities_Implementation_CommandLineUtils);
