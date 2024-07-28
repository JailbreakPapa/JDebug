#pragma once

#include <Core/World/Component.h>

/// \brief Base class for settings components, of which only one per type should exist in each world.
///
/// Settings components are used to store global scene specific settings, e.g. for physics it would be the scene gravity,
/// for rendering it might be the time of day, fog settings, etc.
///
/// Components of this type should be managed by an nsSettingsComponentManager, which makes it easy to query for the one instance
/// in the world.
class NS_CORE_DLL nsSettingsComponent : public nsComponent
{
  NS_ADD_DYNAMIC_REFLECTION(nsSettingsComponent, nsComponent);

  //////////////////////////////////////////////////////////////////////////
  // nsSettingsComponent

public:
  /// \brief The constructor marks the component as modified.
  nsSettingsComponent();
  ~nsSettingsComponent();

  /// \brief Marks the component as modified. Individual bits can be used to mark only specific settings (groups) as modified.
  void SetModified(nsUInt32 uiBits = 0xFFFFFFFF) { m_uiSettingsModified |= uiBits; }

  /// \brief Checks whether the component (or some settings group) was marked as modified.
  bool IsModified(nsUInt32 uiBits = 0xFFFFFFFF) const { return (m_uiSettingsModified & uiBits) != 0; }

  /// \brief Marks the settings as not-modified.
  void ResetModified(nsUInt32 uiBits = 0xFFFFFFFF) { m_uiSettingsModified &= ~uiBits; }

private:
  nsUInt32 m_uiSettingsModified = 0xFFFFFFFF;
};
