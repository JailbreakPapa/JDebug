#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_OSX)
#  if NS_ENABLED(NS_USE_POSIX_FILE_API)
#    include <Foundation/Platform/Posix/MemoryMappedFile_Posix.h>
#  else
#    include <Foundation/Platform/NoImpl/MemoryMappedFile_NoImpl.h>
#  endif
#endif
