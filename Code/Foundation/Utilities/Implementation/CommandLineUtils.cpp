#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/OSFile.h>
#include <Foundation/Utilities/CommandLineUtils.h>
#include <Foundation/Utilities/ConversionUtils.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#  include <shellapi.h>
#endif

static nsCommandLineUtils g_pCmdLineInstance;

nsCommandLineUtils* nsCommandLineUtils::GetGlobalInstance()
{
  return &g_pCmdLineInstance;
}

void nsCommandLineUtils::SplitCommandLineString(const char* szCommandString, bool bAddExecutableDir, nsDynamicArray<nsString>& out_args, nsDynamicArray<const char*>& out_argsV)
{
  // Add application dir as first argument as customary on other platforms.
  if (bAddExecutableDir)
  {
#if NS_ENABLED(NS_PLATFORM_WINDOWS)
    wchar_t moduleFilename[256];
    GetModuleFileNameW(nullptr, moduleFilename, 256);
    out_args.PushBack(nsStringUtf8(moduleFilename).GetData());
#else
    NS_ASSERT_NOT_IMPLEMENTED;
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
      nsStringBuilder path = nsStringView(lastEnd, currentChar);
      path.Trim(" \"");
      out_args.PushBack(path);
      lastEnd = currentChar + 1;
    }
    nsUnicodeUtils::MoveToNextUtf8(currentChar).IgnoreResult();
  }

  out_argsV.Reserve(out_argsV.GetCount());
  for (nsString& str : out_args)
    out_argsV.PushBack(str.GetData());
}

void nsCommandLineUtils::SetCommandLine(nsUInt32 uiArgc, const char** pArgv, ArgMode mode /*= UseArgcArgv*/)
{
#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
  if (mode == ArgMode::PreferOsArgs)
  {
    SetCommandLine();
    return;
  }
#endif

  m_Commands.Clear();
  m_Commands.Reserve(uiArgc);

  for (nsUInt32 i = 0; i < uiArgc; ++i)
    m_Commands.PushBack(pArgv[i]);
}

void nsCommandLineUtils::SetCommandLine(nsArrayPtr<nsString> commands)
{
  m_Commands = commands;
}

#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)

void nsCommandLineUtils::SetCommandLine()
{
  int argc = 0;

  LPWSTR* argvw = CommandLineToArgvW(::GetCommandLineW(), &argc);

  NS_ASSERT_RELEASE(argvw != nullptr, "CommandLineToArgvW failed");

  nsArrayPtr<nsStringUtf8> ArgvUtf8 = NS_DEFAULT_NEW_ARRAY(nsStringUtf8, argc);
  nsArrayPtr<const char*> argv = NS_DEFAULT_NEW_ARRAY(const char*, argc);

  for (nsInt32 i = 0; i < argc; ++i)
  {
    ArgvUtf8[i] = argvw[i];
    argv[i] = ArgvUtf8[i].GetData();
  }

  SetCommandLine(argc, argv.GetPtr(), ArgMode::UseArgcArgv);


  NS_DEFAULT_DELETE_ARRAY(ArgvUtf8);
  NS_DEFAULT_DELETE_ARRAY(argv);
  LocalFree(argvw);
}

#elif NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
// Not implemented on Windows UWP.
#elif NS_ENABLED(NS_PLATFORM_OSX)
// Not implemented on OSX.
#elif NS_ENABLED(NS_PLATFORM_LINUX)
// Not implemented on Linux.
#elif NS_ENABLED(NS_PLATFORM_ANDROID)
// Not implemented on Android.
#elif NS_ENABLED(NS_PLATFORM_PLAYSTATION_5)
#else
#  error "nsCommandLineUtils::SetCommandLine(): Abstraction missing."
#endif

const nsDynamicArray<nsString>& nsCommandLineUtils::GetCommandLineArray() const
{
  return m_Commands;
}

