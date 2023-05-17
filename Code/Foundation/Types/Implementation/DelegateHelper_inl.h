
/// \brief [Internal] Storage for lambdas with captures in wdDelegate.
struct WD_FOUNDATION_DLL wdLambdaDelegateStorageBase
{
  wdLambdaDelegateStorageBase() = default;
  virtual ~wdLambdaDelegateStorageBase() = default;
  virtual wdLambdaDelegateStorageBase* Clone(wdAllocatorBase* pAllocator) const = 0;
  virtual void InplaceCopy(wdUInt8* pBuffer) const = 0;
  virtual void InplaceMove(wdUInt8* pBuffer) = 0;

private:
  wdLambdaDelegateStorageBase(const wdLambdaDelegateStorageBase&) = delete;
  wdLambdaDelegateStorageBase& operator=(const wdLambdaDelegateStorageBase&) = delete;
  wdLambdaDelegateStorageBase(wdLambdaDelegateStorageBase&&) = delete;
  wdLambdaDelegateStorageBase& operator=(wdLambdaDelegateStorageBase&&) = delete;
};

template <typename Function>
struct wdLambdaDelegateStorage : public wdLambdaDelegateStorageBase
{
  wdLambdaDelegateStorage(Function&& func)
    : m_func(std::move(func))
  {
  }

private:
  template <typename = typename std::enable_if<std::is_copy_constructible<Function>::value>>
  wdLambdaDelegateStorage(const Function& func)
    : m_func(func)
  {
  }

public:
  virtual wdLambdaDelegateStorageBase* Clone(wdAllocatorBase* pAllocator) const override
  {
    if constexpr (std::is_copy_constructible<Function>::value)
    {
      return WD_NEW(pAllocator, wdLambdaDelegateStorage<Function>, m_func);
    }
    else
    {
      WD_REPORT_FAILURE("The wdDelegate stores a lambda that is not copyable. Copying this wdDelegate is not supported.");
      return nullptr;
    }
  }

  virtual void InplaceCopy(wdUInt8* pBuffer) const override
  {
    if constexpr (std::is_copy_constructible<Function>::value)
    {
      new (pBuffer) wdLambdaDelegateStorage<Function>(m_func);
    }
    else
    {
      WD_REPORT_FAILURE("The wdDelegate stores a lambda that is not copyable. Copying this wdDelegate is not supported.");
    }
  }

  virtual void InplaceMove(wdUInt8* pBuffer) override
  {
    if constexpr (std::is_move_constructible<Function>::value)
    {
      new (pBuffer) wdLambdaDelegateStorage<Function>(std::move(m_func));
    }
    else
    {
      WD_REPORT_FAILURE("The wdDelegate stores a lambda that is not movable. Moving this wdDelegate is not supported.");
    }
  }

  Function m_func;
};


template <typename R, class... Args, wdUInt32 DataSize>
struct wdDelegate<R(Args...), DataSize> : public wdDelegateBase
{
private:
  using SelfType = wdDelegate<R(Args...), DataSize>;
  constexpr const void* HeapLambda() const { return reinterpret_cast<const void*>((size_t)-1); }
  constexpr const void* InplaceLambda() const { return reinterpret_cast<const void*>((size_t)-2); }

public:
  WD_ALWAYS_INLINE wdDelegate()
    : m_DispatchFunction(nullptr)
  {
  }

  WD_ALWAYS_INLINE wdDelegate(const SelfType& other) { *this = other; }

  WD_ALWAYS_INLINE wdDelegate(SelfType&& other) { *this = std::move(other); }

  /// \brief Constructs the delegate from a member function type and takes the class instance on which to call the function later.
  template <typename Method, typename Class>
  WD_FORCE_INLINE wdDelegate(Method method, Class* pInstance)
  {
    CopyMemberFunctionToInplaceStorage(method);

    m_Instance.m_Ptr = pInstance;
    m_DispatchFunction = &DispatchToMethod<Method, Class>;
  }

  /// \brief Constructs the delegate from a member function type and takes the (const) class instance on which to call the function later.
  template <typename Method, typename Class>
  WD_FORCE_INLINE wdDelegate(Method method, const Class* pInstance)
  {
    CopyMemberFunctionToInplaceStorage(method);

    m_Instance.m_ConstPtr = pInstance;
    m_DispatchFunction = &DispatchToConstMethod<Method, Class>;
  }

