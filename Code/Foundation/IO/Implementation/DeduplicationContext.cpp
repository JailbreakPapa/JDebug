#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/DeduplicationReadContext.h>
#include <Foundation/IO/DeduplicationWriteContext.h>

WD_IMPLEMENT_SERIALIZATION_CONTEXT(wdDeduplicationReadContext);

wdDeduplicationReadContext::wdDeduplicationReadContext() = default;
wdDeduplicationReadContext::~wdDeduplicationReadContext() = default;

//////////////////////////////////////////////////////////////////////////

WD_IMPLEMENT_SERIALIZATION_CONTEXT(wdDeduplicationWriteContext);

wdDeduplicationWriteContext::wdDeduplicationWriteContext() = default;
wdDeduplicationWriteContext::~wdDeduplicationWriteContext() = default;



WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_DeduplicationContext);
