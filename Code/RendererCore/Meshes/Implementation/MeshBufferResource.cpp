#include <RendererCore/RendererCorePCH.h>

#include <Core/Graphics/Geometry.h>
#include <RendererCore/Meshes/MeshBufferResource.h>
#include <RendererCore/Meshes/MeshBufferUtils.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Resources/Buffer.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMeshBufferResource, 1, nsRTTIDefaultAllocator<nsMeshBufferResource>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsMeshBufferResource);
// clang-format on

nsMeshBufferResourceDescriptor::nsMeshBufferResourceDescriptor()
{
  m_Topology = nsGALPrimitiveTopology::Triangles;
  m_uiVertexSize = 0;
  m_uiVertexCount = 0;
}

nsMeshBufferResourceDescriptor::~nsMeshBufferResourceDescriptor() = default;

void nsMeshBufferResourceDescriptor::Clear()
{
  m_Topology = nsGALPrimitiveTopology::Triangles;
  m_uiVertexSize = 0;
  m_uiVertexCount = 0;
  m_VertexDeclaration.m_uiHash = 0;
  m_VertexDeclaration.m_VertexStreams.Clear();
  m_VertexStreamData.Clear();
  m_IndexBufferData.Clear();
}

nsArrayPtr<const nsUInt8> nsMeshBufferResourceDescriptor::GetVertexBufferData() const
{
  return m_VertexStreamData.GetArrayPtr();
}

nsArrayPtr<const nsUInt8> nsMeshBufferResourceDescriptor::GetIndexBufferData() const
{
  return m_IndexBufferData.GetArrayPtr();
}

nsDynamicArray<nsUInt8, nsAlignedAllocatorWrapper>& nsMeshBufferResourceDescriptor::GetVertexBufferData()
{
  return m_VertexStreamData;
}

nsDynamicArray<nsUInt8, nsAlignedAllocatorWrapper>& nsMeshBufferResourceDescriptor::GetIndexBufferData()
{
  return m_IndexBufferData;
}

nsUInt32 nsMeshBufferResourceDescriptor::AddStream(nsGALVertexAttributeSemantic::Enum semantic, nsGALResourceFormat::Enum format)
{
  NS_ASSERT_DEV(m_VertexStreamData.IsEmpty(), "This function can only be called before 'AllocateStreams' is called");

  for (nsUInt32 i = 0; i < m_VertexDeclaration.m_VertexStreams.GetCount(); ++i)
  {
    NS_ASSERT_DEV(m_VertexDeclaration.m_VertexStreams[i].m_Semantic != semantic, "The given semantic {0} is already used by a previous stream", semantic);
  }

  nsVertexStreamInfo si;

  si.m_Semantic = semantic;
  si.m_Format = format;
  si.m_uiOffset = 0;
  si.m_uiElementSize = static_cast<nsUInt16>(nsGALResourceFormat::GetBitsPerElement(format) / 8);
  m_uiVertexSize += si.m_uiElementSize;

  NS_ASSERT_DEV(si.m_uiElementSize > 0, "Invalid Element Size. Format not supported?");

  if (!m_VertexDeclaration.m_VertexStreams.IsEmpty())
    si.m_uiOffset = m_VertexDeclaration.m_VertexStreams.PeekBack().m_uiOffset + m_VertexDeclaration.m_VertexStreams.PeekBack().m_uiElementSize;

  m_VertexDeclaration.m_VertexStreams.PushBack(si);

  return m_VertexDeclaration.m_VertexStreams.GetCount() - 1;
}

void nsMeshBufferResourceDescriptor::AddCommonStreams()
{
  AddStream(nsGALVertexAttributeSemantic::Position, nsGALResourceFormat::XYZFloat);
  AddStream(nsGALVertexAttributeSemantic::TexCoord0, nsMeshTexCoordPrecision::ToResourceFormat(nsMeshTexCoordPrecision::Default));
  AddStream(nsGALVertexAttributeSemantic::Normal, nsMeshNormalPrecision::ToResourceFormatNormal(nsMeshNormalPrecision::Default));
  AddStream(nsGALVertexAttributeSemantic::Tangent, nsMeshNormalPrecision::ToResourceFormatTangent(nsMeshNormalPrecision::Default));
}

