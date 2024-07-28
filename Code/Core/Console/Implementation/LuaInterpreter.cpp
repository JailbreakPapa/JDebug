#include <Core/CorePCH.h>

#include <Core/Console/LuaInterpreter.h>
#include <Core/Console/QuakeConsole.h>
#include <Core/Scripting/LuaWrapper.h>

#ifdef BUILDSYSTEM_ENABLE_LUA_SUPPORT

static void AllowScriptCVarAccess(nsLuaWrapper& ref_script);

static const nsString GetNextWord(nsStringView& ref_sString)
{
  const char* szStartWord = nsStringUtils::SkipCharacters(ref_sString.GetStartPointer(), nsStringUtils::IsWhiteSpace, false);
  const char* szEndWord = nsStringUtils::FindWordEnd(szStartWord, nsStringUtils::IsIdentifierDelimiter_C_Code, true);

  ref_sString = nsStringView(szEndWord);

  return nsStringView(szStartWord, szEndWord);
}

static nsString GetRestWords(nsStringView sString)
{
  return nsStringUtils::SkipCharacters(sString.GetStartPointer(), nsStringUtils::IsWhiteSpace, false);
}

static int LUAFUNC_ConsoleFunc(lua_State* pState)
{
  nsLuaWrapper s(pState);

  nsConsoleFunctionBase* pFunc = (nsConsoleFunctionBase*)s.GetFunctionLightUserData();

  if (pFunc->GetNumParameters() != s.GetNumberOfFunctionParameters())
  {
    nsLog::Error("Function '{0}' expects {1} parameters, {2} were provided.", pFunc->GetName(), pFunc->GetNumParameters(), s.GetNumberOfFunctionParameters());
    return s.ReturnToScript();
  }

  nsHybridArray<nsVariant, 8> m_Params;
  m_Params.SetCount(pFunc->GetNumParameters());

  for (nsUInt32 p = 0; p < pFunc->GetNumParameters(); ++p)
  {
    switch (pFunc->GetParameterType(p))
    {
      case nsVariant::Type::Bool:
        m_Params[p] = s.GetBoolParameter(p);
        break;
      case nsVariant::Type::Int8:
      case nsVariant::Type::Int16:
      case nsVariant::Type::Int32:
      case nsVariant::Type::Int64:
      case nsVariant::Type::UInt8:
      case nsVariant::Type::UInt16:
      case nsVariant::Type::UInt32:
      case nsVariant::Type::UInt64:
        m_Params[p] = s.GetIntParameter(p);
        break;
      case nsVariant::Type::Float:
      case nsVariant::Type::Double:
        m_Params[p] = s.GetFloatParameter(p);
        break;
      case nsVariant::Type::String:
        m_Params[p] = s.GetStringParameter(p);
        break;
      default:
        nsLog::Error("Function '{0}': Type of parameter {1} is not supported by the Lua interpreter.", pFunc->GetName(), p);
        return s.ReturnToScript();
    }
  }

  if (!m_Params.IsEmpty())
    pFunc->Call(nsArrayPtr<nsVariant>(&m_Params[0], m_Params.GetCount())).IgnoreResult();
  else
    pFunc->Call(nsArrayPtr<nsVariant>()).IgnoreResult();

  return s.ReturnToScript();
}

static void SanitizeCVarNames(nsStringBuilder& ref_sCommand)
{
  nsStringBuilder sanitizedCVarName;

  for (const nsCVar* pCVar = nsCVar::GetFirstInstance(); pCVar != nullptr; pCVar = pCVar->GetNextInstance())
  {
    sanitizedCVarName = pCVar->GetName();
    sanitizedCVarName.ReplaceAll(".", "_");

    ref_sCommand.ReplaceAll(pCVar->GetName(), sanitizedCVarName);
  }
}

