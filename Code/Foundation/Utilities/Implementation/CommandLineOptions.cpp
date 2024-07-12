#include <Foundation/FoundationPCH.h>

#include <Foundation/Application/Application.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Utilities/CommandLineOptions.h>
#include <Foundation/Utilities/ConversionUtils.h>

NS_ENUMERABLE_CLASS_IMPLEMENTATION(nsCommandLineOption);

void nsCommandLineOption::GetSortingGroup(nsStringBuilder& ref_sOut) const
{
  ref_sOut = m_sSortingGroup;
}

void nsCommandLineOption::GetSplitOptions(nsStringBuilder& out_sAll, nsDynamicArray<nsStringView>& ref_splitOptions) const
{
  GetOptions(out_sAll);
  out_sAll.Split(false, ref_splitOptions, ";", "|");
}

bool nsCommandLineOption::IsHelpRequested(const nsCommandLineUtils* pUtils /*= nsCommandLineUtils::GetGlobalInstance()*/)
{
  return pUtils->GetBoolOption("-help") || pUtils->GetBoolOption("--help") || pUtils->GetBoolOption("-h") || pUtils->GetBoolOption("-?");
}

nsResult nsCommandLineOption::RequireOptions(nsStringView sRequiredOptions, nsString* pMissingOption /*= nullptr*/, const nsCommandLineUtils* pUtils /*= nsCommandLineUtils::GetGlobalInstance()*/)
{
  nsStringBuilder tmp;
  nsStringBuilder allOpts = sRequiredOptions;
  nsHybridArray<nsStringView, 16> options;
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

      return NS_FAILURE;
    }
  }

  if (pMissingOption)
  {
    pMissingOption->Clear();
  }

  return NS_SUCCESS;
}

