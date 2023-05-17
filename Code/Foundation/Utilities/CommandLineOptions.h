#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Utilities/CommandLineUtils.h>
#include <Foundation/Utilities/EnumerableClass.h>

class wdStringBuilder;
class wdLogInterface;

/// \brief wdCommandLineOption (and derived types) are used to define options that the application supports.
///
/// Command line options are created as global variables anywhere throughout the code, wherever they are needed.
/// The point of using them over going through wdCommandLineUtils directly, is that the options can be listed automatically
/// and thus an application can print all available options, when the user requests help.
///
/// Consequently, their main purpose is to make options discoverable and to document them in a consistent manner.
///
/// Additionally, classes like wdCommandLineOptionEnum add functionality that makes some options easier to setup.
class WD_FOUNDATION_DLL wdCommandLineOption : public wdEnumerable<wdCommandLineOption>
{
  WD_DECLARE_ENUMERABLE_CLASS(wdCommandLineOption);

public:
  enum class LogAvailableModes
  {
    Always,         ///< Logs the available modes no matter what
    IfHelpRequested ///< Only logs the modes, if '-h', '-help', '-?' or something similar was specified
  };

  /// \brief Describes whether the value of an option (and whether something went wrong), should be printed to wdLog.
  enum class LogMode
  {
    Never,                ///< Don't log anything.
    FirstTime,            ///< Only print the information the first time a value is accessed.
    FirstTimeIfSpecified, ///< Only on first access and only if the user specified the value on the command line.
    Always,               ///< Always log the options value on access.
    AlwaysIfSpecified,    ///< Always log values, if the user specified non-default ones.
  };

  /// \brief Checks whether a command line was passed that requests help output.
  static bool IsHelpRequested(const wdCommandLineUtils* pUtils = wdCommandLineUtils::GetGlobalInstance()); // [tested]

  /// \brief Checks whether all required options are passed to the command line.
  ///
  /// The options are passed as a semicolon-separated list (spare spaces are stripped away), for instance "-opt1; -opt2"
  static wdResult RequireOptions(const char* szRequiredOptions, wdString* pMissingOption = nullptr, const wdCommandLineUtils* pUtils = wdCommandLineUtils::GetGlobalInstance()); // [tested]

  /// \brief Prints all available options to the wdLog.
  ///
  /// \param szGroupFilter
  ///   If this is empty, all options from all 'sorting groups' are logged.
  ///   If non-empty, only options from sorting groups that appear in this string will be logged.
  static bool LogAvailableOptions(LogAvailableModes mode, const char* szGroupFilter = nullptr, const wdCommandLineUtils* pUtils = wdCommandLineUtils::GetGlobalInstance()); // [tested]

  /// \brief Same as LogAvailableOptions() but captures the output from wdLog and returns it in an wdStringBuilder.
  static bool LogAvailableOptionsToBuffer(wdStringBuilder& out_sBuffer, LogAvailableModes mode, const char* szGroupFilter = nullptr, const wdCommandLineUtils* pUtils = wdCommandLineUtils::GetGlobalInstance()); // [tested]

public:
  /// \param szSortingGroup
  ///   This string is used to sort options. Application options should start with an underscore, such that they appear first
  ///   in the output.
  wdCommandLineOption(const char* szSortingGroup) { m_szSortingGroup = szSortingGroup; }

  /// \brief Writes the sorting group name to 'out'.
  virtual void GetSortingGroup(wdStringBuilder& ref_sOut) const;

  /// \brief Writes all the supported options (e.g. '-arg') to 'out'.
  /// If more than one option is allowed, they should be separated with semicolons or pipes.
  virtual void GetOptions(wdStringBuilder& ref_sOut) const = 0;

  /// \brief Returns the supported option names (e.g. '-arg') as split strings.
  void GetSplitOptions(wdStringBuilder& out_sAll, wdDynamicArray<wdStringView>& ref_splitOptions) const;

  /// \brief Returns a very short description of the option (type). For example "<int>" or "<enum>".
  virtual void GetParamShortDesc(wdStringBuilder& ref_sOut) const = 0;

  /// \brief Returns a very short string for the options default value. For example "0" or "auto".
  virtual void GetParamDefaultValueDesc(wdStringBuilder& ref_sOut) const = 0;

  /// \brief Returns a proper description of the option.
  ///
  /// The long description is allowed to contain newlines (\n) and the output will be formatted accordingly.
  virtual void GetLongDesc(wdStringBuilder& ref_sOut) const = 0;

  /// \brief Returns a string indicating the exact implementation type.
  virtual const char* GetType() = 0;

protected:
  const char* m_szSortingGroup = nullptr;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief wdCommandLineOptionDoc can be used to document a command line option whose logic might be more complex than what the other option types provide.
///
/// This class is meant to be used for options that are actually queried directly through wdCommandLineUtils,
/// but should still show up in the command line option documentation, such that the user can discover them.
///
class WD_FOUNDATION_DLL wdCommandLineOptionDoc : public wdCommandLineOption
{
public:
  wdCommandLineOptionDoc(const char* szSortingGroup, const char* szArgument, const char* szParamShortDesc, const char* szLongDesc, const char* szDefaultValue, bool bCaseSensitive = false);

