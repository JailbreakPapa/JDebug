#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Utilities/AssetFileHeader.h>
#include <RendererCore/AnimationSystem/Implementation/OzzUtils.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/io/archive.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSkeletonResource, 1, nsRTTIDefaultAllocator<nsSkeletonResource>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsSkeletonResource);
// clang-format on

nsSkeletonResource::nsSkeletonResource()
  : nsResource(DoUpdate::OnAnyThread, 1)
{
}

nsSkeletonResource::~nsSkeletonResource() = default;

NS_RESOURCE_IMPLEMENT_CREATEABLE(nsSkeletonResource, nsSkeletonResourceDescriptor)
{
  m_pDescriptor = NS_DEFAULT_NEW(nsSkeletonResourceDescriptor);
  *m_pDescriptor = std::move(descriptor);

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Loaded;

  return res;
}

nsResourceLoadDesc nsSkeletonResource::UnloadData(Unload WhatToUnload)
{
  m_pDescriptor.Clear();

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Unloaded;

  return res;
}

nsResourceLoadDesc nsSkeletonResource::UpdateContent(nsStreamReader* Stream)
{
  NS_LOG_BLOCK("nsSkeletonResource::UpdateContent", GetResourceIdOrDescription());

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;

  if (Stream == nullptr)
  {
    res.m_State = nsResourceState::LoadedResourceMissing;
    return res;
  }

  // skip the absolute file path data that the standard file reader writes into the stream
  {
    nsStringBuilder sAbsFilePath;
    (*Stream) >> sAbsFilePath;
  }

  // skip the asset file header at the start of the file
  nsAssetFileHeader AssetHash;
  AssetHash.Read(*Stream).IgnoreResult();

  m_pDescriptor = NS_DEFAULT_NEW(nsSkeletonResourceDescriptor);
  m_pDescriptor->Deserialize(*Stream).IgnoreResult();

  res.m_State = nsResourceState::Loaded;
  return res;
}

void nsSkeletonResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryGPU = 0;
  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(nsSkeletonResource); // TODO
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

nsSkeletonResourceDescriptor::nsSkeletonResourceDescriptor() = default;
nsSkeletonResourceDescriptor::~nsSkeletonResourceDescriptor() = default;
nsSkeletonResourceDescriptor::nsSkeletonResourceDescriptor(nsSkeletonResourceDescriptor&& rhs)
{
  *this = std::move(rhs);
}

void nsSkeletonResourceDescriptor::operator=(nsSkeletonResourceDescriptor&& rhs)
{
  m_Skeleton = std::move(rhs.m_Skeleton);
  m_Geometry = std::move(rhs.m_Geometry);
}

nsUInt64 nsSkeletonResourceDescriptor::GetHeapMemoryUsage() const
{
  return m_Geometry.GetHeapMemoryUsage() + m_Skeleton.GetHeapMemoryUsage();
}

nsResult nsSkeletonResourceDescriptor::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream.WriteVersion(7);

  m_Skeleton.Save(inout_stream);
  inout_stream << m_RootTransform;
  inout_stream << m_fMaxImpulse;

  const nsUInt16 uiNumGeom = static_cast<nsUInt16>(m_Geometry.GetCount());
  inout_stream << uiNumGeom;

  for (nsUInt32 i = 0; i < uiNumGeom; ++i)
  {
    const auto& geo = m_Geometry[i];

    inout_stream << geo.m_uiAttachedToJoint;
    inout_stream << geo.m_Type;
    inout_stream << geo.m_Transform;

    NS_SUCCEED_OR_RETURN(inout_stream.WriteArray(geo.m_VertexPositions));
    NS_SUCCEED_OR_RETURN(inout_stream.WriteArray(geo.m_TriangleIndices));
  }

  return NS_SUCCESS;
}

nsResult nsSkeletonResourceDescriptor::Deserialize(nsStreamReader& inout_stream)
{
  const nsTypeVersion version = inout_stream.ReadVersion(7);

  if (version < 6)
    return NS_FAILURE;

  m_Skeleton.Load(inout_stream);

  inout_stream >> m_RootTransform;

  if (version >= 7)
  {
    inout_stream >> m_fMaxImpulse;
  }

  m_Geometry.Clear();

  nsUInt16 uiNumGeom = 0;
  inout_stream >> uiNumGeom;
  m_Geometry.Reserve(uiNumGeom);

  for (nsUInt32 i = 0; i < uiNumGeom; ++i)
  {
    auto& geo = m_Geometry.ExpandAndGetRef();

    inout_stream >> geo.m_uiAttachedToJoint;
    inout_stream >> geo.m_Type;
    inout_stream >> geo.m_Transform;

    if (version <= 6)
    {
      nsStringBuilder sName;
      nsSurfaceResourceHandle hSurface;
      nsUInt8 uiCollisionLayer;

      inout_stream >> sName;
      inout_stream >> hSurface;
      inout_stream >> uiCollisionLayer;
    }

    if (version >= 7)
    {
      NS_SUCCEED_OR_RETURN(inout_stream.ReadArray(geo.m_VertexPositions));
      NS_SUCCEED_OR_RETURN(inout_stream.ReadArray(geo.m_TriangleIndices));
    }
  }

  // make sure the geometry is sorted by bones
  // this allows to make the algorithm for creating the bone geometry more efficient
  m_Geometry.Sort([](const nsSkeletonResourceGeometry& lhs, const nsSkeletonResourceGeometry& rhs) -> bool
    { return lhs.m_uiAttachedToJoint < rhs.m_uiAttachedToJoint; });

  return NS_SUCCESS;
}


NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_Implementation_SkeletonResource);
