#include <Core/CorePCH.h>

#include <Core/Console/LuaInterpreter.h>
#include <Core/Console/QuakeConsole.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>

NS_ENUMERABLE_CLASS_IMPLEMENTATION(nsConsoleFunctionBase);

nsQuakeConsole::nsQuakeConsole()
{
  ClearInputLine();

  m_bLogOutputEnabled = false;
  m_bDefaultInputHandlingInitialized = false;
  m_uiMaxConsoleStrings = 1000;

  EnableLogOutput(true);

#ifdef BUILDSYSTEM_ENABLE_LUA_SUPPORT
  SetCommandInterpreter(NS_DEFAULT_NEW(nsCommandInterpreterLua));
#endif
}

nsQuakeConsole::~nsQuakeConsole()
{
  EnableLogOutput(false);
}

void nsQuakeConsole::AddConsoleString(nsStringView sText, nsConsoleString::Type type)
{
  NS_LOCK(m_Mutex);

  m_ConsoleStrings.PushFront();

  nsConsoleString& cs = m_ConsoleStrings.PeekFront();
  cs.m_sText = sText;
  cs.m_Type = type;

  if (m_ConsoleStrings.GetCount() > m_uiMaxConsoleStrings)
    m_ConsoleStrings.PopBack(m_ConsoleStrings.GetCount() - m_uiMaxConsoleStrings);

  nsConsole::AddConsoleString(sText, type);
}

const nsDeque<nsConsoleString>& nsQuakeConsole::GetConsoleStrings() const
{
  if (m_bUseFilteredStrings)
  {
    return m_FilteredConsoleStrings;
  }

  return m_ConsoleStrings;
}

void nsQuakeConsole::LogHandler(const nsLoggingEventData& data)
{
  nsConsoleString::Type type = nsConsoleString::Type::Default;

  switch (data.m_EventType)
  {
    case nsLogMsgType::GlobalDefault:
    case nsLogMsgType::Flush:
    case nsLogMsgType::BeginGroup:
    case nsLogMsgType::EndGroup:
    case nsLogMsgType::None:
    case nsLogMsgType::ENUM_COUNT:
    case nsLogMsgType::All:
      return;

    case nsLogMsgType::ErrorMsg:
      type = nsConsoleString::Type::Error;
      break;

    case nsLogMsgType::SeriousWarningMsg:
      type = nsConsoleString::Type::SeriousWarning;
      break;

    case nsLogMsgType::WarningMsg:
      type = nsConsoleString::Type::Warning;
      break;

    case nsLogMsgType::SuccessMsg:
      type = nsConsoleString::Type::Success;
      break;

    case nsLogMsgType::InfoMsg:
      break;

    case nsLogMsgType::DevMsg:
      type = nsConsoleString::Type::Dev;
      break;

    case nsLogMsgType::DebugMsg:
      type = nsConsoleString::Type::Debug;
      break;
  }

  nsStringBuilder sFormat;
  sFormat.SetPrintf("%*s", data.m_uiIndentation, "");
  sFormat.Append(data.m_sText);

  AddConsoleString(sFormat.GetData(), type);
}

void nsQuakeConsole::InputStringChanged()
{
  m_bUseFilteredStrings = false;
  m_FilteredConsoleStrings.Clear();

  if (m_sInputLine.StartsWith("*"))
  {
    nsStringBuilder input = m_sInputLine;

    input.Shrink(1, 0);
    input.Trim(" ");

    if (input.IsEmpty())
      return;

    m_FilteredConsoleStrings.Clear();
    m_bUseFilteredStrings = true;

    for (const auto& e : m_ConsoleStrings)
    {
      if (e.m_sText.FindSubString_NoCase(input))
      {
        m_FilteredConsoleStrings.PushBack(e);
      }
    }

    Scroll(0); // clamp scroll position
  }
}

