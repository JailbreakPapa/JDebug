#pragma once

#include <Foundation/Basics.h>
#include <GuiFoundation/Action/ActionMap.h>


class WD_GUIFOUNDATION_DLL wdActionMapManager
{
public:
  static wdResult RegisterActionMap(const char* szMapping);
  static wdResult UnregisterActionMap(const char* szMapping);
  static wdActionMap* GetActionMap(const char* szMapping);

private:
  WD_MAKE_SUBSYSTEM_STARTUP_FRIEND(GuiFoundation, ActionMapManager);

  static void Startup();
  static void Shutdown();

private:
  static wdMap<wdString, wdActionMap*> s_Mappings;
};
