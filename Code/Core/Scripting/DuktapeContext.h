#pragma once

#include <Core/Scripting/DuktapeFunction.h>
#include <Foundation/Memory/CommonAllocators.h>

#ifdef BUILDSYSTEM_ENABLE_DUKTAPE_SUPPORT

struct duk_hthread;
using duk_context = duk_hthread;
using duk_c_function = int (*)(duk_context*);


class NS_CORE_DLL nsDuktapeContext : public nsDuktapeHelper
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsDuktapeContext);

public:
  nsDuktapeContext(nsStringView sWrapperName);
  ~nsDuktapeContext();

  /// \name Basics
  ///@{

  /// \brief Enables support for loading modules via the 'require' function
  void EnableModuleSupport(duk_c_function moduleSearchFunction);

  ///@}

private:
  void InitializeContext();
  void DestroyContext();

  static void FatalErrorHandler(void* pUserData, const char* szMsg);
  static void* DukAlloc(void* pUserData, size_t size);
  static void* DukRealloc(void* pUserData, void* pPointer, size_t size);
  static void DukFree(void* pUserData, void* pPointer);

protected:
  bool m_bInitializedModuleSupport = false;

private:
#  if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  nsAllocatorWithPolicy<nsAllocPolicyHeap, nsAllocatorTrackingMode::AllocationStats> m_Allocator;
#  else
  nsAllocatorWithPolicy<nsAllocPolicyHeap, nsAllocatorTrackingMode::Nothing> m_Allocator;
#  endif
};

#endif // BUILDSYSTEM_ENABLE_DUKTAPE_SUPPORT
