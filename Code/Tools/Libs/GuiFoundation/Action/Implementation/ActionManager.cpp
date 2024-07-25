#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/DeferredFileWriter.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/IO/OpenDdlWriter.h>
#include <Foundation/Logging/Log.h>
#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/CommandHistoryActions.h>
#include <GuiFoundation/Action/DocumentActions.h>
#include <GuiFoundation/Action/EditActions.h>
#include <GuiFoundation/Action/StandardMenus.h>
#include <ToolsFoundation/Application/ApplicationServices.h>

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(GuiFoundation, ActionManager)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "ToolsFoundation"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    nsActionManager::Startup();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsActionManager::Shutdown();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsEvent<const nsActionManager::Event&> nsActionManager::s_Events;
nsIdTable<nsActionId, nsActionDescriptor*> nsActionManager::s_ActionTable;
nsMap<nsString, nsActionManager::CategoryData> nsActionManager::s_CategoryPathToActions;
nsMap<nsString, nsString> nsActionManager::s_ShortcutOverride;

////////////////////////////////////////////////////////////////////////
// nsActionManager public functions
////////////////////////////////////////////////////////////////////////

nsActionDescriptorHandle nsActionManager::RegisterAction(const nsActionDescriptor& desc)
{
  nsActionDescriptorHandle hType = GetActionHandle(desc.m_sCategoryPath, desc.m_sActionName);
  NS_ASSERT_DEV(hType.IsInvalidated(), "The action '{0}' in category '{1}' was already registered!", desc.m_sActionName, desc.m_sCategoryPath);

  nsActionDescriptor* pDesc = CreateActionDesc(desc);

  // apply shortcut override
  {
    auto ovride = s_ShortcutOverride.Find(desc.m_sActionName);
    if (ovride.IsValid())
      pDesc->m_sShortcut = ovride.Value();
  }

  hType = nsActionDescriptorHandle(s_ActionTable.Insert(pDesc));
  pDesc->m_Handle = hType;

  auto it = s_CategoryPathToActions.FindOrAdd(pDesc->m_sCategoryPath);
  it.Value().m_Actions.Insert(hType);
  it.Value().m_ActionNameToHandle[pDesc->m_sActionName.GetData()] = hType;

  {
    Event msg;
    msg.m_Type = Event::Type::ActionAdded;
    msg.m_pDesc = pDesc;
    msg.m_Handle = hType;
    s_Events.Broadcast(msg);
  }
  return hType;
}

bool nsActionManager::UnregisterAction(nsActionDescriptorHandle& ref_hAction)
{
  nsActionDescriptor* pDesc = nullptr;
  if (!s_ActionTable.TryGetValue(ref_hAction, pDesc))
  {
    ref_hAction.Invalidate();
    return false;
  }

  auto it = s_CategoryPathToActions.Find(pDesc->m_sCategoryPath);
  NS_ASSERT_DEV(it.IsValid(), "Action is present but not mapped in its category path!");
  NS_VERIFY(it.Value().m_Actions.Remove(ref_hAction), "Action is present but not in its category data!");
  NS_VERIFY(it.Value().m_ActionNameToHandle.Remove(pDesc->m_sActionName), "Action is present but its name is not in the map!");
  if (it.Value().m_Actions.IsEmpty())
  {
    s_CategoryPathToActions.Remove(it);
  }

  s_ActionTable.Remove(ref_hAction);
  DeleteActionDesc(pDesc);
  ref_hAction.Invalidate();
  return true;
}

const nsActionDescriptor* nsActionManager::GetActionDescriptor(nsActionDescriptorHandle hAction)
{
  nsActionDescriptor* pDesc = nullptr;
  if (s_ActionTable.TryGetValue(hAction, pDesc))
    return pDesc;

  return nullptr;
}

const nsIdTable<nsActionId, nsActionDescriptor*>::ConstIterator nsActionManager::GetActionIterator()
{
  return s_ActionTable.GetIterator();
}

nsActionDescriptorHandle nsActionManager::GetActionHandle(const char* szCategoryPath, const char* szActionName)
{
  nsActionDescriptorHandle hAction;
  auto it = s_CategoryPathToActions.Find(szCategoryPath);
  if (!it.IsValid())
    return hAction;

  it.Value().m_ActionNameToHandle.TryGetValue(szActionName, hAction);

  return hAction;
}

nsString nsActionManager::FindActionCategory(const char* szActionName)
{
  for (auto itCat : s_CategoryPathToActions)
  {
    if (itCat.Value().m_ActionNameToHandle.Contains(szActionName))
      return itCat.Key();
  }

  return nsString();
}

