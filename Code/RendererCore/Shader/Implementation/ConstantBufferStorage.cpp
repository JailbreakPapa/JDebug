#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererFoundation/CommandEncoder/CommandEncoder.h>
#include <RendererFoundation/Device/Device.h>

nsConstantBufferStorageBase::nsConstantBufferStorageBase(nsUInt32 uiSizeInBytes)

{
  m_Data = nsMakeArrayPtr(static_cast<nsUInt8*>(nsFoundation::GetAlignedAllocator()->Allocate(uiSizeInBytes, 16)), uiSizeInBytes);
  nsMemoryUtils::ZeroFill(m_Data.GetPtr(), m_Data.GetCount());

  m_hGALConstantBuffer = nsGALDevice::GetDefaultDevice()->CreateConstantBuffer(uiSizeInBytes);
}

nsConstantBufferStorageBase::~nsConstantBufferStorageBase()
{
  nsGALDevice::GetDefaultDevice()->DestroyBuffer(m_hGALConstantBuffer);

  nsFoundation::GetAlignedAllocator()->Deallocate(m_Data.GetPtr());
  m_Data.Clear();
}

nsArrayPtr<nsUInt8> nsConstantBufferStorageBase::GetRawDataForWriting()
{
  m_bHasBeenModified = true;
  return m_Data;
}

nsArrayPtr<const nsUInt8> nsConstantBufferStorageBase::GetRawDataForReading() const
{
  return m_Data;
}

void nsConstantBufferStorageBase::UploadData(nsGALCommandEncoder* pCommandEncoder)
{
  if (!m_bHasBeenModified)
    return;

  m_bHasBeenModified = false;

  nsUInt32 uiNewHash = nsHashingUtils::xxHash32(m_Data.GetPtr(), m_Data.GetCount());
  if (m_uiLastHash != uiNewHash)
  {
    pCommandEncoder->UpdateBuffer(m_hGALConstantBuffer, 0, m_Data);
    m_uiLastHash = uiNewHash;
  }
}
