#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Strings/String.h>

/// \brief This is a helper class to interact with environment variables.
class NS_FOUNDATION_DLL nsEnvironmentVariableUtils
{
public:
  /// \brief Returns the current value of the request environment variable. If it isn't set szDefault will be returned.
  static nsString GetValueString(nsStringView sName, nsStringView sDefault = nullptr);

  /// \brief Sets the environment variable for the current execution environment (i.e. this process and child processes created after this call).
  static nsResult SetValueString(nsStringView sName, nsStringView sValue);

  /// \brief Returns the current value of the request environment variable. If it isn't set iDefault will be returned.
  static nsInt32 GetValueInt(nsStringView sName, nsInt32 iDefault = -1);

  /// \brief Sets the environment variable for the current execution environment.
  static nsResult SetValueInt(nsStringView sName, nsInt32 iValue);

  /// \brief Returns true if the environment variable with the given name is set, false otherwise.
  static bool IsVariableSet(nsStringView sName);

  /// \brief Removes an environment variable from the current execution context (i.e. this process and child processes created after this call).
  static nsResult UnsetVariable(nsStringView sName);

private:
  /// \brief [internal]
  static nsString GetValueStringImpl(nsStringView sName, nsStringView sDefault);

  /// \brief [internal]
  static nsResult SetValueStringImpl(nsStringView sName, nsStringView sValue);

  /// \brief [internal]
  static bool IsVariableSetImpl(nsStringView sName);

  /// \brief [internal]
  static nsResult UnsetVariableImpl(nsStringView sName);
};
