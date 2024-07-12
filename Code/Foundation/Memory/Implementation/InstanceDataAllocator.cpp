#include <Foundation/FoundationPCH.h>

#include <Foundation/Memory/InstanceDataAllocator.h>

nsUInt32 nsInstanceDataAllocator::AddDesc(const nsInstanceDataDesc& desc)
{
  m_Descs.PushBack(desc);

  const nsUInt32 uiOffset = nsMemoryUtils::AlignSize(m_uiTotalDataSize, desc.m_uiTypeAlignment);
  m_uiTotalDataSize = uiOffset + desc.m_uiTypeSize;

  return uiOffset;
}

void nsInstanceDataAllocator::ClearDescs()
{
  m_Descs.Clear();
  m_uiTotalDataSize = 0;
}

nsBlob nsInstanceDataAllocator::AllocateAndConstruct() const
{
  nsBlob blob;
  if (m_uiTotalDataSize > 0)
  {
    blob.SetCountUninitialized(m_uiTotalDataSize);
    blob.ZeroFill();

    Construct(blob.GetByteBlobPtr());
  }

  return blob;
}

void nsInstanceDataAllocator::DestructAndDeallocate(nsBlob& ref_blob) const
{
  NS_ASSERT_DEV(ref_blob.GetByteBlobPtr().GetCount() == m_uiTotalDataSize, "Passed blob has not the expected size");
  Destruct(ref_blob.GetByteBlobPtr());

  ref_blob.Clear();
}

void nsInstanceDataAllocator::Construct(nsByteBlobPtr blobPtr) const
{
  nsUInt32 uiOffset = 0;
  for (auto& desc : m_Descs)
  {
    uiOffset = nsMemoryUtils::AlignSize(uiOffset, desc.m_uiTypeAlignment);

    if (desc.m_ConstructorFunction != nullptr)
    {
      desc.m_ConstructorFunction(GetInstanceData(blobPtr, uiOffset));
    }

    uiOffset += desc.m_uiTypeSize;
  }
}

void nsInstanceDataAllocator::Destruct(nsByteBlobPtr blobPtr) const
{
  nsUInt32 uiOffset = 0;
  for (auto& desc : m_Descs)
  {
    uiOffset = nsMemoryUtils::AlignSize(uiOffset, desc.m_uiTypeAlignment);

    if (desc.m_DestructorFunction != nullptr)
    {
      desc.m_DestructorFunction(GetInstanceData(blobPtr, uiOffset));
    }

    uiOffset += desc.m_uiTypeSize;
  }
}
