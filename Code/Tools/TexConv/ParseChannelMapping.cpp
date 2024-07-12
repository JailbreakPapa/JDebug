/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <TexConv/TexConvPCH.h>

#include <TexConv/TexConv.h>

static nsStringView ToString(nsTexConvChannelValue::Enum e)
{
  switch (e)
  {
    case nsTexConvChannelValue::Red:
      return "Red";
    case nsTexConvChannelValue::Green:
      return "Green";
    case nsTexConvChannelValue::Blue:
      return "Blue";
    case nsTexConvChannelValue::Alpha:
      return "Alpha";
    case nsTexConvChannelValue::Black:
      return "Black";
    case nsTexConvChannelValue::White:
      return "White";

    default:
      NS_ASSERT_NOT_IMPLEMENTED;
  }

  return "";
}

nsResult nsTexConv::ParseChannelMappings()
{
  if (m_Processor.m_Descriptor.m_OutputType == nsTexConvOutputType::Atlas)
    return NS_SUCCESS;

  auto& mappings = m_Processor.m_Descriptor.m_ChannelMappings;

  NS_SUCCEED_OR_RETURN(ParseChannelSliceMapping(-1));

  for (nsUInt32 slice = 0; slice < 64; ++slice)
  {
    const nsUInt32 uiPrevMappings = mappings.GetCount();

    NS_SUCCEED_OR_RETURN(ParseChannelSliceMapping(slice));

    if (uiPrevMappings == mappings.GetCount())
    {
      // if no new mapping was found, don't try to find more
      break;
    }
  }

  if (!mappings.IsEmpty())
  {
    nsLog::Info("Custom output channel mapping:");
    for (nsUInt32 m = 0; m < mappings.GetCount(); ++m)
    {
      nsLog::Info("Slice {}, R -> Input file {}, {}", m, mappings[m].m_Channel[0].m_iInputImageIndex, ToString(mappings[m].m_Channel[0].m_ChannelValue));
      nsLog::Info("Slice {}, G -> Input file {}, {}", m, mappings[m].m_Channel[1].m_iInputImageIndex, ToString(mappings[m].m_Channel[1].m_ChannelValue));
      nsLog::Info("Slice {}, B -> Input file {}, {}", m, mappings[m].m_Channel[2].m_iInputImageIndex, ToString(mappings[m].m_Channel[2].m_ChannelValue));
      nsLog::Info("Slice {}, A -> Input file {}, {}", m, mappings[m].m_Channel[3].m_iInputImageIndex, ToString(mappings[m].m_Channel[3].m_ChannelValue));
    }
  }

  return NS_SUCCESS;
}

nsResult nsTexConv::ParseChannelSliceMapping(nsInt32 iSlice)
{
  const auto pCmd = nsCommandLineUtils::GetGlobalInstance();
  auto& mappings = m_Processor.m_Descriptor.m_ChannelMappings;
  nsStringBuilder tmp, param;

  const nsUInt32 uiMappingIdx = iSlice < 0 ? 0 : iSlice;

  // input to output mappings
  {
    param = "-rgba";
    if (iSlice != -1)
      param.AppendFormat("{}", iSlice);

    tmp = pCmd->GetStringOption(param);
    if (!tmp.IsEmpty())
    {
      mappings.EnsureCount(uiMappingIdx + 1);
      NS_SUCCEED_OR_RETURN(ParseChannelMappingConfig(mappings[uiMappingIdx].m_Channel[0], tmp, 0, false));
      NS_SUCCEED_OR_RETURN(ParseChannelMappingConfig(mappings[uiMappingIdx].m_Channel[1], tmp, 1, false));
      NS_SUCCEED_OR_RETURN(ParseChannelMappingConfig(mappings[uiMappingIdx].m_Channel[2], tmp, 2, false));
      NS_SUCCEED_OR_RETURN(ParseChannelMappingConfig(mappings[uiMappingIdx].m_Channel[3], tmp, 3, false));
    }

    param = "-rgb";
    if (iSlice != -1)
      param.AppendFormat("{}", iSlice);

    tmp = pCmd->GetStringOption(param);
    if (!tmp.IsEmpty())
    {
      mappings.EnsureCount(uiMappingIdx + 1);
      NS_SUCCEED_OR_RETURN(ParseChannelMappingConfig(mappings[uiMappingIdx].m_Channel[0], tmp, 0, false));
      NS_SUCCEED_OR_RETURN(ParseChannelMappingConfig(mappings[uiMappingIdx].m_Channel[1], tmp, 1, false));
      NS_SUCCEED_OR_RETURN(ParseChannelMappingConfig(mappings[uiMappingIdx].m_Channel[2], tmp, 2, false));
    }

    param = "-rg";
    if (iSlice != -1)
      param.AppendFormat("{}", iSlice);

    tmp = pCmd->GetStringOption(param);
    if (!tmp.IsEmpty())
    {
      mappings.EnsureCount(uiMappingIdx + 1);
      NS_SUCCEED_OR_RETURN(ParseChannelMappingConfig(mappings[uiMappingIdx].m_Channel[0], tmp, 0, false));
      NS_SUCCEED_OR_RETURN(ParseChannelMappingConfig(mappings[uiMappingIdx].m_Channel[1], tmp, 1, false));
    }

    param = "-r";
    if (iSlice != -1)
      param.AppendFormat("{}", iSlice);

    tmp = pCmd->GetStringOption(param);
    if (!tmp.IsEmpty())
    {
      mappings.EnsureCount(uiMappingIdx + 1);
      NS_SUCCEED_OR_RETURN(ParseChannelMappingConfig(mappings[uiMappingIdx].m_Channel[0], tmp, 0, true));
    }

    param = "-g";
    if (iSlice != -1)
      param.AppendFormat("{}", iSlice);

    tmp = pCmd->GetStringOption(param);
    if (!tmp.IsEmpty())
    {
      mappings.EnsureCount(uiMappingIdx + 1);
      NS_SUCCEED_OR_RETURN(ParseChannelMappingConfig(mappings[uiMappingIdx].m_Channel[1], tmp, 1, true));
    }

    param = "-b";
    if (iSlice != -1)
      param.AppendFormat("{}", iSlice);

    tmp = pCmd->GetStringOption(param);
    if (!tmp.IsEmpty())
    {
      mappings.EnsureCount(uiMappingIdx + 1);
      NS_SUCCEED_OR_RETURN(ParseChannelMappingConfig(mappings[uiMappingIdx].m_Channel[2], tmp, 2, true));
    }

    param = "-a";
    if (iSlice != -1)
      param.AppendFormat("{}", iSlice);

    tmp = pCmd->GetStringOption(param);
    if (!tmp.IsEmpty())
    {
      mappings.EnsureCount(uiMappingIdx + 1);
      NS_SUCCEED_OR_RETURN(ParseChannelMappingConfig(mappings[uiMappingIdx].m_Channel[3], tmp, 3, true));
    }
  }
  return NS_SUCCESS;
}

