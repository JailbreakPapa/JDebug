#pragma once

#include <Foundation/Memory/BlockStorage.h>
#include <Foundation/Memory/LargeBlockAllocator.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/Bitflags.h>
#include <Foundation/Types/Id.h>

#include <Core/CoreDLL.h>

#ifndef NS_WORLD_INDEX_BITS
#  define NS_WORLD_INDEX_BITS 8
#endif

#define NS_MAX_WORLDS (1 << NS_WORLD_INDEX_BITS)

class nsWorld;
class nsSpatialSystem;
class nsCoordinateSystemProvider;

namespace nsInternal
{
  class WorldData;

  enum
  {
    DEFAULT_BLOCK_SIZE = 1024 * 16
  };

  using WorldLargeBlockAllocator = nsLargeBlockAllocator<DEFAULT_BLOCK_SIZE>;
} // namespace nsInternal

class nsGameObject;
struct nsGameObjectDesc;

class nsComponentManagerBase;
class nsComponent;

struct nsMsgDeleteGameObject;

/// \brief Internal game object id used by nsGameObjectHandle.
struct nsGameObjectId
{
  using StorageType = nsUInt64;

  NS_DECLARE_ID_TYPE(nsGameObjectId, 32, 8);

  static_assert(NS_WORLD_INDEX_BITS > 0 && NS_WORLD_INDEX_BITS <= 24);

  NS_FORCE_INLINE nsGameObjectId(StorageType instanceIndex, nsUInt8 uiGeneration, nsUInt8 uiWorldIndex = 0)
  {
    m_Data = 0;
    m_InstanceIndex = static_cast<nsUInt32>(instanceIndex);
    m_Generation = uiGeneration;
    m_WorldIndex = uiWorldIndex;
  }

  union
  {
    StorageType m_Data;
    struct
    {
      StorageType m_InstanceIndex : 32;
      StorageType m_Generation : 8;
      StorageType m_WorldIndex : NS_WORLD_INDEX_BITS;
    };
  };
};

/// \brief A handle to a game object.
///
/// Never store a direct pointer to a game object. Always store a handle instead. A pointer to a game object can
/// be received by calling nsWorld::TryGetObject with the handle.
/// Note that the object might have been deleted so always check the return value of TryGetObject.
struct nsGameObjectHandle
{
  NS_DECLARE_HANDLE_TYPE(nsGameObjectHandle, nsGameObjectId);

  friend class nsWorld;
  friend class nsGameObject;
};

/// \brief HashHelper implementation so game object handles can be used as key in a hash table.
template <>
struct nsHashHelper<nsGameObjectHandle>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(nsGameObjectHandle value) { return nsHashHelper<nsUInt64>::Hash(value.GetInternalID().m_Data); }

  NS_ALWAYS_INLINE static bool Equal(nsGameObjectHandle a, nsGameObjectHandle b) { return a == b; }
};

/// \brief Currently not implemented as it is not needed for game object handles.
NS_CORE_DLL void operator<<(nsStreamWriter& inout_stream, const nsGameObjectHandle& hValue);
NS_CORE_DLL void operator>>(nsStreamReader& inout_stream, nsGameObjectHandle& ref_hValue);

NS_DECLARE_REFLECTABLE_TYPE(NS_CORE_DLL, nsGameObjectHandle);
NS_DECLARE_CUSTOM_VARIANT_TYPE(nsGameObjectHandle);
#define NS_COMPONENT_TYPE_INDEX_BITS (24 - NS_WORLD_INDEX_BITS)
#define NS_MAX_COMPONENT_TYPES (1 << NS_COMPONENT_TYPE_INDEX_BITS)

/// \brief Internal component id used by nsComponentHandle.
struct nsComponentId
{
  using StorageType = nsUInt64;

  NS_DECLARE_ID_TYPE(nsComponentId, 32, 8);

  static_assert(NS_COMPONENT_TYPE_INDEX_BITS > 0 && NS_COMPONENT_TYPE_INDEX_BITS <= 16);

