#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/CVar.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Utilities/CommandLineOptions.h>
#include <Foundation/Utilities/CommandLineUtils.h>
#include <Foundation/Utilities/ConversionUtils.h>

// clang-format off
NS_ENUMERABLE_CLASS_IMPLEMENTATION(nsCVar);

// The CVars need to be saved and loaded whenever plugins are loaded and unloaded.
// Therefore we register as early as possible (Base Startup) at the plugin system,
// to be informed about plugin changes.
NS_BEGIN_SUBSYSTEM_DECLARATION(Foundation, CVars)

  // for saving and loading we need the filesystem, so make sure we are initialized after
  // and shutdown before the filesystem is
  BEGIN_SUBSYSTEM_DEPENDENCIES
    "FileSystem"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    nsPlugin::Events().AddEventHandler(nsCVar::PluginEventHandler);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    // save the CVars every time the core is shut down
    // at this point the filesystem might already be uninitialized by the user (data dirs)
    // in that case the variables cannot be saved, but it will fail silently
    // if it succeeds, the most recent state will be serialized though
    nsCVar::SaveCVars();

    nsPlugin::Events().RemoveEventHandler(nsCVar::PluginEventHandler);
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
    // save the CVars every time the engine is shut down
    // at this point the filesystem should usually still be configured properly
    nsCVar::SaveCVars();
  }

  // The user is responsible to call 'nsCVar::SetStorageFolder' to define where the CVars are
  // actually stored. That call will automatically load all CVar states.

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on


nsString nsCVar::s_sStorageFolder;
nsEvent<const nsCVarEvent&> nsCVar::s_AllCVarEvents;

void nsCVar::AssignSubSystemPlugin(nsStringView sPluginName)
{
  nsCVar* pCVar = nsCVar::GetFirstInstance();

  while (pCVar)
  {
    if (pCVar->m_sPluginName.IsEmpty())
      pCVar->m_sPluginName = sPluginName;

    pCVar = pCVar->GetNextInstance();
  }
}

void nsCVar::PluginEventHandler(const nsPluginEvent& EventData)
{
  switch (EventData.m_EventType)
  {
    case nsPluginEvent::BeforeLoading:
    {
      // before a new plugin is loaded, make sure all currently available CVars
      // are assigned to the proper plugin
      // all not-yet assigned cvars cannot be in any plugin, so assign them to the 'static' plugin
      AssignSubSystemPlugin("Static");
    }
    break;

    case nsPluginEvent::AfterLoadingBeforeInit:
    {
      // after we loaded a new plugin, but before it is initialized,
      // find all new CVars and assign them to that new plugin
      AssignSubSystemPlugin(EventData.m_sPluginBinary);

      // now load the state of all CVars
      LoadCVars();
    }
    break;

    case nsPluginEvent::BeforeUnloading:
    {
      SaveCVars();
    }
    break;

    default:
      break;
  }
}

nsCVar::nsCVar(nsStringView sName, nsBitflags<nsCVarFlags> Flags, nsStringView sDescription)
  : m_sName(sName)
  , m_sDescription(sDescription)
  , m_Flags(Flags)
{
  NS_ASSERT_DEV(!m_sDescription.IsEmpty(), "Please add a useful description for CVar '{}'.", sName);
}

nsCVar* nsCVar::FindCVarByName(nsStringView sName)
{
  nsCVar* pCVar = nsCVar::GetFirstInstance();

  while (pCVar)
  {
    if (pCVar->GetName() == sName)
      return pCVar;

    pCVar = pCVar->GetNextInstance();
  }

  return nullptr;
}

void nsCVar::SetStorageFolder(nsStringView sFolder)
{
  s_sStorageFolder = sFolder;
}

nsCommandLineOptionBool opt_NoFileCVars("cvar", "-no-file-cvars", "Disables loading CVar values from the user-specific, persisted configuration file.", false);

void nsCVar::SaveCVarsToFile(nsStringView sPath, bool bIgnoreSaveFlag)
{
  nsHybridArray<nsCVar*, 128> allCVars;

  for (nsCVar* pCVar = nsCVar::GetFirstInstance(); pCVar != nullptr; pCVar = pCVar->GetNextInstance())
  {
    if (bIgnoreSaveFlag || pCVar->GetFlags().IsAnySet(nsCVarFlags::Save))
    {
      allCVars.PushBack(pCVar);
    }
  }

  SaveCVarsToFileInternal(sPath, allCVars);
}