void nsMeshBufferResourceDescriptor::AllocateStreams(nsUInt32 uiNumVertices, nsGALPrimitiveTopology::Enum topology, nsUInt32 uiNumPrimitives, bool bZeroFill /*= false*/)
{
  NS_ASSERT_DEV(!m_VertexDeclaration.m_VertexStreams.IsEmpty(), "You have to add streams via 'AddStream' before calling this function");

  m_Topology = topology;
  m_uiVertexCount = uiNumVertices;
  const nsUInt32 uiVertexStreamSize = m_uiVertexSize * uiNumVertices;

  if (bZeroFill)
  {
    m_VertexStreamData.SetCount(uiVertexStreamSize);
  }
  else
  {
    m_VertexStreamData.SetCountUninitialized(uiVertexStreamSize);
  }

  if (uiNumPrimitives > 0)
  {
    // use an index buffer at all
    nsUInt32 uiIndexBufferSize = uiNumPrimitives * nsGALPrimitiveTopology::VerticesPerPrimitive(topology);

    if (Uses32BitIndices())
    {
      uiIndexBufferSize *= sizeof(nsUInt32);
    }
    else
    {
      uiIndexBufferSize *= sizeof(nsUInt16);
    }

    m_IndexBufferData.SetCountUninitialized(uiIndexBufferSize);
  }
}

