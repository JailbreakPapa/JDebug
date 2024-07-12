#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Utilities/CommandLineUtils.h>
#include <Foundation/Utilities/EnumerableClass.h>

class nsStringBuilder;
class nsLogInterface;

/// \brief nsCommandLineOption (and derived types) are used to define options that the application supports.
///
/// Command line options are created as global variables anywhere throughout the code, wherever they are needed.
/// The point of using them over going through nsCommandLineUtils directly, is that the options can be listed automatically
/// and thus an application can print all available options, when the user requests help.
///
/// Consequently, their main purpose is to make options discoverable and to document them in a consistent manner.
///
/// Additionally, classes like nsCommandLineOptionEnum add functionality that makes some options easier to setup.
class NS_FOUNDATION_DLL nsCommandLineOption : public nsEnumerable<nsCommandLineOption>
{
  NS_DECLARE_ENUMERABLE_CLASS(nsCommandLineOption);

public:
  enum class LogAvailableModes
  {
    Always,         ///< Logs the available modes no matter what
    IfHelpRequested ///< Only logs the modes, if '-h', '-help', '-?' or something similar was specified
  };

  /// \brief Describes whether the value of an option (and whether something went wrong), should be printed to nsLog.
  enum class LogMode
  {
    Never,                ///< Don't log anything.
    FirstTime,            ///< Only print the information the first time a value is accessed.
    FirstTimeIfSpecified, ///< Only on first access and only if the user specified the value on the command line.
    Always,               ///< Always log the options value on access.
    AlwaysIfSpecified,    ///< Always log values, if the user specified non-default ones.
  };

  /// \brief Checks whether a command line was passed that requests help output.
  static bool IsHelpRequested(const nsCommandLineUtils* pUtils = nsCommandLineUtils::GetGlobalInstance()); // [tested]

  /// \brief Checks whether all required options are passed to the command line.
  ///
  /// The options are passed as a semicolon-separated list (spare spaces are stripped away), for instance "-opt1; -opt2"
  static nsResult RequireOptions(nsStringView sRequiredOptions, nsString* pMissingOption = nullptr, const nsCommandLineUtils* pUtils = nsCommandLineUtils::GetGlobalInstance()); // [tested]

  /// \brief Prints all available options to the nsLog.
  ///
  /// \param szGroupFilter
  ///   If this is empty, all options from all 'sorting groups' are logged.
  ///   If non-empty, only options from sorting groups that appear in this string will be logged.
  static bool LogAvailableOptions(LogAvailableModes mode, nsStringView sGroupFilter = {}, const nsCommandLineUtils* pUtils = nsCommandLineUtils::GetGlobalInstance()); // [tested]

  /// \brief Same as LogAvailableOptions() but captures the output from nsLog and returns it in an nsStringBuilder.
  static bool LogAvailableOptionsToBuffer(nsStringBuilder& out_sBuffer, LogAvailableModes mode, nsStringView sGroupFilter = {}, const nsCommandLineUtils* pUtils = nsCommandLineUtils::GetGlobalInstance()); // [tested]

public:
  /// \param szSortingGroup
  ///   This string is used to sort options. Application options should start with an underscore, such that they appear first
  ///   in the output.
  nsCommandLineOption(nsStringView sSortingGroup) { m_sSortingGroup = sSortingGroup; }

  /// \brief Writes the sorting group name to 'out'.
  virtual void GetSortingGroup(nsStringBuilder& ref_sOut) const;

  /// \brief Writes all the supported options (e.g. '-arg') to 'out'.
  /// If more than one option is allowed, they should be separated with semicolons or pipes.
  virtual void GetOptions(nsStringBuilder& ref_sOut) const = 0;

  /// \brief Returns the supported option names (e.g. '-arg') as split strings.
  void GetSplitOptions(nsStringBuilder& out_sAll, nsDynamicArray<nsStringView>& ref_splitOptions) const;

  /// \brief Returns a very short description of the option (type). For example "<int>" or "<enum>".
  virtual void GetParamShortDesc(nsStringBuilder& ref_sOut) const = 0;

  /// \brief Returns a very short string for the options default value. For example "0" or "auto".
  virtual void GetParamDefaultValueDesc(nsStringBuilder& ref_sOut) const = 0;

  /// \brief Returns a proper description of the option.
  ///
  /// The long description is allowed to contain newlines (\n) and the output will be formatted accordingly.
  virtual void GetLongDesc(nsStringBuilder& ref_sOut) const = 0;

