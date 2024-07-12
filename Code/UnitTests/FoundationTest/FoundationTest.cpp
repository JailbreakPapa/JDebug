#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Threading/ThreadUtils.h>
#include <Foundation/Utilities/CommandLineUtils.h>
#include <TestFramework/Framework/TestFramework.h>
#include <TestFramework/Utilities/TestSetup.h>
#include <iostream>

nsInt32 nsConstructionCounter::s_iConstructions = 0;
nsInt32 nsConstructionCounter::s_iDestructions = 0;
nsInt32 nsConstructionCounter::s_iConstructionsLast = 0;
nsInt32 nsConstructionCounter::s_iDestructionsLast = 0;

nsInt32 nsConstructionCounterRelocatable::s_iConstructions = 0;
nsInt32 nsConstructionCounterRelocatable::s_iDestructions = 0;
nsInt32 nsConstructionCounterRelocatable::s_iConstructionsLast = 0;
nsInt32 nsConstructionCounterRelocatable::s_iDestructionsLast = 0;

NS_TESTFRAMEWORK_ENTRY_POINT_BEGIN("FoundationTest", "Foundation Tests")
{
  nsCommandLineUtils cmd;
  cmd.SetCommandLine(argc, (const char**)argv, nsCommandLineUtils::PreferOsArgs);

  // if the -cmd switch is set, FoundationTest.exe will execute a couple of simple operations and then close
  // this is used to test process launching (e.g. nsProcess)
  if (cmd.GetBoolOption("-cmd"))
  {
    // print something to stdout
    nsStringView sStdOut = cmd.GetStringOption("-stdout");
    if (!sStdOut.IsEmpty())
    {
      nsStringBuilder tmp;
      std::cout << sStdOut.GetData(tmp);
    }

    nsStringView sStdErr = cmd.GetStringOption("-stderr");
    if (!sStdErr.IsEmpty())
    {
      nsStringBuilder tmp;
      std::cerr << sStdErr.GetData(tmp);
    }

    // wait a little
    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(cmd.GetIntOption("-sleep")));

    // shutdown with exit code
    nsTestSetup::DeInitTestFramework(true);
    return cmd.GetIntOption("-exitcode");
  }
}
NS_TESTFRAMEWORK_ENTRY_POINT_END()
