#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Configuration/CVar.h>
#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Utilities/CommandLineUtils.h>

NS_CREATE_SIMPLE_TEST_GROUP(Configuration);

#define nsCVarValueDefault nsCVarValue::Default
#define nsCVarValueStored nsCVarValue::Stored
#define nsCVarValueRestart nsCVarValue::DelayedSync

// Interestingly using 'nsCVarValue::Default' directly inside a macro does not work. (?!)
#define CHECK_CVAR(var, Current, Default, Stored, Restart)      \
  NS_TEST_BOOL(var != nullptr);                                 \
  if (var != nullptr)                                           \
  {                                                             \
    NS_TEST_BOOL(var->GetValue() == Current);                   \
    NS_TEST_BOOL(var->GetValue(nsCVarValueDefault) == Default); \
    NS_TEST_BOOL(var->GetValue(nsCVarValueStored) == Stored);   \
    NS_TEST_BOOL(var->GetValue(nsCVarValueRestart) == Restart); \
  }

static nsInt32 iChangedValue = 0;
static nsInt32 iChangedRestart = 0;

#if NS_ENABLED(NS_SUPPORTS_DYNAMIC_PLUGINS) && NS_ENABLED(NS_COMPILE_ENGINE_AS_DLL)

static void ChangedCVar(const nsCVarEvent& e)
{
  switch (e.m_EventType)
  {
    case nsCVarEvent::ValueChanged:
      ++iChangedValue;
      break;
    case nsCVarEvent::DelayedSyncValueChanged:
      ++iChangedRestart;
      break;
    default:
      break;
  }
}

#endif