static void UnSanitizeCVarName(nsStringBuilder& ref_sCvarName)
{
  nsStringBuilder sanitizedCVarName;

  for (const nsCVar* pCVar = nsCVar::GetFirstInstance(); pCVar != nullptr; pCVar = pCVar->GetNextInstance())
  {
    sanitizedCVarName = pCVar->GetName();
    sanitizedCVarName.ReplaceAll(".", "_");

    if (ref_sCvarName == sanitizedCVarName)
    {
      ref_sCvarName = pCVar->GetName();
      return;
    }
  }
}

void nsCommandInterpreterLua::Interpret(nsCommandInterpreterState& inout_state)
{
  inout_state.m_sOutput.Clear();

  nsStringBuilder sRealCommand = inout_state.m_sInput;

  if (sRealCommand.IsEmpty())
  {
    inout_state.AddOutputLine("");
    return;
  }

  sRealCommand.Trim(" \t\n\r");
  nsStringBuilder sSanitizedCommand = sRealCommand;
  SanitizeCVarNames(sSanitizedCommand);

  nsStringView sCommandIt = sSanitizedCommand;

  const nsString sSanitizedVarName = GetNextWord(sCommandIt);
  nsStringBuilder sRealVarName = sSanitizedVarName;
  UnSanitizeCVarName(sRealVarName);

  while (nsStringUtils::IsWhiteSpace(sCommandIt.GetCharacter()))
  {
    sCommandIt.Shrink(1, 0);
  }

  const bool bSetValue = sCommandIt.StartsWith("=");

  if (bSetValue)
  {
    sCommandIt.Shrink(1, 0);
  }

  nsStringBuilder sValue = GetRestWords(sCommandIt);
  bool bValueEmpty = sValue.IsEmpty();

  nsStringBuilder sTemp;

  nsLuaWrapper Script;
  AllowScriptCVarAccess(Script);

  // Register all ConsoleFunctions
  {
    nsConsoleFunctionBase* pFunc = nsConsoleFunctionBase::GetFirstInstance();
    while (pFunc)
    {
      Script.RegisterCFunction(pFunc->GetName().GetData(sTemp), LUAFUNC_ConsoleFunc, pFunc);

      pFunc = pFunc->GetNextInstance();
    }
  }

  sTemp = "> ";
  sTemp.Append(sRealCommand);
  inout_state.AddOutputLine(sTemp, nsConsoleString::Type::Executed);

  nsCVar* pCVAR = nsCVar::FindCVarByName(sRealVarName.GetData());
  if (pCVAR != nullptr)
  {
    if ((bSetValue) && (sValue == "") && (pCVAR->GetType() == nsCVarType::Bool))
    {
      // someone typed "myvar =" -> on bools this is the short form for "myvar = not myvar" (toggle), so insert the rest here

      bValueEmpty = false;

      sSanitizedCommand.AppendFormat(" not {0}", sSanitizedVarName);
    }

    if (bSetValue && !bValueEmpty)
    {
      nsMuteLog muteLog;

      if (Script.ExecuteString(sSanitizedCommand, "console", &muteLog).Failed())
      {
        inout_state.AddOutputLine("  Error Executing Command.", nsConsoleString::Type::Error);
        return;
      }
      else
      {
        if (pCVAR->GetFlags().IsAnySet(nsCVarFlags::ShowRequiresRestartMsg))
        {
          inout_state.AddOutputLine("  This change takes only effect after a restart.", nsConsoleString::Type::Note);
        }

        sTemp.SetFormat("  {0} = {1}", sRealVarName, nsQuakeConsole::GetFullInfoAsString(pCVAR));
        inout_state.AddOutputLine(sTemp, nsConsoleString::Type::Success);
      }
    }
    else
    {
      sTemp.SetFormat("{0} = {1}", sRealVarName, nsQuakeConsole::GetFullInfoAsString(pCVAR));
      inout_state.AddOutputLine(sTemp);

      if (!pCVAR->GetDescription().IsEmpty())
      {
        sTemp.SetFormat("  Description: {0}", pCVAR->GetDescription());
        inout_state.AddOutputLine(sTemp, nsConsoleString::Type::Success);
      }
      else
        inout_state.AddOutputLine("  No Description available.", nsConsoleString::Type::Success);
    }

    return;
  }
  else
  {
    nsMuteLog muteLog;

    if (Script.ExecuteString(sSanitizedCommand, "console", &muteLog).Failed())
    {
      inout_state.AddOutputLine("  Error Executing Command.", nsConsoleString::Type::Error);
      return;
    }
  }
}