  /// \brief Returns a string indicating the exact implementation type.
  virtual nsStringView GetType() = 0;

protected:
  nsStringView m_sSortingGroup;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief nsCommandLineOptionDoc can be used to document a command line option whose logic might be more complex than what the other option types provide.
///
/// This class is meant to be used for options that are actually queried directly through nsCommandLineUtils,
/// but should still show up in the command line option documentation, such that the user can discover them.
///
class NS_FOUNDATION_DLL nsCommandLineOptionDoc : public nsCommandLineOption
{
public:
  nsCommandLineOptionDoc(nsStringView sSortingGroup, nsStringView sArgument, nsStringView sParamShortDesc, nsStringView sLongDesc, nsStringView sDefaultValue, bool bCaseSensitive = false);

  virtual void GetOptions(nsStringBuilder& ref_sOut) const override;               // [tested]

  virtual void GetParamShortDesc(nsStringBuilder& ref_sOut) const override;        // [tested]

  virtual void GetParamDefaultValueDesc(nsStringBuilder& ref_sOut) const override; // [tested]

  virtual void GetLongDesc(nsStringBuilder& ref_sOut) const override;              // [tested]

  /// \brief Returns "Doc"
  virtual nsStringView GetType() override { return "Doc"; }

  /// \brief Checks whether any of the option variants is set on the command line, and returns which one. For example '-h' or '-help'.
  bool IsOptionSpecified(nsStringBuilder* out_pWhich = nullptr, const nsCommandLineUtils* pUtils = nsCommandLineUtils::GetGlobalInstance()) const; // [tested]

protected:
  bool ShouldLog(LogMode mode, bool bWasSpecified) const;
  void LogOption(nsStringView sOption, nsStringView sValue, bool bWasSpecified) const;

  nsStringView m_sArgument;
  nsStringView m_sParamShortDesc;
  nsStringView m_sParamDefaultValue;
  nsStringView m_sLongDesc;
  bool m_bCaseSensitive = false;
  mutable bool m_bLoggedOnce = false;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief This command line option exposes simple on/off switches.
class NS_FOUNDATION_DLL nsCommandLineOptionBool : public nsCommandLineOptionDoc
{
public:
  nsCommandLineOptionBool(nsStringView sSortingGroup, nsStringView sArgument, nsStringView sLongDesc, bool bDefaultValue, bool bCaseSensitive = false);

  /// \brief Returns the value of this option. Either what was specified on the command line, or the default value.
  bool GetOptionValue(LogMode logMode, const nsCommandLineUtils* pUtils = nsCommandLineUtils::GetGlobalInstance()) const; // [tested]

  /// \brief Modifies the default value
  void SetDefaultValue(bool value)
  {
    m_bDefaultValue = value;
  }

  /// \brief Returns the default value.
  bool GetDefaultValue() const { return m_bDefaultValue; }

  /// \brief Returns "Bool"
  virtual nsStringView GetType() override { return "Bool"; }

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
class NS_FOUNDATION_DLL nsCommandLineOptionInt : public nsCommandLineOptionDoc
{
public:
  nsCommandLineOptionInt(nsStringView sSortingGroup, nsStringView sArgument, nsStringView sLongDesc, int iDefaultValue, int iMinValue = nsMath::MinValue<int>(), int iMaxValue = nsMath::MaxValue<int>(), bool bCaseSensitive = false);

  virtual void GetParamDefaultValueDesc(nsStringBuilder& ref_sOut) const override; // [tested]

  virtual void GetParamShortDesc(nsStringBuilder& ref_sOut) const override;        // [tested]

  /// \brief Returns the value of this option. Either what was specified on the command line, or the default value.
  int GetOptionValue(LogMode logMode, const nsCommandLineUtils* pUtils = nsCommandLineUtils::GetGlobalInstance()) const; // [tested]

  /// \brief Modifies the default value
  void SetDefaultValue(nsInt32 value)
  {
    m_iDefaultValue = value;
  }

  /// \brief Returns "Int"
  virtual nsStringView GetType() override { return "Int"; }

  /// \brief Returns the minimum value.
  nsInt32 GetMinValue() const { return m_iMinValue; }

  /// \brief Returns the maximum value.
  nsInt32 GetMaxValue() const { return m_iMaxValue; }

  /// \brief Returns the default value.
  nsInt32 GetDefaultValue() const { return m_iDefaultValue; }

protected:
  nsInt32 m_iDefaultValue = 0;
  nsInt32 m_iMinValue = 0;
  nsInt32 m_iMaxValue = 0;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief This command line option exposes float values, optionally with a min/max range.
///
/// If the user specified a value outside the allowed range, a warning is printed, and the default value is used instead.
/// It is valid for the default value to be outside the min/max range, which can be used to detect whether the user provided any value at all.
class NS_FOUNDATION_DLL nsCommandLineOptionFloat : public nsCommandLineOptionDoc
{
public:
  nsCommandLineOptionFloat(nsStringView sSortingGroup, nsStringView sArgument, nsStringView sLongDesc, float fDefaultValue, float fMinValue = nsMath::MinValue<float>(), float fMaxValue = nsMath::MaxValue<float>(), bool bCaseSensitive = false);

