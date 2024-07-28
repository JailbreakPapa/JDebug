#include <RendererCore/RendererCorePCH.h>

#include <Core/Graphics/Geometry.h>
#include <Core/World/World.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Math/Mat3.h>
#include <Foundation/Utilities/GraphicsUtils.h>
#include <RendererCore/Meshes/CpuMeshResource.h>
#include <RendererCore/Meshes/MeshBufferUtils.h>
#include <RendererCore/Utils/WorldGeoExtractionUtil.h>

// clang-format off
NS_IMPLEMENT_MESSAGE_TYPE(nsMsgExtractGeometry);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgExtractGeometry, 1, nsRTTIDefaultAllocator<nsMsgExtractGeometry>)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsExcludeFromScript()
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsWorldGeoExtractionUtil::ExtractWorldGeometry(MeshObjectList& ref_objects, const nsWorld& world, ExtractionMode mode, nsTagSet* pExcludeTags /*= nullptr*/)
{
  NS_PROFILE_SCOPE("ExtractWorldGeometry");
  NS_LOG_BLOCK("ExtractWorldGeometry", world.GetName());

  nsMsgExtractGeometry msg;
  msg.m_Mode = mode;
  msg.m_pMeshObjects = &ref_objects;

  NS_LOCK(world.GetReadMarker());

  for (auto it = world.GetObjects(); it.IsValid(); ++it)
  {
    if (pExcludeTags != nullptr && it->GetTags().IsAnySet(*pExcludeTags))
      continue;

    it->SendMessage(msg);
  }
}

void nsWorldGeoExtractionUtil::ExtractWorldGeometry(MeshObjectList& ref_objects, const nsWorld& world, ExtractionMode mode, const nsDeque<nsGameObjectHandle>& selection)
{
  NS_PROFILE_SCOPE("ExtractWorldGeometry");
  NS_LOG_BLOCK("ExtractWorldGeometry", world.GetName());

  nsMsgExtractGeometry msg;
  msg.m_Mode = mode;
  msg.m_pMeshObjects = &ref_objects;

  NS_LOCK(world.GetReadMarker());

  for (nsGameObjectHandle hObject : selection)
  {
    const nsGameObject* pObject;
    if (!world.TryGetObject(hObject, pObject))
      continue;

    pObject->SendMessage(msg);
  }
}