  /// \brief Constructs the delegate from a regular C function type.
  template <typename Function>
  WD_FORCE_INLINE wdDelegate(Function function, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator())
  {
    WD_CHECK_AT_COMPILETIME_MSG(DataSize >= 16, "DataSize must be at least 16 bytes");

    // Pure function pointers or lambdas that can be cast into pure functions (no captures) can be
    // copied directly into the inplace storage of the delegate.
    // Lambdas with captures need to be wrapped into an wdLambdaDelegateStorage object as they can
    // capture non-pod or non-memmoveable data. This wrapper can also be stored inplace if it is small enough,
    // otherwise it will be heap allocated with the specified allocator.
    constexpr size_t functionSize = sizeof(Function);
    using signature = R(Args...);
    if constexpr (functionSize <= DataSize && std::is_assignable<signature*&, Function>::value)
    {
      // Lambdas with no capture have a size of 1.
      // Lambdas with no capture actually have no data. Do not copy the 1 uninitialized byte.
      // Propper function pointers have a size of > 4 or 8 (depending on pointer size)
      if constexpr (functionSize > 1)
      {
        CopyFunctionToInplaceStorage(function);
      }
      else
      {
        memset(m_Data, 0, DataSize);
      }

      m_Instance.m_ConstPtr = nullptr;
      m_DispatchFunction = &DispatchToFunction<Function>;
    }
    else
    {
      constexpr size_t storageSize = sizeof(wdLambdaDelegateStorage<Function>);
      if constexpr (storageSize <= DataSize)
      {
        m_Instance.m_ConstPtr = InplaceLambda();
        new (m_Data) wdLambdaDelegateStorage<Function>(std::move(function));
        memset(m_Data + storageSize, 0, DataSize - storageSize);
        m_DispatchFunction = &DispatchToInplaceLambda<Function>;
      }
      else
      {
        m_Instance.m_ConstPtr = HeapLambda();
        m_pLambdaStorage = WD_NEW(pAllocator, wdLambdaDelegateStorage<Function>, std::move(function));
        m_pAllocator = pAllocator;
        memset(m_Data + 2 * sizeof(void*), 0, DataSize - 2 * sizeof(void*));
        m_DispatchFunction = &DispatchToHeapLambda<Function>;
      }
    }
  }

  WD_ALWAYS_INLINE ~wdDelegate() { Invalidate(); }

  /// \brief Copies the data from another delegate.
  WD_FORCE_INLINE void operator=(const SelfType& other)
  {
    Invalidate();

    if (other.IsHeapLambda())
    {
      m_pAllocator = other.m_pAllocator;
      m_pLambdaStorage = other.m_pLambdaStorage->Clone(m_pAllocator);
    }
    else if (other.IsInplaceLambda())
    {
      auto pOtherLambdaStorage = reinterpret_cast<wdLambdaDelegateStorageBase*>(&other.m_Data);
      pOtherLambdaStorage->InplaceCopy(m_Data);
    }
    else
    {
      memcpy(m_Data, other.m_Data, DataSize);
    }

    m_Instance = other.m_Instance;
    m_DispatchFunction = other.m_DispatchFunction;
  }

  /// \brief Moves the data from another delegate.
  WD_FORCE_INLINE void operator=(SelfType&& other)
  {
    Invalidate();
    m_Instance = other.m_Instance;
    m_DispatchFunction = other.m_DispatchFunction;

    if (other.IsInplaceLambda())
    {
      auto pOtherLambdaStorage = reinterpret_cast<wdLambdaDelegateStorageBase*>(&other.m_Data);
      pOtherLambdaStorage->InplaceMove(m_Data);
    }
    else
    {
      memcpy(m_Data, other.m_Data, DataSize);
    }

    other.m_Instance.m_Ptr = nullptr;
    other.m_DispatchFunction = nullptr;
    memset(other.m_Data, 0, DataSize);
  }

  /// \brief Resets a delegate to an invalid state.
  WD_FORCE_INLINE void operator=(std::nullptr_t) { Invalidate(); }

  /// \brief Function call operator. This will call the function that is bound to the delegate, or assert if nothing was bound.
  WD_FORCE_INLINE R operator()(Args... params) const
  {
    WD_ASSERT_DEBUG(m_DispatchFunction != nullptr, "Delegate is not bound.");
    return (*m_DispatchFunction)(*this, params...);
  }

  /// \brief This function only exists to make code compile, but it will assert when used. Use IsEqualIfNotHeapAllocated() instead.
  WD_ALWAYS_INLINE bool operator==(const SelfType& other) const
  {
    WD_REPORT_FAILURE("operator== for wdDelegate must not be used. Use IsEqualIfNotHeapAllocated() and read its documentation!");
    return false;
  }

  /// \brief Checks whether two delegates are bound to the exact same function, including the class instance.
  /// \note If \a this or \a other or both return false for IsComparable(), the function returns always false!
  /// Therefore, do not use this to search for delegates that are not comparable. wdEvent uses this function, but goes to great lengths to
  /// assert that it is used correctly. It is best to not use this function at all.
  WD_ALWAYS_INLINE bool IsEqualIfComparable(const SelfType& other) const
  {
    return m_Instance.m_Ptr == other.m_Instance.m_Ptr && m_DispatchFunction == other.m_DispatchFunction &&
           memcmp(m_Data, other.m_Data, DataSize) == 0;
  }

  /// \brief Returns true when the delegate is bound to a valid non-nullptr function.
  WD_ALWAYS_INLINE bool IsValid() const { return m_DispatchFunction != nullptr; }

  /// \brief Resets a delegate to an invalid state.
  WD_FORCE_INLINE void Invalidate()
  {
    m_DispatchFunction = nullptr;
    if (IsHeapLambda())
    {
      WD_DELETE(m_pAllocator, m_pLambdaStorage);
    }
    else if (IsInplaceLambda())
    {
      auto pLambdaStorage = reinterpret_cast<wdLambdaDelegateStorageBase*>(&m_Data);
      pLambdaStorage->~wdLambdaDelegateStorageBase();
    }

    m_Instance.m_Ptr = nullptr;
    memset(m_Data, 0, DataSize);
  }

