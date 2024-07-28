#pragma once

#include <Core/ResourceManager/Resource.h>
#include <Foundation/Math/BoundingBoxSphere.h>
#include <Foundation/Memory/AllocatorWrapper.h>
#include <RendererCore/RendererCoreDLL.h>
#include <RendererFoundation/Descriptors/Descriptors.h>

using nsMeshBufferResourceHandle = nsTypedResourceHandle<class nsMeshBufferResource>;
class nsGeometry;

struct NS_RENDERERCORE_DLL nsVertexStreamInfo : public nsHashableStruct<nsVertexStreamInfo>
{
  NS_DECLARE_POD_TYPE();

  nsGALVertexAttributeSemantic::Enum m_Semantic;
  nsUInt8 m_uiVertexBufferSlot = 0;
  nsGALResourceFormat::Enum m_Format;
  nsUInt16 m_uiOffset;      ///< at which byte offset the first element starts
  nsUInt16 m_uiElementSize; ///< the number of bytes for this element type (depends on the format); this is not the stride between elements!
};

struct NS_RENDERERCORE_DLL nsVertexDeclarationInfo
{
  void ComputeHash();

  nsHybridArray<nsVertexStreamInfo, 8> m_VertexStreams;
  nsUInt32 m_uiHash;
};


struct NS_RENDERERCORE_DLL nsMeshBufferResourceDescriptor
{
public:
  nsMeshBufferResourceDescriptor();
  ~nsMeshBufferResourceDescriptor();

  void Clear();

  /// \brief Use this function to add vertex streams to the mesh buffer. The return value is the index of the just added stream.
  nsUInt32 AddStream(nsGALVertexAttributeSemantic::Enum semantic, nsGALResourceFormat::Enum format);

  /// \brief Adds common vertex streams to the mesh buffer.
  ///
  /// The streams are added in this order (with the corresponding stream indices):
  /// * Position (index 0)
  /// * TexCoord0 (index 1)
  /// * Normal (index 2)
  /// * Tangent (index 3)
  void AddCommonStreams();

  /// \brief After all streams are added, call this to allocate the data for the streams. If uiNumPrimitives is 0, the mesh buffer will not
  /// use indexed rendering.
  void AllocateStreams(nsUInt32 uiNumVertices, nsGALPrimitiveTopology::Enum topology = nsGALPrimitiveTopology::Triangles, nsUInt32 uiNumPrimitives = 0, bool bZeroFill = false);

  /// \brief Creates streams and fills them with data from the nsGeometry. Only the geometry matching the given topology is used.
  ///  Streams that do not match any of the data inside the nsGeometry directly are skipped.
  void AllocateStreamsFromGeometry(const nsGeometry& geom, nsGALPrimitiveTopology::Enum topology = nsGALPrimitiveTopology::Triangles);

  /// \brief Gives read access to the allocated vertex data
  nsArrayPtr<const nsUInt8> GetVertexBufferData() const;

  /// \brief Gives read access to the allocated index data
  nsArrayPtr<const nsUInt8> GetIndexBufferData() const;

  /// \brief Allows write access to the allocated vertex data. This can be used for copying data fast into the array.
  nsDynamicArray<nsUInt8, nsAlignedAllocatorWrapper>& GetVertexBufferData();

  /// \brief Allows write access to the allocated index data. This can be used for copying data fast into the array.
  nsDynamicArray<nsUInt8, nsAlignedAllocatorWrapper>& GetIndexBufferData();

  /// \brief Slow, but convenient method to write one piece of vertex data at a time into the stream buffer.
  ///
  /// uiStream is the index of the data stream to write to.
  /// uiVertexIndex is the index of the vertex for which to write the data.
  /// data is the piece of data to write to the stream.
  template <typename TYPE>
  void SetVertexData(nsUInt32 uiStream, nsUInt32 uiVertexIndex, const TYPE& data)
  {
    reinterpret_cast<TYPE&>(m_VertexStreamData[m_uiVertexSize * uiVertexIndex + m_VertexDeclaration.m_VertexStreams[uiStream].m_uiOffset]) = data;
  }

