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
WD_ENUMERABLE_CLASS_IMPLEMENTATION(wdCVar);

// The CVars need to be saved and loaded whenever plugins are loaded and unloaded.
// Therefore we register as early as possible (Base Startup) at the plugin system,
// to be informed about plugin changes.
WD_BEGIN_SUBSYSTEM_DECLARATION(Foundation, CVars)

  // for saving and loading we need the filesystem, so make sure we are initialized after
  // and shutdown before the filesystem is
  BEGIN_SUBSYSTEM_DEPENDENCIES
    "FileSystem"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    wdPlugin::Events().AddEventHandler(wdCVar::PluginEventHandler);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    // save the CVars every time the core is shut down
    // at this point the filesystem might already be uninitialized by the user (data dirs)
    // in that case the variables cannot be saved, but it will fail silently
    // if it succeeds, the most recent state will be serialized though
    wdCVar::SaveCVars();

    wdPlugin::Events().RemoveEventHandler(wdCVar::PluginEventHandler);
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
    // save the CVars every time the engine is shut down
    // at this point the filesystem should usually still be configured properly
    wdCVar::SaveCVars();
  }

  // The user is responsible to call 'wdCVar::SetStorageFolder' to define where the CVars are
  // actually stored. That call will automatically load all CVar states.

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on


wdString wdCVar::s_sStorageFolder;
wdEvent<const wdCVarEvent&> wdCVar::s_AllCVarEvents;

void wdCVar::AssignSubSystemPlugin(wdStringView sPluginName)
{
  wdCVar* pCVar = wdCVar::GetFirstInstance();

  while (pCVar)
  {
    if (pCVar->m_sPluginName.IsEmpty())
      pCVar->m_sPluginName = sPluginName;

    pCVar = pCVar->GetNextInstance();
  }
}

void wdCVar::PluginEventHandler(const wdPluginEvent& EventData)
{
  switch (EventData.m_EventType)
  {
    case wdPluginEvent::BeforeLoading:
    {
      // before a new plugin is loaded, make sure all currently available CVars
      // are assigned to the proper plugin
      // all not-yet assigned cvars cannot be in any plugin, so assign them to the 'static' plugin
      AssignSubSystemPlugin("Static");
    }
    break;

    case wdPluginEvent::AfterLoadingBeforeInit:
    {
      // after we loaded a new plugin, but before it is initialized,
      // find all new CVars and assign them to that new plugin
      AssignSubSystemPlugin(EventData.m_szPluginBinary);

      // now load the state of all CVars
      LoadCVars();
    }
    break;

    case wdPluginEvent::BeforeUnloading:
    {
      SaveCVars();
    }
    break;

    default:
      break;
  }
}

wdCVar::wdCVar(wdStringView sName, wdBitflags<wdCVarFlags> Flags, wdStringView sDescription)
{
  m_bHasNeverBeenLoaded = true; // next time 'LoadCVars' is called, its state will be changed

  m_sName = sName;
  m_Flags = Flags;
  m_sDescription = sDescription;

  // 'RequiresRestart' only works together with 'Save'
  if (m_Flags.IsAnySet(wdCVarFlags::RequiresRestart))
    m_Flags.Add(wdCVarFlags::Save);

  WD_ASSERT_DEV(!m_sDescription.IsEmpty(), "Please add a useful description for CVar '{}'.", sName);
}

wdCVar* wdCVar::FindCVarByName(wdStringView sName)
{
  wdCVar* pCVar = wdCVar::GetFirstInstance();

  while (pCVar)
  {
    if (pCVar->GetName() == sName)
      return pCVar;

    pCVar = pCVar->GetNextInstance();
  }

  return nullptr;
}

void wdCVar::SetStorageFolder(wdStringView sFolder)
{
  s_sStorageFolder = sFolder;
}

wdCommandLineOptionBool opt_NoFileCVars("cvar", "-no-file-cvars", "Disables loading CVar values from the user-specific, persisted configuration file.", false);

