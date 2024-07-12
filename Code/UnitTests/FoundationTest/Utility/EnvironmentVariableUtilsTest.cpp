#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/System/EnvironmentVariableUtils.h>

#if NS_DISABLED(NS_PLATFORM_WINDOWS_UWP)

static nsUInt32 uiVersionForVariableSetting = 0;

NS_CREATE_SIMPLE_TEST(Utility, EnvironmentVariableUtils)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetValueString / GetValueInt")
  {
#  if NS_ENABLED(NS_PLATFORM_WINDOWS)

    // Windows will have "NUMBER_OF_PROCESSORS" and "USERNAME" set, let's see if we can get them
    NS_TEST_BOOL(nsEnvironmentVariableUtils::IsVariableSet("NUMBER_OF_PROCESSORS"));

    nsInt32 iNumProcessors = nsEnvironmentVariableUtils::GetValueInt("NUMBER_OF_PROCESSORS", -23);
    NS_TEST_BOOL(iNumProcessors > 0);

    NS_TEST_BOOL(nsEnvironmentVariableUtils::IsVariableSet("USERNAME"));
    nsString szUserName = nsEnvironmentVariableUtils::GetValueString("USERNAME");
    NS_TEST_BOOL(szUserName.GetElementCount() > 0);

#  elif NS_ENABLED(NS_PLATFORM_OSX) || NS_ENABLED(NS_PLATFORM_LINUX)

    // Mac OS & Linux will have "USER" set
    NS_TEST_BOOL(nsEnvironmentVariableUtils::IsVariableSet("USER"));
    nsString szUserName = nsEnvironmentVariableUtils::GetValueString("USER");
    NS_TEST_BOOL(szUserName.GetElementCount() > 0);

#  endif
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsVariableSet/SetValue")
  {
    nsStringBuilder szVarName;
    szVarName.SetFormat("NS_THIS_SHOULDNT_EXIST_NOW_OR_THIS_TEST_WILL_FAIL_{0}", uiVersionForVariableSetting++);

    NS_TEST_BOOL(!nsEnvironmentVariableUtils::IsVariableSet(szVarName));

    nsEnvironmentVariableUtils::SetValueString(szVarName, "NOW_IT_SHOULD_BE").IgnoreResult();
    NS_TEST_BOOL(nsEnvironmentVariableUtils::IsVariableSet(szVarName));

    NS_TEST_STRING(nsEnvironmentVariableUtils::GetValueString(szVarName), "NOW_IT_SHOULD_BE");

    // Test overwriting the same value again
    nsEnvironmentVariableUtils::SetValueString(szVarName, "NOW_IT_SHOULD_BE_SOMETHING_ELSE").IgnoreResult();
    NS_TEST_STRING(nsEnvironmentVariableUtils::GetValueString(szVarName), "NOW_IT_SHOULD_BE_SOMETHING_ELSE");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Variable with very long value")
  {
    // The Windows implementation has a 64 wchar_t buffer for example. Let's try setting a really
    // long variable and getting it back
    const char* szLongVariable =
      "SOME REALLY LONG VALUE, LETS TEST SOME LIMITS WE MIGHT HIT - 012456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz";

    nsStringBuilder szVarName;
    szVarName.SetFormat("NS_LONG_VARIABLE_TEST_{0}", uiVersionForVariableSetting++);

    NS_TEST_BOOL(!nsEnvironmentVariableUtils::IsVariableSet(szVarName));

    nsEnvironmentVariableUtils::SetValueString(szVarName, szLongVariable).IgnoreResult();
    NS_TEST_BOOL(nsEnvironmentVariableUtils::IsVariableSet(szVarName));

    NS_TEST_STRING(nsEnvironmentVariableUtils::GetValueString(szVarName), szLongVariable);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Unsetting variables")
  {
    const char* szVarName = "NS_TEST_HELLO_WORLD";
    NS_TEST_BOOL(!nsEnvironmentVariableUtils::IsVariableSet(szVarName));

    nsEnvironmentVariableUtils::SetValueString(szVarName, "TEST").IgnoreResult();

    NS_TEST_BOOL(nsEnvironmentVariableUtils::IsVariableSet(szVarName));

    nsEnvironmentVariableUtils::UnsetVariable(szVarName).IgnoreResult();
    NS_TEST_BOOL(!nsEnvironmentVariableUtils::IsVariableSet(szVarName));
  }
}

#endif