void nsMeshBufferResourceDescriptor::AllocateStreamsFromGeometry(const nsGeometry& geom, nsGALPrimitiveTopology::Enum topology)
{
  nsLogBlock _("Allocate Streams From Geometry");

  // Index Buffer Generation
  nsDynamicArray<nsUInt32> Indices;

  if (topology == nsGALPrimitiveTopology::Points)
  {
    // Leaving indices empty disables indexed rendering.
  }
  else if (topology == nsGALPrimitiveTopology::Lines)
  {
    Indices.Reserve(geom.GetLines().GetCount() * 2);

    for (nsUInt32 p = 0; p < geom.GetLines().GetCount(); ++p)
    {
      Indices.PushBack(geom.GetLines()[p].m_uiStartVertex);
      Indices.PushBack(geom.GetLines()[p].m_uiEndVertex);
    }
  }
  else if (topology == nsGALPrimitiveTopology::Triangles)
  {
    Indices.Reserve(geom.GetPolygons().GetCount() * 6);

    for (nsUInt32 p = 0; p < geom.GetPolygons().GetCount(); ++p)
    {
      for (nsUInt32 v = 0; v < geom.GetPolygons()[p].m_Vertices.GetCount() - 2; ++v)
      {
        Indices.PushBack(geom.GetPolygons()[p].m_Vertices[0]);
        Indices.PushBack(geom.GetPolygons()[p].m_Vertices[v + 1]);
        Indices.PushBack(geom.GetPolygons()[p].m_Vertices[v + 2]);
      }
    }
  }
  AllocateStreams(geom.GetVertices().GetCount(), topology, Indices.GetCount() / (topology + 1));

  // Fill vertex buffer.
  for (nsUInt32 s = 0; s < m_VertexDeclaration.m_VertexStreams.GetCount(); ++s)
  {
    const nsVertexStreamInfo& si = m_VertexDeclaration.m_VertexStreams[s];
    switch (si.m_Semantic)
    {
      case nsGALVertexAttributeSemantic::Position:
      {
        if (si.m_Format == nsGALResourceFormat::XYZFloat)
        {
          for (nsUInt32 v = 0; v < geom.GetVertices().GetCount(); ++v)
          {
            SetVertexData<nsVec3>(s, v, geom.GetVertices()[v].m_vPosition);
          }
        }
        else
        {
          nsLog::Error("Position stream with format '{0}' is not supported.", (int)si.m_Format);
        }
      }
      break;

      case nsGALVertexAttributeSemantic::Normal:
      {
        for (nsUInt32 v = 0; v < geom.GetVertices().GetCount(); ++v)
        {
          if (nsMeshBufferUtils::EncodeNormal(geom.GetVertices()[v].m_vNormal, GetVertexData(s, v), si.m_Format).Failed())
          {
            nsLog::Error("Normal stream with format '{0}' is not supported.", (int)si.m_Format);
            break;
          }
        }
      }
      break;

      case nsGALVertexAttributeSemantic::Tangent:
      {
        for (nsUInt32 v = 0; v < geom.GetVertices().GetCount(); ++v)
        {
          if (nsMeshBufferUtils::EncodeTangent(geom.GetVertices()[v].m_vTangent, geom.GetVertices()[v].m_fBiTangentSign, GetVertexData(s, v), si.m_Format).Failed())
          {
            nsLog::Error("Tangent stream with format '{0}' is not supported.", (int)si.m_Format);
            break;
          }
        }
      }
      break;

      case nsGALVertexAttributeSemantic::Color0:
      case nsGALVertexAttributeSemantic::Color1:
      {
        if (si.m_Format == nsGALResourceFormat::RGBAUByteNormalized)
        {
          for (nsUInt32 v = 0; v < geom.GetVertices().GetCount(); ++v)
          {
            SetVertexData<nsColorLinearUB>(s, v, geom.GetVertices()[v].m_Color);
          }
        }
        else
        {
          nsLog::Error("Color stream with format '{0}' is not supported.", (int)si.m_Format);
        }
      }
      break;

      case nsGALVertexAttributeSemantic::TexCoord0:
      case nsGALVertexAttributeSemantic::TexCoord1:
      {
        for (nsUInt32 v = 0; v < geom.GetVertices().GetCount(); ++v)
        {
          if (nsMeshBufferUtils::EncodeTexCoord(geom.GetVertices()[v].m_vTexCoord, GetVertexData(s, v), si.m_Format).Failed())
          {
            nsLog::Error("UV stream with format '{0}' is not supported.", (int)si.m_Format);
            break;
          }
        }
      }
      break;

      case nsGALVertexAttributeSemantic::BoneIndices0:
      {
        // if a bone index array is available, move the custom index into it

        if (si.m_Format == nsGALResourceFormat::RGBAUByte)
        {
          for (nsUInt32 v = 0; v < geom.GetVertices().GetCount(); ++v)
          {
            nsVec4U16 boneIndices = geom.GetVertices()[v].m_BoneIndices;
            nsVec4U8 storage(static_cast<nsUInt8>(boneIndices.x), static_cast<nsUInt8>(boneIndices.y), static_cast<nsUInt8>(boneIndices.z), static_cast<nsUInt8>(boneIndices.w));
            SetVertexData<nsVec4U8>(s, v, storage);
          }
        }
        else if (si.m_Format == nsGALResourceFormat::RGBAUShort)
        {
          for (nsUInt32 v = 0; v < geom.GetVertices().GetCount(); ++v)
          {
            SetVertexData<nsVec4U16>(s, v, geom.GetVertices()[v].m_BoneIndices);
          }
        }
      }
      break;

      case nsGALVertexAttributeSemantic::BoneWeights0:
      {
        // if a bone weight array is available, set it to fully use the first bone

        if (si.m_Format == nsGALResourceFormat::RGBAUByteNormalized)
        {
          for (nsUInt32 v = 0; v < geom.GetVertices().GetCount(); ++v)
          {
            SetVertexData<nsColorLinearUB>(s, v, geom.GetVertices()[v].m_BoneWeights);
          }
        }

        if (si.m_Format == nsGALResourceFormat::XYZWFloat)
        {
          for (nsUInt32 v = 0; v < geom.GetVertices().GetCount(); ++v)
          {
            SetVertexData<nsVec4>(s, v, nsColor(geom.GetVertices()[v].m_BoneWeights).GetAsVec4());
          }
        }
      }
      break;

      case nsGALVertexAttributeSemantic::BoneIndices1:
      case nsGALVertexAttributeSemantic::BoneWeights1:
        // Don't error out for these semantics as they may be used by the user (e.g. breakable mesh construction)
        break;

      default:
      {
        nsLog::Error("Streams semantic '{0}' is not supported.", (int)si.m_Semantic);
      }
      break;
    }
  }

  // Fill index buffer.
  if (topology == nsGALPrimitiveTopology::Points)
  {
    for (nsUInt32 t = 0; t < Indices.GetCount(); t += 1)
    {
      SetPointIndices(t, Indices[t]);
    }
  }
  else if (topology == nsGALPrimitiveTopology::Triangles)
  {
    for (nsUInt32 t = 0; t < Indices.GetCount(); t += 3)
    {
      SetTriangleIndices(t / 3, Indices[t], Indices[t + 1], Indices[t + 2]);
    }
  }
  else if (topology == nsGALPrimitiveTopology::Lines)
  {
    for (nsUInt32 t = 0; t < Indices.GetCount(); t += 2)
    {
      SetLineIndices(t / 2, Indices[t], Indices[t + 1]);
    }
  }
}

