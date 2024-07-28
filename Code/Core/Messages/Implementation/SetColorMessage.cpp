#include <Core/CorePCH.h>

#include <Core/Messages/SetColorMessage.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsSetColorMode, 1)
NS_ENUM_CONSTANTS(nsSetColorMode::SetRGBA, nsSetColorMode::SetRGB, nsSetColorMode::SetAlpha, nsSetColorMode::AlphaBlend, nsSetColorMode::Additive, nsSetColorMode::Modulate)
NS_END_STATIC_REFLECTED_ENUM;

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgSetColor);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgSetColor, 1, nsRTTIDefaultAllocator<nsMsgSetColor>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Color", m_Color),
    NS_ENUM_MEMBER_PROPERTY("Mode", nsSetColorMode, m_Mode)
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgSetCustomData);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgSetCustomData, 1, nsRTTIDefaultAllocator<nsMsgSetCustomData>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Data", m_vData),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsMsgSetColor::ModifyColor(nsColor& ref_color) const
{
  switch (m_Mode)
  {
    case nsSetColorMode::SetRGB:
      ref_color.SetRGB(m_Color.r, m_Color.g, m_Color.b);
      break;

    case nsSetColorMode::SetAlpha:
      ref_color.a = m_Color.a;
      break;

    case nsSetColorMode::AlphaBlend:
      ref_color = nsMath::Lerp(ref_color, m_Color, m_Color.a);
      break;

    case nsSetColorMode::Additive:
      ref_color += m_Color;
      break;

    case nsSetColorMode::Modulate:
      ref_color *= m_Color;
      break;

    case nsSetColorMode::SetRGBA:
    default:
      ref_color = m_Color;
      break;
  }
}

void nsMsgSetColor::ModifyColor(nsColorGammaUB& ref_color) const
{
  nsColor temp = ref_color;
  ModifyColor(temp);
  ref_color = temp;
}

void nsMsgSetColor::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream << m_Color;
  inout_stream << m_Mode;
}

void nsMsgSetColor::Deserialize(nsStreamReader& inout_stream, nsUInt8 uiTypeVersion)
{
  inout_stream >> m_Color;
  inout_stream >> m_Mode;
}

//////////////////////////////////////////////////////////////////////////

void nsMsgSetCustomData::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream << m_vData;
}

void nsMsgSetCustomData::Deserialize(nsStreamReader& inout_stream, nsUInt8 uiTypeVersion)
{
  inout_stream >> m_vData;
}



NS_STATICLINK_FILE(Core, Core_Messages_Implementation_SetColorMessage);
