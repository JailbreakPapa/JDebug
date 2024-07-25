#pragma once

#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <GuiFoundation/GuiFoundationDLL.h>

/// \brief Stores the valid values and names for 'dynamic' enums.
///
/// The names and valid values for dynamic enums may change due to user configuration changes.
/// The UI should show these user specified names without restarting the tool.
///
/// Call the static function GetDynamicEnum() to create or get the nsDynamicEnum for a specific type.
class NS_GUIFOUNDATION_DLL nsDynamicEnum
{
public:
  /// \brief Returns a nsDynamicEnum under the given name. Creates a new one, if the name has not been used before.
  static nsDynamicEnum& GetDynamicEnum(const char* szEnumName);

  /// \brief Returns all enum values and current names.
  const nsMap<nsInt32, nsString>& GetAllValidValues() const { return m_ValidValues; }

  /// \brief Resets the internal data.
  void Clear();

  /// \brief Sets the name for the given enum value.
  void SetValueAndName(nsInt32 iValue, nsStringView sNewName);

  /// \brief Removes a certain enum value, if it exists.
  void RemoveValue(nsInt32 iValue);

  /// \brief Returns whether a certain value is known.
  bool IsValueValid(nsInt32 iValue) const;

  /// \brief Returns the name for the given value. Returns "<invalid value>" if the value is not in use.
  nsStringView GetValueName(nsInt32 iValue) const;

private:
  nsMap<nsInt32, nsString> m_ValidValues;

  static nsMap<nsString, nsDynamicEnum> s_DynamicEnums;
};
