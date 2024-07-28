#pragma once

#include <Core/CoreDLL.h>

#include <Core/Physics/SurfaceResourceDescriptor.h>
#include <Core/ResourceManager/Resource.h>
#include <Core/World/Declarations.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Reflection/Reflection.h>

class nsWorld;
class nsUuid;

struct nsSurfaceResourceEvent
{
  enum class Type
  {
    Created,
    Destroyed
  };

  Type m_Type;
  nsSurfaceResource* m_pSurface;
};

class NS_CORE_DLL nsSurfaceResource : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsSurfaceResource, nsResource);
  NS_RESOURCE_DECLARE_COMMON_CODE(nsSurfaceResource);
  NS_RESOURCE_DECLARE_CREATEABLE(nsSurfaceResource, nsSurfaceResourceDescriptor);

public:
  nsSurfaceResource();
  ~nsSurfaceResource();

  const nsSurfaceResourceDescriptor& GetDescriptor() const { return m_Descriptor; }

  static nsEvent<const nsSurfaceResourceEvent&, nsMutex> s_Events;

  void* m_pPhysicsMaterialPhysX = nullptr;
  void* m_pPhysicsMaterialJolt = nullptr;

  /// \brief Spawns the prefab that was defined for the given interaction at the given position and using the configured orientation.
  /// Returns false, if the interaction type was not defined in this surface or any of its base surfaces
  bool InteractWithSurface(nsWorld* pWorld, nsGameObjectHandle hObject, const nsVec3& vPosition, const nsVec3& vSurfaceNormal, const nsVec3& vIncomingDirection, const nsTempHashedString& sInteraction, const nsUInt16* pOverrideTeamID, float fImpulseSqr = 0.0f) const;

  bool IsBasedOn(const nsSurfaceResource* pThisOrBaseSurface) const;

  bool IsBasedOn(const nsSurfaceResourceHandle hThisOrBaseSurface) const;

private:
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

private:
  static const nsSurfaceInteraction* FindInteraction(const nsSurfaceResource* pCurSurf, nsUInt64 uiHash, float fImpulseSqr, float& out_fImpulseParamValue);

  nsSurfaceResourceDescriptor m_Descriptor;

  struct SurfInt
  {
    nsUInt64 m_uiInteractionTypeHash = 0;
    const nsSurfaceInteraction* m_pInteraction;
  };

  nsDynamicArray<SurfInt> m_Interactions;
};
