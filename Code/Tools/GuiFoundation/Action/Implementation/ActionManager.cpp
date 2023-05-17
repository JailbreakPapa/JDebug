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
WD_BEGIN_SUBSYSTEM_DECLARATION(GuiFoundation, ActionManager)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "ToolsFoundation"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    wdActionManager::Startup();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    wdActionManager::Shutdown();
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

wdEvent<const wdActionManager::Event&> wdActionManager::s_Events;
wdIdTable<wdActionId, wdActionDescriptor*> wdActionManager::s_ActionTable;
wdMap<wdString, wdActionManager::CategoryData> wdActionManager::s_CategoryPathToActions;
wdMap<wdString, wdString> wdActionManager::s_ShortcutOverride;

////////////////////////////////////////////////////////////////////////
// wdActionManager public functions
////////////////////////////////////////////////////////////////////////

wdActionDescriptorHandle wdActionManager::RegisterAction(const wdActionDescriptor& desc)
{
  wdActionDescriptorHandle hType = GetActionHandle(desc.m_sCategoryPath, desc.m_sActionName);
  WD_ASSERT_DEV(hType.IsInvalidated(), "The action '{0}' in category '{1}' was already registered!", desc.m_sActionName, desc.m_sCategoryPath);

  wdActionDescriptor* pDesc = CreateActionDesc(desc);

  // apply shortcut override
  {
    auto ovride = s_ShortcutOverride.Find(desc.m_sActionName);
    if (ovride.IsValid())
      pDesc->m_sShortcut = ovride.Value();
  }

  hType = wdActionDescriptorHandle(s_ActionTable.Insert(pDesc));
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

bool wdActionManager::UnregisterAction(wdActionDescriptorHandle& ref_hAction)
{
  wdActionDescriptor* pDesc = nullptr;
  if (!s_ActionTable.TryGetValue(ref_hAction, pDesc))
  {
    ref_hAction.Invalidate();
    return false;
  }

  auto it = s_CategoryPathToActions.Find(pDesc->m_sCategoryPath);
  WD_ASSERT_DEV(it.IsValid(), "Action is present but not mapped in its category path!");
  WD_VERIFY(it.Value().m_Actions.Remove(ref_hAction), "Action is present but not in its category data!");
  WD_VERIFY(it.Value().m_ActionNameToHandle.Remove(pDesc->m_sActionName), "Action is present but its name is not in the map!");
  if (it.Value().m_Actions.IsEmpty())
  {
    s_CategoryPathToActions.Remove(it);
  }

  s_ActionTable.Remove(ref_hAction);
  DeleteActionDesc(pDesc);
  ref_hAction.Invalidate();
  return true;
}

const wdActionDescriptor* wdActionManager::GetActionDescriptor(wdActionDescriptorHandle hAction)
{
  wdActionDescriptor* pDesc = nullptr;
  if (s_ActionTable.TryGetValue(hAction, pDesc))
    return pDesc;

  return nullptr;
}

const wdIdTable<wdActionId, wdActionDescriptor*>::ConstIterator wdActionManager::GetActionIterator()
{
  return s_ActionTable.GetIterator();
}

wdActionDescriptorHandle wdActionManager::GetActionHandle(const char* szCategoryPath, const char* szActionName)
{
  wdActionDescriptorHandle hAction;
  auto it = s_CategoryPathToActions.Find(szCategoryPath);
  if (!it.IsValid())
    return hAction;

  it.Value().m_ActionNameToHandle.TryGetValue(szActionName, hAction);

  return hAction;
}

wdString wdActionManager::FindActionCategory(const char* szActionName)
{
  for (auto itCat : s_CategoryPathToActions)
  {
    if (itCat.Value().m_ActionNameToHandle.Contains(szActionName))
      return itCat.Key();
  }

  return wdString();
}

wdResult wdActionManager::ExecuteAction(
  const char* szCategory, const char* szActionName, const wdActionContext& context, const wdVariant& value /*= wdVariant()*/)
{
  wdString sCategory = szCategory;

  if (szCategory == nullptr)
  {
    sCategory = FindActionCategory(szActionName);
  }

  auto hAction = wdActionManager::GetActionHandle(sCategory, szActionName);

  if (hAction.IsInvalidated())
    return WD_FAILURE;

  const wdActionDescriptor* pDesc = wdActionManager::GetActionDescriptor(hAction);

  if (pDesc == nullptr)
    return WD_FAILURE;

  wdAction* pAction = pDesc->CreateAction(context);

  if (pAction == nullptr)
    return WD_FAILURE;

  pAction->Execute(value);
  pDesc->DeleteAction(pAction);

  return WD_SUCCESS;
}

void wdActionManager::SaveShortcutAssignment()
{
  wdStringBuilder sFile = wdApplicationServices::GetSingleton()->GetApplicationPreferencesFolder();
  sFile.AppendPath("Settings/Shortcuts.ddl");

  WD_LOG_BLOCK("LoadShortcutAssignment", sFile.GetData());

  wdDeferredFileWriter file;
  file.SetOutput(sFile);

  wdOpenDdlWriter writer;
  writer.SetOutputStream(&file);
  writer.SetCompactMode(false);
  writer.SetPrimitiveTypeStringMode(wdOpenDdlWriter::TypeStringMode::Compliant);

  wdStringBuilder sKey;

  for (auto it = GetActionIterator(); it.IsValid(); ++it)
  {
    auto pAction = it.Value();

    if (pAction->m_Type != wdActionType::Action)
      continue;

    if (pAction->m_sShortcut == pAction->m_sDefaultShortcut)
      sKey.Set("default: ", pAction->m_sShortcut);
    else
      sKey = pAction->m_sShortcut;

    writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::String, pAction->m_sActionName);
    writer.WriteString(sKey);
    writer.EndPrimitiveList();
  }

  if (file.Close().Failed())
  {
    wdLog::Error("Failed to write shortcuts config file '{0}'", sFile);
  }
}

