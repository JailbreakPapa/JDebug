#include <Foundation/FoundationPCH.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)
#  include <Foundation/IO/Implementation/Win/MemoryMappedFile_win.h>
#elif WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
#  include <Foundation/IO/Implementation/Win/MemoryMappedFile_uwp.h>
#elif WD_ENABLED(WD_USE_POSIX_FILE_API)
#  include <Foundation/IO/Implementation/Posix/MemoryMappedFile_posix.h>
#else
#  error "Unknown Platform."
#endif


WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_MemoryMappedFile);