nsString nsCommandLineUtils::GetCommandLineString() const
{
  nsStringBuilder commandLine;
  for (const nsString& command : m_Commands)
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

nsUInt32 nsCommandLineUtils::GetParameterCount() const
{
  return m_Commands.GetCount();
}

const nsString& nsCommandLineUtils::GetParameter(nsUInt32 uiParam) const
{
  return m_Commands[uiParam];
}

nsInt32 nsCommandLineUtils::GetOptionIndex(nsStringView sOption, bool bCaseSensitive) const
{
  NS_ASSERT_DEV(sOption.StartsWith("-"), "All command line option names must start with a hyphen (e.g. -file)");

  for (nsUInt32 i = 0; i < m_Commands.GetCount(); ++i)
  {
    if ((bCaseSensitive && m_Commands[i].IsEqual(sOption)) || (!bCaseSensitive && m_Commands[i].IsEqual_NoCase(sOption)))
      return i;
  }

  return -1;
}

bool nsCommandLineUtils::HasOption(nsStringView sOption, bool bCaseSensitive /*= false*/) const
{
  return GetOptionIndex(sOption, bCaseSensitive) >= 0;
}

nsUInt32 nsCommandLineUtils::GetStringOptionArguments(nsStringView sOption, bool bCaseSensitive) const
{
  const nsInt32 iIndex = GetOptionIndex(sOption, bCaseSensitive);

  // not found -> no parameters
  if (iIndex < 0)
    return 0;

  nsUInt32 uiParamCount = 0;

  for (nsUInt32 uiParam = iIndex + 1; uiParam < m_Commands.GetCount(); ++uiParam)
  {
    if (m_Commands[uiParam].StartsWith("-")) // next command is the next option -> no parameters
      break;

    ++uiParamCount;
  }

  return uiParamCount;
}

nsStringView nsCommandLineUtils::GetStringOption(nsStringView sOption, nsUInt32 uiArgument, nsStringView sDefault, bool bCaseSensitive) const
{
  const nsInt32 iIndex = GetOptionIndex(sOption, bCaseSensitive);

  // not found -> no parameters
  if (iIndex < 0)
    return sDefault;

  nsUInt32 uiParamCount = 0;

  for (nsUInt32 uiParam = iIndex + 1; uiParam < m_Commands.GetCount(); ++uiParam)
  {
    if (m_Commands[uiParam].StartsWith("-")) // next command is the next option -> not enough parameters
      return sDefault;

    // found the right one, return it
    if (uiParamCount == uiArgument)
      return m_Commands[uiParam].GetData();

    ++uiParamCount;
  }

  return sDefault;
}

const nsString nsCommandLineUtils::GetAbsolutePathOption(nsStringView sOption, nsUInt32 uiArgument /*= 0*/, nsStringView sDefault /*= {} */, bool bCaseSensitive /*= false*/) const
{
  nsStringView sPath = GetStringOption(sOption, uiArgument, sDefault, bCaseSensitive);

  if (sPath.IsEmpty())
    return sPath;

  return nsOSFile::MakePathAbsoluteWithCWD(sPath);
}

bool nsCommandLineUtils::GetBoolOption(nsStringView sOption, bool bDefault, bool bCaseSensitive) const
{
  const nsInt32 iIndex = GetOptionIndex(sOption, bCaseSensitive);

  if (iIndex < 0)
    return bDefault;

  if (iIndex + 1 == m_Commands.GetCount())    // last command, treat this as 'on'
    return true;

  if (m_Commands[iIndex + 1].StartsWith("-")) // next command is the next option -> treat this as 'on' as well
    return true;

  // otherwise try to convert the next option to a boolean
  bool bRes = bDefault;
  nsConversionUtils::StringToBool(m_Commands[iIndex + 1].GetData(), bRes).IgnoreResult();

  return bRes;
}

nsInt32 nsCommandLineUtils::GetIntOption(nsStringView sOption, nsInt32 iDefault, bool bCaseSensitive) const
{
  const nsInt32 iIndex = GetOptionIndex(sOption, bCaseSensitive);

  if (iIndex < 0)
    return iDefault;

  if (iIndex + 1 == m_Commands.GetCount()) // last command
    return iDefault;

  // try to convert the next option to a number
  nsInt32 iRes = iDefault;
  nsConversionUtils::StringToInt(m_Commands[iIndex + 1].GetData(), iRes).IgnoreResult();

  return iRes;
}

nsUInt32 nsCommandLineUtils::GetUIntOption(nsStringView sOption, nsUInt32 uiDefault, bool bCaseSensitive) const
{
  const nsInt32 iIndex = GetOptionIndex(sOption, bCaseSensitive);

  if (iIndex < 0)
    return uiDefault;

  if (iIndex + 1 == m_Commands.GetCount()) // last command
    return uiDefault;

  // try to convert the next option to a number
  nsUInt32 uiRes = uiDefault;
  nsConversionUtils::StringToUInt(m_Commands[iIndex + 1].GetData(), uiRes).IgnoreResult();

  return uiRes;
}

double nsCommandLineUtils::GetFloatOption(nsStringView sOption, double fDefault, bool bCaseSensitive) const
{
  const nsInt32 iIndex = GetOptionIndex(sOption, bCaseSensitive);

  if (iIndex < 0)
    return fDefault;

  if (iIndex + 1 == m_Commands.GetCount()) // last command
    return fDefault;

  // try to convert the next option to a number
  double fRes = fDefault;
  nsConversionUtils::StringToFloat(m_Commands[iIndex + 1].GetData(), fRes).IgnoreResult();

  return fRes;
}

void nsCommandLineUtils::InjectCustomArgument(nsStringView sArgument)
{
  m_Commands.PushBack(sArgument);
}