  virtual void GetParamDefaultValueDesc(nsStringBuilder& ref_sOut) const override; // [tested]

  virtual void GetParamShortDesc(nsStringBuilder& ref_sOut) const override;        // [tested]

  /// \brief Returns the value of this option. Either what was specified on the command line, or the default value.
  float GetOptionValue(LogMode logMode, const nsCommandLineUtils* pUtils = nsCommandLineUtils::GetGlobalInstance()) const; // [tested]

  /// \brief Modifies the default value
  void SetDefaultValue(float value)
  {
    m_fDefaultValue = value;
  }

  /// \brief Returns "Float"
  virtual nsStringView GetType() override { return "Float"; }

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
class NS_FOUNDATION_DLL nsCommandLineOptionString : public nsCommandLineOptionDoc
{
public:
  nsCommandLineOptionString(nsStringView sSortingGroup, nsStringView sArgument, nsStringView sLongDesc, nsStringView sDefaultValue, bool bCaseSensitive = false);

  /// \brief Returns the value of this option. Either what was specified on the command line, or the default value.
  nsStringView GetOptionValue(LogMode logMode, const nsCommandLineUtils* pUtils = nsCommandLineUtils::GetGlobalInstance()) const; // [tested]

  /// \brief Modifies the default value
  void SetDefaultValue(nsStringView sValue)
  {
    m_sDefaultValue = sValue;
  }

  /// \brief Returns the default value.
  nsStringView GetDefaultValue() const { return m_sDefaultValue; }

  /// \brief Returns "String"
  virtual nsStringView GetType() override { return "String"; }

protected:
  nsStringView m_sDefaultValue;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief This command line option exposes absolute paths. If the user provides a relative path, it will be concatenated with the current working directory.
class NS_FOUNDATION_DLL nsCommandLineOptionPath : public nsCommandLineOptionDoc
{
public:
  nsCommandLineOptionPath(nsStringView sSortingGroup, nsStringView sArgument, nsStringView sLongDesc, nsStringView sDefaultValue, bool bCaseSensitive = false);

  /// \brief Returns the value of this option. Either what was specified on the command line, or the default value.
  nsString GetOptionValue(LogMode logMode, const nsCommandLineUtils* pUtils = nsCommandLineUtils::GetGlobalInstance()) const; // [tested]

  /// \brief Modifies the default value
  void SetDefaultValue(nsStringView sValue)
  {
    m_sDefaultValue = sValue;
  }

  /// \brief Returns the default value.
  nsStringView GetDefaultValue() const { return m_sDefaultValue; }

  /// \brief Returns "Path"
  virtual nsStringView GetType() override { return "Path"; }

protected:
  nsStringView m_sDefaultValue;
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
class NS_FOUNDATION_DLL nsCommandLineOptionEnum : public nsCommandLineOptionDoc
{
public:
  nsCommandLineOptionEnum(nsStringView sSortingGroup, nsStringView sArgument, nsStringView sLongDesc, nsStringView sEnumKeysAndValues, nsInt32 iDefaultValue, bool bCaseSensitive = false);

  /// \brief Returns the value of this option. Either what was specified on the command line, or the default value.
  nsInt32 GetOptionValue(LogMode logMode, const nsCommandLineUtils* pUtils = nsCommandLineUtils::GetGlobalInstance()) const; // [tested]

  virtual void GetParamShortDesc(nsStringBuilder& ref_sOut) const override;                                                  // [tested]

  virtual void GetParamDefaultValueDesc(nsStringBuilder& ref_sOut) const override;                                           // [tested]

  struct EnumKeyValue
  {
    nsStringView m_Key;
    nsInt32 m_iValue = 0;
  };

  /// \brief Returns the enum keys (names) and values (integers) extracted from the string that was passed to the constructor.
  void GetEnumKeysAndValues(nsDynamicArray<EnumKeyValue>& out_keysAndValues) const;

  /// \brief Modifies the default value
  void SetDefaultValue(nsInt32 value)
  {
    m_iDefaultValue = value;
  }

  /// \brief Returns the default value.
  nsInt32 GetDefaultValue() const { return m_iDefaultValue; }

  /// \brief Returns "Enum"
  virtual nsStringView GetType() override { return "Enum"; }

protected:
  nsInt32 m_iDefaultValue = 0;
  nsStringView m_sEnumKeysAndValues;
};