  NS_ALWAYS_INLINE nsComponentId(StorageType instanceIndex, nsUInt8 uiGeneration, nsUInt16 uiTypeId = 0, nsUInt8 uiWorldIndex = 0)
  {
    m_Data = 0;
    m_InstanceIndex = static_cast<nsUInt32>(instanceIndex);
    m_Generation = uiGeneration;
    m_TypeId = uiTypeId;
    m_WorldIndex = uiWorldIndex;
  }

  union
  {
    StorageType m_Data;
    struct
    {
      StorageType m_InstanceIndex : 32;
      StorageType m_Generation : 8;
      StorageType m_WorldIndex : NS_WORLD_INDEX_BITS;
      StorageType m_TypeId : NS_COMPONENT_TYPE_INDEX_BITS;
    };
  };
};

/// \brief A handle to a component.
///
/// Never store a direct pointer to a component. Always store a handle instead. A pointer to a component can
/// be received by calling nsWorld::TryGetComponent or TryGetComponent on the corresponding component manager.
/// Note that the component might have been deleted so always check the return value of TryGetComponent.
struct nsComponentHandle
{
  NS_DECLARE_HANDLE_TYPE(nsComponentHandle, nsComponentId);

  friend class nsWorld;
  friend class nsComponentManagerBase;
  friend class nsComponent;
};

/// \brief A typed handle to a component.
///
/// This should be preferred if the component type to be stored inside the handle is known, as it provides
/// compile time checks against wrong usages (e.g. assigning unrelated types) and more clearly conveys intent.
///
/// See struct \see nsComponentHandle for more information about general component handle usage.
template <typename TYPE>
struct nsTypedComponentHandle : public nsComponentHandle
{
  nsTypedComponentHandle() = default;
  explicit nsTypedComponentHandle(const nsComponentHandle& hUntyped)
  {
    m_InternalId = hUntyped.GetInternalID();
  }

  template <typename T, std::enable_if_t<std::is_convertible_v<T*, TYPE*>, bool> = true>
  explicit nsTypedComponentHandle(const nsTypedComponentHandle<T>& other)
    : nsTypedComponentHandle(static_cast<const nsComponentHandle&>(other))
  {
  }

  template <typename T, std::enable_if_t<std::is_convertible_v<T*, TYPE*>, bool> = true>
  NS_ALWAYS_INLINE void operator=(const nsTypedComponentHandle<T>& other)
  {
    nsComponentHandle::operator=(other);
  }
};

/// \brief HashHelper implementation so component handles can be used as key in a hashtable.
template <>
struct nsHashHelper<nsComponentHandle>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(nsComponentHandle value)
  {
    nsComponentId id = value.GetInternalID();
    nsUInt64 data = *reinterpret_cast<nsUInt64*>(&id);
    return nsHashHelper<nsUInt64>::Hash(data);
  }

  NS_ALWAYS_INLINE static bool Equal(nsComponentHandle a, nsComponentHandle b) { return a == b; }
};

/// \brief Currently not implemented as it is not needed for component handles.
NS_CORE_DLL void operator<<(nsStreamWriter& inout_stream, const nsComponentHandle& hValue);
NS_CORE_DLL void operator>>(nsStreamReader& inout_stream, nsComponentHandle& ref_hValue);

NS_DECLARE_REFLECTABLE_TYPE(NS_CORE_DLL, nsComponentHandle);
NS_DECLARE_CUSTOM_VARIANT_TYPE(nsComponentHandle);

/// \brief Internal flags of game objects or components.
struct nsObjectFlags
{
  using StorageType = nsUInt32;

