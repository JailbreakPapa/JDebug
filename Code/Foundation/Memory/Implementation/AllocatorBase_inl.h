
WD_ALWAYS_INLINE wdAllocatorBase::wdAllocatorBase() = default;

WD_ALWAYS_INLINE wdAllocatorBase::~wdAllocatorBase() = default;


namespace wdMath
{
  // due to #include order issues, we have to forward declare this function here

  WD_FOUNDATION_DLL wdUInt64 SafeMultiply64(wdUInt64 a, wdUInt64 b, wdUInt64 c, wdUInt64 d);
} // namespace wdMath

namespace wdInternal
{
  template <typename T>
  struct NewInstance
  {
    WD_ALWAYS_INLINE NewInstance(T* pInstance, wdAllocatorBase* pAllocator)
    {
      m_pInstance = pInstance;
      m_pAllocator = pAllocator;
    }

    template <typename U>
    WD_ALWAYS_INLINE NewInstance(NewInstance<U>&& other)
    {
      m_pInstance = other.m_pInstance;
      m_pAllocator = other.m_pAllocator;

      other.m_pInstance = nullptr;
      other.m_pAllocator = nullptr;
    }

    WD_ALWAYS_INLINE NewInstance(std::nullptr_t) {}

    template <typename U>
    WD_ALWAYS_INLINE NewInstance<U> Cast()
    {
      return NewInstance<U>(static_cast<U*>(m_pInstance), m_pAllocator);
    }

    WD_ALWAYS_INLINE operator T*() { return m_pInstance; }

    WD_ALWAYS_INLINE T* operator->() { return m_pInstance; }

    T* m_pInstance = nullptr;
    wdAllocatorBase* m_pAllocator = nullptr;
  };

  template <typename T>
  WD_ALWAYS_INLINE bool operator<(const NewInstance<T>& lhs, T* rhs)
  {
    return lhs.m_pInstance < rhs;
  }

  template <typename T>
  WD_ALWAYS_INLINE bool operator<(T* lhs, const NewInstance<T>& rhs)
  {
    return lhs < rhs.m_pInstance;
  }

  template <typename T>
  WD_FORCE_INLINE void Delete(wdAllocatorBase* pAllocator, T* pPtr)
  {
    if (pPtr != nullptr)
    {
      wdMemoryUtils::Destruct(pPtr, 1);
      pAllocator->Deallocate(pPtr);
    }
  }

  template <typename T>
  WD_FORCE_INLINE T* CreateRawBuffer(wdAllocatorBase* pAllocator, size_t uiCount)
  {
    wdUInt64 safeAllocationSize = wdMath::SafeMultiply64(uiCount, sizeof(T));
    return static_cast<T*>(pAllocator->Allocate(static_cast<size_t>(safeAllocationSize), WD_ALIGNMENT_OF(T))); // Down-cast to size_t for 32-bit
  }

  WD_FORCE_INLINE void DeleteRawBuffer(wdAllocatorBase* pAllocator, void* pPtr)
  {
    if (pPtr != nullptr)
    {
      pAllocator->Deallocate(pPtr);
    }
  }

  template <typename T>
  inline wdArrayPtr<T> CreateArray(wdAllocatorBase* pAllocator, wdUInt32 uiCount)
  {
    T* buffer = CreateRawBuffer<T>(pAllocator, uiCount);
    wdMemoryUtils::Construct(buffer, uiCount);

    return wdArrayPtr<T>(buffer, uiCount);
  }

  template <typename T>
  inline void DeleteArray(wdAllocatorBase* pAllocator, wdArrayPtr<T> arrayPtr)
  {
    T* buffer = arrayPtr.GetPtr();
    if (buffer != nullptr)
    {
      wdMemoryUtils::Destruct(buffer, arrayPtr.GetCount());
      pAllocator->Deallocate(buffer);
    }
  }

  template <typename T>
  WD_FORCE_INLINE T* ExtendRawBuffer(T* pPtr, wdAllocatorBase* pAllocator, size_t uiCurrentCount, size_t uiNewCount, wdTypeIsPod)
  {
    return (T*)pAllocator->Reallocate(pPtr, uiCurrentCount * sizeof(T), uiNewCount * sizeof(T), WD_ALIGNMENT_OF(T));
  }

  template <typename T>
  WD_FORCE_INLINE T* ExtendRawBuffer(T* pPtr, wdAllocatorBase* pAllocator, size_t uiCurrentCount, size_t uiNewCount, wdTypeIsMemRelocatable)
  {
    return (T*)pAllocator->Reallocate(pPtr, uiCurrentCount * sizeof(T), uiNewCount * sizeof(T), WD_ALIGNMENT_OF(T));
  }

  template <typename T>
  WD_FORCE_INLINE T* ExtendRawBuffer(T* pPtr, wdAllocatorBase* pAllocator, size_t uiCurrentCount, size_t uiNewCount, wdTypeIsClass)
  {
    WD_CHECK_AT_COMPILETIME_MSG(!std::is_trivial<T>::value,
      "POD type is treated as class. Use WD_DECLARE_POD_TYPE(YourClass) or WD_DEFINE_AS_POD_TYPE(ExternalClass) to mark it as POD.");

    T* pNewMem = CreateRawBuffer<T>(pAllocator, uiNewCount);
    wdMemoryUtils::RelocateConstruct(pNewMem, pPtr, uiCurrentCount);
    DeleteRawBuffer(pAllocator, pPtr);
    return pNewMem;
  }

  template <typename T>
  WD_FORCE_INLINE T* ExtendRawBuffer(T* pPtr, wdAllocatorBase* pAllocator, size_t uiCurrentCount, size_t uiNewCount)
  {
    WD_ASSERT_DEV(uiCurrentCount < uiNewCount, "Shrinking of a buffer is not implemented yet");
    WD_ASSERT_DEV(!(uiCurrentCount == uiNewCount), "Same size passed in twice.");
    if (pPtr == nullptr)
    {
      WD_ASSERT_DEV(uiCurrentCount == 0, "current count must be 0 if ptr is nullptr");

      return CreateRawBuffer<T>(pAllocator, uiNewCount);
    }
    return ExtendRawBuffer(pPtr, pAllocator, uiCurrentCount, uiNewCount, wdGetTypeClass<T>());
  }
} // namespace wdInternal
