#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Communication/Message.h>

/// \brief Describes how a color should be applied to another color.
struct nsSetColorMode
{
  using StorageType = nsUInt32;

  enum Enum
  {
    SetRGBA,    ///< Overrides all four RGBA values.
    SetRGB,     ///< Overrides the RGB values but leaves Alpha untouched.
    SetAlpha,   ///< Overrides Alpha, leaves RGB untouched.

    AlphaBlend, ///< Modifies the target RGBA values by interpolating from the previous color towards the incoming color using the incoming alpha value.
    Additive,   ///< Adds to the RGBA values.
    Modulate,   /// Multiplies the RGBA values.

    Default = SetRGBA
  };
};

NS_DECLARE_REFLECTABLE_TYPE(NS_CORE_DLL, nsSetColorMode);

/// \brief A message to modify the main color of some thing.
///
/// Components that handle this message use it to change their main color.
/// For instance a light component may change its light color, a mesh component will change the main mesh color.
struct NS_CORE_DLL nsMsgSetColor : public nsMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgSetColor, nsMessage);

  /// \brief The color to apply to the target.
  nsColor m_Color;

  /// \brief The mode with which to apply the color to the target.
  nsEnum<nsSetColorMode> m_Mode;

  /// \brief Applies m_Color using m_Mode to the given color.
  void ModifyColor(nsColor& ref_color) const;

  /// \brief Applies m_Color using m_Mode to the given color.
  void ModifyColor(nsColorGammaUB& ref_color) const;

  virtual void Serialize(nsStreamWriter& inout_stream) const override;
  virtual void Deserialize(nsStreamReader& inout_stream, nsUInt8 uiTypeVersion) override;
};

struct NS_CORE_DLL nsMsgSetCustomData : public nsMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgSetCustomData, nsMessage);

  nsVec4 m_vData;

  virtual void Serialize(nsStreamWriter& inout_stream) const override;
  virtual void Deserialize(nsStreamReader& inout_stream, nsUInt8 uiTypeVersion) override;
};