static int LUAFUNC_ReadCVAR(lua_State* pState)
{
  nsLuaWrapper s(pState);

  nsStringBuilder cvarName = s.GetStringParameter(0);
  UnSanitizeCVarName(cvarName);

  nsCVar* pCVar = nsCVar::FindCVarByName(cvarName);

  if (pCVar == nullptr)
  {
    s.PushReturnValueNil();
    return s.ReturnToScript();
  }

  switch (pCVar->GetType())
  {
    case nsCVarType::Int:
    {
      nsCVarInt* pVar = (nsCVarInt*)pCVar;
      s.PushReturnValue(pVar->GetValue());
    }
    break;
    case nsCVarType::Bool:
    {
      nsCVarBool* pVar = (nsCVarBool*)pCVar;
      s.PushReturnValue(pVar->GetValue());
    }
    break;
    case nsCVarType::Float:
    {
      nsCVarFloat* pVar = (nsCVarFloat*)pCVar;
      s.PushReturnValue(pVar->GetValue());
    }
    break;
    case nsCVarType::String:
    {
      nsCVarString* pVar = (nsCVarString*)pCVar;
      s.PushReturnValue(pVar->GetValue().GetData());
    }
    break;
    case nsCVarType::ENUM_COUNT:
      break;
  }

  return s.ReturnToScript();
}


static int LUAFUNC_WriteCVAR(lua_State* pState)
{
  nsLuaWrapper s(pState);

  nsStringBuilder cvarName = s.GetStringParameter(0);
  UnSanitizeCVarName(cvarName);

  nsCVar* pCVar = nsCVar::FindCVarByName(cvarName);

  if (pCVar == nullptr)
  {
    s.PushReturnValue(false);
    return s.ReturnToScript();
  }

  s.PushReturnValue(true);

  switch (pCVar->GetType())
  {
    case nsCVarType::Int:
    {
      nsCVarInt* pVar = (nsCVarInt*)pCVar;
      *pVar = s.GetIntParameter(1);
    }
    break;
    case nsCVarType::Bool:
    {
      nsCVarBool* pVar = (nsCVarBool*)pCVar;
      *pVar = s.GetBoolParameter(1);
    }
    break;
    case nsCVarType::Float:
    {
      nsCVarFloat* pVar = (nsCVarFloat*)pCVar;
      *pVar = s.GetFloatParameter(1);
    }
    break;
    case nsCVarType::String:
    {
      nsCVarString* pVar = (nsCVarString*)pCVar;
      *pVar = s.GetStringParameter(1);
    }
    break;
    case nsCVarType::ENUM_COUNT:
      break;
  }

  return s.ReturnToScript();
}

static void AllowScriptCVarAccess(nsLuaWrapper& ref_script)
{
  ref_script.RegisterCFunction("ReadCVar", LUAFUNC_ReadCVAR);
  ref_script.RegisterCFunction("WriteCVar", LUAFUNC_WriteCVAR);

  nsStringBuilder sInit = "\
function readcvar (t, key)\n\
return (ReadCVar (key))\n\
end\n\
\n\
function writecvar (t, key, value)\n\
if not WriteCVar (key, value) then\n\
rawset (t, key, value or false)\n\
end\n\
end\n\
\n\
setmetatable (_G, {\n\
__newindex = writecvar,\n\
__index = readcvar,\n\
__metatable = \"Access Denied\",\n\
})";

  ref_script.ExecuteString(sInit.GetData()).IgnoreResult();
}

#endif // BUILDSYSTEM_ENABLE_LUA_SUPPORT
