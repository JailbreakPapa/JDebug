#pragma once

#include <Foundation/Profiling/Profiling.h>
#include <RendererFoundation/RendererFoundationDLL.h>

struct GPUTimingScope;

/// Sets profiling marker and GPU timings for the current scope.
class NS_RENDERERFOUNDATION_DLL nsProfilingScopeAndMarker : public nsProfilingScope
{
public:
  static GPUTimingScope* Start(nsGALCommandEncoder* pCommandEncoder, const char* szName);
  static void Stop(nsGALCommandEncoder* pCommandEncoder, GPUTimingScope*& ref_pTimingScope);

  nsProfilingScopeAndMarker(nsGALCommandEncoder* pCommandEncoder, const char* szName);

  ~nsProfilingScopeAndMarker();

protected:
  nsGALCommandEncoder* m_pCommandEncoder;
  GPUTimingScope* m_pTimingScope;
};

#if NS_ENABLED(NS_USE_PROFILING) || defined(NS_DOCS)

/// \brief Profiles the current scope using the given name and also inserts a marker with the given GALContext.
#  define NS_PROFILE_AND_MARKER(GALContext, szName) nsProfilingScopeAndMarker NS_CONCAT(_nsProfilingScope, NS_SOURCE_LINE)(GALContext, szName)

#else

#  define NS_PROFILE_AND_MARKER(GALContext, szName) /*empty*/

#endif