  /// \brief Returns the class instance that is used to call a member function pointer on.
  WD_ALWAYS_INLINE void* GetClassInstance() const { return IsComparable() ? m_Instance.m_Ptr : nullptr; }

  /// \brief Returns whether the delegate is comparable with other delegates of the same type. This is not the case for i.e. lambdas with captures.
  WD_ALWAYS_INLINE bool IsComparable() const { return m_Instance.m_ConstPtr < InplaceLambda(); } // [tested]

private:
  template <typename Function>
  WD_FORCE_INLINE void CopyFunctionToInplaceStorage(Function function)
  {
    WD_ASSERT_DEBUG(
      wdMemoryUtils::IsAligned(&m_Data, WD_ALIGNMENT_OF(Function)), "Wrong alignment. Expected {0} bytes alignment", WD_ALIGNMENT_OF(Function));

    memcpy(m_Data, &function, sizeof(Function));
    memset(m_Data + sizeof(Function), 0, DataSize - sizeof(Function));
  }

  template <typename Method>
  WD_FORCE_INLINE void CopyMemberFunctionToInplaceStorage(Method method)
  {
    WD_CHECK_AT_COMPILETIME_MSG(DataSize >= 16, "DataSize must be at least 16 bytes");
    WD_CHECK_AT_COMPILETIME_MSG(sizeof(Method) <= DataSize, "Member function pointer must not be bigger than 16 bytes");

    CopyFunctionToInplaceStorage(method);

    // Member Function Pointers in MSVC are 12 bytes in size and have 4 byte padding
    // MSVC builds a member function pointer on the stack writing only 12 bytes and then copies it
    // to the final location by copying 16 bytes. Thus the 4 byte padding get a random value (whatever is on the stack at that time).
    // To make the delegate comparable by memcmp we zero out those 4 byte padding.
    // Apparently clang does the same on windows but not on linux etc.
#if WD_ENABLED(WD_COMPILER_MSVC) || (WD_ENABLED(WD_PLATFORM_WINDOWS) && WD_ENABLED(WD_COMPILER_CLANG))
    *reinterpret_cast<wdUInt32*>(m_Data + 12) = 0;
#endif
  }

  WD_ALWAYS_INLINE bool IsInplaceLambda() const { return m_Instance.m_ConstPtr == InplaceLambda(); }
  WD_ALWAYS_INLINE bool IsHeapLambda() const { return m_Instance.m_ConstPtr == HeapLambda(); }

  template <typename Method, typename Class>
  static WD_FORCE_INLINE R DispatchToMethod(const SelfType& self, Args... params)
  {
    WD_ASSERT_DEBUG(self.m_Instance.m_Ptr != nullptr, "Instance must not be null.");
    Method method = *reinterpret_cast<Method*>(&self.m_Data);
    return (static_cast<Class*>(self.m_Instance.m_Ptr)->*method)(params...);
  }

  template <typename Method, typename Class>
  static WD_FORCE_INLINE R DispatchToConstMethod(const SelfType& self, Args... params)
  {
    WD_ASSERT_DEBUG(self.m_Instance.m_ConstPtr != nullptr, "Instance must not be null.");
    Method method = *reinterpret_cast<Method*>(&self.m_Data);
    return (static_cast<const Class*>(self.m_Instance.m_ConstPtr)->*method)(params...);
  }

  template <typename Function>
  static WD_ALWAYS_INLINE R DispatchToFunction(const SelfType& self, Args... params)
  {
    return (*reinterpret_cast<Function*>(&self.m_Data))(params...);
  }

  template <typename Function>
  static WD_ALWAYS_INLINE R DispatchToHeapLambda(const SelfType& self, Args... params)
  {
    return static_cast<wdLambdaDelegateStorage<Function>*>(self.m_pLambdaStorage)->m_func(params...);
  }

  template <typename Function>
  static WD_ALWAYS_INLINE R DispatchToInplaceLambda(const SelfType& self, Args... params)
  {
    return reinterpret_cast<wdLambdaDelegateStorage<Function>*>(&self.m_Data)->m_func(params...);
  }

  using DispatchFunction = R (*)(const SelfType&, Args...);
  DispatchFunction m_DispatchFunction;

  union
  {
    mutable wdUInt8 m_Data[DataSize];
    struct
    {
      wdLambdaDelegateStorageBase* m_pLambdaStorage;
      wdAllocatorBase* m_pAllocator;
    };
  };
};

template <typename T>
struct wdMakeDelegateHelper
{
};

template <typename Class, typename R, typename... Args>
struct wdMakeDelegateHelper<R (Class::*)(Args...)>
{
  using DelegateType = wdDelegate<R(Args...)>;
};

template <typename Class, typename R, typename... Args>
struct wdMakeDelegateHelper<R (Class::*)(Args...) const>
{
  using DelegateType = wdDelegate<R(Args...)>;
};
