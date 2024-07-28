#pragma once

#include <Core/ResourceManager/Resource.h>
#include <Core/ResourceManager/ResourceHandle.h>
#include <RendererCore/AnimationSystem/Declarations.h>
#include <RendererCore/AnimationSystem/Skeleton.h>
#include <RendererCore/RendererCoreDLL.h>

using nsSurfaceResourceHandle = nsTypedResourceHandle<class nsSurfaceResource>;

struct nsSkeletonResourceGeometry
{
  // scale is used to resize a unit sphere / box / capsule
  nsTransform m_Transform;
  nsUInt16 m_uiAttachedToJoint = 0;
  nsEnum<nsSkeletonJointGeometryType> m_Type;

  // for convex geometry
  nsDynamicArray<nsVec3> m_VertexPositions;
  nsDynamicArray<nsUInt8> m_TriangleIndices;
};

struct NS_RENDERERCORE_DLL nsSkeletonResourceDescriptor
{
  nsSkeletonResourceDescriptor();
  ~nsSkeletonResourceDescriptor();
  nsSkeletonResourceDescriptor(const nsSkeletonResourceDescriptor& rhs) = delete;
  nsSkeletonResourceDescriptor(nsSkeletonResourceDescriptor&& rhs);
  void operator=(nsSkeletonResourceDescriptor&& rhs);
  void operator=(const nsSkeletonResourceDescriptor& rhs) = delete;

  nsResult Serialize(nsStreamWriter& inout_stream) const;
  nsResult Deserialize(nsStreamReader& inout_stream);

  nsUInt64 GetHeapMemoryUsage() const;

  nsTransform m_RootTransform = nsTransform::MakeIdentity();
  nsSkeleton m_Skeleton;
  float m_fMaxImpulse = nsMath::HighValue<float>();

  nsDynamicArray<nsSkeletonResourceGeometry> m_Geometry;
};

using nsSkeletonResourceHandle = nsTypedResourceHandle<class nsSkeletonResource>;

class NS_RENDERERCORE_DLL nsSkeletonResource : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsSkeletonResource, nsResource);
  NS_RESOURCE_DECLARE_COMMON_CODE(nsSkeletonResource);
  NS_RESOURCE_DECLARE_CREATEABLE(nsSkeletonResource, nsSkeletonResourceDescriptor);

public:
  nsSkeletonResource();
  ~nsSkeletonResource();

  const nsSkeletonResourceDescriptor& GetDescriptor() const { return *m_pDescriptor; }

private:
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

  nsUniquePtr<nsSkeletonResourceDescriptor> m_pDescriptor;
};