void nsCVar::SaveCVars()
{
  if (s_sStorageFolder.IsEmpty())
    return;

  // this command line disables loading and saving CVars to and from files
  if (opt_NoFileCVars.GetOptionValue(nsCommandLineOption::LogMode::FirstTimeIfSpecified))
    return;

  // first gather all the cvars by plugin
  nsMap<nsString, nsHybridArray<nsCVar*, 128>> PluginCVars;

  {
    nsCVar* pCVar = nsCVar::GetFirstInstance();
    while (pCVar)
    {
      // only store cvars that should be saved
      if (pCVar->GetFlags().IsAnySet(nsCVarFlags::Save))
      {
        if (!pCVar->m_sPluginName.IsEmpty())
          PluginCVars[pCVar->m_sPluginName].PushBack(pCVar);
        else
          PluginCVars["Static"].PushBack(pCVar);
      }

      pCVar = pCVar->GetNextInstance();
    }
  }

  nsMap<nsString, nsHybridArray<nsCVar*, 128>>::Iterator it = PluginCVars.GetIterator();

  nsStringBuilder sTemp;

  // now save all cvars in their plugin specific file
  while (it.IsValid())
  {
    // create the plugin specific file
    sTemp.SetFormat("{0}/CVars_{1}.cfg", s_sStorageFolder, it.Key());

    SaveCVarsToFileInternal(sTemp, it.Value());

    // continue with the next plugin
    ++it;
  }
}

void nsCVar::SaveCVarsToFileInternal(nsStringView path, const nsDynamicArray<nsCVar*>& vars)
{
  nsStringBuilder sTemp;
  nsFileWriter File;
  if (File.Open(path.GetData(sTemp)) == NS_SUCCESS)
  {
    // write one line for each cvar, to save its current value
    for (nsUInt32 var = 0; var < vars.GetCount(); ++var)
    {
      nsCVar* pCVar = vars[var];

      switch (pCVar->GetType())
      {
        case nsCVarType::Int:
        {
          nsCVarInt* pInt = (nsCVarInt*)pCVar;
          sTemp.SetFormat("{0} = {1}\n", pCVar->GetName(), pInt->GetValue(nsCVarValue::DelayedSync));
        }
        break;
        case nsCVarType::Bool:
        {
          nsCVarBool* pBool = (nsCVarBool*)pCVar;
          sTemp.SetFormat("{0} = {1}\n", pCVar->GetName(), pBool->GetValue(nsCVarValue::DelayedSync) ? "true" : "false");
        }
        break;
        case nsCVarType::Float:
        {
          nsCVarFloat* pFloat = (nsCVarFloat*)pCVar;
          sTemp.SetFormat("{0} = {1}\n", pCVar->GetName(), pFloat->GetValue(nsCVarValue::DelayedSync));
        }
        break;
        case nsCVarType::String:
        {
          nsCVarString* pString = (nsCVarString*)pCVar;
          sTemp.SetFormat("{0} = \"{1}\"\n", pCVar->GetName(), pString->GetValue(nsCVarValue::DelayedSync));
        }
        break;
        default:
          NS_REPORT_FAILURE("Unknown CVar Type: {0}", pCVar->GetType());
          break;
      }

      // add the one line for that cvar to the config file
      File.WriteBytes(sTemp.GetData(), sTemp.GetElementCount()).IgnoreResult();
    }
  }
}

void nsCVar::LoadCVars(bool bOnlyNewOnes /*= true*/, bool bSetAsCurrentValue /*= true*/)
{
  LoadCVarsFromCommandLine(bOnlyNewOnes, bSetAsCurrentValue);
  LoadCVarsFromFile(bOnlyNewOnes, bSetAsCurrentValue);
}

static nsResult ParseLine(const nsString& sLine, nsStringBuilder& out_sVarName, nsStringBuilder& out_sVarValue)
{
  const char* szSign = sLine.FindSubString("=");

  if (szSign == nullptr)
    return NS_FAILURE;

  {
    nsStringView sSubString(sLine.GetData(), szSign);

    // remove all trailing spaces
    while (sSubString.EndsWith(" "))
      sSubString.Shrink(0, 1);

    out_sVarName = sSubString;
  }

  {
    nsStringView sSubString(szSign + 1);

    // remove all spaces
    while (sSubString.StartsWith(" "))
      sSubString.Shrink(1, 0);

    // remove all trailing spaces
    while (sSubString.EndsWith(" "))
      sSubString.Shrink(0, 1);


    // remove " and start and end

    if (sSubString.StartsWith("\""))
      sSubString.Shrink(1, 0);

    if (sSubString.EndsWith("\""))
      sSubString.Shrink(0, 1);

    out_sVarValue = sSubString;
  }

  return NS_SUCCESS;
}

