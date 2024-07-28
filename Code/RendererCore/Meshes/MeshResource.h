#pragma once

#include <RendererCore/Meshes/MeshBufferResource.h>
#include <RendererCore/Meshes/MeshResourceDescriptor.h>

using nsMaterialResourceHandle = nsTypedResourceHandle<class nsMaterialResource>;

class NS_RENDERERCORE_DLL nsMeshResource : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsMeshResource, nsResource);
  NS_RESOURCE_DECLARE_COMMON_CODE(nsMeshResource);
  NS_RESOURCE_DECLARE_CREATEABLE(nsMeshResource, nsMeshResourceDescriptor);

public:
  nsMeshResource();

  /// \brief Returns the array of sub-meshes in this mesh.
  nsArrayPtr<const nsMeshResourceDescriptor::SubMesh> GetSubMeshes() const { return m_SubMeshes; }

  /// \brief Returns the mesh buffer that is used by this resource.
  const nsMeshBufferResourceHandle& GetMeshBuffer() const { return m_hMeshBuffer; }

  /// \brief Returns the default materials for this mesh.
  nsArrayPtr<const nsMaterialResourceHandle> GetMaterials() const { return m_Materials; }

  /// \brief Returns the bounds of this mesh.
  const nsBoundingBoxSphere& GetBounds() const { return m_Bounds; }

  // TODO: clean up
  nsSkeletonResourceHandle m_hDefaultSkeleton;
  nsHashTable<nsHashedString, nsMeshResourceDescriptor::BoneData> m_Bones;
  float m_fMaxBoneVertexOffset = 0.0f; // the maximum distance between any vertex and its influencing bones, can be used for adjusting the bounding box of a pose

private:
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

  nsDynamicArray<nsMeshResourceDescriptor::SubMesh> m_SubMeshes;
  nsMeshBufferResourceHandle m_hMeshBuffer;
  nsDynamicArray<nsMaterialResourceHandle> m_Materials;

  nsBoundingBoxSphere m_Bounds;

  static nsUInt32 s_uiMeshBufferNameSuffix;
};

using nsMeshResourceHandle = nsTypedResourceHandle<class nsMeshResource>;
