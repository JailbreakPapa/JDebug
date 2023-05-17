#include <Foundation/FoundationPCH.h>

#include <Foundation/Application/Application.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Utilities/CommandLineOptions.h>
#include <Foundation/Utilities/ConversionUtils.h>

WD_ENUMERABLE_CLASS_IMPLEMENTATION(wdCommandLineOption);

void wdCommandLineOption::GetSortingGroup(wdStringBuilder& ref_sOut) const
{
  ref_sOut = m_szSortingGroup;
}

void wdCommandLineOption::GetSplitOptions(wdStringBuilder& out_sAll, wdDynamicArray<wdStringView>& ref_splitOptions) const
{
  GetOptions(out_sAll);
  out_sAll.Split(false, ref_splitOptions, ";", "|");
}

bool wdCommandLineOption::IsHelpRequested(const wdCommandLineUtils* pUtils /*= wdCommandLineUtils::GetGlobalInstance()*/)
{
  return pUtils->GetBoolOption("-help") || pUtils->GetBoolOption("--help") || pUtils->GetBoolOption("-h") || pUtils->GetBoolOption("-?");
}

wdResult wdCommandLineOption::RequireOptions(const char* szRequiredOptions, wdString* pMissingOption /*= nullptr*/, const wdCommandLineUtils* pUtils /*= wdCommandLineUtils::GetGlobalInstance()*/)
{
  wdStringBuilder tmp;
  wdStringBuilder allOpts = szRequiredOptions;
  wdHybridArray<wdStringView, 16> options;
  allOpts.Split(false, options, ";");

  for (auto opt : options)
  {
    opt.Trim(" ");

    if (pUtils->GetOptionIndex(opt.GetData(tmp)) < 0)
    {
      if (pMissingOption)
      {
        *pMissingOption = opt;
      }

      return WD_FAILURE;
    }
  }

  if (pMissingOption)
  {
    pMissingOption->Clear();
  }

  return WD_SUCCESS;
}

bool wdCommandLineOption::LogAvailableOptions(LogAvailableModes mode, const char* szGroupFilter /*= nullptr*/, const wdCommandLineUtils* pUtils /*= wdCommandLineUtils::GetGlobalInstance()*/)
{
  if (mode == LogAvailableModes::IfHelpRequested)
  {
    if (!IsHelpRequested(pUtils))
      return false;
  }

  wdMap<wdString, wdHybridArray<wdCommandLineOption*, 16>> sorted;

  wdStringBuilder sGroupFilter;
  if (!wdStringUtils::IsNullOrEmpty(szGroupFilter))
  {
    sGroupFilter.Set(";", szGroupFilter, ";");
  }

  for (wdCommandLineOption* pOpt = wdCommandLineOption::GetFirstInstance(); pOpt != nullptr; pOpt = pOpt->GetNextInstance())
  {
    wdStringBuilder sGroup;
    pOpt->GetSortingGroup(sGroup);
    sGroup.Prepend(";");
    sGroup.Append(";");

    if (!sGroupFilter.IsEmpty())
    {
      if (sGroupFilter.FindSubString_NoCase(sGroup) == nullptr)
        continue;
    }

    sorted[sGroup].PushBack(pOpt);
  }

  if (wdApplication::GetApplicationInstance())
  {
    wdLog::Info("");
    wdLog::Info("{} command line options:", wdApplication::GetApplicationInstance()->GetApplicationName());
  }

  if (sorted.IsEmpty())
  {
    wdLog::Info("This application has no documented command line options.");
    return true;
  }

  wdStringBuilder sLine;

  for (auto optIt : sorted)
  {
    for (auto pOpt : optIt.Value())
    {
      wdStringBuilder sOptions, sParamShort, sParamDefault, sLongDesc;

      sLine.Clear();

      pOpt->GetOptions(sOptions);
      pOpt->GetParamShortDesc(sParamShort);
      pOpt->GetParamDefaultValueDesc(sParamDefault);
      pOpt->GetLongDesc(sLongDesc);

      wdHybridArray<wdStringView, 4> lines;

      sOptions.Split(false, lines, ";", "|");

      for (auto o : lines)
      {
        sLine.AppendWithSeparator(", ", o);
      }

      if (!sParamShort.IsEmpty())
      {
        sLine.Append(" ", sParamShort);

        if (!sParamDefault.IsEmpty())
        {
          sLine.Append(" = ", sParamDefault);
        }
      }

      wdLog::Info("");
      wdLog::Info(sLine);

      sLongDesc.Trim(" \t\n\r");
      sLongDesc.Split(true, lines, "\n");

      for (auto o : lines)
      {
        sLine = o;
        sLine.Trim("\t\n\r");
        sLine.Prepend("    ");

        wdLog::Info(sLine);
      }
    }

    wdLog::Info("");
  }

  wdLog::Info("");

  return true;
}


