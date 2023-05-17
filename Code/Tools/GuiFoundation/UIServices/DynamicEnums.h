#pragma once

#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <GuiFoundation/GuiFoundationDLL.h>

/// \brief Stores the valid values and names for 'dynamic' enums.
///
/// The names and valid values for dynamic enums may change due to user configuration changes.
/// The UI should show these user specified names without restarting the tool.
///
/// Call the static function GetDynamicEnum() to create or get the wdDynamicEnum for a specific type.
class WD_GUIFOUNDATION_DLL wdDynamicEnum
{
public:
  /// \brief Returns a wdDynamicEnum under the given name. Creates a new one, if the name has not been used before.
  static wdDynamicEnum& GetDynamicEnum(const char* szEnumName);

  /// \brief Returns all enum values and current names.
  const wdMap<wdInt32, wdString>& GetAllValidValues() const { return m_ValidValues; }

  /// \brief Resets the internal data.
  void Clear();

  /// \brief Sets the name for the given enum value.
  void SetValueAndName(wdInt32 iValue, const char* szNewName);

  /// \brief Removes a certain enum value, if it exists.
  void RemoveValue(wdInt32 iValue);

  /// \brief Returns whether a certain value is known.
  bool IsValueValid(wdInt32 iValue) const;

  /// \brief Returns the name for the given value. Returns "<invalid value>" if the value is not in use.
  const char* GetValueName(wdInt32 iValue) const;

private:
  wdMap<wdInt32, wdString> m_ValidValues;

  static wdMap<wdString, wdDynamicEnum> s_DynamicEnums;
};