  virtual void GetOptions(wdStringBuilder& ref_sOut) const override; // [tested]

  virtual void GetParamShortDesc(wdStringBuilder& ref_sOut) const override; // [tested]

  virtual void GetParamDefaultValueDesc(wdStringBuilder& ref_sOut) const override; // [tested]

  virtual void GetLongDesc(wdStringBuilder& ref_sOut) const override; // [tested]

  /// \brief Returns "Doc"
  virtual const char* GetType() override { return "Doc"; }

  /// \brief Checks whether any of the option variants is set on the command line, and returns which one. For example '-h' or '-help'.
  bool IsOptionSpecified(wdStringBuilder* out_pWhich = nullptr, const wdCommandLineUtils* pUtils = wdCommandLineUtils::GetGlobalInstance()) const; // [tested]

protected:
  bool ShouldLog(LogMode mode, bool bWasSpecified) const;
  void LogOption(const char* szOption, const char* szValue, bool bWasSpecified) const;

  const char* m_szArgument = nullptr;
  const char* m_szParamShortDesc = nullptr;
  const char* m_szParamDefaultValue = nullptr;
  const char* m_szLongDesc = nullptr;
  bool m_bCaseSensitive = false;
  mutable bool m_bLoggedOnce = false;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief This command line option exposes simple on/off switches.
class WD_FOUNDATION_DLL wdCommandLineOptionBool : public wdCommandLineOptionDoc
{
public:
  wdCommandLineOptionBool(const char* szSortingGroup, const char* szArgument, const char* szLongDesc, bool bDefaultValue, bool bCaseSensitive = false);

  /// \brief Returns the value of this option. Either what was specified on the command line, or the default value.
  bool GetOptionValue(LogMode logMode, const wdCommandLineUtils* pUtils = wdCommandLineUtils::GetGlobalInstance()) const; // [tested]

  /// \brief Modifies the default value
  void SetDefaultValue(bool value)
  {
    m_bDefaultValue = value;
  }

  /// \brief Returns the default value.
  bool GetDefaultValue() const { return m_bDefaultValue; }

  /// \brief Returns "Bool"
  virtual const char* GetType() override { return "Bool"; }

protected:
  bool m_bDefaultValue = false;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief This command line option exposes integer values, optionally with a min/max range.
///
/// If the user specified a value outside the allowed range, a warning is printed, and the default value is used instead.
/// It is valid for the default value to be outside the min/max range, which can be used to detect whether the user provided any value at all.
class WD_FOUNDATION_DLL wdCommandLineOptionInt : public wdCommandLineOptionDoc
{
public:
  wdCommandLineOptionInt(const char* szSortingGroup, const char* szArgument, const char* szLongDesc, int iDefaultValue, int iMinValue = wdMath::MinValue<int>(), int iMaxValue = wdMath::MaxValue<int>(), bool bCaseSensitive = false);

  virtual void GetParamDefaultValueDesc(wdStringBuilder& ref_sOut) const override; // [tested]

  virtual void GetParamShortDesc(wdStringBuilder& ref_sOut) const override; // [tested]

  /// \brief Returns the value of this option. Either what was specified on the command line, or the default value.
  int GetOptionValue(LogMode logMode, const wdCommandLineUtils* pUtils = wdCommandLineUtils::GetGlobalInstance()) const; // [tested]

  /// \brief Modifies the default value
  void SetDefaultValue(wdInt32 value)
  {
    m_iDefaultValue = value;
  }

  /// \brief Returns "Int"
  virtual const char* GetType() override { return "Int"; }

  /// \brief Returns the minimum value.
  wdInt32 GetMinValue() const { return m_iMinValue; }

  /// \brief Returns the maximum value.
  wdInt32 GetMaxValue() const { return m_iMaxValue; }

  /// \brief Returns the default value.
  wdInt32 GetDefaultValue() const { return m_iDefaultValue; }

protected:
  wdInt32 m_iDefaultValue = 0;
  wdInt32 m_iMinValue = 0;
  wdInt32 m_iMaxValue = 0;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief This command line option exposes float values, optionally with a min/max range.
///
/// If the user specified a value outside the allowed range, a warning is printed, and the default value is used instead.
/// It is valid for the default value to be outside the min/max range, which can be used to detect whether the user provided any value at all.
class WD_FOUNDATION_DLL wdCommandLineOptionFloat : public wdCommandLineOptionDoc
{
public:
  wdCommandLineOptionFloat(const char* szSortingGroup, const char* szArgument, const char* szLongDesc, float fDefaultValue, float fMinValue = wdMath::MinValue<float>(), float fMaxValue = wdMath::MaxValue<float>(), bool bCaseSensitive = false);

  virtual void GetParamDefaultValueDesc(wdStringBuilder& ref_sOut) const override; // [tested]