  /// \brief Slow, but convenient method to access one piece of vertex data at a time into the stream buffer.
  ///
  /// uiStream is the index of the data stream to write to.
  /// uiVertexIndex is the index of the vertex for which to write the data.
  nsArrayPtr<nsUInt8> GetVertexData(nsUInt32 uiStream, nsUInt32 uiVertexIndex) { return m_VertexStreamData.GetArrayPtr().GetSubArray(m_uiVertexSize * uiVertexIndex + m_VertexDeclaration.m_VertexStreams[uiStream].m_uiOffset); }

  /// \brief Writes the vertex index for the given point into the index buffer.
  void SetPointIndices(nsUInt32 uiPoint, nsUInt32 uiVertex0);

  /// \brief Writes the two vertex indices for the given line into the index buffer.
  void SetLineIndices(nsUInt32 uiLine, nsUInt32 uiVertex0, nsUInt32 uiVertex1);

  /// \brief Writes the three vertex indices for the given triangle into the index buffer.
  void SetTriangleIndices(nsUInt32 uiTriangle, nsUInt32 uiVertex0, nsUInt32 uiVertex1, nsUInt32 uiVertex2);

  /// \brief Allows to read the stream info of the descriptor, which is filled out by AddStream()
  const nsVertexDeclarationInfo& GetVertexDeclaration() const { return m_VertexDeclaration; }

  /// \brief Returns the byte size of all the data for one vertex.
  nsUInt32 GetVertexDataSize() const { return m_uiVertexSize; }

  /// \brief Return the number of vertices, with which AllocateStreams() was called.
  nsUInt32 GetVertexCount() const { return m_uiVertexCount; }

  /// \brief Returns the number of primitives that the array holds.
  nsUInt32 GetPrimitiveCount() const;

  /// \brief Returns whether 16 or 32 Bit indices are to be used.
  bool Uses32BitIndices() const { return m_uiVertexCount > 0xFFFF; }

  /// \brief Returns whether an index buffer is available.
  bool HasIndexBuffer() const { return !m_IndexBufferData.IsEmpty(); }

  /// \brief Calculates the bounds using the data from the position stream
  nsBoundingBoxSphere ComputeBounds() const;

  /// \brief Returns the primitive topology
  nsGALPrimitiveTopology::Enum GetTopology() const { return m_Topology; }

  nsResult RecomputeNormals();

private:
  nsGALPrimitiveTopology::Enum m_Topology;
  nsUInt32 m_uiVertexSize;
  nsUInt32 m_uiVertexCount;
  nsVertexDeclarationInfo m_VertexDeclaration;
  nsDynamicArray<nsUInt8, nsAlignedAllocatorWrapper> m_VertexStreamData;
  nsDynamicArray<nsUInt8, nsAlignedAllocatorWrapper> m_IndexBufferData;
};

class NS_RENDERERCORE_DLL nsMeshBufferResource : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsMeshBufferResource, nsResource);
  NS_RESOURCE_DECLARE_COMMON_CODE(nsMeshBufferResource);
  NS_RESOURCE_DECLARE_CREATEABLE(nsMeshBufferResource, nsMeshBufferResourceDescriptor);

public:
  nsMeshBufferResource()
    : nsResource(DoUpdate::OnAnyThread, 1)

  {
  }
  ~nsMeshBufferResource();

  NS_ALWAYS_INLINE nsUInt32 GetPrimitiveCount() const { return m_uiPrimitiveCount; }

  NS_ALWAYS_INLINE nsGALBufferHandle GetVertexBuffer() const { return m_hVertexBuffer; }

  NS_ALWAYS_INLINE nsGALBufferHandle GetIndexBuffer() const { return m_hIndexBuffer; }

  NS_ALWAYS_INLINE nsGALPrimitiveTopology::Enum GetTopology() const { return m_Topology; }

  /// \brief Returns the vertex declaration used by this mesh buffer.
  const nsVertexDeclarationInfo& GetVertexDeclaration() const { return m_VertexDeclaration; }

  /// \brief Returns the bounds of the mesh
  const nsBoundingBoxSphere& GetBounds() const { return m_Bounds; }

private:
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

  nsBoundingBoxSphere m_Bounds;
  nsVertexDeclarationInfo m_VertexDeclaration;
  nsUInt32 m_uiPrimitiveCount = 0;
  nsGALBufferHandle m_hVertexBuffer;
  nsGALBufferHandle m_hIndexBuffer;
  nsGALPrimitiveTopology::Enum m_Topology = nsGALPrimitiveTopology::Enum::Default;
};