nsResult nsActionManager::ExecuteAction(const char* szCategory, const char* szActionName, const nsActionContext& context, const nsVariant& value /*= nsVariant()*/)
{
  nsString sCategory = szCategory;

  if (szCategory == nullptr)
  {
    sCategory = FindActionCategory(szActionName);
  }

  auto hAction = nsActionManager::GetActionHandle(sCategory, szActionName);

  if (hAction.IsInvalidated())
    return NS_FAILURE;

  const nsActionDescriptor* pDesc = nsActionManager::GetActionDescriptor(hAction);

  if (pDesc == nullptr)
    return NS_FAILURE;

  nsAction* pAction = pDesc->CreateAction(context);

  if (pAction == nullptr)
    return NS_FAILURE;

  pAction->Execute(value);
  pDesc->DeleteAction(pAction);

  return NS_SUCCESS;
}

void nsActionManager::SaveShortcutAssignment()
{
  nsStringBuilder sFile = nsApplicationServices::GetSingleton()->GetApplicationPreferencesFolder();
  sFile.AppendPath("Settings/Shortcuts.ddl");

  NS_LOG_BLOCK("LoadShortcutAssignment", sFile.GetData());

  nsDeferredFileWriter file;
  file.SetOutput(sFile);

  nsOpenDdlWriter writer;
  writer.SetOutputStream(&file);
  writer.SetCompactMode(false);
  writer.SetPrimitiveTypeStringMode(nsOpenDdlWriter::TypeStringMode::Compliant);

  nsStringBuilder sKey;

  for (auto it = GetActionIterator(); it.IsValid(); ++it)
  {
    auto pAction = it.Value();

    if (pAction->m_Type != nsActionType::Action)
      continue;

    if (pAction->m_sShortcut == pAction->m_sDefaultShortcut)
      sKey.Set("default: ", pAction->m_sShortcut);
    else
      sKey = pAction->m_sShortcut;

    writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::String, pAction->m_sActionName);
    writer.WriteString(sKey);
    writer.EndPrimitiveList();
  }

  if (file.Close().Failed())
  {
    nsLog::Error("Failed to write shortcuts config file '{0}'", sFile);
  }
}

void nsActionManager::LoadShortcutAssignment()
{
  nsStringBuilder sFile = nsApplicationServices::GetSingleton()->GetApplicationPreferencesFolder();
  sFile.AppendPath("Settings/Shortcuts.ddl");

  NS_LOG_BLOCK("LoadShortcutAssignment", sFile.GetData());

  nsFileReader file;
  if (file.Open(sFile).Failed())
  {
    nsLog::Dev("No shortcuts file '{0}' was found", sFile);
    return;
  }

  nsOpenDdlReader reader;
  if (reader.ParseDocument(file, 0, nsLog::GetThreadLocalLogSystem()).Failed())
    return;

  const auto obj = reader.GetRootElement();

  nsStringBuilder sKey, sValue;

  for (auto pElement = obj->GetFirstChild(); pElement != nullptr; pElement = pElement->GetSibling())
  {
    if (!pElement->HasName() || !pElement->HasPrimitives(nsOpenDdlPrimitiveType::String))
      continue;

    sKey = pElement->GetName();
    sValue = pElement->GetPrimitivesString()[0];

    if (sValue.FindSubString_NoCase("default") != nullptr)
      continue;

    s_ShortcutOverride[sKey] = sValue;
  }

  // apply overrides
  for (auto it = GetActionIterator(); it.IsValid(); ++it)
  {
    auto pAction = it.Value();

    if (pAction->m_Type != nsActionType::Action)
      continue;

    auto ovride = s_ShortcutOverride.Find(pAction->m_sActionName);
    if (ovride.IsValid())
      pAction->m_sShortcut = ovride.Value();
  }
}

////////////////////////////////////////////////////////////////////////
// nsActionManager private functions
////////////////////////////////////////////////////////////////////////

void nsActionManager::Startup()
{
  nsDocumentActions::RegisterActions();
  nsStandardMenus::RegisterActions();
  nsCommandHistoryActions::RegisterActions();
  nsEditActions::RegisterActions();
}

void nsActionManager::Shutdown()
{
  nsDocumentActions::UnregisterActions();
  nsStandardMenus::UnregisterActions();
  nsCommandHistoryActions::UnregisterActions();
  nsEditActions::UnregisterActions();

  NS_ASSERT_DEV(s_ActionTable.IsEmpty(), "Some actions were registered but not unregistred.");
  NS_ASSERT_DEV(s_CategoryPathToActions.IsEmpty(), "Some actions were registered but not unregistred.");

  s_ActionTable.Clear();
  s_CategoryPathToActions.Clear();
  s_ShortcutOverride.Clear();
}

nsActionDescriptor* nsActionManager::CreateActionDesc(const nsActionDescriptor& desc)
{
  nsActionDescriptor* pDesc = NS_DEFAULT_NEW(nsActionDescriptor);
  *pDesc = desc;
  return pDesc;
}

void nsActionManager::DeleteActionDesc(nsActionDescriptor* pDesc)
{
  NS_DEFAULT_DELETE(pDesc);
}