void nsMeshBufferResourceDescriptor::SetPointIndices(nsUInt32 uiPoint, nsUInt32 uiVertex0)
{
  NS_ASSERT_DEBUG(m_Topology == nsGALPrimitiveTopology::Points, "Wrong topology");

  if (Uses32BitIndices())
  {
    nsUInt32* pIndices = reinterpret_cast<nsUInt32*>(&m_IndexBufferData[uiPoint * sizeof(nsUInt32) * 1]);
    pIndices[0] = uiVertex0;
  }
  else
  {
    nsUInt16* pIndices = reinterpret_cast<nsUInt16*>(&m_IndexBufferData[uiPoint * sizeof(nsUInt16) * 1]);
    pIndices[0] = static_cast<nsUInt16>(uiVertex0);
  }
}

void nsMeshBufferResourceDescriptor::SetLineIndices(nsUInt32 uiLine, nsUInt32 uiVertex0, nsUInt32 uiVertex1)
{
  NS_ASSERT_DEBUG(m_Topology == nsGALPrimitiveTopology::Lines, "Wrong topology");

  if (Uses32BitIndices())
  {
    nsUInt32* pIndices = reinterpret_cast<nsUInt32*>(&m_IndexBufferData[uiLine * sizeof(nsUInt32) * 2]);
    pIndices[0] = uiVertex0;
    pIndices[1] = uiVertex1;
  }
  else
  {
    nsUInt16* pIndices = reinterpret_cast<nsUInt16*>(&m_IndexBufferData[uiLine * sizeof(nsUInt16) * 2]);
    pIndices[0] = static_cast<nsUInt16>(uiVertex0);
    pIndices[1] = static_cast<nsUInt16>(uiVertex1);
  }
}

void nsMeshBufferResourceDescriptor::SetTriangleIndices(nsUInt32 uiTriangle, nsUInt32 uiVertex0, nsUInt32 uiVertex1, nsUInt32 uiVertex2)
{
  NS_ASSERT_DEBUG(m_Topology == nsGALPrimitiveTopology::Triangles, "Wrong topology");
  NS_ASSERT_DEBUG(uiVertex0 < m_uiVertexCount && uiVertex1 < m_uiVertexCount && uiVertex2 < m_uiVertexCount, "Vertex indices out of range.");

  if (Uses32BitIndices())
  {
    nsUInt32* pIndices = reinterpret_cast<nsUInt32*>(&m_IndexBufferData[uiTriangle * sizeof(nsUInt32) * 3]);
    pIndices[0] = uiVertex0;
    pIndices[1] = uiVertex1;
    pIndices[2] = uiVertex2;
  }
  else
  {
    nsUInt16* pIndices = reinterpret_cast<nsUInt16*>(&m_IndexBufferData[uiTriangle * sizeof(nsUInt16) * 3]);
    pIndices[0] = static_cast<nsUInt16>(uiVertex0);
    pIndices[1] = static_cast<nsUInt16>(uiVertex1);
    pIndices[2] = static_cast<nsUInt16>(uiVertex2);
  }
}

nsUInt32 nsMeshBufferResourceDescriptor::GetPrimitiveCount() const
{
  const nsUInt32 divider = m_Topology + 1;

  if (!m_IndexBufferData.IsEmpty())
  {
    if (Uses32BitIndices())
      return (m_IndexBufferData.GetCount() / sizeof(nsUInt32)) / divider;
    else
      return (m_IndexBufferData.GetCount() / sizeof(nsUInt16)) / divider;
  }
  else
  {
    return m_uiVertexCount / divider;
  }
}

nsBoundingBoxSphere nsMeshBufferResourceDescriptor::ComputeBounds() const
{
  nsBoundingBoxSphere bounds = nsBoundingBoxSphere::MakeInvalid();

  for (nsUInt32 i = 0; i < m_VertexDeclaration.m_VertexStreams.GetCount(); ++i)
  {
    if (m_VertexDeclaration.m_VertexStreams[i].m_Semantic == nsGALVertexAttributeSemantic::Position)
    {
      NS_ASSERT_DEBUG(m_VertexDeclaration.m_VertexStreams[i].m_Format == nsGALResourceFormat::XYZFloat, "Position format is not usable");

      const nsUInt32 offset = m_VertexDeclaration.m_VertexStreams[i].m_uiOffset;

      if (!m_VertexStreamData.IsEmpty() && m_uiVertexCount > 0)
      {
        bounds = nsBoundingBoxSphere::MakeFromPoints(reinterpret_cast<const nsVec3*>(&m_VertexStreamData[offset]), m_uiVertexCount, m_uiVertexSize);
      }

      return bounds;
    }
  }

  if (!bounds.IsValid())
  {
    bounds = nsBoundingBoxSphere::MakeFromCenterExtents(nsVec3::MakeZero(), nsVec3(0.1f), 0.1f);
  }

  return bounds;
}

