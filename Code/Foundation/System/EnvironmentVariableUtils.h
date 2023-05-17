#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Strings/String.h>

/// \brief This is a helper class to interact with environment variables.
class WD_FOUNDATION_DLL wdEnvironmentVariableUtils
{
public:
  /// \brief Returns the current value of the request environment variable. If it isn't set szDefault will be returned.
  static wdString GetValueString(const char* szName, const char* szDefault = nullptr);

  /// \brief Sets the environment variable for the current execution environment (i.e. this process and child processes created after this call).
  static wdResult SetValueString(const char* szName, const char* szValue);

  /// \brief Returns the current value of the request environment variable. If it isn't set iDefault will be returned.
  static wdInt32 GetValueInt(const char* szName, wdInt32 iDefault = -1);

  /// \brief Sets the environment variable for the current execution environment.
  static wdResult SetValueInt(const char* szName, wdInt32 iValue);

  /// \brief Returns true if the environment variable with the given name is set, false otherwise.
  static bool IsVariableSet(const char* szName);

  /// \brief Removes an environment variable from the current execution context (i.e. this process and child processes created after this call).
  static wdResult UnsetVariable(const char* szName);

private:
  /// \brief [internal]
  static wdString GetValueStringImpl(const char* szName, const char* szDefault);

  /// \brief [internal]
  static wdResult SetValueStringImpl(const char* szName, const char* szValue);

  /// \brief [internal]
  static bool IsVariableSetImpl(const char* szName);

  /// \brief [internal]
  static wdResult UnsetVariableImpl(const char* szName);
};
