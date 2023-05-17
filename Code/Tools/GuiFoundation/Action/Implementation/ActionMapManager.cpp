#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/Action/DocumentActions.h>

wdMap<wdString, wdActionMap*> wdActionMapManager::s_Mappings;

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(GuiFoundation, ActionMapManager)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "ActionManager"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    wdActionMapManager::Startup();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    wdActionMapManager::Shutdown();
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

////////////////////////////////////////////////////////////////////////
// wdActionMapManager public functions
////////////////////////////////////////////////////////////////////////

wdResult wdActionMapManager::RegisterActionMap(const char* szMapping)
{
  auto it = s_Mappings.Find(szMapping);
  if (it.IsValid())
    return WD_FAILURE;

  s_Mappings.Insert(szMapping, WD_DEFAULT_NEW(wdActionMap));
  return WD_SUCCESS;
}

wdResult wdActionMapManager::UnregisterActionMap(const char* szMapping)
{
  auto it = s_Mappings.Find(szMapping);
  if (!it.IsValid())
    return WD_FAILURE;

  WD_DEFAULT_DELETE(it.Value());
  s_Mappings.Remove(it);
  return WD_SUCCESS;
}

wdActionMap* wdActionMapManager::GetActionMap(const char* szMapping)
{
  auto it = s_Mappings.Find(szMapping);
  if (!it.IsValid())
    return nullptr;

  return it.Value();
}


////////////////////////////////////////////////////////////////////////
// wdActionMapManager private functions
////////////////////////////////////////////////////////////////////////

void wdActionMapManager::Startup()
{
  wdActionMapManager::RegisterActionMap("DocumentWindowTabMenu").IgnoreResult();
  wdDocumentActions::MapActions("DocumentWindowTabMenu", "", false);
}

void wdActionMapManager::Shutdown()
{
  wdActionMapManager::UnregisterActionMap("DocumentWindowTabMenu").IgnoreResult();

  while (!s_Mappings.IsEmpty())
  {
    wdResult res = UnregisterActionMap(s_Mappings.GetIterator().Key());
    WD_ASSERT_DEV(res == WD_SUCCESS, "Failed to call UnregisterActionMap successfully!");
    res.IgnoreResult();
  }
}