void wdCVar::SaveCVars()
{
  if (s_sStorageFolder.IsEmpty())
    return;

  // this command line disables loading and saving CVars to and from files
  if (opt_NoFileCVars.GetOptionValue(wdCommandLineOption::LogMode::FirstTimeIfSpecified))
    return;

  // first gather all the cvars by plugin
  wdMap<wdString, wdHybridArray<wdCVar*, 128>> PluginCVars;

  {
    wdCVar* pCVar = wdCVar::GetFirstInstance();
    while (pCVar)
    {
      // only store cvars that should be saved
      if (pCVar->GetFlags().IsAnySet(wdCVarFlags::Save))
      {
        if (!pCVar->m_sPluginName.IsEmpty())
          PluginCVars[pCVar->m_sPluginName].PushBack(pCVar);
        else
          PluginCVars["Static"].PushBack(pCVar);
      }

      pCVar = pCVar->GetNextInstance();
    }
  }

  wdMap<wdString, wdHybridArray<wdCVar*, 128>>::Iterator it = PluginCVars.GetIterator();

  wdStringBuilder sTemp;

  // now save all cvars in their plugin specific file
  while (it.IsValid())
  {
    // create the plugin specific file
    sTemp.Format("{0}/CVars_{1}.cfg", s_sStorageFolder, it.Key());

    wdFileWriter File;
    if (File.Open(sTemp.GetData()) == WD_SUCCESS)
    {
      // write one line for each cvar, to save its current value
      for (wdUInt32 var = 0; var < it.Value().GetCount(); ++var)
      {
        wdCVar* pCVar = it.Value()[var];

        switch (pCVar->GetType())
        {
          case wdCVarType::Int:
          {
            wdCVarInt* pInt = (wdCVarInt*)pCVar;
            sTemp.Format("{0} = {1}\n", pCVar->GetName(), pInt->GetValue(wdCVarValue::Restart));
          }
          break;
          case wdCVarType::Bool:
          {
            wdCVarBool* pBool = (wdCVarBool*)pCVar;
            sTemp.Format("{0} = {1}\n", pCVar->GetName(), pBool->GetValue(wdCVarValue::Restart) ? "true" : "false");
          }
          break;
          case wdCVarType::Float:
          {
            wdCVarFloat* pFloat = (wdCVarFloat*)pCVar;
            sTemp.Format("{0} = {1}\n", pCVar->GetName(), pFloat->GetValue(wdCVarValue::Restart));
          }
          break;
          case wdCVarType::String:
          {
            wdCVarString* pString = (wdCVarString*)pCVar;
            sTemp.Format("{0} = \"{1}\"\n", pCVar->GetName(), pString->GetValue(wdCVarValue::Restart));
          }
          break;
          default:
            WD_REPORT_FAILURE("Unknown CVar Type: {0}", pCVar->GetType());
            break;
        }

        // add the one line for that cvar to the config file
        File.WriteBytes(sTemp.GetData(), sTemp.GetElementCount()).IgnoreResult();
      }
    }

    // continue with the next plugin
    ++it;
  }
}

void wdCVar::LoadCVars(bool bOnlyNewOnes /*= true*/, bool bSetAsCurrentValue /*= true*/)
{
  LoadCVarsFromCommandLine(bOnlyNewOnes, bSetAsCurrentValue);
  LoadCVarsFromFile(bOnlyNewOnes, bSetAsCurrentValue);
}

static wdResult ReadLine(wdStreamReader& inout_stream, wdStringBuilder& ref_sLine)
{
  ref_sLine.Clear();

  char c[2];
  c[0] = '\0';
  c[1] = '\0';

  // read the first character
  if (inout_stream.ReadBytes(c, 1) == 0)
    return WD_FAILURE;

  // skip all white-spaces at the beginning
  // also skip all empty lines
  while ((c[0] == '\n' || c[0] == '\r' || c[0] == ' ' || c[0] == '\t') && (inout_stream.ReadBytes(c, 1) > 0))
  {
  }

  // we found something that is not empty, so now read till the end of the line
  while (c[0] != '\0' && c[0] != '\n')
  {
    // skip all tabs and carriage returns
    if (c[0] != '\r' && c[0] != '\t')
    {
      ref_sLine.Append(c);
    }

    // stop if we reached the end of the file
    if (inout_stream.ReadBytes(c, 1) == 0)
      break;
  }

  if (ref_sLine.IsEmpty())
    return WD_FAILURE;

  return WD_SUCCESS;
}