void nsWorldGeoExtractionUtil::WriteWorldGeometryToOBJ(const char* szFile, const MeshObjectList& objects, const nsMat3& mTransform)
{
  NS_LOG_BLOCK("Write World Geometry to OBJ", szFile);

  nsFileWriter file;
  if (file.Open(szFile).Failed())
  {
    nsLog::Error("Failed to open file for writing: '{0}'", szFile);
    return;
  }

  nsMat4 transform = nsMat4::MakeIdentity();
  transform.SetRotationalPart(mTransform);

  nsStringBuilder line;

  line = "\n\n# vertices\n\n";
  file.WriteBytes(line.GetData(), line.GetElementCount()).IgnoreResult();

  nsUInt32 uiVertexOffset = 0;
  nsDeque<nsUInt32> indices;

  for (const MeshObject& object : objects)
  {
    nsResourceLock<nsCpuMeshResource> pCpuMesh(object.m_hMeshResource, nsResourceAcquireMode::BlockTillLoaded_NeverFail);
    if (pCpuMesh.GetAcquireResult() != nsResourceAcquireResult::Final)
    {
      continue;
    }

    const auto& meshBufferDesc = pCpuMesh->GetDescriptor().MeshBufferDesc();

    const nsVec3* pPositions = nullptr;
    nsUInt32 uiElementStride = 0;
    if (nsMeshBufferUtils::GetPositionStream(meshBufferDesc, pPositions, uiElementStride).Failed())
    {
      continue;
    }

    nsMat4 finalTransform = transform * object.m_GlobalTransform.GetAsMat4();

    // write out all vertices
    for (nsUInt32 i = 0; i < meshBufferDesc.GetVertexCount(); ++i)
    {
      const nsVec3 pos = finalTransform.TransformPosition(*pPositions);

      line.SetFormat("v {0} {1} {2}\n", nsArgF(pos.x, 8), nsArgF(pos.y, 8), nsArgF(pos.z, 8));
      file.WriteBytes(line.GetData(), line.GetElementCount()).IgnoreResult();

      pPositions = nsMemoryUtils::AddByteOffset(pPositions, uiElementStride);
    }

    // collect all indices
    bool flip = nsGraphicsUtils::IsTriangleFlipRequired(finalTransform.GetRotationalPart());

    if (meshBufferDesc.HasIndexBuffer())
    {
      if (meshBufferDesc.Uses32BitIndices())
      {
        const nsUInt32* pTypedIndices = reinterpret_cast<const nsUInt32*>(meshBufferDesc.GetIndexBufferData().GetPtr());

        for (nsUInt32 p = 0; p < meshBufferDesc.GetPrimitiveCount(); ++p)
        {
          indices.PushBack(pTypedIndices[p * 3 + (flip ? 2 : 0)] + uiVertexOffset);
          indices.PushBack(pTypedIndices[p * 3 + 1] + uiVertexOffset);
          indices.PushBack(pTypedIndices[p * 3 + (flip ? 0 : 2)] + uiVertexOffset);
        }
      }
      else
      {
        const nsUInt16* pTypedIndices = reinterpret_cast<const nsUInt16*>(meshBufferDesc.GetIndexBufferData().GetPtr());

        for (nsUInt32 p = 0; p < meshBufferDesc.GetPrimitiveCount(); ++p)
        {
          indices.PushBack(pTypedIndices[p * 3 + (flip ? 2 : 0)] + uiVertexOffset);
          indices.PushBack(pTypedIndices[p * 3 + 1] + uiVertexOffset);
          indices.PushBack(pTypedIndices[p * 3 + (flip ? 0 : 2)] + uiVertexOffset);
        }
      }
    }
    else
    {
      for (nsUInt32 v = 0; v < meshBufferDesc.GetVertexCount(); ++v)
      {
        indices.PushBack(uiVertexOffset + v);
      }
    }

    uiVertexOffset += meshBufferDesc.GetVertexCount();
  }

  line = "\n\n# triangles\n\n";
  file.WriteBytes(line.GetData(), line.GetElementCount()).IgnoreResult();

  for (nsUInt32 i = 0; i < indices.GetCount(); i += 3)
  {
    // indices are 1 based in obj
    line.SetFormat("f {0} {1} {2}\n", indices[i + 0] + 1, indices[i + 1] + 1, indices[i + 2] + 1);
    file.WriteBytes(line.GetData(), line.GetElementCount()).IgnoreResult();
  }

  nsLog::Success("Wrote world geometry to '{0}'", file.GetFilePathAbsolute().GetView());
}

//////////////////////////////////////////////////////////////////////////

void nsMsgExtractGeometry::AddMeshObject(const nsTransform& transform, nsCpuMeshResourceHandle hMeshResource)
{
  m_pMeshObjects->PushBack({transform, hMeshResource});
}

void nsMsgExtractGeometry::AddBox(const nsTransform& transform, nsVec3 vExtents)
{
  const char* szResourceName = "CpuMesh-UnitBox";
  nsCpuMeshResourceHandle hBoxMesh = nsResourceManager::GetExistingResource<nsCpuMeshResource>(szResourceName);
  if (hBoxMesh.IsValid() == false)
  {
    nsGeometry geom;
    geom.AddBox(nsVec3(1), false);
    geom.TriangulatePolygons();
    geom.ComputeTangents();

    nsMeshResourceDescriptor desc;
    desc.SetMaterial(0, "{ 1c47ee4c-0379-4280-85f5-b8cda61941d2 }"); // Data/Base/Materials/Common/Pattern.nsMaterialAsset

    desc.MeshBufferDesc().AddCommonStreams();
    desc.MeshBufferDesc().AllocateStreamsFromGeometry(geom, nsGALPrimitiveTopology::Triangles);

    desc.AddSubMesh(desc.MeshBufferDesc().GetPrimitiveCount(), 0, 0);

    desc.ComputeBounds();

    hBoxMesh = nsResourceManager::GetOrCreateResource<nsCpuMeshResource>(szResourceName, std::move(desc), szResourceName);
  }

  auto& meshObject = m_pMeshObjects->ExpandAndGetRef();
  meshObject.m_GlobalTransform = transform;
  meshObject.m_GlobalTransform.m_vScale *= vExtents;
  meshObject.m_hMeshResource = hBoxMesh;
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Utils_Implementation_WorldGeoExtractionUtil);