void nsQuakeConsole::EnableLogOutput(bool bEnable)
{
  if (m_bLogOutputEnabled == bEnable)
    return;

  m_bLogOutputEnabled = bEnable;

  if (bEnable)
  {
    nsGlobalLog::AddLogWriter(nsMakeDelegate(&nsQuakeConsole::LogHandler, this));
  }
  else
  {
    nsGlobalLog::RemoveLogWriter(nsMakeDelegate(&nsQuakeConsole::LogHandler, this));
  }
}

void nsQuakeConsole::SaveState(nsStreamWriter& inout_stream) const
{
  NS_LOCK(m_Mutex);

  const nsUInt8 uiVersion = 1;
  inout_stream << uiVersion;

  inout_stream << m_InputHistory.GetCount();
  for (nsUInt32 i = 0; i < m_InputHistory.GetCount(); ++i)
  {
    inout_stream << m_InputHistory[i];
  }

  inout_stream << m_BoundKeys.GetCount();
  for (auto it = m_BoundKeys.GetIterator(); it.IsValid(); ++it)
  {
    inout_stream << it.Key();
    inout_stream << it.Value();
  }
}

void nsQuakeConsole::LoadState(nsStreamReader& inout_stream)
{
  NS_LOCK(m_Mutex);

  nsUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  if (uiVersion == 1)
  {
    nsUInt32 count = 0;
    inout_stream >> count;
    m_InputHistory.SetCount(count);

    for (nsUInt32 i = 0; i < m_InputHistory.GetCount(); ++i)
    {
      inout_stream >> m_InputHistory[i];
    }

    inout_stream >> count;

    nsString sKey;
    nsString sValue;

    for (nsUInt32 i = 0; i < count; ++i)
    {
      inout_stream >> sKey;
      inout_stream >> sValue;

      m_BoundKeys[sKey] = sValue;
    }
  }
}

void nsCommandInterpreterState::AddOutputLine(const nsFormatString& text, nsConsoleString::Type type /*= nsCommandOutputLine::Type::Default*/)
{
  auto& line = m_sOutput.ExpandAndGetRef();
  line.m_Type = type;

  nsStringBuilder tmp;
  line.m_sText = text.GetText(tmp);
}

