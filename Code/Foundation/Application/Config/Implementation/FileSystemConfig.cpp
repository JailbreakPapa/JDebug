#include <Foundation/FoundationPCH.h>

#include <Foundation/Application/Config/FileSystemConfig.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/IO/OpenDdlUtils.h>
#include <Foundation/IO/OpenDdlWriter.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsApplicationFileSystemConfig, nsNoBase, 1, nsRTTIDefaultAllocator<nsApplicationFileSystemConfig>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ARRAY_MEMBER_PROPERTY("DataDirs", m_DataDirs),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsApplicationFileSystemConfig_DataDirConfig, nsNoBase, 1, nsRTTIDefaultAllocator<nsApplicationFileSystemConfig_DataDirConfig>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("RelativePath", m_sDataDirSpecialPath),
    NS_MEMBER_PROPERTY("Writable", m_bWritable),
    NS_MEMBER_PROPERTY("RootName", m_sRootName),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

nsResult nsApplicationFileSystemConfig::Save(nsStringView sPath)
{
  nsFileWriter file;
  if (file.Open(sPath).Failed())
    return NS_FAILURE;

  nsOpenDdlWriter writer;
  writer.SetOutputStream(&file);
  writer.SetCompactMode(false);
  writer.SetPrimitiveTypeStringMode(nsOpenDdlWriter::TypeStringMode::Compliant);

  for (nsUInt32 i = 0; i < m_DataDirs.GetCount(); ++i)
  {
    writer.BeginObject("DataDir");

    nsOpenDdlUtils::StoreString(writer, m_DataDirs[i].m_sDataDirSpecialPath, "Path");
    nsOpenDdlUtils::StoreString(writer, m_DataDirs[i].m_sRootName, "RootName");
    nsOpenDdlUtils::StoreBool(writer, m_DataDirs[i].m_bWritable, "Writable");

    writer.EndObject();
  }

  return NS_SUCCESS;
}

void nsApplicationFileSystemConfig::Load(nsStringView sPath)
{
  NS_LOG_BLOCK("nsApplicationFileSystemConfig::Load()");

  m_DataDirs.Clear();

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
    nsLog::Dev("File-system config file '{0}' does not exist.", sPath);
    return;
  }

  nsOpenDdlReader reader;
  if (reader.ParseDocument(file, 0, nsLog::GetThreadLocalLogSystem()).Failed())
  {
    nsLog::Error("Failed to parse file-system config file '{0}'", sPath);
    return;
  }

  const nsOpenDdlReaderElement* pTree = reader.GetRootElement();

  for (const nsOpenDdlReaderElement* pDirs = pTree->GetFirstChild(); pDirs != nullptr; pDirs = pDirs->GetSibling())
  {
    if (!pDirs->IsCustomType("DataDir"))
      continue;

    DataDirConfig cfg;
    cfg.m_bWritable = false;

    const nsOpenDdlReaderElement* pPath = pDirs->FindChildOfType(nsOpenDdlPrimitiveType::String, "Path");
    const nsOpenDdlReaderElement* pRoot = pDirs->FindChildOfType(nsOpenDdlPrimitiveType::String, "RootName");
    const nsOpenDdlReaderElement* pWrite = pDirs->FindChildOfType(nsOpenDdlPrimitiveType::Bool, "Writable");

    if (pPath)
      cfg.m_sDataDirSpecialPath = pPath->GetPrimitivesString()[0];
    if (pRoot)
      cfg.m_sRootName = pRoot->GetPrimitivesString()[0];
    if (pWrite)
      cfg.m_bWritable = pWrite->GetPrimitivesBool()[0];

    /// \todo Temp fix for backwards compatibility
    {
      if (cfg.m_sRootName == "project")
      {
        cfg.m_sDataDirSpecialPath = ">project/";
      }
      else if (cfg.m_sDataDirSpecialPath.StartsWith_NoCase(":project/"))
      {
        nsStringBuilder temp(">project/");
        temp.AppendPath(cfg.m_sDataDirSpecialPath.GetData() + 9);
        cfg.m_sDataDirSpecialPath = temp;
      }
      else if (cfg.m_sDataDirSpecialPath.StartsWith_NoCase(":sdk/"))
      {
        nsStringBuilder temp(">sdk/");
        temp.AppendPath(cfg.m_sDataDirSpecialPath.GetData() + 5);
        cfg.m_sDataDirSpecialPath = temp;
      }
      else if (!cfg.m_sDataDirSpecialPath.StartsWith_NoCase(">sdk/"))
      {
        nsStringBuilder temp(">sdk/");
        temp.AppendPath(cfg.m_sDataDirSpecialPath);
        cfg.m_sDataDirSpecialPath = temp;
      }
    }

    m_DataDirs.PushBack(cfg);
  }
}

void nsApplicationFileSystemConfig::Apply()
{
  NS_LOG_BLOCK("nsApplicationFileSystemConfig::Apply");

  // nsStringBuilder s;

  // Make sure previous calls to Apply do not accumulate
  Clear();

  for (const auto& var : m_DataDirs)
  {
    // if (nsFileSystem::ResolveSpecialDirectory(var.m_sDataDirSpecialPath, s).Succeeded())
    {
      nsFileSystem::AddDataDirectory(var.m_sDataDirSpecialPath, "AppFileSystemConfig", var.m_sRootName, (!var.m_sRootName.IsEmpty() && var.m_bWritable) ? nsFileSystem::DataDirUsage::AllowWrites : nsFileSystem::DataDirUsage::ReadOnly).IgnoreResult();
    }
  }
}


void nsApplicationFileSystemConfig::Clear()
{
  nsFileSystem::RemoveDataDirectoryGroup("AppFileSystemConfig");
}

nsResult nsApplicationFileSystemConfig::CreateDataDirStubFiles()
{
  NS_LOG_BLOCK("nsApplicationFileSystemConfig::CreateDataDirStubFiles");

  nsStringBuilder s;
  nsResult res = NS_SUCCESS;

  for (const auto& var : m_DataDirs)
  {
    if (nsFileSystem::ResolveSpecialDirectory(var.m_sDataDirSpecialPath, s).Failed())
    {
      nsLog::Error("Failed to get special directory '{0}'", var.m_sDataDirSpecialPath);
      res = NS_FAILURE;
      continue;
    }

    s.AppendPath("DataDir.nsManifest");

    nsOSFile file;
    if (file.Open(s, nsFileOpenMode::Write).Failed())
    {
      nsLog::Error("Failed to create stub file '{0}'", s);
      res = NS_FAILURE;
    }
  }

  return NS_SUCCESS;
}



NS_STATICLINK_FILE(Foundation, Foundation_Application_Config_Implementation_FileSystemConfig);
