#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/System/Process.h>
#include <Foundation/Utilities/CommandLineUtils.h>

NS_CREATE_SIMPLE_TEST_GROUP(System);

#if NS_ENABLED(NS_SUPPORTS_PROCESSES)

NS_CREATE_SIMPLE_TEST(System, Process)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Command Line")
  {
    nsProcessOptions proc;
    proc.m_Arguments.PushBack("-bla");
    proc.m_Arguments.PushBack("blub blub");
    proc.m_Arguments.PushBack("\"di dub\"");
    proc.AddArgument(" -test ");
    proc.AddArgument("-hmpf {}", 27);
    proc.AddCommandLine("-a b   -c  d  -e \"f g h\" ");

    nsStringBuilder cmdLine;
    proc.BuildCommandLineString(cmdLine);

    NS_TEST_STRING(cmdLine, "-bla \"blub blub\" \"di dub\" -test \"-hmpf 27\" -a b -c d -e \"f g h\"");
  }

  static const char* g_szTestMsg = "Tell me more!\nAnother line\n520CharactersInOneLineAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA_END\nThat's all";
  static const char* g_szTestMsgLine0 = "Tell me more!\n";
  static const char* g_szTestMsgLine1 = "Another line\n";
  static const char* g_szTestMsgLine2 = "520CharactersInOneLineAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA_END\n";
  static const char* g_szTestMsgLine3 = "That's all";


  // we can launch FoundationTest with the -cmd parameter to execute a couple of useful things to test launching process
  const nsStringBuilder pathToSelf = nsCommandLineUtils::GetGlobalInstance()->GetParameter(0);

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Execute")
  {
    nsProcessOptions opt;
    opt.m_sProcess = pathToSelf;
    opt.m_Arguments.PushBack("-cmd");
    opt.m_Arguments.PushBack("-sleep");
    opt.m_Arguments.PushBack("500");

    nsInt32 exitCode = -1;

    if (!NS_TEST_BOOL_MSG(nsProcess::Execute(opt, &exitCode).Succeeded(), "Failed to start process."))
      return;

    NS_TEST_INT(exitCode, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Launch / WaitToFinish")
  {
    nsProcessOptions opt;
    opt.m_sProcess = pathToSelf;
    opt.m_Arguments.PushBack("-cmd");
    opt.m_Arguments.PushBack("-sleep");
    opt.m_Arguments.PushBack("500");

    nsProcess proc;
    NS_TEST_BOOL(proc.GetState() == nsProcessState::NotStarted);

    if (!NS_TEST_BOOL_MSG(proc.Launch(opt).Succeeded(), "Failed to start process."))
      return;

    NS_TEST_BOOL(proc.GetState() == nsProcessState::Running);
    NS_TEST_BOOL(proc.WaitToFinish(nsTime::MakeFromSeconds(5)).Succeeded());
    NS_TEST_BOOL(proc.GetState() == nsProcessState::Finished);
    NS_TEST_INT(proc.GetExitCode(), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Launch / Terminate")
  {
    nsProcessOptions opt;
    opt.m_sProcess = pathToSelf;
    opt.m_Arguments.PushBack("-cmd");
    opt.m_Arguments.PushBack("-sleep");
    opt.m_Arguments.PushBack("10000");
    opt.m_Arguments.PushBack("-exitcode");
    opt.m_Arguments.PushBack("0");

    nsProcess proc;
    NS_TEST_BOOL(proc.GetState() == nsProcessState::NotStarted);

    if (!NS_TEST_BOOL_MSG(proc.Launch(opt).Succeeded(), "Failed to start process."))
      return;

    NS_TEST_BOOL(proc.GetState() == nsProcessState::Running);
    NS_TEST_BOOL(proc.Terminate().Succeeded());
    NS_TEST_BOOL(proc.GetState() == nsProcessState::Finished);
    NS_TEST_INT(proc.GetExitCode(), -1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Launch / Detach")
  {
    nsTime tTerminate;

    {
      nsProcessOptions opt;
      opt.m_sProcess = pathToSelf;
      opt.m_Arguments.PushBack("-cmd");
      opt.m_Arguments.PushBack("-sleep");
      opt.m_Arguments.PushBack("10000");

      nsProcess proc;
      if (!NS_TEST_BOOL_MSG(proc.Launch(opt).Succeeded(), "Failed to start process."))
        return;

      proc.Detach();

      tTerminate = nsTime::Now();
    }

    const nsTime tDiff = nsTime::Now() - tTerminate;
    NS_TEST_BOOL_MSG(tDiff < nsTime::MakeFromSeconds(1.0), "Destruction of nsProcess should be instant after Detach() was used.");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "STDOUT")
  {
    nsDynamicArray<nsStringBuilder> lines;
    nsStringBuilder out;
    nsProcessOptions opt;
    opt.m_onStdOut = [&](nsStringView sView)
    {
      out.Append(sView);
      lines.PushBack(sView);
    };

    opt.m_sProcess = pathToSelf;
    opt.m_Arguments.PushBack("-cmd");
    opt.m_Arguments.PushBack("-stdout");
    opt.m_Arguments.PushBack(g_szTestMsg);

    if (!NS_TEST_BOOL_MSG(nsProcess::Execute(opt).Succeeded(), "Failed to start process."))
      return;

    if (NS_TEST_BOOL(lines.GetCount() == 4))
    {
      lines[0].ReplaceAll("\r\n", "\n");
      NS_TEST_STRING(lines[0], g_szTestMsgLine0);
      lines[1].ReplaceAll("\r\n", "\n");
      NS_TEST_STRING(lines[1], g_szTestMsgLine1);
      lines[2].ReplaceAll("\r\n", "\n");
      NS_TEST_STRING(lines[2], g_szTestMsgLine2);
      lines[3].ReplaceAll("\r\n", "\n");
      NS_TEST_STRING(lines[3], g_szTestMsgLine3);
    }

    out.ReplaceAll("\r\n", "\n");
    NS_TEST_STRING(out, g_szTestMsg);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "STDERROR")
  {
    nsStringBuilder err;
    nsProcessOptions opt;
    opt.m_onStdError = [&err](nsStringView sView)
    { err.Append(sView); };

    opt.m_sProcess = pathToSelf;
    opt.m_Arguments.PushBack("-cmd");
    opt.m_Arguments.PushBack("-stderr");
    opt.m_Arguments.PushBack("NOT A VALID COMMAND");
    opt.m_Arguments.PushBack("-exitcode");
    opt.m_Arguments.PushBack("1");

    nsInt32 exitCode = 0;

    if (!NS_TEST_BOOL_MSG(nsProcess::Execute(opt, &exitCode).Succeeded(), "Failed to start process."))
      return;

    NS_TEST_BOOL_MSG(!err.IsEmpty(), "Error stream should contain something.");
    NS_TEST_INT(exitCode, 1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "STDOUT_STDERROR")
  {
    nsDynamicArray<nsStringBuilder> lines;
    nsStringBuilder out;
    nsStringBuilder err;
    nsProcessOptions opt;
    opt.m_onStdOut = [&](nsStringView sView)
    {
      out.Append(sView);
      lines.PushBack(sView);
    };
    opt.m_onStdError = [&err](nsStringView sView)
    { err.Append(sView); };
    opt.m_sProcess = pathToSelf;
    opt.m_Arguments.PushBack("-cmd");
    opt.m_Arguments.PushBack("-stdout");
    opt.m_Arguments.PushBack(g_szTestMsg);

    if (!NS_TEST_BOOL_MSG(nsProcess::Execute(opt).Succeeded(), "Failed to start process."))
      return;

    if (NS_TEST_BOOL(lines.GetCount() == 4))
    {
      lines[0].ReplaceAll("\r\n", "\n");
      NS_TEST_STRING(lines[0], g_szTestMsgLine0);
      lines[1].ReplaceAll("\r\n", "\n");
      NS_TEST_STRING(lines[1], g_szTestMsgLine1);
      lines[2].ReplaceAll("\r\n", "\n");
      NS_TEST_STRING(lines[2], g_szTestMsgLine2);
      lines[3].ReplaceAll("\r\n", "\n");
      NS_TEST_STRING(lines[3], g_szTestMsgLine3);
    }

    out.ReplaceAll("\r\n", "\n");
    NS_TEST_STRING(out, g_szTestMsg);
    NS_TEST_BOOL_MSG(err.IsEmpty(), "Error stream should be empty.");
  }
}
#endif
