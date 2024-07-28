#pragma once

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Containers/IdTable.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Memory/BlockStorage.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/Delegate.h>

#include <Core/World/Component.h>
#include <Core/World/Declarations.h>
#include <Core/World/WorldModule.h>

/// \brief Base class for all component managers. Do not derive directly from this class, but derive from nsComponentManager instead.
///
/// Every component type has its corresponding manager type. The manager stores the components in memory blocks to minimize overhead
/// on creation and deletion of components. Each manager can also register update functions to update its components during
/// the different update phases of nsWorld.
/// Use nsWorld::CreateComponentManager to create an instance of a component manager within a specific world.
class NS_CORE_DLL nsComponentManagerBase : public nsWorldModule
{
  NS_ADD_DYNAMIC_REFLECTION(nsComponentManagerBase, nsWorldModule);

protected:
  nsComponentManagerBase(nsWorld* pWorld);
  virtual ~nsComponentManagerBase();

public:
  /// \brief Checks whether the given handle references a valid component.
  bool IsValidComponent(const nsComponentHandle& hComponent) const;

  /// \brief Returns if a component with the given handle exists and if so writes out the corresponding pointer to out_pComponent.
  bool TryGetComponent(const nsComponentHandle& hComponent, nsComponent*& out_pComponent);

  /// \brief Returns if a component with the given handle exists and if so writes out the corresponding pointer to out_pComponent.
  bool TryGetComponent(const nsComponentHandle& hComponent, const nsComponent*& out_pComponent) const;

  /// \brief Returns the number of components managed by this manager.
  nsUInt32 GetComponentCount() const;

  /// \brief Create a new component instance and returns a handle to it.
  nsComponentHandle CreateComponent(nsGameObject* pOwnerObject);

  /// \brief Create a new component instance and returns a handle to it.
  template <typename ComponentType>
  nsTypedComponentHandle<ComponentType> CreateComponent(nsGameObject* pOwnerObject, ComponentType*& out_pComponent);

  /// \brief Deletes the given component. Note that the component will be invalidated first and the actual deletion is postponed.
  void DeleteComponent(const nsComponentHandle& hComponent);

  /// \brief Deletes the given component. Note that the component will be invalidated first and the actual deletion is postponed.
  void DeleteComponent(nsComponent* pComponent);

  /// \brief Adds all components that this manager handles to the given array (array is not cleared).
  /// Prefer to use more efficient methods on derived classes, only use this if you need to go through a nsComponentManagerBase pointer.
  virtual void CollectAllComponents(nsDynamicArray<nsComponentHandle>& out_allComponents, bool bOnlyActive) = 0;

  /// \brief Adds all components that this manager handles to the given array (array is not cleared).
  /// Prefer to use more efficient methods on derived classes, only use this if you need to go through a nsComponentManagerBase pointer.
  virtual void CollectAllComponents(nsDynamicArray<nsComponent*>& out_allComponents, bool bOnlyActive) = 0;

protected:
  /// \cond
  // internal methods
  friend class nsWorld;
  friend class nsInternal::WorldData;

  virtual void Deinitialize() override;

protected:
  friend class nsWorldReader;

  nsComponentHandle CreateComponentNoInit(nsGameObject* pOwnerObject, nsComponent*& out_pComponent);
  void InitializeComponent(nsComponent* pComponent);
  void DeinitializeComponent(nsComponent* pComponent);
  void PatchIdTable(nsComponent* pComponent);

  virtual nsComponent* CreateComponentStorage() = 0;
  virtual void DeleteComponentStorage(nsComponent* pComponent, nsComponent*& out_pMovedComponent) = 0;

  /// \endcond

  nsIdTable<nsComponentId, nsComponent*> m_Components;
};

template <typename T, nsBlockStorageType::Enum StorageType>
class nsComponentManager : public nsComponentManagerBase
{
public:
  using ComponentType = T;
  using SUPER = nsComponentManagerBase;

