#pragma once

#include <Core/ResourceManager/Resource.h>
#include <RendererCore/Meshes/MeshBufferResource.h>
#include <RendererCore/RendererCoreDLL.h>
#include <RendererFoundation/RendererFoundationDLL.h>

using nsDynamicMeshBufferResourceHandle = nsTypedResourceHandle<class nsDynamicMeshBufferResource>;

struct nsDynamicMeshBufferResourceDescriptor
{
  nsGALPrimitiveTopology::Enum m_Topology = nsGALPrimitiveTopology::Triangles;
  nsGALIndexType::Enum m_IndexType = nsGALIndexType::UInt;
  nsUInt32 m_uiMaxPrimitives = 0;
  nsUInt32 m_uiMaxVertices = 0;
  bool m_bColorStream = false;
};

struct NS_RENDERERCORE_DLL nsDynamicMeshVertex
{
  NS_DECLARE_POD_TYPE();

  nsVec3 m_vPosition;
  nsVec2 m_vTexCoord;
  nsVec3 m_vEncodedNormal;
  nsVec4 m_vEncodedTangent;
  // nsColorLinearUB m_Color;

  NS_ALWAYS_INLINE void EncodeNormal(const nsVec3& vNormal)
  {
    // store in [0; 1] range
    m_vEncodedNormal = vNormal * 0.5f + nsVec3(0.5f);

    // this is the same
    // nsMeshBufferUtils::EncodeNormal(normal, nsByteArrayPtr(reinterpret_cast<nsUInt8*>(&m_vEncodedNormal), sizeof(nsVec3)), nsMeshNormalPrecision::_32Bit).IgnoreResult();
  }

  NS_ALWAYS_INLINE void EncodeTangent(const nsVec3& vTangent, float fBitangentSign)
  {
    // store in [0; 1] range
    m_vEncodedTangent.x = vTangent.x * 0.5f + 0.5f;
    m_vEncodedTangent.y = vTangent.y * 0.5f + 0.5f;
    m_vEncodedTangent.z = vTangent.z * 0.5f + 0.5f;
    m_vEncodedTangent.w = fBitangentSign < 0.0f ? 0.0f : 1.0f;

    // this is the same
    // nsMeshBufferUtils::EncodeTangent(tangent, bitangentSign, nsByteArrayPtr(reinterpret_cast<nsUInt8*>(&m_vEncodedTangent), sizeof(nsVec4)), nsMeshNormalPrecision::_32Bit).IgnoreResult();
  }
};

class NS_RENDERERCORE_DLL nsDynamicMeshBufferResource : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsDynamicMeshBufferResource, nsResource);
  NS_RESOURCE_DECLARE_COMMON_CODE(nsDynamicMeshBufferResource);
  NS_RESOURCE_DECLARE_CREATEABLE(nsDynamicMeshBufferResource, nsDynamicMeshBufferResourceDescriptor);

public:
  nsDynamicMeshBufferResource();
  ~nsDynamicMeshBufferResource();

  NS_ALWAYS_INLINE const nsDynamicMeshBufferResourceDescriptor& GetDescriptor() const { return m_Descriptor; }
  NS_ALWAYS_INLINE nsGALBufferHandle GetVertexBuffer() const { return m_hVertexBuffer; }
  NS_ALWAYS_INLINE nsGALBufferHandle GetIndexBuffer() const { return m_hIndexBuffer; }
  NS_ALWAYS_INLINE nsGALBufferHandle GetColorBuffer() const { return m_hColorBuffer; }

  /// \brief Grants write access to the vertex data, and flags the data as 'dirty'.
  nsArrayPtr<nsDynamicMeshVertex> AccessVertexData()
  {
    m_bAccessedVB = true;
    return m_VertexData;
  }

  /// \brief Grants write access to the 16 bit index data, and flags the data as 'dirty'.
  ///
  /// Accessing this data is only valid, if the buffer was created with 16 bit indices.
  nsArrayPtr<nsUInt16> AccessIndex16Data()
  {
    m_bAccessedIB = true;
    return m_Index16Data;
  }

  /// \brief Grants write access to the 32 bit index data, and flags the data as 'dirty'.
  ///
  /// Accessing this data is only valid, if the buffer was created with 32 bit indices.
  nsArrayPtr<nsUInt32> AccessIndex32Data()
  {
    m_bAccessedIB = true;
    return m_Index32Data;
  }

  /// \brief Grants write access to the color data, and flags the data as 'dirty'.
  ///
  /// Accessing this data is only valid, if creation of the color buffer was enabled.
  nsArrayPtr<nsColorLinearUB> AccessColorData()
  {
    m_bAccessedCB = true;
    return m_ColorData;
  }

  const nsVertexDeclarationInfo& GetVertexDeclaration() const { return m_VertexDeclaration; }

  /// \brief Uploads the current vertex and index data to the GPU.
  ///
  /// If all values are set to default, the entire data is uploaded.
  /// If \a uiNumVertices or \a uiNumIndices is set to the max value, all vertices or indices (after their start offset) are uploaded.
  ///
  /// In all other cases, the number of elements to upload must be within valid bounds.
  ///
  /// This function can be used to only upload a subset of the modified data.
  ///
  /// Note that this function doesn't do anything, if the vertex or index data wasn't recently accessed through AccessVertexData(), AccessIndex16Data() or AccessIndex32Data(). So if you want to upload multiple pieces of the data to the GPU, you have to call these functions in between to flag the uploaded data as out-of-date.
  void UpdateGpuBuffer(nsGALCommandEncoder* pGALCommandEncoder, nsUInt32 uiFirstVertex = 0, nsUInt32 uiNumVertices = nsMath::MaxValue<nsUInt32>(), nsUInt32 uiFirstIndex = 0, nsUInt32 uiNumIndices = nsMath::MaxValue<nsUInt32>(), nsGALUpdateMode::Enum mode = nsGALUpdateMode::Discard);

private:
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

  bool m_bAccessedVB = false;
  bool m_bAccessedIB = false;
  bool m_bAccessedCB = false;

  nsGALBufferHandle m_hVertexBuffer;
  nsGALBufferHandle m_hIndexBuffer;
  nsGALBufferHandle m_hColorBuffer;
  nsDynamicMeshBufferResourceDescriptor m_Descriptor;

  nsVertexDeclarationInfo m_VertexDeclaration;
  nsDynamicArray<nsDynamicMeshVertex, nsAlignedAllocatorWrapper> m_VertexData;
  nsDynamicArray<nsUInt16, nsAlignedAllocatorWrapper> m_Index16Data;
  nsDynamicArray<nsUInt32, nsAlignedAllocatorWrapper> m_Index32Data;
  nsDynamicArray<nsColorLinearUB, nsAlignedAllocatorWrapper> m_ColorData;
};