void wdActionManager::LoadShortcutAssignment()
{
  wdStringBuilder sFile = wdApplicationServices::GetSingleton()->GetApplicationPreferencesFolder();
  sFile.AppendPath("Settings/Shortcuts.ddl");

  WD_LOG_BLOCK("LoadShortcutAssignment", sFile.GetData());

  wdFileReader file;
  if (file.Open(sFile).Failed())
  {
    wdLog::Dev("No shortcuts file '{0}' was found", sFile);
    return;
  }

  wdOpenDdlReader reader;
  if (reader.ParseDocument(file, 0, wdLog::GetThreadLocalLogSystem()).Failed())
    return;

  const auto obj = reader.GetRootElement();

  wdStringBuilder sKey, sValue;

  for (auto pElement = obj->GetFirstChild(); pElement != nullptr; pElement = pElement->GetSibling())
  {
    if (!pElement->HasName() || !pElement->HasPrimitives(wdOpenDdlPrimitiveType::String))
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

    if (pAction->m_Type != wdActionType::Action)
      continue;

    auto ovride = s_ShortcutOverride.Find(pAction->m_sActionName);
    if (ovride.IsValid())
      pAction->m_sShortcut = ovride.Value();
  }
}

////////////////////////////////////////////////////////////////////////
// wdActionManager private functions
////////////////////////////////////////////////////////////////////////

void wdActionManager::Startup()
{
  wdDocumentActions::RegisterActions();
  wdStandardMenus::RegisterActions();
  wdCommandHistoryActions::RegisterActions();
  wdEditActions::RegisterActions();
}

void wdActionManager::Shutdown()
{
  wdDocumentActions::UnregisterActions();
  wdStandardMenus::UnregisterActions();
  wdCommandHistoryActions::UnregisterActions();
  wdEditActions::UnregisterActions();

  WD_ASSERT_DEV(s_ActionTable.IsEmpty(), "Some actions were registered but not unregistred.");
  WD_ASSERT_DEV(s_CategoryPathToActions.IsEmpty(), "Some actions were registered but not unregistred.");

  s_ActionTable.Clear();
  s_CategoryPathToActions.Clear();
  s_ShortcutOverride.Clear();
}

wdActionDescriptor* wdActionManager::CreateActionDesc(const wdActionDescriptor& desc)
{
  wdActionDescriptor* pDesc = WD_DEFAULT_NEW(wdActionDescriptor);
  *pDesc = desc;
  return pDesc;
}

void wdActionManager::DeleteActionDesc(wdActionDescriptor* pDesc)
{
  WD_DEFAULT_DELETE(pDesc);
}