  /// \brief Although the constructor is public always use nsWorld::CreateComponentManager to create an instance.
  nsComponentManager(nsWorld* pWorld);
  virtual ~nsComponentManager();

  /// \brief Returns if a component with the given handle exists and if so writes out the corresponding pointer to out_pComponent.
  bool TryGetComponent(const nsComponentHandle& hComponent, ComponentType*& out_pComponent);

  /// \brief Returns if a component with the given handle exists and if so writes out the corresponding pointer to out_pComponent.
  bool TryGetComponent(const nsComponentHandle& hComponent, const ComponentType*& out_pComponent) const;

  /// \brief Returns an iterator over all components.
  typename nsBlockStorage<ComponentType, nsInternal::DEFAULT_BLOCK_SIZE, StorageType>::Iterator GetComponents(nsUInt32 uiStartIndex = 0);

  /// \brief Returns an iterator over all components.
  typename nsBlockStorage<ComponentType, nsInternal::DEFAULT_BLOCK_SIZE, StorageType>::ConstIterator GetComponents(nsUInt32 uiStartIndex = 0) const;

  /// \brief Returns the type id corresponding to the component type managed by this manager.
  static nsWorldModuleTypeId TypeId();

  virtual void CollectAllComponents(nsDynamicArray<nsComponentHandle>& out_allComponents, bool bOnlyActive) override;
  virtual void CollectAllComponents(nsDynamicArray<nsComponent*>& out_allComponents, bool bOnlyActive) override;

protected:
  friend ComponentType;
  friend class nsComponentManagerFactory;

  virtual nsComponent* CreateComponentStorage() override;
  virtual void DeleteComponentStorage(nsComponent* pComponent, nsComponent*& out_pMovedComponent) override;

  void RegisterUpdateFunction(UpdateFunctionDesc& desc);

  nsBlockStorage<ComponentType, nsInternal::DEFAULT_BLOCK_SIZE, StorageType> m_ComponentStorage;
};


//////////////////////////////////////////////////////////////////////////

struct nsComponentUpdateType
{
  enum Enum
  {
    Always,
    WhenSimulating
  };
};

/// \brief Simple component manager implementation that calls an update method on all components every frame.
template <typename ComponentType, nsComponentUpdateType::Enum UpdateType, nsBlockStorageType::Enum StorageType = nsBlockStorageType::FreeList>
class nsComponentManagerSimple final : public nsComponentManager<ComponentType, StorageType>
{
public:
  nsComponentManagerSimple(nsWorld* pWorld);

  virtual void Initialize() override;

  /// \brief A simple update function that iterates over all components and calls Update() on every component
  void SimpleUpdate(const nsWorldModule::UpdateContext& context);

private:
  static void SimpleUpdateName(nsStringBuilder& out_sName);
};

//////////////////////////////////////////////////////////////////////////

#define NS_ADD_COMPONENT_FUNCTIONALITY(componentType, baseType, managerType)                                            \
public:                                                                                                                 \
  using ComponentManagerType = managerType;                                                                             \
  virtual nsWorldModuleTypeId GetTypeId() const override                                                                \
  {                                                                                                                     \
    return s_TypeId;                                                                                                    \
  }                                                                                                                     \
  static NS_ALWAYS_INLINE nsWorldModuleTypeId TypeId()                                                                  \
  {                                                                                                                     \
    return s_TypeId;                                                                                                    \
  }                                                                                                                     \
  nsTypedComponentHandle<componentType> GetHandle() const                                                               \
  {                                                                                                                     \
    return nsTypedComponentHandle<componentType>(nsComponent::GetHandle());                                             \
  }                                                                                                                     \
  virtual nsComponentMode::Enum GetMode() const override;                                                               \
  static nsTypedComponentHandle<componentType> CreateComponent(nsGameObject* pOwnerObject, componentType*& pComponent); \
  static void DeleteComponent(componentType* pComponent);                                                               \
  void DeleteComponent();                                                                                               \
                                                                                                                        \
