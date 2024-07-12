#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Configuration/CVar.h>

nsCVarInt CVar_TestPlugin1InitializedCount("TestPlugin1InitCount", 0, nsCVarFlags::None, "How often Plugin1 has been initialized.");
nsCVarInt CVar_TestPlugin1UninitializedCount("TestPlugin1UninitCount", 0, nsCVarFlags::None, "How often Plugin1 has been uninitialized.");
nsCVarInt CVar_TestPlugin1Reloaded("TestPlugin1Reloaded", 0, nsCVarFlags::None, "How often Plugin1 has been reloaded (counts init AND de-init).");

nsCVarInt CVar_TestPlugin2InitializedCount("TestPlugin2InitCount", 0, nsCVarFlags::None, "How often Plugin2 has been initialized.");
nsCVarInt CVar_TestPlugin2UninitializedCount("TestPlugin2UninitCount", 0, nsCVarFlags::None, "How often Plugin2 has been uninitialized.");
nsCVarInt CVar_TestPlugin2Reloaded("TestPlugin2Reloaded", 0, nsCVarFlags::None, "How often Plugin2 has been reloaded (counts init AND de-init).");
nsCVarBool CVar_TestPlugin2FoundDependencies("TestPlugin2FoundDependencies", false, nsCVarFlags::None, "Whether Plugin2 found all its dependencies (other plugins).");

NS_CREATE_SIMPLE_TEST(Configuration, Plugin)
{
  CVar_TestPlugin1InitializedCount = 0;
  CVar_TestPlugin1UninitializedCount = 0;
  CVar_TestPlugin1Reloaded = 0;
  CVar_TestPlugin2InitializedCount = 0;
  CVar_TestPlugin2UninitializedCount = 0;
  CVar_TestPlugin2Reloaded = 0;
  CVar_TestPlugin2FoundDependencies = false;

#if NS_ENABLED(NS_SUPPORTS_DYNAMIC_PLUGINS) && NS_ENABLED(NS_COMPILE_ENGINE_AS_DLL)

  NS_TEST_BLOCK(nsTestBlock::Enabled, "LoadPlugin")
  {
    NS_TEST_BOOL(nsPlugin::LoadPlugin(nsFoundationTest_Plugin2) == NS_SUCCESS);
    NS_TEST_BOOL(nsPlugin::LoadPlugin(nsFoundationTest_Plugin2, nsPluginLoadFlags::PluginIsOptional) == NS_SUCCESS); // loading already loaded plugin is always a success

    NS_TEST_INT(CVar_TestPlugin1InitializedCount, 1);
    NS_TEST_INT(CVar_TestPlugin2InitializedCount, 1);

    NS_TEST_INT(CVar_TestPlugin1UninitializedCount, 0);
    NS_TEST_INT(CVar_TestPlugin2UninitializedCount, 0);

    NS_TEST_INT(CVar_TestPlugin1Reloaded, 0);
    NS_TEST_INT(CVar_TestPlugin2Reloaded, 0);

    NS_TEST_BOOL(CVar_TestPlugin2FoundDependencies);

    // this will fail the FoundationTests, as it logs an error
    // NS_TEST_BOOL(nsPlugin::LoadPlugin("Test") == NS_FAILURE); // plugin does not exist
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "UnloadPlugin")
  {
    CVar_TestPlugin2FoundDependencies = false;
    nsPlugin::UnloadAllPlugins();

    NS_TEST_INT(CVar_TestPlugin1InitializedCount, 1);
    NS_TEST_INT(CVar_TestPlugin2InitializedCount, 1);

    NS_TEST_INT(CVar_TestPlugin1UninitializedCount, 1);
    NS_TEST_INT(CVar_TestPlugin2UninitializedCount, 1);

    NS_TEST_INT(CVar_TestPlugin1Reloaded, 0);
    NS_TEST_INT(CVar_TestPlugin2Reloaded, 0);

    NS_TEST_BOOL(CVar_TestPlugin2FoundDependencies);
    NS_TEST_BOOL(nsPlugin::LoadPlugin("Test", nsPluginLoadFlags::PluginIsOptional) == NS_FAILURE); // plugin does not exist
  }

#endif
}
