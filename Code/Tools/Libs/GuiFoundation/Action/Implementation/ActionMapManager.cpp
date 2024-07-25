#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/Action/DocumentActions.h>

nsMap<nsString, nsActionMap*> nsActionMapManager::s_Mappings;

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(GuiFoundation, ActionMapManager)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "ActionManager"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    nsActionMapManager::Startup();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsActionMapManager::Shutdown();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

////////////////////////////////////////////////////////////////////////
// nsActionMapManager public functions
////////////////////////////////////////////////////////////////////////

nsResult nsActionMapManager::RegisterActionMap(nsStringView sMapping)
{
  auto it = s_Mappings.Find(sMapping);
  if (it.IsValid())
    return NS_FAILURE;

  s_Mappings.Insert(sMapping, NS_DEFAULT_NEW(nsActionMap));
  return NS_SUCCESS;
}

nsResult nsActionMapManager::UnregisterActionMap(nsStringView sMapping)
{
  auto it = s_Mappings.Find(sMapping);
  if (!it.IsValid())
    return NS_FAILURE;

  NS_DEFAULT_DELETE(it.Value());
  s_Mappings.Remove(it);
  return NS_SUCCESS;
}

nsActionMap* nsActionMapManager::GetActionMap(nsStringView sMapping)
{
  auto it = s_Mappings.Find(sMapping);
  if (!it.IsValid())
    return nullptr;

  return it.Value();
}


////////////////////////////////////////////////////////////////////////
// nsActionMapManager private functions
////////////////////////////////////////////////////////////////////////

void nsActionMapManager::Startup()
{
  nsActionMapManager::RegisterActionMap("DocumentWindowTabMenu").IgnoreResult();
  nsDocumentActions::MapMenuActions("DocumentWindowTabMenu", "");
}

void nsActionMapManager::Shutdown()
{
  nsActionMapManager::UnregisterActionMap("DocumentWindowTabMenu").IgnoreResult();

  while (!s_Mappings.IsEmpty())
  {
    nsResult res = UnregisterActionMap(s_Mappings.GetIterator().Key());
    NS_ASSERT_DEV(res == NS_SUCCESS, "Failed to call UnregisterActionMap successfully!");
    res.IgnoreResult();
  }
}
