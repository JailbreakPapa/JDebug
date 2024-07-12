#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Utilities/CommandLineOptions.h>

namespace
{
  class LogTestLogInterface : public nsLogInterface
  {
  public:
    virtual void HandleLogMessage(const nsLoggingEventData& le) override
    {
      switch (le.m_EventType)
      {
        case nsLogMsgType::Flush:
          m_Result.Append("[Flush]\n");
          return;
        case nsLogMsgType::BeginGroup:
          m_Result.Append(">", le.m_sTag, " ", le.m_sText, "\n");
          break;
        case nsLogMsgType::EndGroup:
          m_Result.Append("<", le.m_sTag, " ", le.m_sText, "\n");
          break;
        case nsLogMsgType::ErrorMsg:
          m_Result.Append("E:", le.m_sTag, " ", le.m_sText, "\n");
          break;
        case nsLogMsgType::SeriousWarningMsg:
          m_Result.Append("SW:", le.m_sTag, " ", le.m_sText, "\n");
          break;
        case nsLogMsgType::WarningMsg:
          m_Result.Append("W:", le.m_sTag, " ", le.m_sText, "\n");
          break;
        case nsLogMsgType::SuccessMsg:
          m_Result.Append("S:", le.m_sTag, " ", le.m_sText, "\n");
          break;
        case nsLogMsgType::InfoMsg:
          m_Result.Append("I:", le.m_sTag, " ", le.m_sText, "\n");
          break;
        case nsLogMsgType::DevMsg:
          m_Result.Append("E:", le.m_sTag, " ", le.m_sText, "\n");
          break;
        case nsLogMsgType::DebugMsg:
          m_Result.Append("D:", le.m_sTag, " ", le.m_sText, "\n");
          break;

        default:
          NS_REPORT_FAILURE("Invalid msg type");
          break;
      }
    }

    nsStringBuilder m_Result;
  };

} // namespace