NS_CREATE_SIMPLE_TEST(Configuration, CVars)
{
  iChangedValue = 0;
  iChangedRestart = 0;

  // setup the filesystem
  // we need it to test the storing of cvars (during plugin reloading)

  nsStringBuilder sOutputFolder1 = nsTestFramework::GetInstance()->GetAbsOutputPath();

  NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sOutputFolder1.GetData(), "test", "output", nsFileSystem::AllowWrites) == NS_SUCCESS);

  // Delete all cvar setting files
  {
    nsStringBuilder sConfigFile;

    sConfigFile = ":output/CVars/CVars_" nsFoundationTest_Plugin1 ".cfg";

    nsFileSystem::DeleteFile(sConfigFile.GetData());

    sConfigFile = ":output/CVars/CVars_" nsFoundationTest_Plugin2 ".cfg";

    nsFileSystem::DeleteFile(sConfigFile.GetData());
  }

  nsCommandLineUtils::GetGlobalInstance()->InjectCustomArgument("-test1_Int2");
  nsCommandLineUtils::GetGlobalInstance()->InjectCustomArgument("102");

  nsCommandLineUtils::GetGlobalInstance()->InjectCustomArgument("-test1_Float2");
  nsCommandLineUtils::GetGlobalInstance()->InjectCustomArgument("102.2");

  nsCommandLineUtils::GetGlobalInstance()->InjectCustomArgument("-test1_Bool2");
  nsCommandLineUtils::GetGlobalInstance()->InjectCustomArgument("false");

  nsCommandLineUtils::GetGlobalInstance()->InjectCustomArgument("-test1_String2");
  nsCommandLineUtils::GetGlobalInstance()->InjectCustomArgument("test1c");

  nsCVar::SetStorageFolder(":output/CVars");
  nsCVar::LoadCVars(); // should do nothing (no settings files available)

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SaveCVarsToFile and LoadCVarsFromFile again")
  {
    const char* cvarConfigFileDir = nsTestFramework::GetInstance()->GetAbsOutputPath();
    NS_TEST_BOOL_MSG(nsFileSystem::AddDataDirectory(cvarConfigFileDir, "CVarsTest", "CVarConfigTempDir", nsFileSystem::AllowWrites) == NS_SUCCESS, "Failed to mount data dir '%s'", cvarConfigFileDir);
    nsStringView cvarConfigFile = ":CVarConfigTempDir/CVars.cfg";

    nsCVarInt testCVarInt("testCVarInt", 0, nsCVarFlags::Default, "Test");
    nsCVarFloat testCVarFloat("testCVarFloat", 0.0f, nsCVarFlags::Default, "Test");
    nsCVarBool testCVarBool("testCVarBool", false, nsCVarFlags::Save, "Test");
    nsCVarString testCVarString("testCVarString", "", nsCVarFlags::Save, "Test");

    // ignore save flag = false
    {
      testCVarInt = 481516;
      testCVarFloat = 23.42f;
      testCVarBool = true;
      testCVarString = "Hello World!";

      bool bIgnoreSaveFlag = false;
      nsCVar::SaveCVarsToFile(cvarConfigFile, bIgnoreSaveFlag);
      NS_TEST_BOOL(nsFileSystem::ExistsFile(cvarConfigFile) == NS_SUCCESS);

      testCVarInt = 0;
      testCVarFloat = 0.0f;
      testCVarBool = false;
      testCVarString = "";

      nsDynamicArray<nsCVar*> outCVars;
      constexpr bool bOnlyNewOnes = false;
      constexpr bool bSetAsCurrentValue = true;
      nsCVar::LoadCVarsFromFile(cvarConfigFile, bOnlyNewOnes, bSetAsCurrentValue, bIgnoreSaveFlag, &outCVars);

      NS_TEST_INT(testCVarInt, 0);
      NS_TEST_FLOAT(testCVarFloat, 0.0f, nsMath::DefaultEpsilon<float>());
      NS_TEST_BOOL(testCVarBool == true);
      NS_TEST_STRING(testCVarString.GetValue(), "Hello World!");

      NS_TEST_BOOL(outCVars.Contains(&testCVarInt) == false);
      NS_TEST_BOOL(outCVars.Contains(&testCVarFloat) == false);
      NS_TEST_BOOL(outCVars.Contains(&testCVarBool));
      NS_TEST_BOOL(outCVars.Contains(&testCVarString));

      testCVarInt = 0;
      testCVarFloat = 0.0f;
      testCVarBool = false;
      testCVarString = "";

      // Even if we ignore the save flag the result should be same as above since we only stored CVars with the save flag in the file.
      bIgnoreSaveFlag = true;
      nsCVar::LoadCVarsFromFile(cvarConfigFile, bOnlyNewOnes, bSetAsCurrentValue, bIgnoreSaveFlag, &outCVars);

      NS_TEST_INT(testCVarInt, 0);
      NS_TEST_FLOAT(testCVarFloat, 0.0f, nsMath::DefaultEpsilon<float>());
      NS_TEST_BOOL(testCVarBool == true);
      NS_TEST_STRING(testCVarString.GetValue(), "Hello World!");

      NS_TEST_BOOL(outCVars.Contains(&testCVarInt) == false);
      NS_TEST_BOOL(outCVars.Contains(&testCVarFloat) == false);
      NS_TEST_BOOL(outCVars.Contains(&testCVarBool));
      NS_TEST_BOOL(outCVars.Contains(&testCVarString));

      nsFileSystem::DeleteFile(cvarConfigFile);
    }

    // ignore save flag = true
    {
      testCVarInt = 481516;
      testCVarFloat = 23.42f;
      testCVarBool = true;
      testCVarString = "Hello World!";

      bool bIgnoreSaveFlag = true;
      nsCVar::SaveCVarsToFile(cvarConfigFile, bIgnoreSaveFlag);
      NS_TEST_BOOL(nsFileSystem::ExistsFile(cvarConfigFile) == NS_SUCCESS);

      testCVarInt = 0;
      testCVarFloat = 0.0f;
      testCVarBool = false;
      testCVarString = "";

      nsDynamicArray<nsCVar*> outCVars;
      constexpr bool bOnlyNewOnes = false;
      constexpr bool bSetAsCurrentValue = true;
      // Check whether the save flag is correctly checked during load now that we have saved all CVars to the file.
      bIgnoreSaveFlag = false;
      nsCVar::LoadCVarsFromFile(cvarConfigFile, bOnlyNewOnes, bSetAsCurrentValue, bIgnoreSaveFlag, &outCVars);

      NS_TEST_INT(testCVarInt, 0);
      NS_TEST_FLOAT(testCVarFloat, 0.0f, nsMath::DefaultEpsilon<float>());
      NS_TEST_BOOL(testCVarBool == true);
      NS_TEST_STRING(testCVarString.GetValue(), "Hello World!");

      NS_TEST_BOOL(outCVars.Contains(&testCVarInt) == false);
      NS_TEST_BOOL(outCVars.Contains(&testCVarFloat) == false);
      NS_TEST_BOOL(outCVars.Contains(&testCVarBool));
      NS_TEST_BOOL(outCVars.Contains(&testCVarString));

      testCVarInt = 0;
      testCVarFloat = 0.0f;
      testCVarBool = false;
      testCVarString = "";

      // Now load all cvars stored in the file.
      bIgnoreSaveFlag = true;
      nsCVar::LoadCVarsFromFile(cvarConfigFile, bOnlyNewOnes, bSetAsCurrentValue, bIgnoreSaveFlag, &outCVars);

      NS_TEST_INT(testCVarInt, 481516);
      NS_TEST_FLOAT(testCVarFloat, 23.42f, nsMath::DefaultEpsilon<float>());
      NS_TEST_BOOL(testCVarBool == true);
      NS_TEST_STRING(testCVarString.GetValue(), "Hello World!");

      NS_TEST_BOOL(outCVars.Contains(&testCVarInt));
      NS_TEST_BOOL(outCVars.Contains(&testCVarFloat));
      NS_TEST_BOOL(outCVars.Contains(&testCVarBool));
      NS_TEST_BOOL(outCVars.Contains(&testCVarString));

      nsFileSystem::DeleteFile(cvarConfigFile);
    }


    NS_TEST_BOOL(nsFileSystem::RemoveDataDirectory("CVarConfigTempDir"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "No Plugin Loaded")
  {
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_Int") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_Float") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_Bool") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_String") == nullptr);

    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_Int") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_Float") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_Bool") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_String") == nullptr);
  }

