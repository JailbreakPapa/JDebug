#include <Foundation/FoundationPCH.h>

#include <Foundation/Basics.h>
#include <Foundation/Memory/MemoryUtils.h>

#include <Foundation/DataProcessing/Stream/ProcessingStream.h>
#include <Foundation/DataProcessing/Stream/ProcessingStreamGroup.h>

#include <Foundation/DataProcessing/Stream/DefaultImplementations/ZeroInitializer.h>

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdProcessingStreamSpawnerZeroInitialized, 1, wdRTTIDefaultAllocator<wdProcessingStreamSpawnerZeroInitialized>)
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdProcessingStreamSpawnerZeroInitialized::wdProcessingStreamSpawnerZeroInitialized()
  : m_pStream(nullptr)
{
}

void wdProcessingStreamSpawnerZeroInitialized::SetStreamName(wdStringView sStreamName)
{
  m_sStreamName.Assign(sStreamName);
}

wdResult wdProcessingStreamSpawnerZeroInitialized::UpdateStreamBindings()
{
  WD_ASSERT_DEBUG(!m_sStreamName.IsEmpty(), "wdProcessingStreamSpawnerZeroInitialized: Stream name has not been configured");

  m_pStream = m_pStreamGroup->GetStreamByName(m_sStreamName);
  return m_pStream ? WD_SUCCESS : WD_FAILURE;
}


void wdProcessingStreamSpawnerZeroInitialized::InitializeElements(wdUInt64 uiStartIndex, wdUInt64 uiNumElements)
{
  const wdUInt64 uiElementSize = m_pStream->GetElementSize();
  const wdUInt64 uiElementStride = m_pStream->GetElementStride();

  for (wdUInt64 i = uiStartIndex; i < uiStartIndex + uiNumElements; ++i)
  {
    wdMemoryUtils::ZeroFill<wdUInt8>(
      static_cast<wdUInt8*>(wdMemoryUtils::AddByteOffset(m_pStream->GetWritableData(), static_cast<ptrdiff_t>(i * uiElementStride))),
      static_cast<size_t>(uiElementSize));
  }
}



WD_STATICLINK_FILE(Foundation, Foundation_DataProcessing_Stream_DefaultImplementations_Implementation_ZeroInitializer);
