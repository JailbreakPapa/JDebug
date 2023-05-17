#include <Foundation/FoundationPCH.h>

#include <Foundation/Basics.h>
#include <Foundation/DataProcessing/Stream/ProcessingStreamGroup.h>
#include <Foundation/DataProcessing/Stream/ProcessingStreamProcessor.h>

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdProcessingStreamProcessor, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdProcessingStreamProcessor::wdProcessingStreamProcessor()
  : m_pStreamGroup(nullptr)
{
}

wdProcessingStreamProcessor::~wdProcessingStreamProcessor()
{
  m_pStreamGroup = nullptr;
}



WD_STATICLINK_FILE(Foundation, Foundation_DataProcessing_Stream_Implementation_ProcessingStreamProcessor);
