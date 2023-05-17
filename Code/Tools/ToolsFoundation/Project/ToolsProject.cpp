#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/IO/OSFile.h>
#include <ToolsFoundation/Document/DocumentManager.h>
#include <ToolsFoundation/Project/ToolsProject.h>

WD_IMPLEMENT_SINGLETON(wdToolsProject);

wdEvent<const wdToolsProjectEvent&> wdToolsProject::s_Events;
wdEvent<wdToolsProjectRequest&> wdToolsProject::s_Requests;


wdToolsProjectRequest::wdToolsProjectRequest()
{
  m_Type = Type::CanCloseProject;
  m_bCanClose = true;
  m_iContainerWindowUniqueIdentifier = 0;
}

wdToolsProject::wdToolsProject(const char* szProjectPath)
  : m_SingletonRegistrar(this)
{
  m_bIsClosing = false;

  m_sProjectPath = szProjectPath;
  WD_ASSERT_DEV(!m_sProjectPath.IsEmpty(), "Path cannot be empty.");
}

wdToolsProject::~wdToolsProject() {}

wdStatus wdToolsProject::Create()
{
  {
    wdOSFile ProjectFile;
    if (ProjectFile.Open(m_sProjectPath, wdFileOpenMode::Write).Failed())
    {
      return wdStatus(wdFmt("Could not open/create the project file for writing: '{0}'", m_sProjectPath));
    }
    else
    {
      const char* szToken = "wdEditor Project File";

      WD_SUCCEED_OR_RETURN(ProjectFile.Write(szToken, wdStringUtils::GetStringElementCount(szToken) + 1));
      ProjectFile.Close();
    }
  }

  wdToolsProjectEvent e;
  e.m_pProject = this;
  e.m_Type = wdToolsProjectEvent::Type::ProjectCreated;
  s_Events.Broadcast(e);

  return Open();
}

wdStatus wdToolsProject::Open()
{
  wdOSFile ProjectFile;
  if (ProjectFile.Open(m_sProjectPath, wdFileOpenMode::Read).Failed())
  {
    return wdStatus(wdFmt("Could not open the project file for reading: '{0}'", m_sProjectPath));
  }

  ProjectFile.Close();

  wdToolsProjectEvent e;
  e.m_pProject = this;
  e.m_Type = wdToolsProjectEvent::Type::ProjectOpened;
  s_Events.Broadcast(e);

  return wdStatus(WD_SUCCESS);
}

void wdToolsProject::CreateSubFolder(const char* szFolder) const
{
  wdStringBuilder sPath;

  sPath = m_sProjectPath;
  sPath.PathParentDirectory();
  sPath.AppendPath(szFolder);

  wdOSFile::CreateDirectoryStructure(sPath).IgnoreResult();
}

void wdToolsProject::CloseProject()
{
  if (GetSingleton())
  {
    GetSingleton()->m_bIsClosing = true;

    wdToolsProjectEvent e;
    e.m_pProject = GetSingleton();
    e.m_Type = wdToolsProjectEvent::Type::ProjectClosing;
    s_Events.Broadcast(e);

    wdDocumentManager::CloseAllDocuments();

    delete GetSingleton();

    e.m_Type = wdToolsProjectEvent::Type::ProjectClosed;
    s_Events.Broadcast(e);
  }
}

void wdToolsProject::SaveProjectState()
{
  if (GetSingleton())
  {
    GetSingleton()->m_bIsClosing = true;

    wdToolsProjectEvent e;
    e.m_pProject = GetSingleton();
    e.m_Type = wdToolsProjectEvent::Type::ProjectSaveState;
    s_Events.Broadcast(e, 1);
  }
}

bool wdToolsProject::CanCloseProject()
{
  if (GetSingleton() == nullptr)
    return true;

  wdToolsProjectRequest e;
  e.m_Type = wdToolsProjectRequest::Type::CanCloseProject;
  e.m_bCanClose = true;
  s_Requests.Broadcast(e, 1); // when the save dialog pops up and the user presses 'Save' we need to allow one more recursion

  return e.m_bCanClose;
}

bool wdToolsProject::CanCloseDocuments(wdArrayPtr<wdDocument*> documents)
{
  if (GetSingleton() == nullptr)
    return true;

  wdToolsProjectRequest e;
  e.m_Type = wdToolsProjectRequest::Type::CanCloseDocuments;
  e.m_bCanClose = true;
  e.m_Documents = documents;
  s_Requests.Broadcast(e);

  return e.m_bCanClose;
}

wdInt32 wdToolsProject::SuggestContainerWindow(wdDocument* pDoc)
{
  if (pDoc == nullptr)
  {
    return 0;
  }
  wdToolsProjectRequest e;
  e.m_Type = wdToolsProjectRequest::Type::SuggestContainerWindow;
  e.m_Documents.PushBack(pDoc);
  s_Requests.Broadcast(e);

  return e.m_iContainerWindowUniqueIdentifier;
}

