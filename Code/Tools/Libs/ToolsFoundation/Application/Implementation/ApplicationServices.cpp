#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Application/Application.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/OSFile.h>
#include <ToolsFoundation/Application/ApplicationServices.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/Project/ToolsProject.h>

NS_IMPLEMENT_SINGLETON(nsApplicationServices);

static nsApplicationServices g_instance;

nsApplicationServices::nsApplicationServices()
  : m_SingletonRegistrar(this)
{
}

nsString nsApplicationServices::GetApplicationUserDataFolder() const
{
  nsStringBuilder path = nsOSFile::GetUserDataFolder();
  path.AppendPath("nsEngine Project", nsApplication::GetApplicationInstance()->GetApplicationName());
  path.MakeCleanPath();

  return path;
}

nsString nsApplicationServices::GetApplicationDataFolder() const
{
  nsStringBuilder sAppDir(">sdk/Data/Tools/", nsApplication::GetApplicationInstance()->GetApplicationName());

  nsStringBuilder result;
  nsFileSystem::ResolveSpecialDirectory(sAppDir, result).IgnoreResult();
  result.MakeCleanPath();

  return result;
}

nsString nsApplicationServices::GetApplicationPreferencesFolder() const
{
  return GetApplicationUserDataFolder();
}

nsString nsApplicationServices::GetProjectPreferencesFolder() const
{
  return GetProjectPreferencesFolder(nsToolsProject::GetSingleton()->GetProjectDirectory());
}

nsString nsApplicationServices::GetProjectPreferencesFolder(nsStringView sProjectFilePath) const
{
  nsStringBuilder path = GetApplicationUserDataFolder();

  sProjectFilePath.TrimWordEnd("nsProject");
  sProjectFilePath.TrimWordEnd("nsRemoteProject");
  sProjectFilePath.Trim("/\\");

  nsStringBuilder ProjectName = sProjectFilePath;

  nsStringBuilder ProjectPath = ProjectName;
  ProjectPath.PathParentDirectory();

  const nsUInt64 uiPathHash = nsHashingUtils::StringHash(ProjectPath.GetData());

  ProjectName = ProjectName.GetFileName();

  path.AppendFormat("/Projects/{}_{}", uiPathHash, ProjectName);

  path.MakeCleanPath();
  return path;
}

nsString nsApplicationServices::GetDocumentPreferencesFolder(const nsDocument* pDocument) const
{
  nsStringBuilder path = GetProjectPreferencesFolder();

  nsStringBuilder sGuid;
  nsConversionUtils::ToString(pDocument->GetGuid(), sGuid);

  path.AppendPath(sGuid);

  path.MakeCleanPath();
  return path;
}

nsString nsApplicationServices::GetPrecompiledToolsFolder(bool bUsePrecompiledTools) const
{
  nsStringBuilder sPath = nsOSFile::GetApplicationDirectory();

  if (bUsePrecompiledTools)
  {
    sPath.AppendPath("../../../Data/Tools/Precompiled");
  }

  sPath.MakeCleanPath();

  return sPath;
}

nsString nsApplicationServices::GetSampleProjectsFolder() const
{
  nsStringBuilder sPath = nsOSFile::GetApplicationDirectory();

  sPath.AppendPath("../../../Data/Samples");

  sPath.MakeCleanPath();

  return sPath;
}
