/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <Foundation/Basics.h>
#include <GuiFoundation/Action/ActionMap.h>

/// \brief A central place for creating and retrieving action maps.
class NS_GUIFOUNDATION_DLL nsActionMapManager
{
public:
  /// \brief Adds a new action map with the given name. Returns NS_FAILURE if the name was already used before.
  static nsResult RegisterActionMap(nsStringView sMapping);

  /// \brief Deletes the action map with the given name. Returns NS_FAILURE, if no such map exists.
  static nsResult UnregisterActionMap(nsStringView sMapping);

  /// \brief Returns the action map with the given name, or nullptr, if it doesn't exist.
  static nsActionMap* GetActionMap(nsStringView sMapping);

private:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(GuiFoundation, ActionMapManager);

  static void Startup();
  static void Shutdown();

private:
  static nsMap<nsString, nsActionMap*> s_Mappings;
};
