#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/IO/OSFile.h>
#include <ToolsFoundation/Document/DocumentManager.h>
#include <ToolsFoundation/Project/ToolsProject.h>

NS_IMPLEMENT_SINGLETON(nsToolsProject);

nsEvent<const nsToolsProjectEvent&> nsToolsProject::s_Events;
nsEvent<nsToolsProjectRequest&> nsToolsProject::s_Requests;


nsToolsProjectRequest::nsToolsProjectRequest()
{
  m_Type = Type::CanCloseProject;
  m_bCanClose = true;
  m_iContainerWindowUniqueIdentifier = 0;
}

nsToolsProject::nsToolsProject(nsStringView sProjectPath)
  : m_SingletonRegistrar(this)
{
  m_bIsClosing = false;

  m_sProjectPath = sProjectPath;
  NS_ASSERT_DEV(!m_sProjectPath.IsEmpty(), "Path cannot be empty.");
}

nsToolsProject::~nsToolsProject() = default;

nsStatus nsToolsProject::Create()
{
  {
    nsOSFile ProjectFile;
    if (ProjectFile.Open(m_sProjectPath, nsFileOpenMode::Write).Failed())
    {
      return nsStatus(nsFmt("Could not open/create the project file for writing: '{0}'", m_sProjectPath));
    }
    else
    {
      nsStringView szToken = "nsEditor Project File";

      NS_SUCCEED_OR_RETURN(ProjectFile.Write(szToken.GetStartPointer(), szToken.GetElementCount() + 1));
      ProjectFile.Close();
    }
  }

  nsToolsProjectEvent e;
  e.m_pProject = this;
  e.m_Type = nsToolsProjectEvent::Type::ProjectCreated;
  s_Events.Broadcast(e);

  return Open();
}

nsStatus nsToolsProject::Open()
{
  nsOSFile ProjectFile;
  if (ProjectFile.Open(m_sProjectPath, nsFileOpenMode::Read).Failed())
  {
    return nsStatus(nsFmt("Could not open the project file for reading: '{0}'", m_sProjectPath));
  }

  ProjectFile.Close();

  nsToolsProjectEvent e;
  e.m_pProject = this;
  e.m_Type = nsToolsProjectEvent::Type::ProjectOpened;
  s_Events.Broadcast(e);

  return nsStatus(NS_SUCCESS);
}

void nsToolsProject::CreateSubFolder(nsStringView sFolder) const
{
  nsStringBuilder sPath;

  sPath = m_sProjectPath;
  sPath.PathParentDirectory();
  sPath.AppendPath(sFolder);

  nsOSFile::CreateDirectoryStructure(sPath).IgnoreResult();
}

void nsToolsProject::CloseProject()
{
  if (GetSingleton())
  {
    GetSingleton()->m_bIsClosing = true;

    nsToolsProjectEvent e;
    e.m_pProject = GetSingleton();
    e.m_Type = nsToolsProjectEvent::Type::ProjectClosing;
    s_Events.Broadcast(e);

    nsDocumentManager::CloseAllDocuments();

    delete GetSingleton();

    e.m_Type = nsToolsProjectEvent::Type::ProjectClosed;
    s_Events.Broadcast(e);
  }
}

void nsToolsProject::SaveProjectState()
{
  if (GetSingleton())
  {
    GetSingleton()->m_bIsClosing = true;

    nsToolsProjectEvent e;
    e.m_pProject = GetSingleton();
    e.m_Type = nsToolsProjectEvent::Type::ProjectSaveState;
    s_Events.Broadcast(e, 1);
  }
}

bool nsToolsProject::CanCloseProject()
{
  if (GetSingleton() == nullptr)
    return true;

  nsToolsProjectRequest e;
  e.m_Type = nsToolsProjectRequest::Type::CanCloseProject;
  e.m_bCanClose = true;
  s_Requests.Broadcast(e, 1); // when the save dialog pops up and the user presses 'Save' we need to allow one more recursion

  return e.m_bCanClose;
}

bool nsToolsProject::CanCloseDocuments(nsArrayPtr<nsDocument*> documents)
{
  if (GetSingleton() == nullptr)
    return true;

  nsToolsProjectRequest e;
  e.m_Type = nsToolsProjectRequest::Type::CanCloseDocuments;
  e.m_bCanClose = true;
  e.m_Documents = documents;
  s_Requests.Broadcast(e);

  return e.m_bCanClose;
}

nsInt32 nsToolsProject::SuggestContainerWindow(nsDocument* pDoc)
{
  if (pDoc == nullptr)
  {
    return 0;
  }
  nsToolsProjectRequest e;
  e.m_Type = nsToolsProjectRequest::Type::SuggestContainerWindow;
  e.m_Documents.PushBack(pDoc);
  s_Requests.Broadcast(e);

  return e.m_iContainerWindowUniqueIdentifier;
}