void nsCVar::LoadCVarsFromFile(bool bOnlyNewOnes, bool bSetAsCurrentValue, nsDynamicArray<nsCVar*>* pOutCVars)
{
  if (s_sStorageFolder.IsEmpty())
    return;

  // this command line disables loading and saving CVars to and from files
  if (opt_NoFileCVars.GetOptionValue(nsCommandLineOption::LogMode::FirstTimeIfSpecified))
    return;

  nsMap<nsString, nsHybridArray<nsCVar*, 128>> PluginCVars;

  // first gather all the cvars by plugin
  {
    for (nsCVar* pCVar = nsCVar::GetFirstInstance(); pCVar != nullptr; pCVar = pCVar->GetNextInstance())
    {
      // only load cvars that should be saved
      if (pCVar->GetFlags().IsAnySet(nsCVarFlags::Save))
      {
        if (!bOnlyNewOnes || pCVar->m_bHasNeverBeenLoaded)
        {
          if (!pCVar->m_sPluginName.IsEmpty())
            PluginCVars[pCVar->m_sPluginName].PushBack(pCVar);
          else
            PluginCVars["Static"].PushBack(pCVar);
        }
      }

      // it doesn't matter whether the CVar could be loaded from file, either it works the first time, or it stays at its current value
      pCVar->m_bHasNeverBeenLoaded = false;
    }
  }

  {
    nsMap<nsString, nsHybridArray<nsCVar*, 128>>::Iterator it = PluginCVars.GetIterator();

    nsStringBuilder sTemp;

    while (it.IsValid())
    {
      // create the plugin specific file
      sTemp.SetFormat("{0}/CVars_{1}.cfg", s_sStorageFolder, it.Key());

      LoadCVarsFromFileInternal(sTemp.GetView(), it.Value(), bOnlyNewOnes, bSetAsCurrentValue, pOutCVars);

      // continue with the next plugin
      ++it;
    }
  }
}

void nsCVar::LoadCVarsFromFile(nsStringView sPath, bool bOnlyNewOnes, bool bSetAsCurrentValue, bool bIgnoreSaveFlag, nsDynamicArray<nsCVar*>* pOutCVars)
{
  nsHybridArray<nsCVar*, 128> allCVars;

  for (nsCVar* pCVar = nsCVar::GetFirstInstance(); pCVar != nullptr; pCVar = pCVar->GetNextInstance())
  {
    if (bIgnoreSaveFlag || pCVar->GetFlags().IsAnySet(nsCVarFlags::Save))
    {
      if (!bOnlyNewOnes || pCVar->m_bHasNeverBeenLoaded)
      {
        allCVars.PushBack(pCVar);
      }
    }

    // it doesn't matter whether the CVar could be loaded from file, either it works the first time, or it stays at its current value
    pCVar->m_bHasNeverBeenLoaded = false;
  }

  LoadCVarsFromFileInternal(sPath, allCVars, bOnlyNewOnes, bSetAsCurrentValue, pOutCVars);
}

void nsCVar::LoadCVarsFromFileInternal(nsStringView path, const nsDynamicArray<nsCVar*>& vars, bool bOnlyNewOnes, bool bSetAsCurrentValue, nsDynamicArray<nsCVar*>* pOutCVars)
{
  nsFileReader File;
  nsStringBuilder sTemp;

  if (File.Open(path.GetData(sTemp)) == NS_SUCCESS)
  {
    nsStringBuilder sContent;
    sContent.ReadAll(File);

    nsDynamicArray<nsString> Lines;
    sContent.ReplaceAll("\r", ""); // remove carriage return

    // splits the string at occurrence of '\n' and adds each line to the 'Lines' container
    sContent.Split(true, Lines, "\n");

    nsStringBuilder sVarName;
    nsStringBuilder sVarValue;

    for (const nsString& sLine : Lines)
    {
      if (ParseLine(sLine, sVarName, sVarValue) == NS_FAILURE)
        continue;

      // now find a variable with the same name
      for (nsUInt32 var = 0; var < vars.GetCount(); ++var)
      {
        nsCVar* pCVar = vars[var];

        if (!sVarName.IsEqual(pCVar->GetName()))
          continue;

        // found the cvar, now convert the text into the proper value *sigh*
        switch (pCVar->GetType())
        {
          case nsCVarType::Int:
          {
            nsInt32 Value = 0;
            if (nsConversionUtils::StringToInt(sVarValue, Value).Succeeded())
            {
              nsCVarInt* pTyped = (nsCVarInt*)pCVar;
              pTyped->m_Values[nsCVarValue::Stored] = Value;
              *pTyped = Value;
            }
          }
          break;
          case nsCVarType::Bool:
          {
            bool Value = sVarValue.IsEqual_NoCase("true");

            nsCVarBool* pTyped = (nsCVarBool*)pCVar;
            pTyped->m_Values[nsCVarValue::Stored] = Value;
            *pTyped = Value;
          }
          break;
          case nsCVarType::Float:
          {
            double Value = 0.0;
            if (nsConversionUtils::StringToFloat(sVarValue, Value).Succeeded())
            {
              nsCVarFloat* pTyped = (nsCVarFloat*)pCVar;
              pTyped->m_Values[nsCVarValue::Stored] = static_cast<float>(Value);
              *pTyped = static_cast<float>(Value);
            }
          }
          break;
          case nsCVarType::String:
          {
            const char* Value = sVarValue.GetData();

            nsCVarString* pTyped = (nsCVarString*)pCVar;
            pTyped->m_Values[nsCVarValue::Stored] = Value;
            *pTyped = Value;
          }
          break;
          default:
            NS_REPORT_FAILURE("Unknown CVar Type: {0}", pCVar->GetType());
            break;
        }

        if (pOutCVars)
        {
          pOutCVars->PushBack(pCVar);
        }

        if (bSetAsCurrentValue)
          pCVar->SetToDelayedSyncValue();
      }
    }
  }
}

