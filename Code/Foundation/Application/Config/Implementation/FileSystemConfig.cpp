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
WD_BEGIN_STATIC_REFLECTED_TYPE(wdApplicationFileSystemConfig, wdNoBase, 1, wdRTTIDefaultAllocator<wdApplicationFileSystemConfig>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_ARRAY_MEMBER_PROPERTY("DataDirs", m_DataDirs),
  }
  WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdApplicationFileSystemConfig_DataDirConfig, wdNoBase, 1, wdRTTIDefaultAllocator<wdApplicationFileSystemConfig_DataDirConfig>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("RelativePath", m_sDataDirSpecialPath),
    WD_MEMBER_PROPERTY("Writable", m_bWritable),
    WD_MEMBER_PROPERTY("RootName", m_sRootName),
  }
  WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;
// clang-format on

wdResult wdApplicationFileSystemConfig::Save(wdStringView sPath)
{
  wdFileWriter file;
  if (file.Open(sPath).Failed())
    return WD_FAILURE;

  wdOpenDdlWriter writer;
  writer.SetOutputStream(&file);
  writer.SetCompactMode(false);
  writer.SetPrimitiveTypeStringMode(wdOpenDdlWriter::TypeStringMode::Compliant);

  for (wdUInt32 i = 0; i < m_DataDirs.GetCount(); ++i)
  {
    writer.BeginObject("DataDir");

    wdOpenDdlUtils::StoreString(writer, m_DataDirs[i].m_sDataDirSpecialPath, "Path");
    wdOpenDdlUtils::StoreString(writer, m_DataDirs[i].m_sRootName, "RootName");
    wdOpenDdlUtils::StoreBool(writer, m_DataDirs[i].m_bWritable, "Writable");

    writer.EndObject();
  }

  return WD_SUCCESS;
}

void wdApplicationFileSystemConfig::Load(wdStringView sPath)
{
  WD_LOG_BLOCK("wdApplicationFileSystemConfig::Load()");

  m_DataDirs.Clear();

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
    wdLog::Dev("File-system config file '{0}' does not exist.", sPath);
    return;
  }

  wdOpenDdlReader reader;
  if (reader.ParseDocument(file, 0, wdLog::GetThreadLocalLogSystem()).Failed())
  {
    wdLog::Error("Failed to parse file-system config file '{0}'", sPath);
    return;
  }

  const wdOpenDdlReaderElement* pTree = reader.GetRootElement();

  for (const wdOpenDdlReaderElement* pDirs = pTree->GetFirstChild(); pDirs != nullptr; pDirs = pDirs->GetSibling())
  {
    if (!pDirs->IsCustomType("DataDir"))
      continue;

    DataDirConfig cfg;
    cfg.m_bWritable = false;

    const wdOpenDdlReaderElement* pPath = pDirs->FindChildOfType(wdOpenDdlPrimitiveType::String, "Path");
    const wdOpenDdlReaderElement* pRoot = pDirs->FindChildOfType(wdOpenDdlPrimitiveType::String, "RootName");
    const wdOpenDdlReaderElement* pWrite = pDirs->FindChildOfType(wdOpenDdlPrimitiveType::Bool, "Writable");

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
        wdStringBuilder temp(">project/");
        temp.AppendPath(cfg.m_sDataDirSpecialPath.GetData() + 9);
        cfg.m_sDataDirSpecialPath = temp;
      }
      else if (cfg.m_sDataDirSpecialPath.StartsWith_NoCase(":sdk/"))
      {
        wdStringBuilder temp(">sdk/");
        temp.AppendPath(cfg.m_sDataDirSpecialPath.GetData() + 5);
        cfg.m_sDataDirSpecialPath = temp;
      }
      else if (!cfg.m_sDataDirSpecialPath.StartsWith_NoCase(">sdk/"))
      {
        wdStringBuilder temp(">sdk/");
        temp.AppendPath(cfg.m_sDataDirSpecialPath);
        cfg.m_sDataDirSpecialPath = temp;
      }
    }

    m_DataDirs.PushBack(cfg);
  }
}

void wdApplicationFileSystemConfig::Apply()
{
  WD_LOG_BLOCK("wdApplicationFileSystemConfig::Apply");

  // wdStringBuilder s;

  // Make sure previous calls to Apply do not accumulate
  Clear();

  for (const auto& var : m_DataDirs)
  {
    // if (wdFileSystem::ResolveSpecialDirectory(var.m_sDataDirSpecialPath, s).Succeeded())
    {
      wdFileSystem::AddDataDirectory(var.m_sDataDirSpecialPath, "AppFileSystemConfig", var.m_sRootName, (!var.m_sRootName.IsEmpty() && var.m_bWritable) ? wdFileSystem::DataDirUsage::AllowWrites : wdFileSystem::DataDirUsage::ReadOnly).IgnoreResult();
    }
  }
}


void wdApplicationFileSystemConfig::Clear()
{
  wdFileSystem::RemoveDataDirectoryGroup("AppFileSystemConfig");
}

wdResult wdApplicationFileSystemConfig::CreateDataDirStubFiles()
{
  WD_LOG_BLOCK("wdApplicationFileSystemConfig::CreateDataDirStubFiles");

  wdStringBuilder s;
  wdResult res = WD_SUCCESS;

  for (const auto& var : m_DataDirs)
  {
    if (wdFileSystem::ResolveSpecialDirectory(var.m_sDataDirSpecialPath, s).Failed())
    {
      wdLog::Error("Failed to get special directory '{0}'", var.m_sDataDirSpecialPath);
      res = WD_FAILURE;
      continue;
    }

    s.AppendPath("DataDir.wdManifest");

    wdOSFile file;
    if (file.Open(s, wdFileOpenMode::Write).Failed())
    {
      wdLog::Error("Failed to create stub file '{0}'", s);
      res = WD_FAILURE;
    }
  }

  return WD_SUCCESS;
}



WD_STATICLINK_FILE(Foundation, Foundation_Application_Config_Implementation_FileSystemConfig);
