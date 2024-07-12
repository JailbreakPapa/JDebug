#include <Foundation/FoundationPCH.h>

#include <Foundation/Basics.h>
#include <Foundation/Memory/MemoryUtils.h>

#include <Foundation/DataProcessing/Stream/ProcessingStream.h>
#include <Foundation/DataProcessing/Stream/ProcessingStreamGroup.h>

#include <Foundation/DataProcessing/Stream/DefaultImplementations/ZeroInitializer.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsProcessingStreamSpawnerZeroInitialized, 1, nsRTTIDefaultAllocator<nsProcessingStreamSpawnerZeroInitialized>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsProcessingStreamSpawnerZeroInitialized::nsProcessingStreamSpawnerZeroInitialized()

  = default;

void nsProcessingStreamSpawnerZeroInitialized::SetStreamName(nsStringView sStreamName)
{
  m_sStreamName.Assign(sStreamName);
}

nsResult nsProcessingStreamSpawnerZeroInitialized::UpdateStreamBindings()
{
  NS_ASSERT_DEBUG(!m_sStreamName.IsEmpty(), "nsProcessingStreamSpawnerZeroInitialized: Stream name has not been configured");

  m_pStream = m_pStreamGroup->GetStreamByName(m_sStreamName);
  return m_pStream ? NS_SUCCESS : NS_FAILURE;
}


void nsProcessingStreamSpawnerZeroInitialized::InitializeElements(nsUInt64 uiStartIndex, nsUInt64 uiNumElements)
{
  const nsUInt64 uiElementSize = m_pStream->GetElementSize();
  const nsUInt64 uiElementStride = m_pStream->GetElementStride();

  for (nsUInt64 i = uiStartIndex; i < uiStartIndex + uiNumElements; ++i)
  {
    nsMemoryUtils::ZeroFill<nsUInt8>(
      static_cast<nsUInt8*>(nsMemoryUtils::AddByteOffset(m_pStream->GetWritableData(), static_cast<std::ptrdiff_t>(i * uiElementStride))),
      static_cast<size_t>(uiElementSize));
  }
}



NS_STATICLINK_FILE(Foundation, Foundation_DataProcessing_Stream_DefaultImplementations_Implementation_ZeroInitializer);