private:                                                                                                                \
  friend managerType;                                                                                                   \
  static nsWorldModuleTypeId s_TypeId

#define NS_ADD_ABSTRACT_COMPONENT_FUNCTIONALITY(componentType, baseType) \
public:                                                                  \
  virtual nsWorldModuleTypeId GetTypeId() const override                 \
  {                                                                      \
    return nsWorldModuleTypeId(-1);                                      \
  }                                                                      \
  static NS_ALWAYS_INLINE nsWorldModuleTypeId TypeId()                   \
  {                                                                      \
    return nsWorldModuleTypeId(-1);                                      \
  }

/// \brief Add this macro to a custom component type inside the type declaration.
#define NS_DECLARE_COMPONENT_TYPE(componentType, baseType, managerType) \
  NS_ADD_DYNAMIC_REFLECTION(componentType, baseType);                   \
  NS_ADD_COMPONENT_FUNCTIONALITY(componentType, baseType, managerType);

/// \brief Add this macro to a custom abstract component type inside the type declaration.
#define NS_DECLARE_ABSTRACT_COMPONENT_TYPE(componentType, baseType) \
  NS_ADD_DYNAMIC_REFLECTION(componentType, baseType);               \
  NS_ADD_ABSTRACT_COMPONENT_FUNCTIONALITY(componentType, baseType);


/// \brief Implements rtti and component specific functionality. Add this macro to a cpp file.
///
/// \see NS_BEGIN_DYNAMIC_REFLECTED_TYPE
#define NS_BEGIN_COMPONENT_TYPE(componentType, version, mode)                                                                            \
  nsWorldModuleTypeId componentType::s_TypeId =                                                                                          \
    nsWorldModuleFactory::GetInstance()->RegisterWorldModule<typename componentType::ComponentManagerType, componentType>();             \
  nsComponentMode::Enum componentType::GetMode() const                                                                                   \
  {                                                                                                                                      \
    return mode;                                                                                                                         \
  }                                                                                                                                      \
  nsTypedComponentHandle<componentType> componentType::CreateComponent(nsGameObject* pOwnerObject, componentType*& out_pComponent)       \
  {                                                                                                                                      \
    return pOwnerObject->GetWorld()->GetOrCreateComponentManager<ComponentManagerType>()->CreateComponent(pOwnerObject, out_pComponent); \
  }                                                                                                                                      \
  void componentType::DeleteComponent(componentType* pComponent)                                                                         \
  {                                                                                                                                      \
    pComponent->GetOwningManager()->DeleteComponent(pComponent->GetHandle());                                                            \
  }                                                                                                                                      \
  void componentType::DeleteComponent()                                                                                                  \
  {                                                                                                                                      \
    GetOwningManager()->DeleteComponent(GetHandle());                                                                                    \
  }                                                                                                                                      \
  NS_BEGIN_DYNAMIC_REFLECTED_TYPE(componentType, version, nsRTTINoAllocator)

/// \brief Implements rtti and abstract component specific functionality. Add this macro to a cpp file.
///
/// \see NS_BEGIN_DYNAMIC_REFLECTED_TYPE
#define NS_BEGIN_ABSTRACT_COMPONENT_TYPE(componentType, version) NS_BEGIN_ABSTRACT_DYNAMIC_REFLECTED_TYPE(componentType, version)

/// \brief Ends the component implementation code block that was opened with NS_BEGIN_COMPONENT_TYPE.
#define NS_END_COMPONENT_TYPE NS_END_DYNAMIC_REFLECTED_TYPE
#define NS_END_ABSTRACT_COMPONENT_TYPE NS_END_ABSTRACT_DYNAMIC_REFLECTED_TYPE

#include <Core/World/Implementation/ComponentManager_inl.h>
