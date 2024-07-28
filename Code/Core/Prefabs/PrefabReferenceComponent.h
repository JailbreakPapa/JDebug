#pragma once

#include <Core/Prefabs/PrefabResource.h>
#include <Core/World/Component.h>
#include <Core/World/World.h>
#include <Foundation/Containers/ArrayMap.h>
#include <Foundation/Types/RangeView.h>

class nsPrefabReferenceComponent;

class NS_CORE_DLL nsPrefabReferenceComponentManager : public nsComponentManager<nsPrefabReferenceComponent, nsBlockStorageType::Compact>
{
public:
  nsPrefabReferenceComponentManager(nsWorld* pWorld);
  ~nsPrefabReferenceComponentManager();

  virtual void Initialize() override;

  void Update(const nsWorldModule::UpdateContext& context);
  void AddToUpdateList(nsPrefabReferenceComponent* pComponent);

private:
  void ResourceEventHandler(const nsResourceEvent& e);

  nsDeque<nsComponentHandle> m_ComponentsToUpdate;
};

/// \brief The central component to instantiate prefabs.
///
/// This component instantiates a prefab and attaches the instantiated objects as children to this object.
/// The component is able to remove and recreate instantiated objects, which is needed at editing time.
/// Whenever the prefab resource changes, this component re-creates the instance.
///
/// It also holds prefab parameters, which are passed through during instantiation.
/// For that it also implements remapping of game object references, so that they can be passed into prefabs during instantiation.
class NS_CORE_DLL nsPrefabReferenceComponent : public nsComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsPrefabReferenceComponent, nsComponent, nsPrefabReferenceComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

public:
  virtual void SerializeComponent(nsWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(nsWorldReader& inout_stream) override;

protected:
  virtual void OnActivated() override;
  virtual void OnDeactivated() override;

  virtual void Deinitialize() override;
  virtual void OnSimulationStarted() override;


  //////////////////////////////////////////////////////////////////////////
  // nsPrefabReferenceComponent

public:
  nsPrefabReferenceComponent();
  ~nsPrefabReferenceComponent();

  void SetPrefabFile(const char* szFile);                                                // [ property ]
  const char* GetPrefabFile() const;                                                     // [ property ]

  void SetPrefab(const nsPrefabResourceHandle& hPrefab);                                 // [ property ]
  NS_ALWAYS_INLINE const nsPrefabResourceHandle& GetPrefab() const { return m_hPrefab; } // [ property ]

  const nsRangeView<const char*, nsUInt32> GetParameters() const;                        // [ property ] (exposed parameter)
  void SetParameter(const char* szKey, const nsVariant& value);                          // [ property ] (exposed parameter)
  void RemoveParameter(const char* szKey);                                               // [ property ] (exposed parameter)
  bool GetParameter(const char* szKey, nsVariant& out_value) const;                      // [ property ] (exposed parameter)

  static void SerializePrefabParameters(const nsWorld& world, nsWorldWriter& inout_stream, nsArrayMap<nsHashedString, nsVariant> parameters);
  static void DeserializePrefabParameters(nsArrayMap<nsHashedString, nsVariant>& out_parameters, nsWorldReader& inout_stream);

private:
  void InstantiatePrefab();
  void ClearPreviousInstances();

  nsPrefabResourceHandle m_hPrefab;
  nsArrayMap<nsHashedString, nsVariant> m_Parameters;
  bool m_bInUpdateList = false;
};
