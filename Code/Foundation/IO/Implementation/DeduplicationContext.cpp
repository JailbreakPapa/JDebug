#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/DeduplicationReadContext.h>
#include <Foundation/IO/DeduplicationWriteContext.h>

NS_IMPLEMENT_SERIALIZATION_CONTEXT(nsDeduplicationReadContext);

nsDeduplicationReadContext::nsDeduplicationReadContext() = default;
nsDeduplicationReadContext::~nsDeduplicationReadContext() = default;

//////////////////////////////////////////////////////////////////////////

NS_IMPLEMENT_SERIALIZATION_CONTEXT(nsDeduplicationWriteContext);

nsDeduplicationWriteContext::nsDeduplicationWriteContext() = default;
nsDeduplicationWriteContext::~nsDeduplicationWriteContext() = default;