  virtual void GetParamShortDesc(wdStringBuilder& ref_sOut) const override; // [tested]

  /// \brief Returns the value of this option. Either what was specified on the command line, or the default value.
  float GetOptionValue(LogMode logMode, const wdCommandLineUtils* pUtils = wdCommandLineUtils::GetGlobalInstance()) const; // [tested]

  /// \brief Modifies the default value
  void SetDefaultValue(float value)
  {
    m_fDefaultValue = value;
  }

  /// \brief Returns "Float"
  virtual const char* GetType() override { return "Float"; }

  /// \brief Returns the minimum value.
  float GetMinValue() const { return m_fMinValue; }

  /// \brief Returns the maximum value.
  float GetMaxValue() const { return m_fMaxValue; }

  /// \brief Returns the default value.
  float GetDefaultValue() const { return m_fDefaultValue; }

protected:
  float m_fDefaultValue = 0;
  float m_fMinValue = 0;
  float m_fMaxValue = 0;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief This command line option exposes simple string values.
class WD_FOUNDATION_DLL wdCommandLineOptionString : public wdCommandLineOptionDoc
{
public:
  wdCommandLineOptionString(const char* szSortingGroup, const char* szArgument, const char* szLongDesc, const char* szDefaultValue, bool bCaseSensitive = false);

  /// \brief Returns the value of this option. Either what was specified on the command line, or the default value.
  const char* GetOptionValue(LogMode logMode, const wdCommandLineUtils* pUtils = wdCommandLineUtils::GetGlobalInstance()) const; // [tested]

  /// \brief Modifies the default value
  void SetDefaultValue(const char* value)
  {
    m_szDefaultValue = value;
  }

  /// \brief Returns the default value.
  const char* GetDefaultValue() const { return m_szDefaultValue; }

  /// \brief Returns "String"
  virtual const char* GetType() override { return "String"; }

protected:
  const char* m_szDefaultValue = "";
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief This command line option exposes absolute paths. If the user provides a relative path, it will be concatenated with the current working directory.
class WD_FOUNDATION_DLL wdCommandLineOptionPath : public wdCommandLineOptionDoc
{
public:
  wdCommandLineOptionPath(const char* szSortingGroup, const char* szArgument, const char* szLongDesc, const char* szDefaultValue, bool bCaseSensitive = false);

  /// \brief Returns the value of this option. Either what was specified on the command line, or the default value.
  wdString GetOptionValue(LogMode logMode, const wdCommandLineUtils* pUtils = wdCommandLineUtils::GetGlobalInstance()) const; // [tested]

  /// \brief Modifies the default value
  void SetDefaultValue(const char* value)
  {
    m_szDefaultValue = value;
  }

  /// \brief Returns the default value.
  const char* GetDefaultValue() const { return m_szDefaultValue; }

  /// \brief Returns "Path"
  virtual const char* GetType() override { return "Path"; }

protected:
  const char* m_szDefaultValue = "";
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief An 'enum' option is a string option that only allows certain phrases ('keys').
///
/// Each phrase has an integer value, and GetOptionValue() returns the integer value of the selected phrase.
/// It is valid for the default value to be different from all the phrase values,
/// which can be used to detect whether the user provided any phrase at all.
///
/// The allowed values are passed in as a single string, in the form "OptA = 0 | OptB = 1 | ..."
/// Phrase values ("= 0" etc) are optional, and if not given are automatically assigned starting at zero.
/// Multiple phrases may share the same value.
class WD_FOUNDATION_DLL wdCommandLineOptionEnum : public wdCommandLineOptionDoc
{
public:
  wdCommandLineOptionEnum(const char* szSortingGroup, const char* szArgument, const char* szLongDesc, const char* szEnumKeysAndValues, wdInt32 iDefaultValue, bool bCaseSensitive = false);

  /// \brief Returns the value of this option. Either what was specified on the command line, or the default value.
  wdInt32 GetOptionValue(LogMode logMode, const wdCommandLineUtils* pUtils = wdCommandLineUtils::GetGlobalInstance()) const; // [tested]

  virtual void GetParamShortDesc(wdStringBuilder& ref_sOut) const override; // [tested]

  virtual void GetParamDefaultValueDesc(wdStringBuilder& ref_sOut) const override; // [tested]

  struct EnumKeyValue
  {
    wdStringView m_Key;
    wdInt32 m_iValue = 0;
  };

  /// \brief Returns the enum keys (names) and values (integers) extracted from the string that was passed to the constructor.
  void GetEnumKeysAndValues(wdDynamicArray<EnumKeyValue>& out_keysAndValues) const;

  /// \brief Modifies the default value
  void SetDefaultValue(wdInt32 value)
  {
    m_iDefaultValue = value;
  }

  /// \brief Returns the default value.
  wdInt32 GetDefaultValue() const { return m_iDefaultValue; }

  /// \brief Returns "Enum"
  virtual const char* GetType() override { return "Enum"; }

protected:
  wdInt32 m_iDefaultValue = 0;
  const char* m_szEnumKeysAndValues = nullptr;
};
