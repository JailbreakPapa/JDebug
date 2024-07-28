#include <Core/CorePCH.h>

#include <Core/Console/QuakeConsole.h>
#include <Foundation/Configuration/CVar.h>

void nsQuakeConsole::ExecuteCommand(nsStringView sInput)
{
  const bool bBind = sInput.StartsWith_NoCase("bind ");
  const bool bUnbind = sInput.StartsWith_NoCase("unbind ");

  if (bBind || bUnbind)
  {
    nsStringBuilder tmp;
    const char* szAfterCmd = nsStringUtils::FindWordEnd(sInput.GetData(tmp), nsStringUtils::IsWhiteSpace);              // skip the word 'bind' or 'unbind'

    const char* szKeyNameStart = nsStringUtils::SkipCharacters(szAfterCmd, nsStringUtils::IsWhiteSpace);                // go to the next word
    const char* szKeyNameEnd = nsStringUtils::FindWordEnd(szKeyNameStart, nsStringUtils::IsIdentifierDelimiter_C_Code); // find its end

    nsStringView sKey(szKeyNameStart, szKeyNameEnd);
    tmp = sKey;                                                                                                         // copy the word into a zero terminated string

    const char* szCommandToBind = nsStringUtils::SkipCharacters(szKeyNameEnd, nsStringUtils::IsWhiteSpace);

    if (bUnbind || nsStringUtils::IsNullOrEmpty(szCommandToBind))
    {
      UnbindKey(tmp);
      return;
    }

    BindKey(tmp, szCommandToBind);
    return;
  }

  nsConsole::ExecuteCommand(sInput);
}

void nsQuakeConsole::BindKey(nsStringView sKey, nsStringView sCommand)
{
  nsStringBuilder s;
  s.SetFormat("Binding key '{0}' to command '{1}'", sKey, sCommand);
  AddConsoleString(s, nsConsoleString::Type::Success);

  m_BoundKeys[sKey] = sCommand;
}

void nsQuakeConsole::UnbindKey(nsStringView sKey)
{
  nsStringBuilder s;
  s.SetFormat("Unbinding key '{0}'", sKey);
  AddConsoleString(s, nsConsoleString::Type::Success);

  m_BoundKeys.Remove(sKey);
}

void nsQuakeConsole::ExecuteBoundKey(nsStringView sKey)
{
  auto it = m_BoundKeys.Find(sKey);

  if (it.IsValid())
  {
    ExecuteCommand(it.Value());
  }
}