nsResult nsMeshBufferResourceDescriptor::RecomputeNormals()
{
  if (m_Topology != nsGALPrimitiveTopology::Triangles)
    return NS_FAILURE; // normals not needed

  const nsUInt32 uiVertexSize = m_uiVertexSize;
  const nsUInt8* pPositions = nullptr;
  nsUInt8* pNormals = nullptr;
  nsGALResourceFormat::Enum normalsFormat = nsGALResourceFormat::XYZFloat;

  for (nsUInt32 i = 0; i < m_VertexDeclaration.m_VertexStreams.GetCount(); ++i)
  {
    if (m_VertexDeclaration.m_VertexStreams[i].m_Semantic == nsGALVertexAttributeSemantic::Position && m_VertexDeclaration.m_VertexStreams[i].m_Format == nsGALResourceFormat::XYZFloat)
    {
      pPositions = GetVertexData(i, 0).GetPtr();
    }

    if (m_VertexDeclaration.m_VertexStreams[i].m_Semantic == nsGALVertexAttributeSemantic::Normal)
    {
      normalsFormat = m_VertexDeclaration.m_VertexStreams[i].m_Format;
      pNormals = GetVertexData(i, 0).GetPtr();
    }
  }

  if (pPositions == nullptr || pNormals == nullptr)
    return NS_FAILURE; // there are no normals that could be recomputed

  nsDynamicArray<nsVec3> newNormals;
  newNormals.SetCountUninitialized(m_uiVertexCount);

  for (auto& n : newNormals)
  {
    n.SetZero();
  }

  nsResult res = NS_SUCCESS;

  const nsUInt16* pIndices16 = reinterpret_cast<const nsUInt16*>(m_IndexBufferData.GetData());
  const nsUInt32* pIndices32 = reinterpret_cast<const nsUInt32*>(m_IndexBufferData.GetData());
  const bool bUseIndices32 = Uses32BitIndices();

  // Compute unnormalized triangle normals and add them to all vertices.
  // This way large triangles have an higher influence on the vertex normal.
  for (nsUInt32 triIdx = 0; triIdx < GetPrimitiveCount(); ++triIdx)
  {
    const nsUInt32 v0 = bUseIndices32 ? pIndices32[triIdx * 3 + 0] : pIndices16[triIdx * 3 + 0];
    const nsUInt32 v1 = bUseIndices32 ? pIndices32[triIdx * 3 + 1] : pIndices16[triIdx * 3 + 1];
    const nsUInt32 v2 = bUseIndices32 ? pIndices32[triIdx * 3 + 2] : pIndices16[triIdx * 3 + 2];

    const nsVec3 p0 = *reinterpret_cast<const nsVec3*>(pPositions + nsMath::SafeMultiply64(uiVertexSize, v0));
    const nsVec3 p1 = *reinterpret_cast<const nsVec3*>(pPositions + nsMath::SafeMultiply64(uiVertexSize, v1));
    const nsVec3 p2 = *reinterpret_cast<const nsVec3*>(pPositions + nsMath::SafeMultiply64(uiVertexSize, v2));

    const nsVec3 d01 = p1 - p0;
    const nsVec3 d02 = p2 - p0;

    const nsVec3 triNormal = d01.CrossRH(d02);

    if (triNormal.IsValid())
    {
      newNormals[v0] += triNormal;
      newNormals[v1] += triNormal;
      newNormals[v2] += triNormal;
    }
  }

  for (nsUInt32 i = 0; i < newNormals.GetCount(); ++i)
  {
    // normalize the new normal
    if (newNormals[i].NormalizeIfNotZero(nsVec3::MakeAxisX()).Failed())
      res = NS_FAILURE;

    // then encode it in the target format precision and write it back to the buffer
    NS_SUCCEED_OR_RETURN(nsMeshBufferUtils::EncodeNormal(newNormals[i], nsByteArrayPtr(pNormals + nsMath::SafeMultiply64(uiVertexSize, i), sizeof(nsVec3)), normalsFormat));
  }

  return res;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

nsMeshBufferResource::~nsMeshBufferResource()
{
  NS_ASSERT_DEBUG(m_hVertexBuffer.IsInvalidated(), "Implementation error");
  NS_ASSERT_DEBUG(m_hIndexBuffer.IsInvalidated(), "Implementation error");
}

nsResourceLoadDesc nsMeshBufferResource::UnloadData(Unload WhatToUnload)
{
  if (!m_hVertexBuffer.IsInvalidated())
  {
    nsGALDevice::GetDefaultDevice()->DestroyBuffer(m_hVertexBuffer);
    m_hVertexBuffer.Invalidate();
  }

  if (!m_hIndexBuffer.IsInvalidated())
  {
    nsGALDevice::GetDefaultDevice()->DestroyBuffer(m_hIndexBuffer);
    m_hIndexBuffer.Invalidate();
  }

  m_uiPrimitiveCount = 0;

  // we cannot compute this in UpdateMemoryUsage(), so we only read the data there, therefore we need to update this information here
  ModifyMemoryUsage().m_uiMemoryGPU = 0;

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Unloaded;

  return res;
}

nsResourceLoadDesc nsMeshBufferResource::UpdateContent(nsStreamReader* Stream)
{
  NS_REPORT_FAILURE("This resource type does not support loading data from file.");

  return nsResourceLoadDesc();
}

void nsMeshBufferResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  // we cannot compute this data here, so we update it wherever we know the memory usage

  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(nsMeshBufferResource);
  out_NewMemoryUsage.m_uiMemoryGPU = ModifyMemoryUsage().m_uiMemoryGPU;
}