wdStringBuilder wdToolsProject::GetPathForDocumentGuid(const wdUuid& guid)
{
  wdToolsProjectRequest e;
  e.m_Type = wdToolsProjectRequest::Type::GetPathForDocumentGuid;
  e.m_documentGuid = guid;
  s_Requests.Broadcast(e, 1); // this can be sent while CanCloseProject is processed, so allow one additional recursion depth
  return e.m_sAbsDocumentPath;
}

wdStatus wdToolsProject::CreateOrOpenProject(const char* szProjectPath, bool bCreate)
{
  CloseProject();

  new wdToolsProject(szProjectPath);

  wdStatus ret;

  if (bCreate)
  {
    ret = GetSingleton()->Create();
    wdToolsProject::SaveProjectState();
  }
  else
    ret = GetSingleton()->Open();

  if (ret.m_Result.Failed())
  {
    delete GetSingleton();
    return ret;
  }

  return wdStatus(WD_SUCCESS);
}

wdStatus wdToolsProject::OpenProject(const char* szProjectPath)
{
  wdStatus status = CreateOrOpenProject(szProjectPath, false);

  return status;
}

wdStatus wdToolsProject::CreateProject(const char* szProjectPath)
{
  return CreateOrOpenProject(szProjectPath, true);
}

void wdToolsProject::BroadcastSaveAll()
{
  wdToolsProjectEvent e;
  e.m_pProject = GetSingleton();
  e.m_Type = wdToolsProjectEvent::Type::SaveAll;

  s_Events.Broadcast(e);
}

void wdToolsProject::BroadcastConfigChanged()
{
  wdToolsProjectEvent e;
  e.m_pProject = GetSingleton();
  e.m_Type = wdToolsProjectEvent::Type::ProjectConfigChanged;

  s_Events.Broadcast(e);
}

void wdToolsProject::AddAllowedDocumentRoot(const char* szPath)
{
  wdStringBuilder s = szPath;
  s.MakeCleanPath();
  s.Trim("", "/");

  m_AllowedDocumentRoots.PushBack(s);
}


bool wdToolsProject::IsDocumentInAllowedRoot(const char* szDocumentPath, wdString* out_pRelativePath) const
{
  for (wdUInt32 i = m_AllowedDocumentRoots.GetCount(); i > 0; --i)
  {
    const auto& root = m_AllowedDocumentRoots[i - 1];

    wdStringBuilder s = szDocumentPath;
    if (!s.IsPathBelowFolder(root))
      continue;

    if (out_pRelativePath)
    {
      wdStringBuilder sText = szDocumentPath;
      sText.MakeRelativeTo(root).IgnoreResult();

      *out_pRelativePath = sText;
    }

    return true;
  }

  return false;
}

const wdString wdToolsProject::GetProjectName(bool bSanitize) const
{
  wdStringBuilder sTemp = wdToolsProject::GetSingleton()->GetProjectFile();
  sTemp.PathParentDirectory();
  sTemp.Trim("/");

  if (!bSanitize)
    return sTemp.GetFileName();

  const wdStringBuilder sOrgName = sTemp.GetFileName();
  sTemp.Clear();

  bool bAnyAscii = false;

  for (wdStringIterator it = sOrgName.GetIteratorFront(); it.IsValid(); ++it)
  {
    const wdUInt32 c = it.GetCharacter();

    if (!wdStringUtils::IsIdentifierDelimiter_C_Code(c))
    {
      bAnyAscii = true;

      // valid character to be used in C as an identifier
      sTemp.Append(c);
    }
    else if (c == ' ')
    {
      // skip
    }
    else
    {
      sTemp.AppendFormat("{}", wdArgU(c, 1, false, 16));
    }
  }

  if (!bAnyAscii)
  {
    const wdUInt32 uiHash = wdHashingUtils::xxHash32String(sTemp);
    sTemp.Format("Project{}", uiHash);
  }

  if (sTemp.IsEmpty())
  {
    sTemp = "Project";
  }

  if (sTemp.GetCharacterCount() > 20)
  {
    sTemp.Shrink(0, sTemp.GetCharacterCount() - 20);
  }

  return sTemp;
}

wdString wdToolsProject::GetProjectDirectory() const
{
  wdStringBuilder s = GetProjectFile();

  s.PathParentDirectory();
  s.Trim("", "/\\");

  return s;
}

wdString wdToolsProject::GetProjectDataFolder() const
{
  wdStringBuilder s = GetProjectFile();
  s.Append("_data");

  return s;
}

wdString wdToolsProject::FindProjectDirectoryForDocument(const char* szDocumentPath)
{
  wdStringBuilder sPath = szDocumentPath;
  sPath.PathParentDirectory();

  wdStringBuilder sTemp;

  while (!sPath.IsEmpty())
  {
    sTemp = sPath;
    sTemp.AppendPath("wdProject");

    if (wdOSFile::ExistsFile(sTemp))
      return sPath;

    sPath.PathParentDirectory();
  }

  return "";
}
