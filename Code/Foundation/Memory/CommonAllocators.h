
#pragma once

/// \file

#include <Foundation/Memory/Allocator.h>

#include <Foundation/Memory/Policies/AlignedHeapAllocation.h>
#include <Foundation/Memory/Policies/GuardedAllocation.h>
#include <Foundation/Memory/Policies/HeapAllocation.h>
#include <Foundation/Memory/Policies/ProxyAllocation.h>


/// \brief Default heap allocator
typedef wdAllocator<wdMemoryPolicies::wdAlignedHeapAllocation> wdAlignedHeapAllocator;

/// \brief Default heap allocator
typedef wdAllocator<wdMemoryPolicies::wdHeapAllocation> wdHeapAllocator;

/// \brief Guarded allocator
typedef wdAllocator<wdMemoryPolicies::wdGuardedAllocation> wdGuardedAllocator;

/// \brief Proxy allocator
typedef wdAllocator<wdMemoryPolicies::wdProxyAllocation> wdProxyAllocator;
