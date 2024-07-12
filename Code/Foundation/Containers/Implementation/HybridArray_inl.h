#include <Foundation/Containers/HybridArray.h>

template <typename T, nsUInt32 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
nsHybridArray<T, Size, AllocatorWrapper>::nsHybridArray()
  : nsDynamicArray<T, AllocatorWrapper>(GetStaticArray(), Size, AllocatorWrapper::GetAllocator())
{
}

template <typename T, nsUInt32 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
nsHybridArray<T, Size, AllocatorWrapper>::nsHybridArray(nsAllocator* pAllocator)
  : nsDynamicArray<T, AllocatorWrapper>(GetStaticArray(), Size, pAllocator)
{
}

template <typename T, nsUInt32 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
nsHybridArray<T, Size, AllocatorWrapper>::nsHybridArray(const nsHybridArray<T, Size, AllocatorWrapper>& other)
  : nsDynamicArray<T, AllocatorWrapper>(GetStaticArray(), Size, AllocatorWrapper::GetAllocator())
{
  *this = other;
}

template <typename T, nsUInt32 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
nsHybridArray<T, Size, AllocatorWrapper>::nsHybridArray(const nsArrayPtr<const T>& other)
  : nsDynamicArray<T, AllocatorWrapper>(GetStaticArray(), Size, AllocatorWrapper::GetAllocator())
{
  *this = other;
}

template <typename T, nsUInt32 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
nsHybridArray<T, Size, AllocatorWrapper>::nsHybridArray(nsHybridArray<T, Size, AllocatorWrapper>&& other) noexcept
  : nsDynamicArray<T, AllocatorWrapper>(GetStaticArray(), Size, other.GetAllocator())
{
  *this = std::move(other);
}

template <typename T, nsUInt32 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
void nsHybridArray<T, Size, AllocatorWrapper>::operator=(const nsHybridArray<T, Size, AllocatorWrapper>& rhs)
{
  nsDynamicArray<T, AllocatorWrapper>::operator=(rhs);
}

template <typename T, nsUInt32 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
void nsHybridArray<T, Size, AllocatorWrapper>::operator=(const nsArrayPtr<const T>& rhs)
{
  nsDynamicArray<T, AllocatorWrapper>::operator=(rhs);
}

template <typename T, nsUInt32 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
void nsHybridArray<T, Size, AllocatorWrapper>::operator=(nsHybridArray<T, Size, AllocatorWrapper>&& rhs) noexcept
{
  nsDynamicArray<T, AllocatorWrapper>::operator=(std::move(rhs));
}
