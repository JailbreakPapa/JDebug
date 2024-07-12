#include <ToolsFoundationTest/ToolsFoundationTestPCH.h>

#include <TestFramework/Framework/TestFramework.h>
#include <TestFramework/Utilities/TestSetup.h>

nsInt32 nsConstructionCounter::s_iConstructions = 0;
nsInt32 nsConstructionCounter::s_iDestructions = 0;
nsInt32 nsConstructionCounter::s_iConstructionsLast = 0;
nsInt32 nsConstructionCounter::s_iDestructionsLast = 0;

NS_TESTFRAMEWORK_ENTRY_POINT("ToolsFoundationTest", "Tools Foundation Tests")
