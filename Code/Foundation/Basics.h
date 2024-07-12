#pragma once

#include <Foundation/Basics/PreprocessorUtils.h>

#include <Foundation/Basics/AllDefinesOff.h>

#include <Foundation/Basics/Platform/DetectArchitecture.h>
#include <Foundation/Basics/Platform/DetectPlatform.h>

#include <Foundation/UserConfig.h>

// Configure the DLL Import/Export Define
#if NS_ENABLED(NS_COMPILE_ENGINE_AS_DLL)
#  ifdef BUILDSYSTEM_BUILDING_FOUNDATION_LIB
#    define NS_FOUNDATION_DLL NS_DECL_EXPORT
#    define NS_FOUNDATION_DLL_FRIEND NS_DECL_EXPORT_FRIEND
#  else
#    define NS_FOUNDATION_DLL NS_DECL_IMPORT
#    define NS_FOUNDATION_DLL_FRIEND NS_DECL_IMPORT_FRIEND
#  endif
#else
#  define NS_FOUNDATION_DLL
#  define NS_FOUNDATION_DLL_FRIEND
#endif

#include <Foundation/FoundationInternal.h>

// include the different headers for the supported platforms
#if NS_ENABLED(NS_PLATFORM_WINDOWS)
#  include <Foundation/Basics/Platform/Win/Platform_win.h>
#elif NS_ENABLED(NS_PLATFORM_OSX)
#  include <Foundation/Basics/Platform/OSX/Platform_OSX.h>
#elif NS_ENABLED(NS_PLATFORM_LINUX) || NS_ENABLED(NS_PLATFORM_ANDROID)
#  include <Foundation/Basics/Platform/Linux/Platform_Linux.h>
#elif NS_ENABLED(NS_PLATFORM_PLAYSTATION_5) && NS_ENABLED(NS_PLATFORM_CONSOLE)
#  include <Foundation/Basics/Platform/Prospero/Platform_Prospero.h>
#elif NS_ENABLED(NS_PLATFORM_XBOXSERIESCONSOLE) && NS_ENABLED(NS_PLATFORM_CONSOLE)
#  include <Foundation/Basics/Platform/Scarlett/Platform_Scarlett.h>
#elif NS_ENABLED(NS_PLATFORM_NINTENDO) && NS_ENABLED(NS_PLATFORM_CONSOLE)
#  include <Foundation/Basics/Platform/Nintendo/Platform_Nintendo.h>
#else
#  error "Undefined platform!"
#endif

// Here all the different features that each platform supports are declared.
#include <Foundation/Basics/Platform/PlatformFeatures.h>

// Include this last, it will ensure the previous includes have setup everything correctly
#include <Foundation/Basics/Platform/CheckDefinitions.h>

// Include common definitions and macros (e.g. NS_CHECK_AT_COMPILETIME)
#include <Foundation/Basics/Platform/Common.h>

// Include magic preprocessor macros
#include <Foundation/Basics/Platform/BlackMagic.h>

// Now declare all fundamental types
#include <Foundation/Types/Types.h>

#ifdef BUILDSYSTEM_BUILDING_FOUNDATION_LIB
#  if BUILDSYSTEM_COMPILE_ENGINE_AS_DLL && NS_DISABLED(NS_COMPILE_ENGINE_AS_DLL)
#    error "The Buildsystem is configured to build the Engine as a shared library, but NS_COMPILE_ENGINE_AS_DLL is not defined in UserConfig.h"
#  endif
#  if !BUILDSYSTEM_COMPILE_ENGINE_AS_DLL && NS_ENABLED(NS_COMPILE_ENGINE_AS_DLL)
#    error "The Buildsystem is configured to build the Engine as a static library, but NS_COMPILE_ENGINE_AS_DLL is defined in UserConfig.h"
#  endif
#endif

// Finally include the rest of basics
#include <Foundation/Basics/Assert.h>

#include <Foundation/Types/TypeTraits.h>

#include <Foundation/Memory/Allocator.h>

#include <Foundation/Configuration/StaticSubSystem.h>
#include <Foundation/Strings/FormatString.h>

class NS_FOUNDATION_DLL nsFoundation
{
public:
  static nsAllocator* s_pDefaultAllocator;
  static nsAllocator* s_pAlignedAllocator;

  /// \brief The default allocator can be used for any kind of allocation if no alignment is required
  NS_ALWAYS_INLINE static nsAllocator* GetDefaultAllocator()
  {
    if (s_bIsInitialized)
      return s_pDefaultAllocator;
    else // the default allocator is not yet set so we return the static allocator instead.
      return GetStaticsAllocator();
  }

  /// \brief The aligned allocator should be used for all allocations which need alignment
  NS_ALWAYS_INLINE static nsAllocator* GetAlignedAllocator()
  {
    NS_ASSERT_RELEASE(s_pAlignedAllocator != nullptr, "nsFoundation must have been initialized before this function can be called. This "
                                                      "error can occur when you have a global variable or a static member variable that "
                                                      "(indirectly) requires an allocator. Check out the documentation for 'nsStatic' for "
                                                      "more information about this issue.");
    return s_pAlignedAllocator;
  }

  /// \brief Returns the allocator that is used by global data and static members before the default allocator is created.
  static nsAllocator* GetStaticsAllocator();

private:
  friend class nsStartup;
  friend struct nsStaticsAllocatorWrapper;

  static void Initialize();
  static bool s_bIsInitialized;
};