  enum Enum
  {
    None = 0,
    Dynamic = NS_BIT(0),                              ///< Usually detected automatically. A dynamic object will not cache render data across frames.
    ForceDynamic = NS_BIT(1),                         ///< Set by the user to enforce the 'Dynamic' mode. Necessary when user code (or scripts) should change
                                                      ///< objects, and the automatic detection cannot know that.
    ActiveFlag = NS_BIT(2),                           ///< The object/component has the 'active flag' set
    ActiveState = NS_BIT(3),                          ///< The object/component and all its parents have the active flag
    Initialized = NS_BIT(4),                          ///< The object/component has been initialized
    Initializing = NS_BIT(5),                         ///< The object/component is currently initializing. Used to prevent recursions during initialization.
    SimulationStarted = NS_BIT(6),                    ///< OnSimulationStarted() has been called on the component
    SimulationStarting = NS_BIT(7),                   ///< Used to prevent recursion during OnSimulationStarted()
    UnhandledMessageHandler = NS_BIT(8),              ///< For components, when a message is not handled, a virtual function is called

    ChildChangesNotifications = NS_BIT(9),            ///< The object should send a notification message when children are added or removed.
    ComponentChangesNotifications = NS_BIT(10),       ///< The object should send a notification message when components are added or removed.
    StaticTransformChangesNotifications = NS_BIT(11), ///< The object should send a notification message if it is static and its transform changes.
    ParentChangesNotifications = NS_BIT(12),          ///< The object should send a notification message when the parent is changes.

    CreatedByPrefab = NS_BIT(13),                     ///< Such flagged objects and components are ignored during scene export (see nsWorldWriter) and will be removed when a prefab needs to be re-instantiated.

    UserFlag0 = NS_BIT(24),
    UserFlag1 = NS_BIT(25),
    UserFlag2 = NS_BIT(26),
    UserFlag3 = NS_BIT(27),
    UserFlag4 = NS_BIT(28),
    UserFlag5 = NS_BIT(29),
    UserFlag6 = NS_BIT(30),
    UserFlag7 = NS_BIT(31),

    Default = None
  };

  struct Bits
  {
    StorageType Dynamic : 1;                             //< 0
    StorageType ForceDynamic : 1;                        //< 1
    StorageType ActiveFlag : 1;                          //< 2
    StorageType ActiveState : 1;                         //< 3
    StorageType Initialized : 1;                         //< 4
    StorageType Initializing : 1;                        //< 5
    StorageType SimulationStarted : 1;                   //< 6
    StorageType SimulationStarting : 1;                  //< 7
    StorageType UnhandledMessageHandler : 1;             //< 8
    StorageType ChildChangesNotifications : 1;           //< 9
    StorageType ComponentChangesNotifications : 1;       //< 10
    StorageType StaticTransformChangesNotifications : 1; //< 11
    StorageType ParentChangesNotifications : 1;          //< 12

    StorageType CreatedByPrefab : 1;                     //< 13

    StorageType Padding : 10;                            // 14 - 23

    StorageType UserFlag0 : 1;                           //< 24
    StorageType UserFlag1 : 1;                           //< 25
    StorageType UserFlag2 : 1;                           //< 26
    StorageType UserFlag3 : 1;                           //< 27
    StorageType UserFlag4 : 1;                           //< 28
    StorageType UserFlag5 : 1;                           //< 29
    StorageType UserFlag6 : 1;                           //< 30
    StorageType UserFlag7 : 1;                           //< 31
  };
};

NS_DECLARE_FLAGS_OPERATORS(nsObjectFlags);

/// \brief Specifies the mode of an object. This enum is only used in the editor.
///
/// \sa nsObjectFlags
struct nsObjectMode
{
  using StorageType = nsUInt8;

  enum Enum : nsUInt8
  {
    Automatic,
    ForceDynamic,

    Default = Automatic
  };
};

NS_DECLARE_REFLECTABLE_TYPE(NS_CORE_DLL, nsObjectMode);

/// \brief Specifies the mode of a component. Dynamic components may change an object's transform, static components must not.
///
/// \sa nsObjectFlags
struct nsComponentMode
{
  enum Enum
  {
    Static,
    Dynamic
  };
};

/// \brief Specifies at which phase the queued message should be processed.
struct nsObjectMsgQueueType
{
  using StorageType = nsUInt8;

  enum Enum : StorageType
  {
    PostAsync,        ///< Process the message in the PostAsync phase.
    PostTransform,    ///< Process the message in the PostTransform phase.
    NextFrame,        ///< Process the message in the PreAsync phase of the next frame.
    AfterInitialized, ///< Process the message after new components have been initialized.
    COUNT
  };
};

