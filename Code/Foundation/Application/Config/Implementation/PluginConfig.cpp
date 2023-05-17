#include <Foundation/FoundationPCH.h>

#include <Foundation/Application/Config/PluginConfig.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/IO/OpenDdlUtils.h>
#include <Foundation/IO/OpenDdlWriter.h>

// clang-format off
WD_BEGIN_STATIC_REFLECTED_TYPE(wdApplicationPluginConfig, wdNoBase, 1, wdRTTIDefaultAllocator<wdApplicationPluginConfig>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_ARRAY_MEMBER_PROPERTY("Plugins", m_Plugins),
  }
  WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdApplicationPluginConfig_PluginConfig, wdNoBase, 1, wdRTTIDefaultAllocator<wdApplicationPluginConfig_PluginConfig>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("RelativePath", m_sAppDirRelativePath),
    WD_MEMBER_PROPERTY("LoadCopy", m_bLoadCopy),
  }
  WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;
// clang-format on

bool wdApplicationPluginConfig::PluginConfig::operator<(const PluginConfig& rhs) const
{
  return m_sAppDirRelativePath < rhs.m_sAppDirRelativePath;
}

bool wdApplicationPluginConfig::AddPlugin(const PluginConfig& cfg0)
{
  PluginConfig cfg = cfg0;

  for (wdUInt32 i = 0; i < m_Plugins.GetCount(); ++i)
  {
    if (m_Plugins[i].m_sAppDirRelativePath == cfg.m_sAppDirRelativePath)
    {
      return false;
    }
  }

  m_Plugins.PushBack(cfg);
  return true;
}


bool wdApplicationPluginConfig::RemovePlugin(const PluginConfig& cfg0)
{
  PluginConfig cfg = cfg0;

  for (wdUInt32 i = 0; i < m_Plugins.GetCount(); ++i)
  {
    if (m_Plugins[i].m_sAppDirRelativePath == cfg.m_sAppDirRelativePath)
    {
      m_Plugins.RemoveAtAndSwap(i);
      return true;
    }
  }

  return false;
}

wdApplicationPluginConfig::wdApplicationPluginConfig() = default;

wdResult wdApplicationPluginConfig::Save(wdStringView sPath) const
{
  m_Plugins.Sort();

  wdFileWriter file;
  if (file.Open(sPath).Failed())
    return WD_FAILURE;

  wdOpenDdlWriter writer;
  writer.SetOutputStream(&file);
  writer.SetCompactMode(false);
  writer.SetPrimitiveTypeStringMode(wdOpenDdlWriter::TypeStringMode::Compliant);

  for (wdUInt32 i = 0; i < m_Plugins.GetCount(); ++i)
  {
    writer.BeginObject("Plugin");

    wdOpenDdlUtils::StoreString(writer, m_Plugins[i].m_sAppDirRelativePath, "Path");
    wdOpenDdlUtils::StoreBool(writer, m_Plugins[i].m_bLoadCopy, "LoadCopy");

    writer.EndObject();
  }

  return WD_SUCCESS;
}

void wdApplicationPluginConfig::Load(wdStringView sPath)
{
  WD_LOG_BLOCK("wdApplicationPluginConfig::Load()");

  m_Plugins.Clear();

#if WD_ENABLED(WD_MIGRATE_RUNTIMECONFIGS)
  wdStringBuilder sOldLoc;
  if (sPath.FindSubString("RuntimeConfigs/"))
  {
    sOldLoc = sPath;
    sOldLoc.ReplaceLast("RuntimeConfigs/", "");
    sPath = wdFileSystem::MigrateFileLocation(sOldLoc, sPath);
  }
#endif

  wdFileReader file;
  if (file.Open(sPath).Failed())
  {
    wdLog::Warning("Could not open plugins config file '{0}'", sPath);
    return;
  }

  wdOpenDdlReader reader;
  if (reader.ParseDocument(file, 0, wdLog::GetThreadLocalLogSystem()).Failed())
  {
    wdLog::Error("Failed to parse plugins config file '{0}'", sPath);
    return;
  }

  const wdOpenDdlReaderElement* pTree = reader.GetRootElement();

  for (const wdOpenDdlReaderElement* pPlugin = pTree->GetFirstChild(); pPlugin != nullptr; pPlugin = pPlugin->GetSibling())
  {
    if (!pPlugin->IsCustomType("Plugin"))
      continue;

    PluginConfig cfg;

    const wdOpenDdlReaderElement* pPath = pPlugin->FindChildOfType(wdOpenDdlPrimitiveType::String, "Path");
    const wdOpenDdlReaderElement* pCopy = pPlugin->FindChildOfType(wdOpenDdlPrimitiveType::Bool, "LoadCopy");

    if (pPath)
      cfg.m_sAppDirRelativePath = pPath->GetPrimitivesString()[0];

    if (pCopy)
    {
      cfg.m_bLoadCopy = pCopy->GetPrimitivesBool()[0];
    }

    // this prevents duplicates
    AddPlugin(cfg);
  }
}

void wdApplicationPluginConfig::Apply()
{
  WD_LOG_BLOCK("wdApplicationPluginConfig::Apply");

  for (const auto& var : m_Plugins)
  {
    wdBitflags<wdPluginLoadFlags> flags;
    flags.AddOrRemove(wdPluginLoadFlags::LoadCopy, var.m_bLoadCopy);
    flags.AddOrRemove(wdPluginLoadFlags::CustomDependency, false);

    wdPlugin::LoadPlugin(var.m_sAppDirRelativePath, flags).IgnoreResult();
  }
}



WD_STATICLINK_FILE(Foundation, Foundation_Application_Config_Implementation_PluginConfig);
