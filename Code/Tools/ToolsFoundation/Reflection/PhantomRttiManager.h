#pragma once

#include <Foundation/Communication/Event.h>
#include <Foundation/Containers/IdTable.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>

class wdPhantomRTTI;

struct wdPhantomRttiManagerEvent
{
  enum class Type
  {
    TypeAdded,
    TypeRemoved,
    TypeChanged,
  };

  wdPhantomRttiManagerEvent()
    : m_Type(Type::TypeAdded)
    , m_pChangedType(nullptr)
  {
  }

  Type m_Type;
  const wdRTTI* m_pChangedType;
};

/// \brief Manages all wdPhantomRTTI types that have been added to him.
///
/// A wdPhantomRTTI cannot be created directly but must be created via this managers
/// RegisterType function with a given wdReflectedTypeDescriptor.
class WD_TOOLSFOUNDATION_DLL wdPhantomRttiManager
{
public:
  /// \brief Adds a reflected type to the list of accessible types.
  ///
  /// Types must be added in the correct order, any type must be added before
  /// it can be referenced in other types. Any base class must be added before
  /// any class deriving from it can be added.
  /// Call the function again if a type has changed during the run of the
  /// program. If the type actually differs the last known class layout the
  /// m_TypeChangedEvent event will be called with the old and new wdRTTI.
  ///
  /// \sa wdReflectionUtils::GetReflectedTypeDescriptorFromRtti
  static const wdRTTI* RegisterType(wdReflectedTypeDescriptor& ref_desc);

  /// \brief Removes a type from the list of accessible types.
  ///
  /// No instance of the given type or storage must still exist when this function is called.
  static bool UnregisterType(const wdRTTI* pRtti);

private:
  WD_MAKE_SUBSYSTEM_STARTUP_FRIEND(ToolsFoundation, ReflectedTypeManager);

  static void Startup();
  static void Shutdown();
  static void PluginEventHandler(const wdPluginEvent& e);

public:
  static wdCopyOnBroadcastEvent<const wdPhantomRttiManagerEvent&> s_Events;

private:
  static wdSet<const wdRTTI*> s_RegisteredConcreteTypes;
  static wdHashTable<const char*, wdPhantomRTTI*> s_NameToPhantom;
};
