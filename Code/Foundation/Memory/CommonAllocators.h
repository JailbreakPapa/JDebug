
#pragma once

/// \file

#include <Foundation/Memory/AllocatorWithPolicy.h>

#include <Foundation/Memory/Policies/AllocPolicyAlignedHeap.h>
#include <Foundation/Memory/Policies/AllocPolicyGuarding.h>
#include <Foundation/Memory/Policies/AllocPolicyHeap.h>
#include <Foundation/Memory/Policies/AllocPolicyProxy.h>


/// \brief Default heap allocator
using nsAlignedHeapAllocator = nsAllocatorWithPolicy<nsAllocPolicyAlignedHeap>;

/// \brief Default heap allocator
using nsHeapAllocator = nsAllocatorWithPolicy<nsAllocPolicyHeap>;

/// \brief Guarded allocator
using nsGuardingAllocator = nsAllocatorWithPolicy<nsAllocPolicyGuarding>;

/// \brief Proxy allocator
using nsProxyAllocator = nsAllocatorWithPolicy<nsAllocPolicyProxy>;
