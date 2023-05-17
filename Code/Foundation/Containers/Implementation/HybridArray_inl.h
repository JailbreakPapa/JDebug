#include <Foundation/Containers/HybridArray.h>

template <typename T, wdUInt32 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
wdHybridArray<T, Size, AllocatorWrapper>::wdHybridArray()
  : wdDynamicArray<T, AllocatorWrapper>(GetStaticArray(), Size, AllocatorWrapper::GetAllocator())
{
}

template <typename T, wdUInt32 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
wdHybridArray<T, Size, AllocatorWrapper>::wdHybridArray(wdAllocatorBase* pAllocator)
  : wdDynamicArray<T, AllocatorWrapper>(GetStaticArray(), Size, pAllocator)
{
}

template <typename T, wdUInt32 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
wdHybridArray<T, Size, AllocatorWrapper>::wdHybridArray(const wdHybridArray<T, Size, AllocatorWrapper>& other)
  : wdDynamicArray<T, AllocatorWrapper>(GetStaticArray(), Size, AllocatorWrapper::GetAllocator())
{
  *this = other;
}

template <typename T, wdUInt32 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
wdHybridArray<T, Size, AllocatorWrapper>::wdHybridArray(const wdArrayPtr<const T>& other)
  : wdDynamicArray<T, AllocatorWrapper>(GetStaticArray(), Size, AllocatorWrapper::GetAllocator())
{
  *this = other;
}

template <typename T, wdUInt32 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
wdHybridArray<T, Size, AllocatorWrapper>::wdHybridArray(wdHybridArray<T, Size, AllocatorWrapper>&& other)
  : wdDynamicArray<T, AllocatorWrapper>(GetStaticArray(), Size, other.GetAllocator())
{
  *this = std::move(other);
}

template <typename T, wdUInt32 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
void wdHybridArray<T, Size, AllocatorWrapper>::operator=(const wdHybridArray<T, Size, AllocatorWrapper>& rhs)
{
  wdDynamicArray<T, AllocatorWrapper>::operator=(rhs);
}

template <typename T, wdUInt32 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
void wdHybridArray<T, Size, AllocatorWrapper>::operator=(const wdArrayPtr<const T>& rhs)
{
  wdDynamicArray<T, AllocatorWrapper>::operator=(rhs);
}

template <typename T, wdUInt32 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
void wdHybridArray<T, Size, AllocatorWrapper>::operator=(wdHybridArray<T, Size, AllocatorWrapper>&& rhs) noexcept
{
  wdDynamicArray<T, AllocatorWrapper>::operator=(std::move(rhs));
}