bool wdCommandLineOption::LogAvailableOptionsToBuffer(wdStringBuilder& out_sBuffer, LogAvailableModes mode, const char* szGroupFilter /*= nullptr*/, const wdCommandLineUtils* pUtils /*= wdCommandLineUtils::GetGlobalInstance()*/)
{
  wdLogSystemToBuffer log;
  wdLogSystemScope ls(&log);

  const bool res = wdCommandLineOption::LogAvailableOptions(mode, szGroupFilter, pUtils);

  out_sBuffer = log.m_sBuffer;

  return res;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

wdCommandLineOptionDoc::wdCommandLineOptionDoc(const char* szSortingGroup, const char* szArgument, const char* szParamShortDesc, const char* szLongDesc, const char* szDefaultValue, bool bCaseSensitive /*= false*/)
  : wdCommandLineOption(szSortingGroup)
{
  m_szArgument = szArgument;
  m_szParamShortDesc = szParamShortDesc;
  m_szParamDefaultValue = szDefaultValue;
  m_szLongDesc = szLongDesc;
  m_bCaseSensitive = bCaseSensitive;
}

void wdCommandLineOptionDoc::GetOptions(wdStringBuilder& ref_sOut) const
{
  ref_sOut = m_szArgument;
}

void wdCommandLineOptionDoc::GetParamShortDesc(wdStringBuilder& ref_sOut) const
{
  ref_sOut = m_szParamShortDesc;
}

void wdCommandLineOptionDoc::GetParamDefaultValueDesc(wdStringBuilder& ref_sOut) const
{
  ref_sOut = m_szParamDefaultValue;
}

void wdCommandLineOptionDoc::GetLongDesc(wdStringBuilder& ref_sOut) const
{
  ref_sOut = m_szLongDesc;
}

bool wdCommandLineOptionDoc::IsOptionSpecified(wdStringBuilder* out_pWhich, const wdCommandLineUtils* pUtils /*= wdCommandLineUtils::GetGlobalInstance()*/) const
{
  wdStringBuilder sOptions, tmp;
  wdHybridArray<wdStringView, 4> eachOption;
  GetSplitOptions(sOptions, eachOption);

  for (auto o : eachOption)
  {
    if (pUtils->GetOptionIndex(o.GetData(tmp), m_bCaseSensitive) >= 0)
    {
      if (out_pWhich)
      {
        *out_pWhich = tmp;
      }

      return true;
    }
  }

  if (out_pWhich)
  {
    *out_pWhich = m_szArgument;
  }

  return false;
}


bool wdCommandLineOptionDoc::ShouldLog(LogMode mode, bool bWasSpecified) const
{
  if (mode == LogMode::Never)
    return false;

  if (m_bLoggedOnce && (mode == LogMode::FirstTime || mode == LogMode::FirstTimeIfSpecified))
    return false;

  if (!bWasSpecified && (mode == LogMode::FirstTimeIfSpecified || mode == LogMode::AlwaysIfSpecified))
    return false;

  return true;
}

void wdCommandLineOptionDoc::LogOption(const char* szOption, const char* szValue, bool bWasSpecified) const
{
  m_bLoggedOnce = true;

  if (bWasSpecified)
  {
    wdLog::Info("Option '{}' is set to '{}'", szOption, szValue);
  }
  else
  {
    wdLog::Info("Option '{}' is not set, default value is '{}'", szOption, szValue);
  }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

wdCommandLineOptionBool::wdCommandLineOptionBool(const char* szSortingGroup, const char* szArgument, const char* szLongDesc, bool bDefaultValue, bool bCaseSensitive /*= false*/)
  : wdCommandLineOptionDoc(szSortingGroup, szArgument, "<bool>", szLongDesc, bDefaultValue ? "true" : "false", bCaseSensitive)
{
  m_bDefaultValue = bDefaultValue;
}

bool wdCommandLineOptionBool::GetOptionValue(LogMode logMode, const wdCommandLineUtils* pUtils /*= wdCommandLineUtils::GetGlobalInstance()*/) const
{
  bool result = m_bDefaultValue;

  wdStringBuilder sOption;
  const bool bSpecified = IsOptionSpecified(&sOption, pUtils);

  if (bSpecified)
  {
    result = pUtils->GetBoolOption(sOption, m_bDefaultValue, m_bCaseSensitive);
  }

  if (ShouldLog(logMode, bSpecified))
  {
    LogOption(sOption, result ? "true" : "false", bSpecified);
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

wdCommandLineOptionInt::wdCommandLineOptionInt(const char* szSortingGroup, const char* szArgument, const char* szLongDesc, int iDefaultValue, int iMinValue /*= wdMath::MinValue<int>()*/, int iMaxValue /*= wdMath::MaxValue<int>()*/, bool bCaseSensitive /*= false*/)
  : wdCommandLineOptionDoc(szSortingGroup, szArgument, "<int>", szLongDesc, "0", bCaseSensitive)
{
  m_iDefaultValue = iDefaultValue;
  m_iMinValue = iMinValue;
  m_iMaxValue = iMaxValue;

  WD_ASSERT_DEV(m_iMinValue < m_iMaxValue, "Invalid min/max value");
}

void wdCommandLineOptionInt::GetParamDefaultValueDesc(wdStringBuilder& ref_sOut) const
{
  ref_sOut.Format("{}", m_iDefaultValue);
}


void wdCommandLineOptionInt::GetParamShortDesc(wdStringBuilder& ref_sOut) const
{
  if (m_iMinValue == wdMath::MinValue<int>() && m_iMaxValue == wdMath::MaxValue<int>())
  {
    ref_sOut = "<int>";
  }
  else
  {
    ref_sOut.Format("<int> [{} .. {}]", m_iMinValue, m_iMaxValue);
  }
}

int wdCommandLineOptionInt::GetOptionValue(LogMode logMode, const wdCommandLineUtils* pUtils /*= wdCommandLineUtils::GetGlobalInstance()*/) const
{
  int result = m_iDefaultValue;

  wdStringBuilder sOption, tmp;
  const bool bSpecified = IsOptionSpecified(&sOption, pUtils);

  if (bSpecified)
  {
    result = pUtils->GetIntOption(sOption, m_iDefaultValue, m_bCaseSensitive);

    if (result < m_iMinValue || result > m_iMaxValue)
    {
      if (ShouldLog(logMode, bSpecified))
      {
        wdLog::Warning("Option '{}' selected value '{}' is outside valid range [{} .. {}]. Using default value instead.", sOption, result, m_iMinValue, m_iMaxValue);
      }

      result = m_iDefaultValue;
    }
  }

  if (ShouldLog(logMode, bSpecified))
  {
    tmp.Format("{}", result);
    LogOption(sOption, tmp, bSpecified);
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

wdCommandLineOptionFloat::wdCommandLineOptionFloat(const char* szSortingGroup, const char* szArgument, const char* szLongDesc, float fDefaultValue, float fMinValue /*= wdMath::MinValue<float>()*/, float fMaxValue /*= wdMath::MaxValue<float>()*/, bool bCaseSensitive /*= false*/)
  : wdCommandLineOptionDoc(szSortingGroup, szArgument, "<float>", szLongDesc, "0", bCaseSensitive)
{
  m_fDefaultValue = fDefaultValue;
  m_fMinValue = fMinValue;
  m_fMaxValue = fMaxValue;

  WD_ASSERT_DEV(m_fMinValue < m_fMaxValue, "Invalid min/max value");
}

void wdCommandLineOptionFloat::GetParamDefaultValueDesc(wdStringBuilder& ref_sOut) const
{
  ref_sOut.Format("{}", m_fDefaultValue);
}

void wdCommandLineOptionFloat::GetParamShortDesc(wdStringBuilder& ref_sOut) const
{
  if (m_fMinValue == wdMath::MinValue<float>() && m_fMaxValue == wdMath::MaxValue<float>())
  {
    ref_sOut = "<float>";
  }
  else
  {
    ref_sOut.Format("<float> [{} .. {}]", m_fMinValue, m_fMaxValue);
  }
}

float wdCommandLineOptionFloat::GetOptionValue(LogMode logMode, const wdCommandLineUtils* pUtils /*= wdCommandLineUtils::GetGlobalInstance()*/) const
{
  float result = m_fDefaultValue;

  wdStringBuilder sOption, tmp;
  const bool bSpecified = IsOptionSpecified(&sOption, pUtils);

  if (bSpecified)
  {
    result = static_cast<float>(pUtils->GetFloatOption(sOption, m_fDefaultValue, m_bCaseSensitive));

    if (result < m_fMinValue || result > m_fMaxValue)
    {
      if (ShouldLog(logMode, bSpecified))
      {
        wdLog::Warning("Option '{}' selected value '{}' is outside valid range [{} .. {}]. Using default value instead.", sOption, result, m_fMinValue, m_fMaxValue);
      }

      result = m_fDefaultValue;
    }
  }

  if (ShouldLog(logMode, bSpecified))
  {
    tmp.Format("{}", result);
    LogOption(sOption, tmp, bSpecified);
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

wdCommandLineOptionString::wdCommandLineOptionString(const char* szSortingGroup, const char* szArgument, const char* szLongDesc, const char* szDefaultValue, bool bCaseSensitive /*= false*/)
  : wdCommandLineOptionDoc(szSortingGroup, szArgument, "<string>", szLongDesc, szDefaultValue, bCaseSensitive)
{
  m_szDefaultValue = szDefaultValue;
}

const char* wdCommandLineOptionString::GetOptionValue(LogMode logMode, const wdCommandLineUtils* pUtils /*= wdCommandLineUtils::GetGlobalInstance()*/) const
{
  const char* result = m_szDefaultValue;

  wdStringBuilder sOption;
  const bool bSpecified = IsOptionSpecified(&sOption, pUtils);

  if (bSpecified)
  {
    result = pUtils->GetStringOption(sOption, 0, m_szDefaultValue, m_bCaseSensitive);
  }

  if (ShouldLog(logMode, bSpecified))
  {
    LogOption(sOption, result, bSpecified);
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

wdCommandLineOptionPath::wdCommandLineOptionPath(const char* szSortingGroup, const char* szArgument, const char* szLongDesc, const char* szDefaultValue, bool bCaseSensitive /*= false*/)
  : wdCommandLineOptionDoc(szSortingGroup, szArgument, "<path>", szLongDesc, szDefaultValue, bCaseSensitive)
{
  m_szDefaultValue = szDefaultValue;
}

wdString wdCommandLineOptionPath::GetOptionValue(LogMode logMode, const wdCommandLineUtils* pUtils /*= wdCommandLineUtils::GetGlobalInstance()*/) const
{
  wdString result = m_szDefaultValue;

  wdStringBuilder sOption;
  const bool bSpecified = IsOptionSpecified(&sOption, pUtils);

  if (bSpecified)
  {
    result = pUtils->GetAbsolutePathOption(sOption, 0, m_szDefaultValue, m_bCaseSensitive);
  }

  if (ShouldLog(logMode, bSpecified))
  {
    LogOption(sOption, result, bSpecified);
  }

  return result;
}

wdCommandLineOptionEnum::wdCommandLineOptionEnum(const char* szSortingGroup, const char* szArgument, const char* szLongDesc, const char* szEnumKeysAndValues, wdInt32 iDefaultValue, bool bCaseSensitive /*= false*/)
  : wdCommandLineOptionDoc(szSortingGroup, szArgument, "<enum>", szLongDesc, "", bCaseSensitive)
{
  m_iDefaultValue = iDefaultValue;
  m_szEnumKeysAndValues = szEnumKeysAndValues;
}

wdInt32 wdCommandLineOptionEnum::GetOptionValue(LogMode logMode, const wdCommandLineUtils* pUtils /*= wdCommandLineUtils::GetGlobalInstance()*/) const
{
  wdInt32 result = m_iDefaultValue;

  wdStringBuilder sOption;
  const bool bSpecified = IsOptionSpecified(&sOption, pUtils);

  wdHybridArray<EnumKeyValue, 16> keysAndValues;
  GetEnumKeysAndValues(keysAndValues);

  if (bSpecified)
  {
    const char* selected = pUtils->GetStringOption(sOption, 0, "", m_bCaseSensitive);

    for (const auto& e : keysAndValues)
    {
      if (e.m_Key.IsEqual_NoCase(selected))
      {
        result = e.m_iValue;
        goto found;
      }
    }

    if (ShouldLog(logMode, bSpecified))
    {
      wdLog::Warning("Option '{}' selected value '{}' is unknown. Using default value instead.", sOption, selected);
    }
  }

found:

  if (ShouldLog(logMode, bSpecified))
  {
    wdStringBuilder opt;

    for (const auto& e : keysAndValues)
    {
      if (e.m_iValue == result)
      {
        opt = e.m_Key;
        break;
      }
    }

    LogOption(sOption, opt, bSpecified);
  }

  return result;
}

void wdCommandLineOptionEnum::GetParamShortDesc(wdStringBuilder& ref_sOut) const
{
  wdHybridArray<EnumKeyValue, 16> keysAndValues;
  GetEnumKeysAndValues(keysAndValues);

  for (const auto& e : keysAndValues)
  {
    ref_sOut.AppendWithSeparator(" | ", e.m_Key);
  }

  ref_sOut.Prepend("<");
  ref_sOut.Append(">");
}

void wdCommandLineOptionEnum::GetParamDefaultValueDesc(wdStringBuilder& ref_sOut) const
{
  wdHybridArray<EnumKeyValue, 16> keysAndValues;
  GetEnumKeysAndValues(keysAndValues);

  for (const auto& e : keysAndValues)
  {
    if (m_iDefaultValue == e.m_iValue)
    {
      ref_sOut = e.m_Key;
      return;
    }
  }
}

void wdCommandLineOptionEnum::GetEnumKeysAndValues(wdDynamicArray<EnumKeyValue>& out_keysAndValues) const
{
  wdStringBuilder tmp = m_szEnumKeysAndValues;

  wdHybridArray<wdStringView, 16> enums;
  tmp.Split(false, enums, ";", "|");

  out_keysAndValues.SetCount(enums.GetCount());

  wdInt32 eVal = 0;
  for (wdUInt32 e = 0; e < enums.GetCount(); ++e)
  {
    wdStringView eName;

    if (const char* eq = enums[e].FindSubString("="))
    {
      eName = wdStringView(enums[e].GetStartPointer(), eq);

      WD_VERIFY(wdConversionUtils::StringToInt(eq + 1, eVal).Succeeded(), "Invalid enum declaration");
    }
    else
    {
      eName = enums[e];
    }

    eName.Trim(" \n\r\t=");

    const char* pStart = m_szEnumKeysAndValues;
    pStart += (wdInt64)eName.GetStartPointer();
    pStart -= (wdInt64)tmp.GetData();

    out_keysAndValues[e].m_iValue = eVal;
    out_keysAndValues[e].m_Key = wdStringView(pStart, eName.GetElementCount());

    eVal++;
  }
}


WD_STATICLINK_FILE(Foundation, Foundation_Utilities_Implementation_CommandLineOptions);