nsCommandLineOptionDoc opt_CVar("cvar", "-CVarName", "<value>", "Forces a CVar to the given value.\n\
Overrides persisted settings.\n\
Examples:\n\
-MyIntVar 42\n\
-MyStringVar \"Hello\"\n\
",
  nullptr);

void nsCVar::LoadCVarsFromCommandLine(bool bOnlyNewOnes /*= true*/, bool bSetAsCurrentValue /*= true*/, nsDynamicArray<nsCVar*>* pOutCVars /*= nullptr*/)
{
  nsStringBuilder sTemp;

  for (nsCVar* pCVar = nsCVar::GetFirstInstance(); pCVar != nullptr; pCVar = pCVar->GetNextInstance())
  {
    if (bOnlyNewOnes && !pCVar->m_bHasNeverBeenLoaded)
      continue;

    sTemp.Set("-", pCVar->GetName());

    if (nsCommandLineUtils::GetGlobalInstance()->GetOptionIndex(sTemp) != -1)
    {
      if (pOutCVars)
      {
        pOutCVars->PushBack(pCVar);
      }

      // has been specified on the command line -> mark it as 'has been loaded'
      pCVar->m_bHasNeverBeenLoaded = false;

      switch (pCVar->GetType())
      {
        case nsCVarType::Int:
        {
          nsCVarInt* pTyped = (nsCVarInt*)pCVar;
          nsInt32 Value = pTyped->m_Values[nsCVarValue::Stored];
          Value = nsCommandLineUtils::GetGlobalInstance()->GetIntOption(sTemp, Value);

          pTyped->m_Values[nsCVarValue::Stored] = Value;
          *pTyped = Value;
        }
        break;
        case nsCVarType::Bool:
        {
          nsCVarBool* pTyped = (nsCVarBool*)pCVar;
          bool Value = pTyped->m_Values[nsCVarValue::Stored];
          Value = nsCommandLineUtils::GetGlobalInstance()->GetBoolOption(sTemp, Value);

          pTyped->m_Values[nsCVarValue::Stored] = Value;
          *pTyped = Value;
        }
        break;
        case nsCVarType::Float:
        {
          nsCVarFloat* pTyped = (nsCVarFloat*)pCVar;
          double Value = pTyped->m_Values[nsCVarValue::Stored];
          Value = nsCommandLineUtils::GetGlobalInstance()->GetFloatOption(sTemp, Value);

          pTyped->m_Values[nsCVarValue::Stored] = static_cast<float>(Value);
          *pTyped = static_cast<float>(Value);
        }
        break;
        case nsCVarType::String:
        {
          nsCVarString* pTyped = (nsCVarString*)pCVar;
          nsString Value = nsCommandLineUtils::GetGlobalInstance()->GetStringOption(sTemp, 0, pTyped->m_Values[nsCVarValue::Stored]);

          pTyped->m_Values[nsCVarValue::Stored] = Value;
          *pTyped = Value;
        }
        break;
        default:
          NS_REPORT_FAILURE("Unknown CVar Type: {0}", pCVar->GetType());
          break;
      }

      if (bSetAsCurrentValue)
        pCVar->SetToDelayedSyncValue();
    }
  }
}

void nsCVar::ListOfCVarsChanged(nsStringView sSetPluginNameTo)
{
  AssignSubSystemPlugin(sSetPluginNameTo);

  LoadCVars();

  nsCVarEvent e(nullptr);
  e.m_EventType = nsCVarEvent::Type::ListOfVarsChanged;

  s_AllCVarEvents.Broadcast(e);
}


NS_STATICLINK_FILE(Foundation, Foundation_Configuration_Implementation_CVar);