bool nsCommandLineOption::LogAvailableOptions(LogAvailableModes mode, nsStringView sGroupFilter0 /*= {} */, const nsCommandLineUtils* pUtils /*= nsCommandLineUtils::GetGlobalInstance()*/)
{
  if (mode == LogAvailableModes::IfHelpRequested)
  {
    if (!IsHelpRequested(pUtils))
      return false;
  }

  nsMap<nsString, nsHybridArray<nsCommandLineOption*, 16>> sorted;

  nsStringBuilder sGroupFilter;
  if (!sGroupFilter0.IsEmpty())
  {
    sGroupFilter.Set(";", sGroupFilter0, ";");
  }

  for (nsCommandLineOption* pOpt = nsCommandLineOption::GetFirstInstance(); pOpt != nullptr; pOpt = pOpt->GetNextInstance())
  {
    nsStringBuilder sGroup;
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

  if (nsApplication::GetApplicationInstance())
  {
    nsLog::Info("");
    nsLog::Info("{} command line options:", nsApplication::GetApplicationInstance()->GetApplicationName());
  }

  if (sorted.IsEmpty())
  {
    nsLog::Info("This application has no documented command line options.");
    return true;
  }

  nsStringBuilder sLine;

  for (auto optIt : sorted)
  {
    for (auto pOpt : optIt.Value())
    {
      nsStringBuilder sOptions, sParamShort, sParamDefault, sLongDesc;

      sLine.Clear();

      pOpt->GetOptions(sOptions);
      pOpt->GetParamShortDesc(sParamShort);
      pOpt->GetParamDefaultValueDesc(sParamDefault);
      pOpt->GetLongDesc(sLongDesc);

      nsHybridArray<nsStringView, 4> lines;

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

      nsLog::Info("");
      nsLog::Info(sLine);

      sLongDesc.Trim(" \t\n\r");
      sLongDesc.Split(true, lines, "\n");

      for (auto o : lines)
      {
        sLine = o;
        sLine.Trim("\t\n\r");
        sLine.Prepend("    ");

        nsLog::Info(sLine);
      }
    }

    nsLog::Info("");
  }

  nsLog::Info("");

  return true;
}


bool nsCommandLineOption::LogAvailableOptionsToBuffer(nsStringBuilder& out_sBuffer, LogAvailableModes mode, nsStringView sGroupFilter /*= {} */, const nsCommandLineUtils* pUtils /*= nsCommandLineUtils::GetGlobalInstance()*/)
{
  nsLogSystemToBuffer log;
  nsLogSystemScope ls(&log);

  const bool res = nsCommandLineOption::LogAvailableOptions(mode, sGroupFilter, pUtils);

  out_sBuffer = log.m_sBuffer;

  return res;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

nsCommandLineOptionDoc::nsCommandLineOptionDoc(nsStringView sSortingGroup, nsStringView sArgument, nsStringView sParamShortDesc, nsStringView sLongDesc, nsStringView sDefaultValue, bool bCaseSensitive /*= false*/)
  : nsCommandLineOption(sSortingGroup)
{
  m_sArgument = sArgument;
  m_sParamShortDesc = sParamShortDesc;
  m_sParamDefaultValue = sDefaultValue;
  m_sLongDesc = sLongDesc;
  m_bCaseSensitive = bCaseSensitive;
}

void nsCommandLineOptionDoc::GetOptions(nsStringBuilder& ref_sOut) const
{
  ref_sOut = m_sArgument;
}

void nsCommandLineOptionDoc::GetParamShortDesc(nsStringBuilder& ref_sOut) const
{
  ref_sOut = m_sParamShortDesc;
}

void nsCommandLineOptionDoc::GetParamDefaultValueDesc(nsStringBuilder& ref_sOut) const
{
  ref_sOut = m_sParamDefaultValue;
}

void nsCommandLineOptionDoc::GetLongDesc(nsStringBuilder& ref_sOut) const
{
  ref_sOut = m_sLongDesc;
}

bool nsCommandLineOptionDoc::IsOptionSpecified(nsStringBuilder* out_pWhich, const nsCommandLineUtils* pUtils /*= nsCommandLineUtils::GetGlobalInstance()*/) const
{
  nsStringBuilder sOptions, tmp;
  nsHybridArray<nsStringView, 4> eachOption;
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
    *out_pWhich = m_sArgument;
  }

  return false;
}


bool nsCommandLineOptionDoc::ShouldLog(LogMode mode, bool bWasSpecified) const
{
  if (mode == LogMode::Never)
    return false;

  if (m_bLoggedOnce && (mode == LogMode::FirstTime || mode == LogMode::FirstTimeIfSpecified))
    return false;

  if (!bWasSpecified && (mode == LogMode::FirstTimeIfSpecified || mode == LogMode::AlwaysIfSpecified))
    return false;

  return true;
}

void nsCommandLineOptionDoc::LogOption(nsStringView sOption, nsStringView sValue, bool bWasSpecified) const
{
  m_bLoggedOnce = true;

  if (bWasSpecified)
  {
    nsLog::Info("Option '{}' is set to '{}'", sOption, sValue);
  }
  else
  {
    nsLog::Info("Option '{}' is not set, default value is '{}'", sOption, sValue);
  }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

nsCommandLineOptionBool::nsCommandLineOptionBool(nsStringView sSortingGroup, nsStringView sArgument, nsStringView sLongDesc, bool bDefaultValue, bool bCaseSensitive /*= false*/)
  : nsCommandLineOptionDoc(sSortingGroup, sArgument, "<bool>", sLongDesc, bDefaultValue ? "true" : "false", bCaseSensitive)
{
  m_bDefaultValue = bDefaultValue;
}

bool nsCommandLineOptionBool::GetOptionValue(LogMode logMode, const nsCommandLineUtils* pUtils /*= nsCommandLineUtils::GetGlobalInstance()*/) const
{
  bool result = m_bDefaultValue;

  nsStringBuilder sOption;
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

nsCommandLineOptionInt::nsCommandLineOptionInt(nsStringView sSortingGroup, nsStringView sArgument, nsStringView sLongDesc, int iDefaultValue, int iMinValue /*= nsMath::MinValue<int>()*/, int iMaxValue /*= nsMath::MaxValue<int>()*/, bool bCaseSensitive /*= false*/)
  : nsCommandLineOptionDoc(sSortingGroup, sArgument, "<int>", sLongDesc, "0", bCaseSensitive)
{
  m_iDefaultValue = iDefaultValue;
  m_iMinValue = iMinValue;
  m_iMaxValue = iMaxValue;

  NS_ASSERT_DEV(m_iMinValue < m_iMaxValue, "Invalid min/max value");
}

void nsCommandLineOptionInt::GetParamDefaultValueDesc(nsStringBuilder& ref_sOut) const
{
  ref_sOut.SetFormat("{}", m_iDefaultValue);
}


void nsCommandLineOptionInt::GetParamShortDesc(nsStringBuilder& ref_sOut) const
{
  if (m_iMinValue == nsMath::MinValue<int>() && m_iMaxValue == nsMath::MaxValue<int>())
  {
    ref_sOut = "<int>";
  }
  else
  {
    ref_sOut.SetFormat("<int> [{} .. {}]", m_iMinValue, m_iMaxValue);
  }
}

int nsCommandLineOptionInt::GetOptionValue(LogMode logMode, const nsCommandLineUtils* pUtils /*= nsCommandLineUtils::GetGlobalInstance()*/) const
{
  int result = m_iDefaultValue;

  nsStringBuilder sOption, tmp;
  const bool bSpecified = IsOptionSpecified(&sOption, pUtils);

  if (bSpecified)
  {
    result = pUtils->GetIntOption(sOption, m_iDefaultValue, m_bCaseSensitive);

    if (result < m_iMinValue || result > m_iMaxValue)
    {
      if (ShouldLog(logMode, bSpecified))
      {
        nsLog::Warning("Option '{}' selected value '{}' is outside valid range [{} .. {}]. Using default value instead.", sOption, result, m_iMinValue, m_iMaxValue);
      }

      result = m_iDefaultValue;
    }
  }

  if (ShouldLog(logMode, bSpecified))
  {
    tmp.SetFormat("{}", result);
    LogOption(sOption, tmp, bSpecified);
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

nsCommandLineOptionFloat::nsCommandLineOptionFloat(nsStringView sSortingGroup, nsStringView sArgument, nsStringView sLongDesc, float fDefaultValue, float fMinValue /*= nsMath::MinValue<float>()*/, float fMaxValue /*= nsMath::MaxValue<float>()*/, bool bCaseSensitive /*= false*/)
  : nsCommandLineOptionDoc(sSortingGroup, sArgument, "<float>", sLongDesc, "0", bCaseSensitive)
{
  m_fDefaultValue = fDefaultValue;
  m_fMinValue = fMinValue;
  m_fMaxValue = fMaxValue;

  NS_ASSERT_DEV(m_fMinValue < m_fMaxValue, "Invalid min/max value");
}

void nsCommandLineOptionFloat::GetParamDefaultValueDesc(nsStringBuilder& ref_sOut) const
{
  ref_sOut.SetFormat("{}", m_fDefaultValue);
}

void nsCommandLineOptionFloat::GetParamShortDesc(nsStringBuilder& ref_sOut) const
{
  if (m_fMinValue == nsMath::MinValue<float>() && m_fMaxValue == nsMath::MaxValue<float>())
  {
    ref_sOut = "<float>";
  }
  else
  {
    ref_sOut.SetFormat("<float> [{} .. {}]", m_fMinValue, m_fMaxValue);
  }
}

float nsCommandLineOptionFloat::GetOptionValue(LogMode logMode, const nsCommandLineUtils* pUtils /*= nsCommandLineUtils::GetGlobalInstance()*/) const
{
  float result = m_fDefaultValue;

  nsStringBuilder sOption, tmp;
  const bool bSpecified = IsOptionSpecified(&sOption, pUtils);

  if (bSpecified)
  {
    result = static_cast<float>(pUtils->GetFloatOption(sOption, m_fDefaultValue, m_bCaseSensitive));

    if (result < m_fMinValue || result > m_fMaxValue)
    {
      if (ShouldLog(logMode, bSpecified))
      {
        nsLog::Warning("Option '{}' selected value '{}' is outside valid range [{} .. {}]. Using default value instead.", sOption, result, m_fMinValue, m_fMaxValue);
      }

      result = m_fDefaultValue;
    }
  }

  if (ShouldLog(logMode, bSpecified))
  {
    tmp.SetFormat("{}", result);
    LogOption(sOption, tmp, bSpecified);
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

nsCommandLineOptionString::nsCommandLineOptionString(nsStringView sSortingGroup, nsStringView sArgument, nsStringView sLongDesc, nsStringView sDefaultValue, bool bCaseSensitive /*= false*/)
  : nsCommandLineOptionDoc(sSortingGroup, sArgument, "<string>", sLongDesc, sDefaultValue, bCaseSensitive)
{
  m_sDefaultValue = sDefaultValue;
}

nsStringView nsCommandLineOptionString::GetOptionValue(LogMode logMode, const nsCommandLineUtils* pUtils /*= nsCommandLineUtils::GetGlobalInstance()*/) const
{
  nsStringView result = m_sDefaultValue;

  nsStringBuilder sOption;
  const bool bSpecified = IsOptionSpecified(&sOption, pUtils);

  if (bSpecified)
  {
    result = pUtils->GetStringOption(sOption, 0, m_sDefaultValue, m_bCaseSensitive);
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

nsCommandLineOptionPath::nsCommandLineOptionPath(nsStringView sSortingGroup, nsStringView sArgument, nsStringView sLongDesc, nsStringView sDefaultValue, bool bCaseSensitive /*= false*/)
  : nsCommandLineOptionDoc(sSortingGroup, sArgument, "<path>", sLongDesc, sDefaultValue, bCaseSensitive)
{
  m_sDefaultValue = sDefaultValue;
}

nsString nsCommandLineOptionPath::GetOptionValue(LogMode logMode, const nsCommandLineUtils* pUtils /*= nsCommandLineUtils::GetGlobalInstance()*/) const
{
  nsString result = m_sDefaultValue;

  nsStringBuilder sOption;
  const bool bSpecified = IsOptionSpecified(&sOption, pUtils);

  if (bSpecified)
  {
    result = pUtils->GetAbsolutePathOption(sOption, 0, m_sDefaultValue, m_bCaseSensitive);
  }

  if (ShouldLog(logMode, bSpecified))
  {
    LogOption(sOption, result, bSpecified);
  }

  return result;
}

nsCommandLineOptionEnum::nsCommandLineOptionEnum(nsStringView sSortingGroup, nsStringView sArgument, nsStringView sLongDesc, nsStringView sEnumKeysAndValues, nsInt32 iDefaultValue, bool bCaseSensitive /*= false*/)
  : nsCommandLineOptionDoc(sSortingGroup, sArgument, "<enum>", sLongDesc, "", bCaseSensitive)
{
  m_iDefaultValue = iDefaultValue;
  m_sEnumKeysAndValues = sEnumKeysAndValues;
}

nsInt32 nsCommandLineOptionEnum::GetOptionValue(LogMode logMode, const nsCommandLineUtils* pUtils /*= nsCommandLineUtils::GetGlobalInstance()*/) const
{
  nsInt32 result = m_iDefaultValue;

  nsStringBuilder sOption;
  const bool bSpecified = IsOptionSpecified(&sOption, pUtils);

  nsHybridArray<EnumKeyValue, 16> keysAndValues;
  GetEnumKeysAndValues(keysAndValues);

  if (bSpecified)
  {
    nsStringView selected = pUtils->GetStringOption(sOption, 0, "", m_bCaseSensitive);

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
      nsLog::Warning("Option '{}' selected value '{}' is unknown. Using default value instead.", sOption, selected);
    }
  }

found:

  if (ShouldLog(logMode, bSpecified))
  {
    nsStringBuilder opt;

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

void nsCommandLineOptionEnum::GetParamShortDesc(nsStringBuilder& ref_sOut) const
{
  nsHybridArray<EnumKeyValue, 16> keysAndValues;
  GetEnumKeysAndValues(keysAndValues);

  for (const auto& e : keysAndValues)
  {
    ref_sOut.AppendWithSeparator(" | ", e.m_Key);
  }

  ref_sOut.Prepend("<");
  ref_sOut.Append(">");
}

void nsCommandLineOptionEnum::GetParamDefaultValueDesc(nsStringBuilder& ref_sOut) const
{
  nsHybridArray<EnumKeyValue, 16> keysAndValues;
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

void nsCommandLineOptionEnum::GetEnumKeysAndValues(nsDynamicArray<EnumKeyValue>& out_keysAndValues) const
{
  nsStringBuilder tmp = m_sEnumKeysAndValues;

  nsHybridArray<nsStringView, 16> enums;
  tmp.Split(false, enums, ";", "|");

  out_keysAndValues.SetCount(enums.GetCount());

  nsInt32 eVal = 0;
  for (nsUInt32 e = 0; e < enums.GetCount(); ++e)
  {
    nsStringView eName;

    if (const char* eq = enums[e].FindSubString("="))
    {
      eName = nsStringView(enums[e].GetStartPointer(), eq);

      NS_VERIFY(nsConversionUtils::StringToInt(eq + 1, eVal).Succeeded(), "Invalid enum declaration");
    }
    else
    {
      eName = enums[e];
    }

    eName.Trim(" \n\r\t=");

    const char* pStart = m_sEnumKeysAndValues.GetStartPointer();
    pStart += (nsInt64)eName.GetStartPointer();
    pStart -= (nsInt64)tmp.GetData();

    out_keysAndValues[e].m_iValue = eVal;
    out_keysAndValues[e].m_Key = nsStringView(pStart, eName.GetElementCount());

    eVal++;
  }
}
