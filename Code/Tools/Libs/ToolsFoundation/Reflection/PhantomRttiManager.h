#pragma once

#include <Foundation/Communication/Event.h>
#include <Foundation/Containers/IdTable.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>

class nsPhantomRTTI;

struct nsPhantomRttiManagerEvent
{
  enum class Type
  {
    TypeAdded,
    TypeRemoved,
    TypeChanged,
  };

  nsPhantomRttiManagerEvent()

    = default;

  Type m_Type = Type::TypeAdded;
  const nsRTTI* m_pChangedType = nullptr;
};

/// \brief Manages all nsPhantomRTTI types that have been added to him.
///
/// A nsPhantomRTTI cannot be created directly but must be created via this managers
/// RegisterType function with a given nsReflectedTypeDescriptor.
class NS_TOOLSFOUNDATION_DLL nsPhantomRttiManager
{
public:
  /// \brief Adds a reflected type to the list of accessible types.
  ///
  /// Types must be added in the correct order, any type must be added before
  /// it can be referenced in other types. Any base class must be added before
  /// any class deriving from it can be added.
  /// Call the function again if a type has changed during the run of the
  /// program. If the type actually differs the last known class layout the
  /// m_TypeChangedEvent event will be called with the old and new nsRTTI.
  ///
  /// \sa nsReflectionUtils::GetReflectedTypeDescriptorFromRtti
  static const nsRTTI* RegisterType(nsReflectedTypeDescriptor& ref_desc);

  /// \brief Removes a type from the list of accessible types.
  ///
  /// No instance of the given type or storage must still exist when this function is called.
  static bool UnregisterType(const nsRTTI* pRtti);

private:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(ToolsFoundation, ReflectedTypeManager);

  static void Startup();
  static void Shutdown();
  static void PluginEventHandler(const nsPluginEvent& e);

public:
  static nsCopyOnBroadcastEvent<const nsPhantomRttiManagerEvent&> s_Events;

private:
  static nsSet<const nsRTTI*> s_RegisteredConcreteTypes;
  static nsHashTable<nsStringView, nsPhantomRTTI*> s_NameToPhantom;
};
