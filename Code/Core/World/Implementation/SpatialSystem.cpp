#include <Core/CorePCH.h>

#include <Core/World/SpatialSystem.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSpatialSystem, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsSpatialSystem::nsSpatialSystem()
  : m_Allocator("Spatial System", nsFoundation::GetDefaultAllocator())
{
}

nsSpatialSystem::~nsSpatialSystem() = default;

void nsSpatialSystem::StartNewFrame()
{
  ++m_uiFrameCounter;
}

void nsSpatialSystem::FindObjectsInSphere(const nsBoundingSphere& sphere, const QueryParams& queryParams, nsDynamicArray<nsGameObject*>& out_objects) const
{
  out_objects.Clear();

  FindObjectsInSphere(
    sphere, queryParams,
    [&](nsGameObject* pObject)
    {
      out_objects.PushBack(pObject);

      return nsVisitorExecution::Continue;
    });
}

void nsSpatialSystem::FindObjectsInBox(const nsBoundingBox& box, const QueryParams& queryParams, nsDynamicArray<nsGameObject*>& out_objects) const
{
  out_objects.Clear();

  FindObjectsInBox(
    box, queryParams,
    [&](nsGameObject* pObject)
    {
      out_objects.PushBack(pObject);

      return nsVisitorExecution::Continue;
    });
}

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
void nsSpatialSystem::GetInternalStats(nsStringBuilder& ref_sSb) const
{
  ref_sSb.Clear();
}
#endif

NS_STATICLINK_FILE(Core, Core_World_Implementation_SpatialSystem);