NS_CREATE_SIMPLE_TEST(Utility, CommandLineOptions)
{
  nsCommandLineOptionDoc optDoc("__test", "-argDoc", "<doc>", "Doc argument", "no value");

  nsCommandLineOptionBool optBool1("__test", "-bool1", "bool argument 1", false);
  nsCommandLineOptionBool optBool2("__test", "-bool2", "bool argument 2", true);

  nsCommandLineOptionInt optInt1("__test", "-int1", "int argument 1", 1);
  nsCommandLineOptionInt optInt2("__test", "-int2", "int argument 2", 0, 4, 8);
  nsCommandLineOptionInt optInt3("__test", "-int3", "int argument 3", 6, -8, 8);

  nsCommandLineOptionFloat optFloat1("__test", "-float1", "float argument 1", 1);
  nsCommandLineOptionFloat optFloat2("__test", "-float2", "float argument 2", 0, 4, 8);
  nsCommandLineOptionFloat optFloat3("__test", "-float3", "float argument 3", 6, -8, 8);

  nsCommandLineOptionString optString1("__test", "-string1", "string argument 1", "default string");

  nsCommandLineOptionPath optPath1("__test", "-path1", "path argument 1", "default path");

  nsCommandLineOptionEnum optEnum1("__test", "-enum1", "enum argument 1", "A | B = 2 | C | D | E = 7", 3);

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsCommandLineOptionBool")
  {
    nsCommandLineUtils cmd;
    cmd.InjectCustomArgument("-bool1");
    cmd.InjectCustomArgument("on");

    NS_TEST_BOOL(optBool1.GetOptionValue(nsCommandLineOption::LogMode::Never, &cmd) == true);
    NS_TEST_BOOL(optBool2.GetOptionValue(nsCommandLineOption::LogMode::Never, &cmd) == true);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsCommandLineOptionInt")
  {
    nsCommandLineUtils cmd;
    cmd.InjectCustomArgument("-int1");
    cmd.InjectCustomArgument("3");

    cmd.InjectCustomArgument("-int2");
    cmd.InjectCustomArgument("10");

    cmd.InjectCustomArgument("-int3");
    cmd.InjectCustomArgument("-2");

    NS_TEST_INT(optInt1.GetOptionValue(nsCommandLineOption::LogMode::Never, &cmd), 3);
    NS_TEST_INT(optInt2.GetOptionValue(nsCommandLineOption::LogMode::Never, &cmd), 0);
    NS_TEST_INT(optInt3.GetOptionValue(nsCommandLineOption::LogMode::Never, &cmd), -2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsCommandLineOptionFloat")
  {
    nsCommandLineUtils cmd;
    cmd.InjectCustomArgument("-float1");
    cmd.InjectCustomArgument("3");

    cmd.InjectCustomArgument("-float2");
    cmd.InjectCustomArgument("10");

    cmd.InjectCustomArgument("-float3");
    cmd.InjectCustomArgument("-2");

    NS_TEST_FLOAT(optFloat1.GetOptionValue(nsCommandLineOption::LogMode::Never, &cmd), 3, 0.001f);
    NS_TEST_FLOAT(optFloat2.GetOptionValue(nsCommandLineOption::LogMode::Never, &cmd), 0, 0.001f);
    NS_TEST_FLOAT(optFloat3.GetOptionValue(nsCommandLineOption::LogMode::Never, &cmd), -2, 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsCommandLineOptionString")
  {
    nsCommandLineUtils cmd;
    cmd.InjectCustomArgument("-string1");
    cmd.InjectCustomArgument("hello");

    NS_TEST_STRING(optString1.GetOptionValue(nsCommandLineOption::LogMode::Never, &cmd), "hello");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsCommandLineOptionPath")
  {
    nsCommandLineUtils cmd;
    cmd.InjectCustomArgument("-path1");
    cmd.InjectCustomArgument("C:/test");

    const nsString path = optPath1.GetOptionValue(nsCommandLineOption::LogMode::Never, &cmd);

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
    NS_TEST_STRING(path, "C:/test");
#else
    NS_TEST_BOOL(path.EndsWith("C:/test"));
#endif
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsCommandLineOptionEnum")
  {
    {
      nsCommandLineUtils cmd;
      cmd.InjectCustomArgument("-enum1");
      cmd.InjectCustomArgument("A");

      NS_TEST_INT(optEnum1.GetOptionValue(nsCommandLineOption::LogMode::Never, &cmd), 0);
    }

    {
      nsCommandLineUtils cmd;
      cmd.InjectCustomArgument("-enum1");
      cmd.InjectCustomArgument("B");

      NS_TEST_INT(optEnum1.GetOptionValue(nsCommandLineOption::LogMode::Never, &cmd), 2);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "LogAvailableOptions")
  {
    nsCommandLineUtils cmd;

    nsStringBuilder result;

    NS_TEST_BOOL(nsCommandLineOption::LogAvailableOptionsToBuffer(result, nsCommandLineOption::LogAvailableModes::Always, "__test", &cmd));

    NS_TEST_STRING(result, "\
\n\
-argDoc <doc> = no value\n\
    Doc argument\n\
\n\
-bool1 <bool> = false\n\
    bool argument 1\n\
\n\
-bool2 <bool> = true\n\
    bool argument 2\n\
\n\
-int1 <int> = 1\n\
    int argument 1\n\
\n\
-int2 <int> [4 .. 8] = 0\n\
    int argument 2\n\
\n\
-int3 <int> [-8 .. 8] = 6\n\
    int argument 3\n\
\n\
-float1 <float> = 1\n\
    float argument 1\n\
\n\
-float2 <float> [4 .. 8] = 0\n\
    float argument 2\n\
\n\
-float3 <float> [-8 .. 8] = 6\n\
    float argument 3\n\
\n\
-string1 <string> = default string\n\
    string argument 1\n\
\n\
-path1 <path> = default path\n\
    path argument 1\n\
\n\
-enum1 <A | B | C | D | E> = C\n\
    enum argument 1\n\
\n\
\n\
");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsHelpRequested")
  {
    nsCommandLineUtils cmd;

    NS_TEST_BOOL(!nsCommandLineOption::IsHelpRequested(&cmd));

    cmd.InjectCustomArgument("-help");

    NS_TEST_BOOL(nsCommandLineOption::IsHelpRequested(&cmd));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RequireOptions")
  {
    nsCommandLineUtils cmd;
    nsString missing;

    NS_TEST_BOOL(nsCommandLineOption::RequireOptions("-opt1 ; -opt2", &missing, &cmd).Failed());
    NS_TEST_STRING(missing, "-opt1");

    cmd.InjectCustomArgument("-opt1");

    NS_TEST_BOOL(nsCommandLineOption::RequireOptions("-opt1 ; -opt2", &missing, &cmd).Failed());
    NS_TEST_STRING(missing, "-opt2");

    cmd.InjectCustomArgument("-opt2");

    NS_TEST_BOOL(nsCommandLineOption::RequireOptions("-opt1 ; -opt2", &missing, &cmd).Succeeded());
    NS_TEST_STRING(missing, "");
  }
}
