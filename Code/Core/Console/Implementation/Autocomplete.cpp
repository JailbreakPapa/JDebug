#include <Core/CorePCH.h>

#include <Core/Console/Console.h>
#include <Core/Console/QuakeConsole.h>

void nsCommandInterpreter::FindPossibleCVars(nsStringView sVariable, nsDeque<nsString>& inout_autoCompleteOptions, nsDeque<nsConsoleString>& inout_autoCompleteDescriptions)
{
  nsStringBuilder sText;

  nsCVar* pCVar = nsCVar::GetFirstInstance();
  while (pCVar)
  {
    if (pCVar->GetName().StartsWith_NoCase(sVariable))
    {
      sText.SetFormat("    {0} = {1}", pCVar->GetName(), nsQuakeConsole::GetFullInfoAsString(pCVar));

      nsConsoleString cs;
      cs.m_sText = sText;
      cs.m_Type = nsConsoleString::Type::VarName;
      inout_autoCompleteDescriptions.PushBack(cs);

      inout_autoCompleteOptions.PushBack(pCVar->GetName());
    }

    pCVar = pCVar->GetNextInstance();
  }
}

void nsCommandInterpreter::FindPossibleFunctions(nsStringView sVariable, nsDeque<nsString>& inout_autoCompleteOptions, nsDeque<nsConsoleString>& inout_autoCompleteDescriptions)
{
  nsStringBuilder sText;

  nsConsoleFunctionBase* pFunc = nsConsoleFunctionBase::GetFirstInstance();
  while (pFunc)
  {
    if (pFunc->GetName().StartsWith_NoCase(sVariable))
    {
      sText.SetFormat("    {0} {1}", pFunc->GetName(), pFunc->GetDescription());

      nsConsoleString cs;
      cs.m_sText = sText;
      cs.m_Type = nsConsoleString::Type::FuncName;
      inout_autoCompleteDescriptions.PushBack(cs);

      inout_autoCompleteOptions.PushBack(pFunc->GetName());
    }

    pFunc = pFunc->GetNextInstance();
  }
}


const nsString nsQuakeConsole::GetValueAsString(nsCVar* pCVar)
{
  nsStringBuilder s = "undefined";

  switch (pCVar->GetType())
  {
    case nsCVarType::Int:
    {
      nsCVarInt* pInt = static_cast<nsCVarInt*>(pCVar);
      s.SetFormat("{0}", pInt->GetValue());
    }
    break;

    case nsCVarType::Bool:
    {
      nsCVarBool* pBool = static_cast<nsCVarBool*>(pCVar);
      if (pBool->GetValue() == true)
        s = "true";
      else
        s = "false";
    }
    break;

    case nsCVarType::String:
    {
      nsCVarString* pString = static_cast<nsCVarString*>(pCVar);
      s.SetFormat("\"{0}\"", pString->GetValue());
    }
    break;

    case nsCVarType::Float:
    {
      nsCVarFloat* pFloat = static_cast<nsCVarFloat*>(pCVar);
      s.SetFormat("{0}", nsArgF(pFloat->GetValue(), 3));
    }
    break;

    case nsCVarType::ENUM_COUNT:
      break;
  }

  return s.GetData();
}

nsString nsQuakeConsole::GetFullInfoAsString(nsCVar* pCVar)
{
  nsStringBuilder s = GetValueAsString(pCVar);

  const bool bAnyFlags = pCVar->GetFlags().IsAnySet(nsCVarFlags::Save | nsCVarFlags::ShowRequiresRestartMsg);

  if (bAnyFlags)
    s.Append(" [ ");

  if (pCVar->GetFlags().IsAnySet(nsCVarFlags::Save))
    s.Append("SAVE ");

  if (pCVar->GetFlags().IsAnySet(nsCVarFlags::ShowRequiresRestartMsg))
    s.Append("RESTART ");

  if (bAnyFlags)
    s.Append("]");

  return s;
}

const nsString nsCommandInterpreter::FindCommonString(const nsDeque<nsString>& strings)
{
  nsStringBuilder sCommon;
  nsUInt32 c;

  nsUInt32 uiPos = 0;
  auto it1 = strings[0].GetIteratorFront();
  while (it1.IsValid())
  {
    c = it1.GetCharacter();

    for (int v = 1; v < (int)strings.GetCount(); v++)
    {
      auto it2 = strings[v].GetIteratorFront();

      it2 += uiPos;

      if (it2.GetCharacter() != c)
        return sCommon;
    }

    sCommon.Append(c);

    ++uiPos;
    ++it1;
  }

  return sCommon;
}

void nsCommandInterpreter::AutoComplete(nsCommandInterpreterState& inout_state)
{
  nsString sVarName = inout_state.m_sInput;

  auto it = rbegin(inout_state.m_sInput);

  // dots are allowed in CVar names
  while (it.IsValid() && (it.GetCharacter() == '.' || !nsStringUtils::IsIdentifierDelimiter_C_Code(*it)))
    ++it;

  const char* szLastWordDelimiter = nullptr;
  if (it.IsValid() && nsStringUtils::IsIdentifierDelimiter_C_Code(*it) && it.GetCharacter() != '.')
    szLastWordDelimiter = it.GetData();

  if (szLastWordDelimiter != nullptr)
    sVarName = szLastWordDelimiter + 1;

  nsDeque<nsString> AutoCompleteOptions;
  nsDeque<nsConsoleString> AutoCompleteDescriptions;

  FindPossibleCVars(sVarName.GetData(), AutoCompleteOptions, AutoCompleteDescriptions);
  FindPossibleFunctions(sVarName.GetData(), AutoCompleteOptions, AutoCompleteDescriptions);

  if (AutoCompleteDescriptions.GetCount() > 1)
  {
    AutoCompleteDescriptions.Sort();

    inout_state.AddOutputLine("");

    for (nsUInt32 i = 0; i < AutoCompleteDescriptions.GetCount(); i++)
    {
      inout_state.AddOutputLine(AutoCompleteDescriptions[i].m_sText.GetData(), AutoCompleteDescriptions[i].m_Type);
    }

    inout_state.AddOutputLine("");
  }

  if (AutoCompleteOptions.GetCount() > 0)
  {
    if (szLastWordDelimiter != nullptr)
      inout_state.m_sInput = nsStringView(inout_state.m_sInput.GetData(), szLastWordDelimiter + 1);
    else
      inout_state.m_sInput.Clear();

    inout_state.m_sInput.Append(FindCommonString(AutoCompleteOptions).GetData());
  }
}
