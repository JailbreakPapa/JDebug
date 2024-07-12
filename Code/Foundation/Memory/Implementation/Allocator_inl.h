
NS_ALWAYS_INLINE nsAllocator::nsAllocator() = default;

NS_ALWAYS_INLINE nsAllocator::~nsAllocator() = default;


namespace nsMath
{
  // due to #include order issues, we have to forward declare this function here

  NS_FOUNDATION_DLL nsUInt64 SafeMultiply64(nsUInt64 a, nsUInt64 b, nsUInt64 c, nsUInt64 d);
} // namespace nsMath

namespace nsInternal
{
  template <typename T>
  struct NewInstance
  {
    NS_ALWAYS_INLINE NewInstance(T* pInstance, nsAllocator* pAllocator)
    {
      m_pInstance = pInstance;
      m_pAllocator = pAllocator;
    }

    template <typename U>
    NS_ALWAYS_INLINE NewInstance(NewInstance<U>&& other)
    {
      m_pInstance = other.m_pInstance;
      m_pAllocator = other.m_pAllocator;

      other.m_pInstance = nullptr;
      other.m_pAllocator = nullptr;
    }

    NS_ALWAYS_INLINE NewInstance(std::nullptr_t) {}

    template <typename U>
    NS_ALWAYS_INLINE NewInstance<U> Cast()
    {
      return NewInstance<U>(static_cast<U*>(m_pInstance), m_pAllocator);
    }

    NS_ALWAYS_INLINE operator T*() { return m_pInstance; }

    NS_ALWAYS_INLINE T* operator->() { return m_pInstance; }

    T* m_pInstance = nullptr;
    nsAllocator* m_pAllocator = nullptr;
  };

  template <typename T>
  NS_ALWAYS_INLINE bool operator<(const NewInstance<T>& lhs, T* rhs)
  {
    return lhs.m_pInstance < rhs;
  }

  template <typename T>
  NS_ALWAYS_INLINE bool operator<(T* lhs, const NewInstance<T>& rhs)
  {
    return lhs < rhs.m_pInstance;
  }

  template <typename T>
  NS_FORCE_INLINE void Delete(nsAllocator* pAllocator, T* pPtr)
  {
    if (pPtr != nullptr)
    {
      nsMemoryUtils::Destruct(pPtr, 1);
      pAllocator->Deallocate(pPtr);
    }
  }

  template <typename T>
  NS_FORCE_INLINE T* CreateRawBuffer(nsAllocator* pAllocator, size_t uiCount)
  {
    nsUInt64 safeAllocationSize = nsMath::SafeMultiply64(uiCount, sizeof(T));
    return static_cast<T*>(pAllocator->Allocate(static_cast<size_t>(safeAllocationSize), NS_ALIGNMENT_OF(T))); // Down-cast to size_t for 32-bit
  }

  NS_FORCE_INLINE void DeleteRawBuffer(nsAllocator* pAllocator, void* pPtr)
  {
    if (pPtr != nullptr)
    {
      pAllocator->Deallocate(pPtr);
    }
  }

  template <typename T>
  inline nsArrayPtr<T> CreateArray(nsAllocator* pAllocator, nsUInt32 uiCount)
  {
    T* buffer = CreateRawBuffer<T>(pAllocator, uiCount);
    nsMemoryUtils::Construct<SkipTrivialTypes>(buffer, uiCount);

    return nsArrayPtr<T>(buffer, uiCount);
  }

  template <typename T>
  inline void DeleteArray(nsAllocator* pAllocator, nsArrayPtr<T> arrayPtr)
  {
    T* buffer = arrayPtr.GetPtr();
    if (buffer != nullptr)
    {
      nsMemoryUtils::Destruct(buffer, arrayPtr.GetCount());
      pAllocator->Deallocate(buffer);
    }
  }

  template <typename T>
  NS_FORCE_INLINE T* ExtendRawBuffer(T* pPtr, nsAllocator* pAllocator, size_t uiCurrentCount, size_t uiNewCount, nsTypeIsPod)
  {
    return (T*)pAllocator->Reallocate(pPtr, uiCurrentCount * sizeof(T), uiNewCount * sizeof(T), NS_ALIGNMENT_OF(T));
  }

  template <typename T>
  NS_FORCE_INLINE T* ExtendRawBuffer(T* pPtr, nsAllocator* pAllocator, size_t uiCurrentCount, size_t uiNewCount, nsTypeIsMemRelocatable)
  {
    return (T*)pAllocator->Reallocate(pPtr, uiCurrentCount * sizeof(T), uiNewCount * sizeof(T), NS_ALIGNMENT_OF(T));
  }

  template <typename T>
  NS_FORCE_INLINE T* ExtendRawBuffer(T* pPtr, nsAllocator* pAllocator, size_t uiCurrentCount, size_t uiNewCount, nsTypeIsClass)
  {
    NS_CHECK_AT_COMPILETIME_MSG(!std::is_trivial<T>::value,
      "POD type is treated as class. Use NS_DECLARE_POD_TYPE(YourClass) or NS_DEFINE_AS_POD_TYPE(ExternalClass) to mark it as POD.");

    T* pNewMem = CreateRawBuffer<T>(pAllocator, uiNewCount);
    nsMemoryUtils::RelocateConstruct(pNewMem, pPtr, uiCurrentCount);
    DeleteRawBuffer(pAllocator, pPtr);
    return pNewMem;
  }

  template <typename T>
  NS_FORCE_INLINE T* ExtendRawBuffer(T* pPtr, nsAllocator* pAllocator, size_t uiCurrentCount, size_t uiNewCount)
  {
    NS_ASSERT_DEV(uiCurrentCount < uiNewCount, "Shrinking of a buffer is not implemented yet");
    NS_ASSERT_DEV(!(uiCurrentCount == uiNewCount), "Same size passed in twice.");
    if (pPtr == nullptr)
    {
      NS_ASSERT_DEV(uiCurrentCount == 0, "current count must be 0 if ptr is nullptr");

      return CreateRawBuffer<T>(pAllocator, uiNewCount);
    }
    return ExtendRawBuffer(pPtr, pAllocator, uiCurrentCount, uiNewCount, nsGetTypeClass<T>());
  }
} // namespace nsInternal
