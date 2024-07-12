#include <TestFramework/Framework/TestFramework.h>
#include <TestFramework/Utilities/ConstructionCounter.h>

#include <Foundation/Configuration/CVar.h>
#include <Foundation/Configuration/Plugin.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Reflection/Reflection.h>

static nsInt32 g_iPluginState = -1;

void OnLoadPlugin();
void OnUnloadPlugin();

nsCVarInt CVar_TestInt("test1_Int", 11, nsCVarFlags::Save, "Desc: test1_Int");
nsCVarFloat CVar_TestFloat("test1_Float", 1.1f, nsCVarFlags::RequiresRestart, "Desc: test1_Float");
nsCVarBool CVar_TestBool("test1_Bool", false, nsCVarFlags::None, "Desc: test1_Bool");
nsCVarString CVar_TestString("test1_String", "test1", nsCVarFlags::Default, "Desc: test1_String");

nsCVarInt CVar_TestInt2("test1_Int2", 21, nsCVarFlags::Default, "Desc: test1_Int2");
nsCVarFloat CVar_TestFloat2("test1_Float2", 2.1f, nsCVarFlags::Default, "Desc: test1_Float2");
nsCVarBool CVar_TestBool2("test1_Bool2", true, nsCVarFlags::Default, "Desc: test1_Bool2");
nsCVarString CVar_TestString2("test1_String2", "test1b", nsCVarFlags::Default, "Desc: test1_String2");

NS_PLUGIN_ON_LOADED()
{
  OnLoadPlugin();
}

NS_PLUGIN_ON_UNLOADED()
{
  OnUnloadPlugin();
}

void OnLoadPlugin()
{
  NS_TEST_BOOL_MSG(g_iPluginState == -1, "Plugin is in an invalid state.");
  g_iPluginState = 1;

  nsCVarInt* pCVar = (nsCVarInt*)nsCVar::FindCVarByName("TestPlugin1InitCount");

  if (pCVar)
    *pCVar = *pCVar + 1;

  nsCVarBool* pCVarPlugin2Inited = (nsCVarBool*)nsCVar::FindCVarByName("test2_Inited");
  if (pCVarPlugin2Inited)
  {
    NS_TEST_BOOL(*pCVarPlugin2Inited == false); // Although Plugin2 is present, it should not yet have been initialized
  }
}

void OnUnloadPlugin()
{
  NS_TEST_BOOL_MSG(g_iPluginState == 1, "Plugin is in an invalid state.");
  g_iPluginState = 2;

  nsCVarInt* pCVar = (nsCVarInt*)nsCVar::FindCVarByName("TestPlugin1UninitCount");

  if (pCVar)
    *pCVar = *pCVar + 1;
}

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(PluginGroup_Plugin1, TestSubSystem1)

  //BEGIN_SUBSYSTEM_DEPENDENCIES
  //  "PluginGroup_Plugin1"
  //END_SUBSYSTEM_DEPENDENCIES

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

struct nsTestStruct2
{
  float m_fFloat2;

  nsTestStruct2() { m_fFloat2 = 42.0f; }
};

NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, nsTestStruct2);

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsTestStruct2, nsNoBase, 1, nsRTTIDefaultAllocator<nsTestStruct2>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Float2", m_fFloat2),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on
