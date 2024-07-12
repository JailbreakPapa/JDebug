#include <Foundation/FoundationPCH.h>

#include <Foundation/Application/Config/PluginConfig.h>
#include <Foundation/IO/FileSystem/DeferredFileWriter.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/IO/OpenDdlUtils.h>
#include <Foundation/IO/OpenDdlWriter.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsApplicationPluginConfig, nsNoBase, 1, nsRTTIDefaultAllocator<nsApplicationPluginConfig>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ARRAY_MEMBER_PROPERTY("Plugins", m_Plugins),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsApplicationPluginConfig_PluginConfig, nsNoBase, 1, nsRTTIDefaultAllocator<nsApplicationPluginConfig_PluginConfig>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("RelativePath", m_sAppDirRelativePath),
    NS_MEMBER_PROPERTY("LoadCopy", m_bLoadCopy),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

bool nsApplicationPluginConfig::PluginConfig::operator<(const PluginConfig& rhs) const
{
  return m_sAppDirRelativePath < rhs.m_sAppDirRelativePath;
}

bool nsApplicationPluginConfig::AddPlugin(const PluginConfig& cfg0)
{
  PluginConfig cfg = cfg0;

  for (nsUInt32 i = 0; i < m_Plugins.GetCount(); ++i)
  {
    if (m_Plugins[i].m_sAppDirRelativePath == cfg.m_sAppDirRelativePath)
    {
      return false;
    }
  }

  m_Plugins.PushBack(cfg);
  return true;
}

bool nsApplicationPluginConfig::RemovePlugin(const PluginConfig& cfg0)
{
  PluginConfig cfg = cfg0;

  for (nsUInt32 i = 0; i < m_Plugins.GetCount(); ++i)
  {
    if (m_Plugins[i].m_sAppDirRelativePath == cfg.m_sAppDirRelativePath)
    {
      m_Plugins.RemoveAtAndSwap(i);
      return true;
    }
  }

  return false;
}

nsApplicationPluginConfig::nsApplicationPluginConfig() = default;

nsResult nsApplicationPluginConfig::Save(nsStringView sPath) const
{
  m_Plugins.Sort();

  nsDeferredFileWriter file;
  file.SetOutput(sPath, true);

  nsOpenDdlWriter writer;
  writer.SetOutputStream(&file);
  writer.SetCompactMode(false);
  writer.SetPrimitiveTypeStringMode(nsOpenDdlWriter::TypeStringMode::Compliant);

  for (nsUInt32 i = 0; i < m_Plugins.GetCount(); ++i)
  {
    writer.BeginObject("Plugin");

    nsOpenDdlUtils::StoreString(writer, m_Plugins[i].m_sAppDirRelativePath, "Path");
    nsOpenDdlUtils::StoreBool(writer, m_Plugins[i].m_bLoadCopy, "LoadCopy");

    writer.EndObject();
  }

  return file.Close();
}

void nsApplicationPluginConfig::Load(nsStringView sPath)
{
  NS_LOG_BLOCK("nsApplicationPluginConfig::Load()");

  m_Plugins.Clear();

#if NS_ENABLED(NS_MIGRATE_RUNTIMECONFIGS)
  nsStringBuilder sOldLoc;
  if (sPath.FindSubString("RuntimeConfigs/"))
  {
    sOldLoc = sPath;
    sOldLoc.ReplaceLast("RuntimeConfigs/", "");
    sPath = nsFileSystem::MigrateFileLocation(sOldLoc, sPath);
  }
#endif

  nsFileReader file;
  if (file.Open(sPath).Failed())
  {
    nsLog::Warning("Could not open plugins config file '{0}'", sPath);
    return;
  }

  nsOpenDdlReader reader;
  if (reader.ParseDocument(file, 0, nsLog::GetThreadLocalLogSystem()).Failed())
  {
    nsLog::Error("Failed to parse plugins config file '{0}'", sPath);
    return;
  }

  const nsOpenDdlReaderElement* pTree = reader.GetRootElement();

  for (const nsOpenDdlReaderElement* pPlugin = pTree->GetFirstChild(); pPlugin != nullptr; pPlugin = pPlugin->GetSibling())
  {
    if (!pPlugin->IsCustomType("Plugin"))
      continue;

    PluginConfig cfg;

    const nsOpenDdlReaderElement* pPath = pPlugin->FindChildOfType(nsOpenDdlPrimitiveType::String, "Path");
    const nsOpenDdlReaderElement* pCopy = pPlugin->FindChildOfType(nsOpenDdlPrimitiveType::Bool, "LoadCopy");

    if (pPath)
    {
      cfg.m_sAppDirRelativePath = pPath->GetPrimitivesString()[0];
    }

    if (pCopy)
    {
      cfg.m_bLoadCopy = pCopy->GetPrimitivesBool()[0];
    }

    // this prevents duplicates
    AddPlugin(cfg);
  }
}

void nsApplicationPluginConfig::Apply()
{
  NS_LOG_BLOCK("nsApplicationPluginConfig::Apply");

  for (const auto& var : m_Plugins)
  {
    nsBitflags<nsPluginLoadFlags> flags;
    flags.AddOrRemove(nsPluginLoadFlags::LoadCopy, var.m_bLoadCopy);
    flags.AddOrRemove(nsPluginLoadFlags::CustomDependency, false);

    nsPlugin::LoadPlugin(var.m_sAppDirRelativePath, flags).IgnoreResult();
  }
}



NS_STATICLINK_FILE(Foundation, Foundation_Application_Config_Implementation_PluginConfig);
