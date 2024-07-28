#include <RendererCore/RendererCorePCH.h>

#include <Core/Graphics/Geometry.h>
#include <Foundation/SimdMath/SimdVec4f.h>
#include <RendererCore/Rasterizer/RasterizerObject.h>
#include <RendererCore/Rasterizer/Thirdparty/Occluder.h>
#include <RendererCore/Rasterizer/Thirdparty/VectorMath.h>

nsMutex nsRasterizerObject::s_Mutex;
nsMap<nsString, nsSharedPtr<nsRasterizerObject>> nsRasterizerObject::s_Objects;

nsRasterizerObject::nsRasterizerObject() = default;
nsRasterizerObject::~nsRasterizerObject() = default;

#if NS_ENABLED(NS_RASTERIZER_SUPPORTED)

// needed for nsHybridArray below
NS_DEFINE_AS_POD_TYPE(__m128);

void nsRasterizerObject::CreateMesh(const nsGeometry& geo)
{
  nsHybridArray<__m128, 64, nsAlignedAllocatorWrapper> vertices;
  vertices.Reserve(geo.GetPolygons().GetCount() * 4);

  Aabb bounds;

  auto addVtx = [&](nsVec3 vtxPos)
  {
    nsSimdVec4f v;
    v.Load<4>(vtxPos.GetAsPositionVec4().GetData());
    vertices.PushBack(v.m_v);
  };

  for (const auto& poly : geo.GetPolygons())
  {
    const nsUInt32 uiNumVertices = poly.m_Vertices.GetCount();
    nsUInt32 uiQuadVtx = 0;

    // ignore complex polygons entirely
    if (uiNumVertices > 4)
      continue;

    for (nsUInt32 i = 0; i < uiNumVertices; ++i)
    {
      if (uiQuadVtx == 4)
      {
        // TODO: restart next quad (also flip this one's front face)
        break;
      }

      const nsUInt32 vtxIdx = poly.m_Vertices[i];

      addVtx(geo.GetVertices()[vtxIdx].m_vPosition);

      bounds.include(vertices.PeekBack());
      ++uiQuadVtx;
    }

    // if the polygon is a triangle, duplicate the last vertex to make it a degenerate quad
    if (uiQuadVtx == 3)
    {
      vertices.PushBack(vertices.PeekBack());
      ++uiQuadVtx;
    }

    if (uiQuadVtx == 4)
    {
      const nsUInt32 n = vertices.GetCount();

      // swap two vertices in the quad to flip the front face (different convention between NS and the rasterizer)
      nsMath::Swap(vertices[n - 1], vertices[n - 3]);
    }

    NS_ASSERT_DEV(uiQuadVtx == 4, "Degenerate polygon encountered");
  }

  // pad vertices to 32 for proper alignment during baking
  while (vertices.GetCount() % 32 != 0)
  {
    vertices.PushBack(vertices[0]);
  }

  m_Occluder.bake(vertices.GetData(), vertices.GetCount(), bounds.m_min, bounds.m_max);
}

nsSharedPtr<const nsRasterizerObject> nsRasterizerObject::GetObject(nsStringView sUniqueName)
{
  NS_LOCK(s_Mutex);

  auto it = s_Objects.Find(sUniqueName);

  if (it.IsValid())
    return it.Value();

  return nullptr;
}

nsSharedPtr<const nsRasterizerObject> nsRasterizerObject::CreateBox(const nsVec3& vFullExtents)
{
  NS_LOCK(s_Mutex);

  nsStringBuilder sName;
  sName.SetFormat("Box-{}-{}-{}", vFullExtents.x, vFullExtents.y, vFullExtents.z);

  nsSharedPtr<nsRasterizerObject>& pObj = s_Objects[sName];

  if (pObj == nullptr)
  {
    pObj = NS_NEW(nsFoundation::GetAlignedAllocator(), nsRasterizerObject);

    nsGeometry geometry;
    geometry.AddBox(vFullExtents, false, {});

    pObj->CreateMesh(geometry);
  }

  return pObj;
}

nsSharedPtr<const nsRasterizerObject> nsRasterizerObject::CreateMesh(nsStringView sUniqueName, const nsGeometry& geometry)
{
  NS_LOCK(s_Mutex);

  nsSharedPtr<nsRasterizerObject>& pObj = s_Objects[sUniqueName];

  if (pObj == nullptr)
  {
    pObj = NS_NEW(nsFoundation::GetAlignedAllocator(), nsRasterizerObject);

    pObj->CreateMesh(geometry);
  }

  return pObj;
}

#else

void nsRasterizerObject::CreateMesh(const nsGeometry& geo)
{
}

nsSharedPtr<const nsRasterizerObject> nsRasterizerObject::GetObject(nsStringView sUniqueName)
{
  return nullptr;
}

nsSharedPtr<const nsRasterizerObject> nsRasterizerObject::CreateBox(const nsVec3& vFullExtents)
{
  return nullptr;
}

nsSharedPtr<const nsRasterizerObject> nsRasterizerObject::CreateMesh(nsStringView sUniqueName, const nsGeometry& geometry)
{
  return nullptr;
}

#endif