nsStringBuilder nsToolsProject::GetPathForDocumentGuid(const nsUuid& guid)
{
  nsToolsProjectRequest e;
  e.m_Type = nsToolsProjectRequest::Type::GetPathForDocumentGuid;
  e.m_documentGuid = guid;
  s_Requests.Broadcast(e, 1); // this can be sent while CanCloseProject is processed, so allow one additional recursion depth
  return e.m_sAbsDocumentPath;
}

nsStatus nsToolsProject::CreateOrOpenProject(nsStringView sProjectPath, bool bCreate)
{
  CloseProject();

  new nsToolsProject(sProjectPath);

  nsStatus ret;

  if (bCreate)
  {
    ret = GetSingleton()->Create();
    nsToolsProject::SaveProjectState();
  }
  else
    ret = GetSingleton()->Open();

  if (ret.m_Result.Failed())
  {
    delete GetSingleton();
    return ret;
  }

  return nsStatus(NS_SUCCESS);
}

nsStatus nsToolsProject::OpenProject(nsStringView sProjectPath)
{
  nsStatus status = CreateOrOpenProject(sProjectPath, false);

  return status;
}

nsStatus nsToolsProject::CreateProject(nsStringView sProjectPath)
{
  return CreateOrOpenProject(sProjectPath, true);
}

void nsToolsProject::BroadcastSaveAll()
{
  nsToolsProjectEvent e;
  e.m_pProject = GetSingleton();
  e.m_Type = nsToolsProjectEvent::Type::SaveAll;

  s_Events.Broadcast(e);
}

void nsToolsProject::BroadcastConfigChanged()
{
  nsToolsProjectEvent e;
  e.m_pProject = GetSingleton();
  e.m_Type = nsToolsProjectEvent::Type::ProjectConfigChanged;

  s_Events.Broadcast(e);
}

void nsToolsProject::AddAllowedDocumentRoot(nsStringView sPath)
{
  nsStringBuilder s = sPath;
  s.MakeCleanPath();
  s.Trim("", "/");

  m_AllowedDocumentRoots.PushBack(s);
}


bool nsToolsProject::IsDocumentInAllowedRoot(nsStringView sDocumentPath, nsString* out_pRelativePath) const
{
  for (nsUInt32 i = m_AllowedDocumentRoots.GetCount(); i > 0; --i)
  {
    const auto& root = m_AllowedDocumentRoots[i - 1];

    nsStringBuilder s = sDocumentPath;
    if (!s.IsPathBelowFolder(root))
      continue;

    if (out_pRelativePath)
    {
      nsStringBuilder sText = sDocumentPath;
      sText.MakeRelativeTo(root).IgnoreResult();

      *out_pRelativePath = sText;
    }

    return true;
  }

  return false;
}

const nsString nsToolsProject::GetProjectName(bool bSanitize) const
{
  nsStringBuilder sTemp = nsToolsProject::GetSingleton()->GetProjectFile();
  sTemp.PathParentDirectory();
  sTemp.Trim("/");

  if (!bSanitize)
    return sTemp.GetFileName();

  const nsStringBuilder sOrgName = sTemp.GetFileName();
  sTemp.Clear();

  bool bAnyAscii = false;

  for (nsStringIterator it = sOrgName.GetIteratorFront(); it.IsValid(); ++it)
  {
    const nsUInt32 c = it.GetCharacter();

    if (!nsStringUtils::IsIdentifierDelimiter_C_Code(c))
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
      sTemp.AppendFormat("{}", nsArgU(c, 1, false, 16));
    }
  }

  if (!bAnyAscii)
  {
    const nsUInt32 uiHash = nsHashingUtils::xxHash32String(sTemp);
    sTemp.SetFormat("Project{}", uiHash);
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

nsString nsToolsProject::GetProjectDirectory() const
{
  nsStringBuilder s = GetProjectFile();

  s.PathParentDirectory();
  s.Trim("", "/\\");

  return s;
}

nsString nsToolsProject::GetProjectDataFolder() const
{
  nsStringBuilder s = GetProjectFile();
  s.Append("_data");

  return s;
}

nsString nsToolsProject::FindProjectDirectoryForDocument(nsStringView sDocumentPath)
{
  nsStringBuilder sPath = sDocumentPath;
  sPath.PathParentDirectory();

  nsStringBuilder sTemp;

  while (!sPath.IsEmpty())
  {
    sTemp = sPath;
    sTemp.AppendPath("nsProject");

    if (nsOSFile::ExistsFile(sTemp))
      return sPath;

    sPath.PathParentDirectory();
  }

  return "";
}
