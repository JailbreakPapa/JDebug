#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Application/Application.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/OSFile.h>
#include <ToolsFoundation/Application/ApplicationServices.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/Project/ToolsProject.h>

WD_IMPLEMENT_SINGLETON(wdApplicationServices);

static wdApplicationServices g_instance;

wdApplicationServices::wdApplicationServices()
  : m_SingletonRegistrar(this)
{
}

wdString wdApplicationServices::GetApplicationUserDataFolder() const
{
  wdStringBuilder path = wdOSFile::GetUserDataFolder();
  path.AppendPath("wdEngine Project", wdApplication::GetApplicationInstance()->GetApplicationName());
  path.MakeCleanPath();

  return path;
}

wdString wdApplicationServices::GetApplicationDataFolder() const
{
  wdStringBuilder sAppDir(">sdk/Data/Tools/", wdApplication::GetApplicationInstance()->GetApplicationName());

  wdStringBuilder result;
  wdFileSystem::ResolveSpecialDirectory(sAppDir, result).IgnoreResult();
  result.MakeCleanPath();

  return result;
}

wdString wdApplicationServices::GetApplicationPreferencesFolder() const
{
  return GetApplicationUserDataFolder();
}

wdString wdApplicationServices::GetProjectPreferencesFolder() const
{
  wdStringBuilder path = GetApplicationUserDataFolder();

  wdStringBuilder ProjectName = wdToolsProject::GetSingleton()->GetProjectDirectory();

  wdStringBuilder ProjectPath = ProjectName;
  ProjectPath.PathParentDirectory();

  const wdUInt64 uiPathHash = wdHashingUtils::StringHash(ProjectPath.GetData());

  ProjectName = ProjectName.GetFileName();

  path.AppendFormat("/Projects/{}_{}", uiPathHash, ProjectName);

  path.MakeCleanPath();
  return path;
}

wdString wdApplicationServices::GetDocumentPreferencesFolder(const wdDocument* pDocument) const
{
  wdStringBuilder path = GetProjectPreferencesFolder();

  wdStringBuilder sGuid;
  wdConversionUtils::ToString(pDocument->GetGuid(), sGuid);

  path.AppendPath(sGuid);

  path.MakeCleanPath();
  return path;
}

wdString wdApplicationServices::GetPrecompiledToolsFolder(bool bUsePrecompiledTools) const
{
  wdStringBuilder sPath = wdOSFile::GetApplicationDirectory();

  if (bUsePrecompiledTools)
  {
    sPath.AppendPath("../../../Data/Tools/Precompiled");
  }

  sPath.MakeCleanPath();

  return sPath;
}

wdString wdApplicationServices::GetSampleProjectsFolder() const
{
  wdStringBuilder sPath = wdOSFile::GetApplicationDirectory();

  sPath.AppendPath("../../../Data/Samples");

  sPath.MakeCleanPath();

  return sPath;
}
