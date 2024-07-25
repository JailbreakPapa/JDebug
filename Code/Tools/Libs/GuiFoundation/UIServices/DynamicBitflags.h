#pragma once

#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <GuiFoundation/GuiFoundationDLL.h>

/// \brief Stores the valid values and names for 'dynamic' bitflags.
///
/// The names and valid values for dynamic bitflags may change due to user configuration changes.
/// The UI should show these user specified names without restarting the tool.
///
/// Call the static function GetDynamicBitflags() to create or get the nsDynamicBitflags for a specific type.
class NS_GUIFOUNDATION_DLL nsDynamicBitflags
{
public:
  /// \brief Returns a nsDynamicBitflags under the given name. Creates a new one, if the name has not been used before.
  static nsDynamicBitflags& GetDynamicBitflags(nsStringView sName);

  /// \brief Returns all bitflag values and current names.
  const nsMap<nsUInt64, nsString>& GetAllValidValues() const { return m_ValidValues; }

  /// \brief Resets stored values.
  void Clear();

  /// \brief Sets the name for the given bit position.
  void SetValueAndName(nsUInt32 uiBitPos, nsStringView sName);

  /// \brief Removes a value, if it exists.
  void RemoveValue(nsUInt32 uiBitPos);

  /// \brief Returns whether a certain value is known.
  bool IsValueValid(nsUInt32 uiBitPos) const;

  /// \brief Returns the name for the given value
  bool TryGetValueName(nsUInt32 uiBitPos, nsStringView& out_sName) const;

private:
  nsMap<nsUInt64, nsString> m_ValidValues;

  static nsMap<nsString, nsDynamicBitflags> s_DynamicBitflags;
};