nsColor nsConsoleString::GetColor() const
{
  switch (m_Type)
  {
    case nsConsoleString::Type::Default:
      return nsColor::White;

    case nsConsoleString::Type::Error:
      return nsColor(1.0f, 0.2f, 0.2f);

    case nsConsoleString::Type::SeriousWarning:
      return nsColor(1.0f, 0.4f, 0.1f);

    case nsConsoleString::Type::Warning:
      return nsColor(1.0f, 0.6f, 0.1f);

    case nsConsoleString::Type::Note:
      return nsColor(1, 200.0f / 255.0f, 0);

    case nsConsoleString::Type::Success:
      return nsColor(0.1f, 1.0f, 0.1f);

    case nsConsoleString::Type::Executed:
      return nsColor(1.0f, 0.5f, 0.0f);

    case nsConsoleString::Type::VarName:
      return nsColorGammaUB(255, 210, 0);

    case nsConsoleString::Type::FuncName:
      return nsColorGammaUB(100, 255, 100);

    case nsConsoleString::Type::Dev:
      return nsColor(0.6f, 0.6f, 0.6f);

    case nsConsoleString::Type::Debug:
      return nsColor(0.4f, 0.6f, 0.8f);

      NS_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  return nsColor::White;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

nsConsole::nsConsole() = default;

nsConsole::~nsConsole()
{
  if (s_pMainConsole == this)
  {
    s_pMainConsole = nullptr;
  }
}

void nsConsole::SetMainConsole(nsConsole* pConsole)
{
  s_pMainConsole = pConsole;
}

nsConsole* nsConsole::GetMainConsole()
{
  return s_pMainConsole;
}

nsConsole* nsConsole::s_pMainConsole = nullptr;

bool nsConsole::AutoComplete(nsStringBuilder& ref_sText)
{
  NS_LOCK(m_Mutex);

  if (m_pCommandInterpreter)
  {
    nsCommandInterpreterState s;
    s.m_sInput = ref_sText;

    m_pCommandInterpreter->AutoComplete(s);

    for (auto& l : s.m_sOutput)
    {
      AddConsoleString(l.m_sText, l.m_Type);
    }

    if (ref_sText != s.m_sInput)
    {
      ref_sText = s.m_sInput;
      return true;
    }
  }

  return false;
}

void nsConsole::ExecuteCommand(nsStringView sInput)
{
  if (sInput.IsEmpty())
    return;

  NS_LOCK(m_Mutex);

  if (m_pCommandInterpreter)
  {
    nsCommandInterpreterState s;
    s.m_sInput = sInput;
    m_pCommandInterpreter->Interpret(s);

    for (auto& l : s.m_sOutput)
    {
      AddConsoleString(l.m_sText, l.m_Type);
    }
  }
  else
  {
    AddConsoleString(sInput);
  }
}

void nsConsole::AddConsoleString(nsStringView sText, nsConsoleString::Type type /*= nsConsoleString::Type::Default*/)
{
  nsConsoleString cs;
  cs.m_sText = sText;
  cs.m_Type = type;

  // Broadcast that we have added a string to the console
  nsConsoleEvent e;
  e.m_Type = nsConsoleEvent::Type::OutputLineAdded;
  e.m_AddedpConsoleString = &cs;

  m_Events.Broadcast(e);
}

void nsConsole::AddToInputHistory(nsStringView sText)
{
  NS_LOCK(m_Mutex);

  m_iCurrentInputHistoryElement = -1;

  if (sText.IsEmpty())
    return;

  for (nsInt32 i = 0; i < (nsInt32)m_InputHistory.GetCount(); i++)
  {
    if (m_InputHistory[i] == sText) // already in the History
    {
      // just move it to the front

      for (nsInt32 j = i - 1; j >= 0; j--)
        m_InputHistory[j + 1] = m_InputHistory[j];

      m_InputHistory[0] = sText;
      return;
    }
  }

  m_InputHistory.SetCount(nsMath::Min<nsUInt32>(m_InputHistory.GetCount() + 1, m_InputHistory.GetCapacity()));

  for (nsUInt32 i = m_InputHistory.GetCount() - 1; i > 0; i--)
    m_InputHistory[i] = m_InputHistory[i - 1];

  m_InputHistory[0] = sText;
}

void nsConsole::RetrieveInputHistory(nsInt32 iHistoryUp, nsStringBuilder& ref_sResult)
{
  NS_LOCK(m_Mutex);

  if (m_InputHistory.IsEmpty())
    return;

  m_iCurrentInputHistoryElement = nsMath::Clamp<nsInt32>(m_iCurrentInputHistoryElement + iHistoryUp, 0, m_InputHistory.GetCount() - 1);

  if (!m_InputHistory[m_iCurrentInputHistoryElement].IsEmpty())
  {
    ref_sResult = m_InputHistory[m_iCurrentInputHistoryElement];
  }
}

nsResult nsConsole::SaveInputHistory(nsStringView sFile)
{
  nsFileWriter file;
  NS_SUCCEED_OR_RETURN(file.Open(sFile));

  nsStringBuilder str;

  for (const nsString& line : m_InputHistory)
  {
    if (line.IsEmpty())
      continue;

    str.Set(line, "\n");

    NS_SUCCEED_OR_RETURN(file.WriteBytes(str.GetData(), str.GetElementCount()));
  }

  return NS_SUCCESS;
}

void nsConsole::LoadInputHistory(nsStringView sFile)
{
  nsFileReader file;
  if (file.Open(sFile).Failed())
    return;

  nsStringBuilder str;
  str.ReadAll(file);

  nsHybridArray<nsStringView, 32> lines;
  str.Split(false, lines, "\n", "\r");

  for (nsUInt32 i = 0; i < lines.GetCount(); ++i)
  {
    AddToInputHistory(lines[lines.GetCount() - 1 - i]);
  }
}