static wdResult ParseLine(const wdStringBuilder& sLine, wdStringBuilder& ref_sVarName, wdStringBuilder& ref_sVarValue)
{
  const char* szSign = sLine.FindSubString("=");

  if (szSign == nullptr)
    return WD_FAILURE;

  {
    wdStringView sSubString(sLine.GetData(), szSign);

    // remove all trailing spaces
    while (sSubString.EndsWith(" "))
      sSubString.Shrink(0, 1);

    ref_sVarName = sSubString;
  }

  {
    wdStringView sSubString(szSign + 1);

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

    ref_sVarValue = sSubString;
  }

  return WD_SUCCESS;
}

void wdCVar::LoadCVarsFromFile(bool bOnlyNewOnes, bool bSetAsCurrentValue)
{
  if (s_sStorageFolder.IsEmpty())
    return;

  // this command line disables loading and saving CVars to and from files
  if (opt_NoFileCVars.GetOptionValue(wdCommandLineOption::LogMode::FirstTimeIfSpecified))
    return;

  wdMap<wdString, wdHybridArray<wdCVar*, 128>> PluginCVars;

  // first gather all the cvars by plugin
  {
    for (wdCVar* pCVar = wdCVar::GetFirstInstance(); pCVar != nullptr; pCVar = pCVar->GetNextInstance())
    {
      // only load cvars that should be saved
      if (pCVar->GetFlags().IsAnySet(wdCVarFlags::Save))
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

  // now load all cvars from their plugin specific file
  {
    wdMap<wdString, wdHybridArray<wdCVar*, 128>>::Iterator it = PluginCVars.GetIterator();

    wdStringBuilder sTemp;

    while (it.IsValid())
    {
      // create the plugin specific file
      sTemp.Format("{0}/CVars_{1}.cfg", s_sStorageFolder, it.Key());

      wdFileReader File;
      if (File.Open(sTemp.GetData()) == WD_SUCCESS)
      {
        wdStringBuilder sLine, sVarName, sVarValue;
        while (ReadLine(File, sLine) == WD_SUCCESS)
        {
          if (ParseLine(sLine, sVarName, sVarValue) == WD_FAILURE)
            continue;

          // now find a variable with the same name
          for (wdUInt32 var = 0; var < it.Value().GetCount(); ++var)
          {
            wdCVar* pCVar = it.Value()[var];

            if (!sVarName.IsEqual(pCVar->GetName()))
              continue;

            // found the cvar, now convert the text into the proper value *sigh*

            switch (pCVar->GetType())
            {
              case wdCVarType::Int:
              {
                wdInt32 Value = 0;
                if (wdConversionUtils::StringToInt(sVarValue, Value).Succeeded())
                {
                  wdCVarInt* pTyped = (wdCVarInt*)pCVar;
                  pTyped->m_Values[wdCVarValue::Stored] = Value;
                  *pTyped = Value;
                }
              }
              break;
              case wdCVarType::Bool:
              {
                bool Value = sVarValue.IsEqual_NoCase("true");

                wdCVarBool* pTyped = (wdCVarBool*)pCVar;
                pTyped->m_Values[wdCVarValue::Stored] = Value;
                *pTyped = Value;
              }
              break;
              case wdCVarType::Float:
              {
                double Value = 0.0;
                if (wdConversionUtils::StringToFloat(sVarValue, Value).Succeeded())
                {
                  wdCVarFloat* pTyped = (wdCVarFloat*)pCVar;
                  pTyped->m_Values[wdCVarValue::Stored] = static_cast<float>(Value);
                  *pTyped = static_cast<float>(Value);
                }
              }
              break;
              case wdCVarType::String:
              {
                const char* Value = sVarValue.GetData();

                wdCVarString* pTyped = (wdCVarString*)pCVar;
                pTyped->m_Values[wdCVarValue::Stored] = Value;
                *pTyped = Value;
              }
              break;
              default:
                WD_REPORT_FAILURE("Unknown CVar Type: {0}", pCVar->GetType());
                break;
            }

            if (bSetAsCurrentValue)
              pCVar->SetToRestartValue();
          }
        }
      }

      // continue with the next plugin
      ++it;
    }
  }
}

wdCommandLineOptionDoc opt_CVar("cvar", "-CVarName", "<value>", "Forces a CVar to the given value.\n\
Overrides persisted settings.\n\
Examples:\n\
-MyIntVar 42\n\
-MyStringVar \"Hello\"\n\
",
  nullptr);

void wdCVar::LoadCVarsFromCommandLine(bool bOnlyNewOnes /*= true*/, bool bSetAsCurrentValue /*= true*/)
{
  wdStringBuilder sTemp;

  for (wdCVar* pCVar = wdCVar::GetFirstInstance(); pCVar != nullptr; pCVar = pCVar->GetNextInstance())
  {
    if (bOnlyNewOnes && !pCVar->m_bHasNeverBeenLoaded)
      continue;

    sTemp.Set("-", pCVar->GetName());

    if (wdCommandLineUtils::GetGlobalInstance()->GetOptionIndex(sTemp) != -1)
    {
      // has been specified on the command line -> mark it as 'has been loaded'
      pCVar->m_bHasNeverBeenLoaded = false;

      switch (pCVar->GetType())
      {
        case wdCVarType::Int:
        {
          wdCVarInt* pTyped = (wdCVarInt*)pCVar;
          wdInt32 Value = pTyped->m_Values[wdCVarValue::Stored];
          Value = wdCommandLineUtils::GetGlobalInstance()->GetIntOption(sTemp, Value);

          pTyped->m_Values[wdCVarValue::Stored] = Value;
          *pTyped = Value;
        }
        break;
        case wdCVarType::Bool:
        {
          wdCVarBool* pTyped = (wdCVarBool*)pCVar;
          bool Value = pTyped->m_Values[wdCVarValue::Stored];
          Value = wdCommandLineUtils::GetGlobalInstance()->GetBoolOption(sTemp, Value);

          pTyped->m_Values[wdCVarValue::Stored] = Value;
          *pTyped = Value;
        }
        break;
        case wdCVarType::Float:
        {
          wdCVarFloat* pTyped = (wdCVarFloat*)pCVar;
          double Value = pTyped->m_Values[wdCVarValue::Stored];
          Value = wdCommandLineUtils::GetGlobalInstance()->GetFloatOption(sTemp, Value);

          pTyped->m_Values[wdCVarValue::Stored] = static_cast<float>(Value);
          *pTyped = static_cast<float>(Value);
        }
        break;
        case wdCVarType::String:
        {
          wdCVarString* pTyped = (wdCVarString*)pCVar;
          wdString Value = wdCommandLineUtils::GetGlobalInstance()->GetStringOption(sTemp, 0, pTyped->m_Values[wdCVarValue::Stored]);

          pTyped->m_Values[wdCVarValue::Stored] = Value;
          *pTyped = Value;
        }
        break;
        default:
          WD_REPORT_FAILURE("Unknown CVar Type: {0}", pCVar->GetType());
          break;
      }

      if (bSetAsCurrentValue)
        pCVar->SetToRestartValue();
    }
  }
}

void wdCVar::ListOfCVarsChanged(wdStringView sSetPluginNameTo)
{
  AssignSubSystemPlugin(sSetPluginNameTo);

  LoadCVars();

  wdCVarEvent e(nullptr);
  e.m_EventType = wdCVarEvent::Type::ListOfVarsChanged;

  s_AllCVarEvents.Broadcast(e);
}


WD_STATICLINK_FILE(Foundation, Foundation_Configuration_Implementation_CVar);
