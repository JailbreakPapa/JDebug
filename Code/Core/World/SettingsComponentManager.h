#pragma once

#include <Core/World/ComponentManager.h>
#include <Foundation/Types/UniquePtr.h>

/// \brief A component manager that does no update at all on components and expects only a single instance to be created per world.
///
/// Easy access to this single component is provided through the GetSingletonComponent() function.
/// If a second component is created, the manager will log an error. The first created component will be used as the 'singleton',
/// all other components are ignored.
/// Use this for components derived from nsSettingsComponent, of which one should only have zero or one per world.
template <typename ComponentType>
class nsSettingsComponentManager : public nsComponentManagerBase
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsSettingsComponentManager);

public:
  nsSettingsComponentManager(nsWorld* pWorld);
  ~nsSettingsComponentManager();

  /// \brief Returns the first component of this type that has been created.
  ComponentType* GetSingletonComponent();
  const ComponentType* GetSingletonComponent() const;

  static nsWorldModuleTypeId TypeId();

  // nsComponentManagerBase implementation
  virtual void CollectAllComponents(nsDynamicArray<nsComponentHandle>& out_allComponents, bool bOnlyActive) override;
  virtual void CollectAllComponents(nsDynamicArray<nsComponent*>& out_allComponents, bool bOnlyActive) override;

private:
  friend class nsComponentManagerFactory;

  virtual nsComponent* CreateComponentStorage() override;
  virtual void DeleteComponentStorage(nsComponent* pComponent, nsComponent*& out_pMovedComponent) override;

  nsHybridArray<nsUniquePtr<ComponentType>, 2> m_Components;
};

#include <Core/World/Implementation/SettingsComponentManager_inl.h>
