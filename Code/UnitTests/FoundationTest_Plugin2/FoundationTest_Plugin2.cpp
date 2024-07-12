#include <TestFramework/Framework/TestFramework.h>
#include <TestFramework/Utilities/ConstructionCounter.h>

#include <Foundation/Configuration/CVar.h>
#include <Foundation/Configuration/Plugin.h>
#include <Foundation/Configuration/Startup.h>

static nsInt32 g_iPluginState = -1;

void OnLoadPlugin();
void OnUnloadPlugin();

NS_PLUGIN_DEPENDENCY(nsFoundationTest_Plugin1);

NS_PLUGIN_ON_LOADED()
{
  OnLoadPlugin();
}

NS_PLUGIN_ON_UNLOADED()
{
  OnUnloadPlugin();
}

nsCVarInt CVar_TestInt("test2_Int", 22, nsCVarFlags::None, "Desc: test2_Int");
nsCVarFloat CVar_TestFloat("test2_Float", 2.2f, nsCVarFlags::Default, "Desc: test2_Float");
nsCVarBool CVar_TestBool("test2_Bool", true, nsCVarFlags::Save, "Desc: test2_Bool");
nsCVarString CVar_TestString("test2_String", "test2", nsCVarFlags::RequiresRestart, "Desc: test2_String");

nsCVarBool CVar_TestInited("test2_Inited", false, nsCVarFlags::None, "Desc: test2_Inited");

void OnLoadPlugin()
{
  NS_TEST_BOOL_MSG(g_iPluginState == -1, "Plugin is in an invalid state.");
  g_iPluginState = 1;

  nsCVarInt* pCVar = (nsCVarInt*)nsCVar::FindCVarByName("TestPlugin2InitCount");

  if (pCVar)
    *pCVar = *pCVar + 1;

  nsCVarBool* pCVarDep = (nsCVarBool*)nsCVar::FindCVarByName("TestPlugin2FoundDependencies");

  if (pCVarDep)
  {
    *pCVarDep = true;

    // check that all CVars from plugin1 are available (ie. plugin1 is already loaded)
    *pCVarDep = *pCVarDep && (nsCVar::FindCVarByName("test1_Int") != nullptr);
    *pCVarDep = *pCVarDep && (nsCVar::FindCVarByName("test1_Float") != nullptr);
    *pCVarDep = *pCVarDep && (nsCVar::FindCVarByName("test1_Bool") != nullptr);
    *pCVarDep = *pCVarDep && (nsCVar::FindCVarByName("test1_String") != nullptr);
  }

  CVar_TestInited = true;
}

void OnUnloadPlugin()
{
  NS_TEST_BOOL_MSG(g_iPluginState == 1, "Plugin is in an invalid state.");
  g_iPluginState = 2;

  nsCVarInt* pCVar = (nsCVarInt*)nsCVar::FindCVarByName("TestPlugin2UninitCount");

  if (pCVar)
    *pCVar = *pCVar + 1;

  nsCVarBool* pCVarDep = (nsCVarBool*)nsCVar::FindCVarByName("TestPlugin2FoundDependencies");

  if (pCVarDep)
  {
    *pCVarDep = true;

    // check that all CVars from plugin1 are STILL available (ie. plugin1 is not yet unloaded)
    *pCVarDep = *pCVarDep && (nsCVar::FindCVarByName("test1_Int") != nullptr);
    *pCVarDep = *pCVarDep && (nsCVar::FindCVarByName("test1_Float") != nullptr);
    *pCVarDep = *pCVarDep && (nsCVar::FindCVarByName("test1_Bool") != nullptr);
    *pCVarDep = *pCVarDep && (nsCVar::FindCVarByName("test1_String") != nullptr);
  }

  CVar_TestInited = false;
}

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(PluginGroup_Plugin2, TestSubSystem2)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "PluginGroup_Plugin1"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
  }

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on
