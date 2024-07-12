#include <Foundation/FoundationPCH.h>

#if (NS_ENABLED(NS_PLATFORM_ANDROID) && NS_ENABLED(NS_SUPPORTS_DIRECTORY_WATCHER))
#  if NS_ENABLED(NS_USE_POSIX_FILE_API)
#    include <Foundation/Platform/Posix/DirectoryWatcher_Posix.h>
#  else
#    error "DirectoryWatcher not implemented."
#  endif
#endif