#if NS_ENABLED(NS_SUPPORTS_DYNAMIC_PLUGINS) && NS_ENABLED(NS_COMPILE_ENGINE_AS_DLL)

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Plugin1 Loaded")
  {
    NS_TEST_BOOL(nsPlugin::LoadPlugin(nsFoundationTest_Plugin1) == NS_SUCCESS);

    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_Int") != nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_Float") != nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_Bool") != nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_String") != nullptr);

    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_Int2") != nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_Float2") != nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_Bool2") != nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_String2") != nullptr);

    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_Int") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_Float") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_Bool") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_String") == nullptr);

    nsPlugin::UnloadAllPlugins();
  }

#endif

  NS_TEST_BLOCK(nsTestBlock::Enabled, "No Plugin Loaded (2)")
  {
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_Int") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_Float") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_Bool") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_String") == nullptr);

    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_Int") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_Float") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_Bool") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_String") == nullptr);
  }

#if NS_ENABLED(NS_SUPPORTS_DYNAMIC_PLUGINS) && NS_ENABLED(NS_COMPILE_ENGINE_AS_DLL)

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Plugin2 Loaded")
  {
    // Plugin2 should automatically load Plugin1 with it

    NS_TEST_BOOL(nsPlugin::LoadPlugin(nsFoundationTest_Plugin2) == NS_SUCCESS);

    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_Int") != nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_Float") != nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_Bool") != nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_String") != nullptr);

    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_Int") != nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_Float") != nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_Bool") != nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_String") != nullptr);

    nsPlugin::UnloadAllPlugins();
  }

#endif

  NS_TEST_BLOCK(nsTestBlock::Enabled, "No Plugin Loaded (2)")
  {
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_Int") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_Float") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_Bool") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test1_String") == nullptr);

    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_Int") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_Float") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_Bool") == nullptr);
    NS_TEST_BOOL(nsCVar::FindCVarByName("test2_String") == nullptr);
  }

