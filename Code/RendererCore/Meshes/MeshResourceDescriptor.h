#pragma once

#include <Foundation/IO/Stream.h>
#include <Foundation/Math/BoundingBoxSphere.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>
#include <RendererCore/Meshes/MeshBufferResource.h>

class NS_RENDERERCORE_DLL nsMeshResourceDescriptor
{
public:
  struct SubMesh
  {
    NS_DECLARE_POD_TYPE();

    nsUInt32 m_uiPrimitiveCount;
    nsUInt32 m_uiFirstPrimitive;
    nsUInt32 m_uiMaterialIndex;

    nsBoundingBoxSphere m_Bounds;
  };

  struct Material
  {
    nsString m_sPath;
  };

  nsMeshResourceDescriptor();

  void Clear();

  nsMeshBufferResourceDescriptor& MeshBufferDesc();

  const nsMeshBufferResourceDescriptor& MeshBufferDesc() const;

  void UseExistingMeshBuffer(const nsMeshBufferResourceHandle& hBuffer);

  void AddSubMesh(nsUInt32 uiPrimitiveCount, nsUInt32 uiFirstPrimitive, nsUInt32 uiMaterialIndex);

  void SetMaterial(nsUInt32 uiMaterialIndex, const char* szPathToMaterial);

  void Save(nsStreamWriter& inout_stream);
  nsResult Save(const char* szFile);

  nsResult Load(nsStreamReader& inout_stream);
  nsResult Load(const char* szFile);

  const nsMeshBufferResourceHandle& GetExistingMeshBuffer() const;

  nsArrayPtr<const Material> GetMaterials() const;

  nsArrayPtr<const SubMesh> GetSubMeshes() const;

  /// \brief Merges all submeshes into just one.
  void CollapseSubMeshes();

  void ComputeBounds();
  const nsBoundingBoxSphere& GetBounds() const;
  void SetBounds(const nsBoundingBoxSphere& bounds) { m_Bounds = bounds; }

  struct BoneData
  {
    nsMat4 m_GlobalInverseRestPoseMatrix;
    nsUInt16 m_uiBoneIndex = nsInvalidJointIndex;

    nsResult Serialize(nsStreamWriter& inout_stream) const;
    nsResult Deserialize(nsStreamReader& inout_stream);
  };

  nsSkeletonResourceHandle m_hDefaultSkeleton;
  nsHashTable<nsHashedString, BoneData> m_Bones;
  float m_fMaxBoneVertexOffset = 0.0f; // the maximum distance between any vertex and its influencing bones, can be used for adjusting the bounding box of a pose

private:
  nsHybridArray<Material, 8> m_Materials;
  nsHybridArray<SubMesh, 8> m_SubMeshes;
  nsMeshBufferResourceDescriptor m_MeshBufferDescriptor;
  nsMeshBufferResourceHandle m_hMeshBuffer;
  nsBoundingBoxSphere m_Bounds;
};