NS_RESOURCE_IMPLEMENT_CREATEABLE(nsMeshBufferResource, nsMeshBufferResourceDescriptor)
{
  NS_ASSERT_DEBUG(m_hVertexBuffer.IsInvalidated(), "Implementation error");
  NS_ASSERT_DEBUG(m_hIndexBuffer.IsInvalidated(), "Implementation error");

  m_VertexDeclaration = descriptor.GetVertexDeclaration();
  m_VertexDeclaration.ComputeHash();

  m_uiPrimitiveCount = descriptor.GetPrimitiveCount();
  m_Topology = descriptor.GetTopology();

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  m_hVertexBuffer = pDevice->CreateVertexBuffer(descriptor.GetVertexDataSize(), descriptor.GetVertexCount(), descriptor.GetVertexBufferData().GetArrayPtr());

  nsStringBuilder sName;
  sName.SetFormat("{0} Vertex Buffer", GetResourceDescription());
  pDevice->GetBuffer(m_hVertexBuffer)->SetDebugName(sName);

  if (descriptor.HasIndexBuffer())
  {
    m_hIndexBuffer = pDevice->CreateIndexBuffer(descriptor.Uses32BitIndices() ? nsGALIndexType::UInt : nsGALIndexType::UShort, m_uiPrimitiveCount * nsGALPrimitiveTopology::VerticesPerPrimitive(m_Topology), descriptor.GetIndexBufferData());

    sName.SetFormat("{0} Index Buffer", GetResourceDescription());
    pDevice->GetBuffer(m_hIndexBuffer)->SetDebugName(sName);

    // we only know the memory usage here, so we write it back to the internal variable directly and then read it in UpdateMemoryUsage() again
    ModifyMemoryUsage().m_uiMemoryGPU = descriptor.GetVertexBufferData().GetCount() + descriptor.GetIndexBufferData().GetCount();
  }
  else
  {
    // we only know the memory usage here, so we write it back to the internal variable directly and then read it in UpdateMemoryUsage() again
    ModifyMemoryUsage().m_uiMemoryGPU = descriptor.GetVertexBufferData().GetCount();
  }


  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Loaded;

  m_Bounds = descriptor.ComputeBounds();

  return res;
}

void nsVertexDeclarationInfo::ComputeHash()
{
  m_uiHash = 0;

  for (const auto& vs : m_VertexStreams)
  {
    m_uiHash += vs.CalculateHash();

    NS_ASSERT_DEBUG(m_uiHash != 0, "Invalid Hash Value");
  }
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Meshes_Implementation_MeshBufferResource);