#if NS_ENABLED(NS_SUPPORTS_DYNAMIC_PLUGINS) && NS_ENABLED(NS_COMPILE_ENGINE_AS_DLL)

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Default Value Test")
  {
    NS_TEST_BOOL(nsPlugin::LoadPlugin(nsFoundationTest_Plugin2) == NS_SUCCESS);

    // CVars from Plugin 1
    {
      nsCVarInt* pInt = (nsCVarInt*)nsCVar::FindCVarByName("test1_Int");
      CHECK_CVAR(pInt, 11, 11, 11, 11);

      if (pInt)
      {
        NS_TEST_BOOL(pInt->GetType() == nsCVarType::Int);
        NS_TEST_BOOL(pInt->GetName() == "test1_Int");
        NS_TEST_BOOL(pInt->GetDescription() == "Desc: test1_Int");

        pInt->m_CVarEvents.AddEventHandler(ChangedCVar);

        *pInt = 12;
        CHECK_CVAR(pInt, 12, 11, 11, 12);
        NS_TEST_INT(iChangedValue, 1);
        NS_TEST_INT(iChangedRestart, 0);

        // no change
        *pInt = 12;
        NS_TEST_INT(iChangedValue, 1);
        NS_TEST_INT(iChangedRestart, 0);
      }

      nsCVarFloat* pFloat = (nsCVarFloat*)nsCVar::FindCVarByName("test1_Float");
      CHECK_CVAR(pFloat, 1.1f, 1.1f, 1.1f, 1.1f);

      if (pFloat)
      {
        NS_TEST_BOOL(pFloat->GetType() == nsCVarType::Float);
        NS_TEST_BOOL(pFloat->GetName() == "test1_Float");
        NS_TEST_BOOL(pFloat->GetDescription() == "Desc: test1_Float");

        pFloat->m_CVarEvents.AddEventHandler(ChangedCVar);

        *pFloat = 1.2f;
        CHECK_CVAR(pFloat, 1.1f, 1.1f, 1.1f, 1.2f);

        NS_TEST_INT(iChangedValue, 1);
        NS_TEST_INT(iChangedRestart, 1);

        // no change
        *pFloat = 1.2f;
        NS_TEST_INT(iChangedValue, 1);
        NS_TEST_INT(iChangedRestart, 1);

        pFloat->SetToDelayedSyncValue();
        CHECK_CVAR(pFloat, 1.2f, 1.1f, 1.1f, 1.2f);

        NS_TEST_INT(iChangedValue, 2);
        NS_TEST_INT(iChangedRestart, 1);
      }

      nsCVarBool* pBool = (nsCVarBool*)nsCVar::FindCVarByName("test1_Bool");
      CHECK_CVAR(pBool, false, false, false, false);

      if (pBool)
      {
        NS_TEST_BOOL(pBool->GetType() == nsCVarType::Bool);
        NS_TEST_BOOL(pBool->GetName() == "test1_Bool");
        NS_TEST_BOOL(pBool->GetDescription() == "Desc: test1_Bool");

        *pBool = true;
        CHECK_CVAR(pBool, true, false, false, true);
      }

      nsCVarString* pString = (nsCVarString*)nsCVar::FindCVarByName("test1_String");
      CHECK_CVAR(pString, "test1", "test1", "test1", "test1");

      if (pString)
      {
        NS_TEST_BOOL(pString->GetType() == nsCVarType::String);
        NS_TEST_BOOL(pString->GetName() == "test1_String");
        NS_TEST_BOOL(pString->GetDescription() == "Desc: test1_String");

        *pString = "test1_value2";
        CHECK_CVAR(pString, "test1_value2", "test1", "test1", "test1_value2");
      }
    }

    // CVars from Plugin 2
    {
      nsCVarInt* pInt = (nsCVarInt*)nsCVar::FindCVarByName("test2_Int");
      CHECK_CVAR(pInt, 22, 22, 22, 22);

      if (pInt)
      {
        pInt->m_CVarEvents.AddEventHandler(ChangedCVar);

        *pInt = 23;
        CHECK_CVAR(pInt, 23, 22, 22, 23);
        NS_TEST_INT(iChangedValue, 3);
        NS_TEST_INT(iChangedRestart, 1);
      }

      nsCVarFloat* pFloat = (nsCVarFloat*)nsCVar::FindCVarByName("test2_Float");
      CHECK_CVAR(pFloat, 2.2f, 2.2f, 2.2f, 2.2f);

      if (pFloat)
      {
        *pFloat = 2.3f;
        CHECK_CVAR(pFloat, 2.3f, 2.2f, 2.2f, 2.3f);
      }

      nsCVarBool* pBool = (nsCVarBool*)nsCVar::FindCVarByName("test2_Bool");
      CHECK_CVAR(pBool, true, true, true, true);

      if (pBool)
      {
        *pBool = false;
        CHECK_CVAR(pBool, false, true, true, false);
      }

      nsCVarString* pString = (nsCVarString*)nsCVar::FindCVarByName("test2_String");
      CHECK_CVAR(pString, "test2", "test2", "test2", "test2");

      if (pString)
      {
        *pString = "test2_value2";
        CHECK_CVAR(pString, "test2", "test2", "test2", "test2_value2");

        pString->SetToDelayedSyncValue();
        CHECK_CVAR(pString, "test2_value2", "test2", "test2", "test2_value2");
      }
    }

    nsPlugin::UnloadAllPlugins();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Loaded Value Test")
  {
    NS_TEST_BOOL(nsPlugin::LoadPlugin(nsFoundationTest_Plugin2) == NS_SUCCESS);

    // CVars from Plugin 1
    {
      nsCVarInt* pInt = (nsCVarInt*)nsCVar::FindCVarByName("test1_Int");
      CHECK_CVAR(pInt, 12, 11, 12, 12);

      nsCVarFloat* pFloat = (nsCVarFloat*)nsCVar::FindCVarByName("test1_Float");
      CHECK_CVAR(pFloat, 1.2f, 1.1f, 1.2f, 1.2f);

      nsCVarBool* pBool = (nsCVarBool*)nsCVar::FindCVarByName("test1_Bool");
      CHECK_CVAR(pBool, false, false, false, false);

      nsCVarString* pString = (nsCVarString*)nsCVar::FindCVarByName("test1_String");
      CHECK_CVAR(pString, "test1", "test1", "test1", "test1");
    }

    // CVars from Plugin 1, overridden by command line
    {
      nsCVarInt* pInt = (nsCVarInt*)nsCVar::FindCVarByName("test1_Int2");
      CHECK_CVAR(pInt, 102, 21, 102, 102);

      nsCVarFloat* pFloat = (nsCVarFloat*)nsCVar::FindCVarByName("test1_Float2");
      CHECK_CVAR(pFloat, 102.2f, 2.1f, 102.2f, 102.2f);

      nsCVarBool* pBool = (nsCVarBool*)nsCVar::FindCVarByName("test1_Bool2");
      CHECK_CVAR(pBool, false, true, false, false);

      nsCVarString* pString = (nsCVarString*)nsCVar::FindCVarByName("test1_String2");
      CHECK_CVAR(pString, "test1c", "test1b", "test1c", "test1c");
    }

    // CVars from Plugin 2
    {
      nsCVarInt* pInt = (nsCVarInt*)nsCVar::FindCVarByName("test2_Int");
      CHECK_CVAR(pInt, 22, 22, 22, 22);

      nsCVarFloat* pFloat = (nsCVarFloat*)nsCVar::FindCVarByName("test2_Float");
      CHECK_CVAR(pFloat, 2.2f, 2.2f, 2.2f, 2.2f);

      nsCVarBool* pBool = (nsCVarBool*)nsCVar::FindCVarByName("test2_Bool");
      CHECK_CVAR(pBool, false, true, false, false);

      nsCVarString* pString = (nsCVarString*)nsCVar::FindCVarByName("test2_String");
      CHECK_CVAR(pString, "test2_value2", "test2", "test2_value2", "test2_value2");
    }

    nsPlugin::UnloadAllPlugins();
  }

#endif

  nsFileSystem::ClearAllDataDirectories();
}