/// \brief Certain components may delete themselves or their owner when they are finished with their main purpose
struct NS_CORE_DLL nsOnComponentFinishedAction
{
  using StorageType = nsUInt8;

  enum Enum : StorageType
  {
    None,             ///< Nothing happens after the action is finished.
    DeleteComponent,  ///< The component deletes only itself, but its game object stays.
    DeleteGameObject, ///< When finished the component deletes its owner game object. If there are multiple objects with this mode, the component instead deletes itself, and only the last such component deletes the game object.

    Default = None
  };

  /// \brief Call this when a component is 'finished' with its work.
  ///
  /// Pass in the desired action (usually configured by the user) and the 'this' pointer of the component.
  /// The helper function will delete this component and maybe also attempt to delete the entire object.
  /// For that it will coordinate with other components, and delay the object deletion, if necessary,
  /// until the last component has finished it's work.
  static void HandleFinishedAction(nsComponent* pComponent, nsOnComponentFinishedAction::Enum action);

  /// \brief Call this function in a message handler for nsMsgDeleteGameObject messages.
  ///
  /// This is needed to coordinate object deletion across multiple components that use the
  /// nsOnComponentFinishedAction mechanism.
  /// Depending on the state of this component, the function will either execute the object deletion,
  /// or delay it, until its own work is done.
  static void HandleDeleteObjectMsg(nsMsgDeleteGameObject& ref_msg, nsEnum<nsOnComponentFinishedAction>& ref_action);
};

NS_DECLARE_REFLECTABLE_TYPE(NS_CORE_DLL, nsOnComponentFinishedAction);

/// \brief Same as nsOnComponentFinishedAction, but additionally includes 'Restart'
struct NS_CORE_DLL nsOnComponentFinishedAction2
{
  using StorageType = nsUInt8;

  enum Enum
  {
    None,             ///< Nothing happens after the action is finished.
    DeleteComponent,  ///< The component deletes only itself, but its game object stays.
    DeleteGameObject, ///< When finished the component deletes its owner game object. If there are multiple objects with this mode, the component instead deletes itself, and only the last such component deletes the game object.
    Restart,          ///< When finished, restart from the beginning.

    Default = None
  };

  /// \brief See nsOnComponentFinishedAction::HandleFinishedAction()
  static void HandleFinishedAction(nsComponent* pComponent, nsOnComponentFinishedAction2::Enum action);

  /// \brief See nsOnComponentFinishedAction::HandleDeleteObjectMsg()
  static void HandleDeleteObjectMsg(nsMsgDeleteGameObject& ref_msg, nsEnum<nsOnComponentFinishedAction2>& ref_action);
};

NS_DECLARE_REFLECTABLE_TYPE(NS_CORE_DLL, nsOnComponentFinishedAction2);

/// \brief Used as return value of visitor functions to define whether calling function should stop or continue visiting.
struct nsVisitorExecution
{
  enum Enum
  {
    Continue, ///< Continue regular iteration
    Skip,     ///< In a depth-first iteration mode this will skip the entire sub-tree below the current object
    Stop      ///< Stop the entire iteration
  };
};

using nsSpatialDataId = nsGenericId<24, 8>;
class nsSpatialDataHandle
{
  NS_DECLARE_HANDLE_TYPE(nsSpatialDataHandle, nsSpatialDataId);
};

#define NS_MAX_WORLD_MODULE_TYPES NS_MAX_COMPONENT_TYPES
using nsWorldModuleTypeId = nsUInt16;
static_assert(nsMath::MaxValue<nsWorldModuleTypeId>() >= NS_MAX_WORLD_MODULE_TYPES - 1);

using nsComponentInitBatchId = nsGenericId<24, 8>;
class nsComponentInitBatchHandle
{
  NS_DECLARE_HANDLE_TYPE(nsComponentInitBatchHandle, nsComponentInitBatchId);
};
