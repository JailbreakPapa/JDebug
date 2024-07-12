#include <FoundationTest/FoundationTestPCH.h>

#if NS_ENABLED(NS_SUPPORTS_PROCESSES)

#  include <Foundation/System/ProcessGroup.h>
#  include <Foundation/Utilities/CommandLineUtils.h>

NS_CREATE_SIMPLE_TEST(System, ProcessGroup)
{
  // we can launch FoundationTest with the -cmd parameter to execute a couple of useful things to test launching process
  const nsStringBuilder pathToSelf = nsCommandLineUtils::GetGlobalInstance()->GetParameter(0);

  NS_TEST_BLOCK(nsTestBlock::Enabled, "WaitToFinish")
  {
    nsProcessGroup pgroup;
    nsStringBuilder out;

    nsMutex mutex;

    for (nsUInt32 i = 0; i < 8; ++i)
    {
      nsProcessOptions opt;
      opt.m_sProcess = pathToSelf;
      opt.m_onStdOut = [&out, &mutex](nsStringView sView)
      {
        NS_LOCK(mutex);
        out.Append(sView);
      };

      opt.m_Arguments.PushBack("-cmd");
      opt.m_Arguments.PushBack("-sleep");
      opt.m_Arguments.PushBack("1000");
      opt.m_Arguments.PushBack("-stdout");
      opt.m_Arguments.PushBack("Na");

      NS_TEST_BOOL(pgroup.Launch(opt).Succeeded());
    }

    // in a debugger with child debugging enabled etc. even 10 seconds can lead to timeouts due to long delays in the IDE
    NS_TEST_BOOL(pgroup.WaitToFinish(nsTime::MakeFromSeconds(60)).Succeeded());
    NS_TEST_STRING(out, "NaNaNaNaNaNaNaNa"); // BATMAN!
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TerminateAll")
  {
    nsProcessGroup pgroup;

    nsHybridArray<nsProcess, 8> procs;

    for (nsUInt32 i = 0; i < 8; ++i)
    {
      nsProcessOptions opt;
      opt.m_sProcess = pathToSelf;

      opt.m_Arguments.PushBack("-cmd");
      opt.m_Arguments.PushBack("-sleep");
      opt.m_Arguments.PushBack("60000");

      NS_TEST_BOOL(pgroup.Launch(opt).Succeeded());
    }

    const nsTime tStart = nsTime::Now();
    NS_TEST_BOOL(pgroup.TerminateAll().Succeeded());
    const nsTime tDiff = nsTime::Now() - tStart;

    NS_TEST_BOOL(tDiff < nsTime::MakeFromSeconds(10));
  }
}
#endif