nsResult nsTexConv::ParseChannelMappingConfig(nsTexConvChannelMapping& out_mapping, nsStringView sCfg, nsInt32 iChannelIndex, bool bSingleChannel)
{
  out_mapping.m_iInputImageIndex = -1;
  out_mapping.m_ChannelValue = nsTexConvChannelValue::White;

  nsStringBuilder tmp = sCfg;

  // '-r black' for setting it to zero
  if (tmp.IsEqual_NoCase("black"))
  {
    out_mapping.m_ChannelValue = nsTexConvChannelValue::Black;
    return NS_SUCCESS;
  }

  // '-r white' for setting it to 255
  if (tmp.IsEqual_NoCase("white"))
  {
    out_mapping.m_ChannelValue = nsTexConvChannelValue::White;
    return NS_SUCCESS;
  }

  // skip the 'in', if found
  // 'in' is optional, one can also write '-r 1.r' for '-r in1.r'
  if (tmp.StartsWith_NoCase("in"))
    tmp.Shrink(2, 0);

  if (tmp.StartsWith("."))
  {
    // no index given, e.g. '-r in.r'
    // in is equal to in0

    out_mapping.m_iInputImageIndex = 0;
  }
  else
  {
    nsInt32 num = -1;
    const char* szLastPos = nullptr;
    if (nsConversionUtils::StringToInt(tmp, num, &szLastPos).Failed())
    {
      nsLog::Error("Could not parse channel mapping '{0}'", sCfg);
      return NS_FAILURE;
    }

    // valid index after the 'in'
    if (num >= 0 && num < (nsInt32)m_Processor.m_Descriptor.m_InputFiles.GetCount())
    {
      out_mapping.m_iInputImageIndex = (nsInt8)num;
    }
    else
    {
      nsLog::Error("Invalid channel mapping input file index '{0}'", num);
      return NS_FAILURE;
    }

    nsStringBuilder dummy = szLastPos;

    // continue after the index
    tmp = dummy;
  }

  // no additional info, e.g. '-g in2' is identical to '-g in2.g' (same channel)
  if (tmp.IsEmpty())
  {
    out_mapping.m_ChannelValue = (nsTexConvChannelValue::Enum)((nsInt32)nsTexConvChannelValue::Red + iChannelIndex);
    return NS_SUCCESS;
  }

  if (!tmp.StartsWith("."))
  {
    nsLog::Error("Invalid channel mapping: Expected '.' after input file index in '{0}'", sCfg);
    return NS_FAILURE;
  }

  tmp.Shrink(1, 0);

  if (!bSingleChannel)
  {
    // in case of '-rgb in1.bgr' map r to b, g to g, b to r, etc.
    // in case of '-rgb in1.r' map everything to the same input
    if (tmp.GetCharacterCount() > 1)
      tmp.Shrink(iChannelIndex, 0);
  }

  // no additional info, e.g. '-rgb in2.rg'
  if (tmp.IsEmpty())
  {
    nsLog::Error("Invalid channel mapping: Too few channel identifiers '{0}'", sCfg);
    return NS_FAILURE;
  }

  {
    const nsUInt32 uiChar = tmp.GetIteratorFront().GetCharacter();

    if (uiChar == 'r')
    {
      out_mapping.m_ChannelValue = nsTexConvChannelValue::Red;
    }
    else if (uiChar == 'g')
    {
      out_mapping.m_ChannelValue = nsTexConvChannelValue::Green;
    }
    else if (uiChar == 'b')
    {
      out_mapping.m_ChannelValue = nsTexConvChannelValue::Blue;
    }
    else if (uiChar == 'a')
    {
      out_mapping.m_ChannelValue = nsTexConvChannelValue::Alpha;
    }
    else
    {
      nsLog::Error("Invalid channel mapping: Unexpected channel identifier in '{}'", sCfg);
      return NS_FAILURE;
    }

    tmp.Shrink(1, 0);
  }

  return NS_SUCCESS;
}
