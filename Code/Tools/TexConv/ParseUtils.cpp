/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <TexConv/TexConvPCH.h>

#include <TexConv/TexConv.h>

nsResult nsTexConv::ParseUIntOption(nsStringView sOption, nsInt32 iMinValue, nsInt32 iMaxValue, nsUInt32& ref_uiResult) const
{
  const auto pCmd = nsCommandLineUtils::GetGlobalInstance();
  const nsUInt32 uiDefault = ref_uiResult;

  const nsInt32 val = pCmd->GetIntOption(sOption, ref_uiResult);

  if (!nsMath::IsInRange(val, iMinValue, iMaxValue))
  {
    nsLog::Error("'{}' value {} is out of valid range [{}; {}]", sOption, val, iMinValue, iMaxValue);
    return NS_FAILURE;
  }

  ref_uiResult = static_cast<nsUInt32>(val);

  if (ref_uiResult == uiDefault)
  {
    nsLog::Info("Using default '{}': '{}'.", sOption, ref_uiResult);
    return NS_SUCCESS;
  }

  nsLog::Info("Selected '{}': '{}'.", sOption, ref_uiResult);

  return NS_SUCCESS;
}

nsResult nsTexConv::ParseStringOption(nsStringView sOption, const nsDynamicArray<KeyEnumValuePair>& allowed, nsInt32& ref_iResult) const
{
  const auto pCmd = nsCommandLineUtils::GetGlobalInstance();
  const nsStringBuilder sValue = pCmd->GetStringOption(sOption, 0);

  if (sValue.IsEmpty())
  {
    ref_iResult = allowed[0].m_iEnumValue;

    nsLog::Info("Using default '{}': '{}'", sOption, allowed[0].m_sKey);
    return NS_SUCCESS;
  }

  for (nsUInt32 i = 0; i < allowed.GetCount(); ++i)
  {
    if (sValue.IsEqual_NoCase(allowed[i].m_sKey))
    {
      ref_iResult = allowed[i].m_iEnumValue;

      nsLog::Info("Selected '{}': '{}'", sOption, allowed[i].m_sKey);
      return NS_SUCCESS;
    }
  }

  nsLog::Error("Unknown value for option '{}': '{}'.", sOption, sValue);

  PrintOptionValues(sOption, allowed);

  return NS_FAILURE;
}

void nsTexConv::PrintOptionValues(nsStringView sOption, const nsDynamicArray<KeyEnumValuePair>& allowed) const
{
  nsLog::Info("Valid values for option '{}' are:", sOption);

  for (nsUInt32 i = 0; i < allowed.GetCount(); ++i)
  {
    nsLog::Info("  {}", allowed[i].m_sKey);
  }
}

void nsTexConv::PrintOptionValuesHelp(nsStringView sOption, const nsDynamicArray<KeyEnumValuePair>& allowed) const
{
  nsStringBuilder out(sOption, " ");

  for (nsUInt32 i = 0; i < allowed.GetCount(); ++i)
  {
    if (i > 0)
      out.Append(" | ");

    out.Append(allowed[i].m_sKey);
  }

  nsLog::Info(out);
}

bool nsTexConv::ParseFile(nsStringView sOption, nsString& ref_sResult) const
{
  const auto pCmd = nsCommandLineUtils::GetGlobalInstance();
  ref_sResult = pCmd->GetAbsolutePathOption(sOption);

  if (!ref_sResult.IsEmpty())
  {
    nsLog::Info("'{}' file: '{}'", sOption, ref_sResult);
    return true;
  }
  else
  {
    nsLog::Info("No '{}' file specified.", sOption);
    return false;
  }
}
