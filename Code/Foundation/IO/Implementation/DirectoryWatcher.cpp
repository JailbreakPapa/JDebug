#include <Foundation/FoundationPCH.h>


#if WD_ENABLED(WD_SUPPORTS_DIRECTORY_WATCHER)
#  if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)
#    include <Foundation/IO/Implementation/Win/DirectoryWatcher_win.h>
#  elif WD_ENABLED(WD_USE_POSIX_FILE_API)
#    include <Foundation/IO/Implementation/Posix/DirectoryWatcher_posix.h>
#  else
#    error "Unknown Platform."
#  endif
#endif


WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_DirectoryWatcher);
