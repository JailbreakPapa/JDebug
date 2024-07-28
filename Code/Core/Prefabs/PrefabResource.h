#pragma once

#include <Core/ResourceManager/Resource.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Foundation/Containers/ArrayMap.h>
#include <Foundation/Reflection/PropertyPath.h>

using nsPrefabResourceHandle = nsTypedResourceHandle<class nsPrefabResource>;

struct NS_CORE_DLL nsPrefabResourceDescriptor
{
};

struct NS_CORE_DLL nsExposedPrefabParameterDesc
{
  nsHashedString m_sExposeName;
  nsUInt32 m_uiWorldReaderChildObject : 1; // 0 -> use root object array, 1 -> use child object array
  nsUInt32 m_uiWorldReaderObjectIndex : 31;
  nsHashedString m_sComponentType;         // nsRTTI type name to identify which component is meant, empty string -> affects game object
  nsHashedString m_sProperty;              // which property to override
  nsPropertyPath m_CachedPropertyPath;     // cached nsPropertyPath to apply a value to the specified property

  void Save(nsStreamWriter& inout_stream) const;
  void Load(nsStreamReader& inout_stream);
};

class NS_CORE_DLL nsPrefabResource : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsPrefabResource, nsResource);
  NS_RESOURCE_DECLARE_COMMON_CODE(nsPrefabResource);
  NS_RESOURCE_DECLARE_CREATEABLE(nsPrefabResource, nsPrefabResourceDescriptor);

public:
  nsPrefabResource();

  enum class InstantiateResult : nsUInt8
  {
    Success,
    NotYetLoaded,
    Error,
  };

  /// \brief Helper function to instantiate a prefab without having to deal with resource acquisition.
  static nsPrefabResource::InstantiateResult InstantiatePrefab(const nsPrefabResourceHandle& hPrefab, bool bBlockTillLoaded, nsWorld& ref_world, const nsTransform& rootTransform, nsPrefabInstantiationOptions options = {}, const nsArrayMap<nsHashedString, nsVariant>* pExposedParamValues = nullptr);

  /// \brief Creates an instance of this prefab in the given world.
  void InstantiatePrefab(nsWorld& ref_world, const nsTransform& rootTransform, nsPrefabInstantiationOptions options, const nsArrayMap<nsHashedString, nsVariant>* pExposedParamValues = nullptr);

  void ApplyExposedParameterValues(const nsArrayMap<nsHashedString, nsVariant>* pExposedParamValues, const nsDynamicArray<nsGameObject*>& createdChildObjects, const nsDynamicArray<nsGameObject*>& createdRootObjects) const;

private:
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

private:
  nsUInt32 FindFirstParamWithName(nsUInt64 uiNameHash) const;

  nsWorldReader m_WorldReader;
  nsDynamicArray<nsExposedPrefabParameterDesc> m_PrefabParamDescs;
};
