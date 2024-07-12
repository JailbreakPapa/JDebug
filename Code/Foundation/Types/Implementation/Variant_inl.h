
NS_WARNING_PUSH()
NS_WARNING_DISABLE_MSVC(4702) // Unreachable code for some reason

NS_ALWAYS_INLINE nsVariant::nsVariant()
{
  m_uiType = Type::Invalid;
  m_bIsShared = false;
}

NS_WARNING_POP()

NS_WARNING_PUSH()
NS_WARNING_DISABLE_CLANG("-Wunused-local-typedef")
NS_WARNING_DISABLE_GCC("-Wunused-local-typedefs")

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsVariant& other)
{
  CopyFrom(other);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(nsVariant&& other) noexcept
{
  MoveFrom(std::move(other));
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const bool& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsInt8& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsUInt8& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsInt16& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsUInt16& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsInt32& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsUInt32& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsInt64& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsUInt64& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const float& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const double& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsColor& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsVec2& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsVec3& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsVec4& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsVec2I32& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsVec3I32& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsVec4I32& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsVec2U32& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsVec3U32& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsVec4U32& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsQuat& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsTime& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsUuid& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsAngle& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsColorGammaUB& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsHashedString& value)
{
  InitInplace(value);
}

NS_ALWAYS_INLINE nsVariant::nsVariant(const nsTempHashedString& value)
{
  InitInplace(value);
}

template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::CustomTypeCast, int>>
NS_ALWAYS_INLINE nsVariant::nsVariant(const T& value)
{
  const constexpr bool forceSharing = TypeDeduction<T>::forceSharing;
  const constexpr bool inlineSized = sizeof(T) <= InlinedStruct::DataSize;
  const constexpr bool isPOD = nsIsPodType<T>::value;
  InitTypedObject(value, nsTraitInt < (!forceSharing && inlineSized && isPOD) ? 1 : 0 > ());
}

template <typename T>
NS_ALWAYS_INLINE nsVariant::nsVariant(const T* value)
{
  constexpr bool bla = !std::is_same<T, void>::value;
  NS_CHECK_AT_COMPILETIME(bla);
  InitTypedPointer(const_cast<T*>(value), nsGetStaticRTTI<T>());
}

NS_ALWAYS_INLINE nsVariant::nsVariant(void* value, const nsRTTI* pType)
{
  InitTypedPointer(value, pType);
}

NS_ALWAYS_INLINE nsVariant::~nsVariant()
{
  Release();
}

NS_ALWAYS_INLINE void nsVariant::operator=(const nsVariant& other)
{
  if (this != &other)
  {
    Release();
    CopyFrom(other);
  }
}

NS_ALWAYS_INLINE void nsVariant::operator=(nsVariant&& other) noexcept
{
  if (this != &other)
  {
    Release();
    MoveFrom(std::move(other));
  }
}

template <typename T>
NS_ALWAYS_INLINE void nsVariant::operator=(const T& value)
{
  *this = nsVariant(value);
}

template <typename T>
NS_FORCE_INLINE bool nsVariant::operator==(const T& other) const
{
  if (IsFloatingPoint())
  {
    if constexpr (TypeDeduction<T>::value > Type::Invalid && TypeDeduction<T>::value <= Type::Double)
    {
      return ConvertNumber<double>() == static_cast<double>(other);
    }

    return false;
  }
  else if (IsNumber())
  {
    if constexpr (TypeDeduction<T>::value > Type::Invalid && TypeDeduction<T>::value <= Type::Double)
    {
      return ConvertNumber<nsInt64>() == static_cast<nsInt64>(other);
    }

    return false;
  }

  if constexpr (std::is_same_v<T, nsHashedString>)
  {
    if (m_uiType == Type::TempHashedString)
    {
      return Cast<nsTempHashedString>() == other;
    }
  }
  else if constexpr (std::is_same_v<T, nsTempHashedString>)
  {
    if (m_uiType == Type::HashedString)
    {
      return Cast<nsHashedString>() == other;
    }
  }
  else if constexpr (std::is_same_v<T, nsStringView>)
  {
    if (m_uiType == Type::String)
    {
      return Cast<nsString>().GetView() == other;
    }
  }
  else if constexpr (std::is_same_v<T, nsString>)
  {
    if (m_uiType == Type::StringView)
    {
      return Cast<nsStringView>() == other.GetView();
    }
  }

  using StorageType = typename TypeDeduction<T>::StorageType;
  NS_ASSERT_DEV(IsA<StorageType>(), "Stored type '{0}' does not match comparison type '{1}'", m_uiType, TypeDeduction<T>::value);
  return Cast<StorageType>() == other;
}

NS_ALWAYS_INLINE bool nsVariant::IsValid() const
{
  return m_uiType != Type::Invalid;
}

NS_ALWAYS_INLINE bool nsVariant::IsNumber() const
{
  return IsNumberStatic(m_uiType);
}

NS_ALWAYS_INLINE bool nsVariant::IsFloatingPoint() const
{
  return IsFloatingPointStatic(m_uiType);
}

NS_ALWAYS_INLINE bool nsVariant::IsString() const
{
  return IsStringStatic(m_uiType);
}

NS_ALWAYS_INLINE bool nsVariant::IsHashedString() const
{
  return IsHashedStringStatic(m_uiType);
}

template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::DirectCast, int>>
NS_ALWAYS_INLINE bool nsVariant::IsA() const
{
  return m_uiType == TypeDeduction<T>::value;
}

template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::PointerCast, int>>
NS_ALWAYS_INLINE bool nsVariant::IsA() const
{
  if (m_uiType == TypeDeduction<T>::value)
  {
    const nsTypedPointer& ptr = *reinterpret_cast<const nsTypedPointer*>(&m_Data);
    // Always allow cast to void*.
    if constexpr (std::is_same<T, void*>::value || std::is_same<T, const void*>::value)
    {
      return true;
    }
    else if (ptr.m_pType)
    {
      using NonPointerT = typename nsTypeTraits<T>::NonConstReferencePointerType;
      const nsRTTI* pType = nsGetStaticRTTI<NonPointerT>();
      return IsDerivedFrom(ptr.m_pType, pType);
    }
    else if (!ptr.m_pObject)
    {
      // nullptr can be converted to anything
      return true;
    }
  }
  return false;
}

template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::TypedObject, int>>
NS_ALWAYS_INLINE bool nsVariant::IsA() const
{
  return m_uiType == TypeDeduction<T>::value;
}

template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::CustomTypeCast, int>>
NS_ALWAYS_INLINE bool nsVariant::IsA() const
{
  using NonRefT = typename nsTypeTraits<T>::NonConstReferenceType;
  if (m_uiType == TypeDeduction<T>::value)
  {
    if (const nsRTTI* pType = GetReflectedType())
    {
      return IsDerivedFrom(pType, nsGetStaticRTTI<NonRefT>());
    }
  }
  return false;
}

NS_ALWAYS_INLINE nsVariant::Type::Enum nsVariant::GetType() const
{
  return static_cast<Type::Enum>(m_uiType);
}

template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::DirectCast, int>>
NS_ALWAYS_INLINE const T& nsVariant::Get() const
{
  NS_ASSERT_DEV(IsA<T>(), "Stored type '{0}' does not match requested type '{1}'", m_uiType, TypeDeduction<T>::value);
  return Cast<T>();
}

template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::PointerCast, int>>
NS_ALWAYS_INLINE T nsVariant::Get() const
{
  NS_ASSERT_DEV(IsA<T>(), "Stored type '{0}' does not match requested type '{1}'", m_uiType, TypeDeduction<T>::value);
  return Cast<T>();
}

template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::TypedObject, int>>
NS_ALWAYS_INLINE const T nsVariant::Get() const
{
  NS_ASSERT_DEV(IsA<T>(), "Stored type '{0}' does not match requested type '{1}'", m_uiType, TypeDeduction<T>::value);
  return Cast<T>();
}

template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::CustomTypeCast, int>>
NS_ALWAYS_INLINE const T& nsVariant::Get() const
{
  NS_ASSERT_DEV(m_uiType == TypeDeduction<T>::value, "Stored type '{0}' does not match requested type '{1}'", m_uiType, TypeDeduction<T>::value);
  return Cast<T>();
}

template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::DirectCast, int>>
NS_ALWAYS_INLINE T& nsVariant::GetWritable()
{
  GetWriteAccess();
  return const_cast<T&>(Get<T>());
}

template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::PointerCast, int>>
NS_ALWAYS_INLINE T nsVariant::GetWritable()
{
  GetWriteAccess();
  return const_cast<T>(Get<T>());
}

template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::CustomTypeCast, int>>
NS_ALWAYS_INLINE T& nsVariant::GetWritable()
{
  GetWriteAccess();
  return const_cast<T&>(Get<T>());
}

NS_ALWAYS_INLINE const void* nsVariant::GetData() const
{
  if (m_uiType == Type::TypedPointer)
  {
    return Cast<nsTypedPointer>().m_pObject;
  }
  return m_bIsShared ? m_Data.shared->m_Ptr : &m_Data;
}

template <typename T>
NS_ALWAYS_INLINE bool nsVariant::CanConvertTo() const
{
  return CanConvertTo(static_cast<Type::Enum>(TypeDeduction<T>::value));
}

template <typename T>
T nsVariant::ConvertTo(nsResult* out_pConversionStatus /* = nullptr*/) const
{
  if (!CanConvertTo<T>())
  {
    if (out_pConversionStatus != nullptr)
      *out_pConversionStatus = NS_FAILURE;

    return T();
  }

  if (m_uiType == TypeDeduction<T>::value)
  {
    if (out_pConversionStatus != nullptr)
      *out_pConversionStatus = NS_SUCCESS;

    return Cast<T>();
  }

  T result = {};
  bool bSuccessful = true;
  nsVariantHelper::To(*this, result, bSuccessful);

  if (out_pConversionStatus != nullptr)
    *out_pConversionStatus = bSuccessful ? NS_SUCCESS : NS_FAILURE;

  return result;
}


/// private methods

template <typename T>
NS_FORCE_INLINE void nsVariant::InitInplace(const T& value)
{
  NS_CHECK_AT_COMPILETIME_MSG(TypeDeduction<T>::value != Type::Invalid, "value of this type cannot be stored in a Variant");
  NS_CHECK_AT_COMPILETIME_MSG(nsGetTypeClass<T>::value <= nsTypeIsMemRelocatable::value, "in place data needs to be POD or mem relocatable");
  NS_CHECK_AT_COMPILETIME_MSG(sizeof(T) <= sizeof(m_Data), "value of this type is too big to bestored inline in a Variant");
  nsMemoryUtils::CopyConstruct(reinterpret_cast<T*>(&m_Data), value, 1);

  m_uiType = TypeDeduction<T>::value;
  m_bIsShared = false;
}

template <typename T>
NS_FORCE_INLINE void nsVariant::InitTypedObject(const T& value, nsTraitInt<0>)
{
  using StorageType = typename TypeDeduction<T>::StorageType;

  NS_CHECK_AT_COMPILETIME_MSG((sizeof(StorageType) > sizeof(InlinedStruct::DataSize)) || TypeDeduction<T>::forceSharing, "Value should be inplace instead.");
  NS_CHECK_AT_COMPILETIME_MSG(TypeDeduction<T>::value == Type::TypedObject, "value of this type cannot be stored in a Variant");
  const nsRTTI* pType = nsGetStaticRTTI<T>();
  m_Data.shared = NS_DEFAULT_NEW(TypedSharedData<StorageType>, value, pType);
  m_uiType = Type::TypedObject;
  m_bIsShared = true;
}

template <typename T>
NS_FORCE_INLINE void nsVariant::InitTypedObject(const T& value, nsTraitInt<1>)
{
  using StorageType = typename TypeDeduction<T>::StorageType;
  NS_CHECK_AT_COMPILETIME_MSG((sizeof(StorageType) <= InlinedStruct::DataSize) && !TypeDeduction<T>::forceSharing, "Value can't be stored inplace.");
  NS_CHECK_AT_COMPILETIME_MSG(TypeDeduction<T>::value == Type::TypedObject, "value of this type cannot be stored in a Variant");
  NS_CHECK_AT_COMPILETIME_MSG(nsIsPodType<T>::value, "in place data needs to be POD");
  nsMemoryUtils::CopyConstruct(reinterpret_cast<T*>(&m_Data), value, 1);
  m_Data.inlined.m_pType = nsGetStaticRTTI<T>();
  m_uiType = Type::TypedObject;
  m_bIsShared = false;
}

inline void nsVariant::Release()
{
  if (m_bIsShared)
  {
    if (m_Data.shared->m_uiRef.Decrement() == 0)
    {
      NS_DEFAULT_DELETE(m_Data.shared);
    }
  }
}

inline void nsVariant::CopyFrom(const nsVariant& other)
{
  m_uiType = other.m_uiType;
  m_bIsShared = other.m_bIsShared;

  if (m_bIsShared)
  {
    m_Data.shared = other.m_Data.shared;
    m_Data.shared->m_uiRef.Increment();
  }
  else if (other.IsValid())
  {
    m_Data = other.m_Data;
  }
}

NS_ALWAYS_INLINE void nsVariant::MoveFrom(nsVariant&& other)
{
  m_uiType = other.m_uiType;
  m_bIsShared = other.m_bIsShared;
  m_Data = other.m_Data;

  other.m_uiType = Type::Invalid;
  other.m_bIsShared = false;
  other.m_Data.shared = nullptr;
}

template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::DirectCast, int>>
const T& nsVariant::Cast() const
{
  const bool validType = nsConversionTest<T, typename TypeDeduction<T>::StorageType>::sameType;
  NS_CHECK_AT_COMPILETIME_MSG(validType, "Invalid Cast, can only cast to storage type");

  return m_bIsShared ? *static_cast<const T*>(m_Data.shared->m_Ptr) : *reinterpret_cast<const T*>(&m_Data);
}

template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::PointerCast, int>>
T nsVariant::Cast() const
{
  const nsTypedPointer& ptr = *reinterpret_cast<const nsTypedPointer*>(&m_Data);

  const nsRTTI* pType = GetReflectedType();
  NS_IGNORE_UNUSED(pType);
  using NonRefPtrT = typename nsTypeTraits<T>::NonConstReferencePointerType;
  if constexpr (!std::is_same<T, void*>::value && !std::is_same<T, const void*>::value)
  {
    NS_ASSERT_DEV(pType == nullptr || IsDerivedFrom(pType, nsGetStaticRTTI<NonRefPtrT>()), "Object of type '{0}' does not derive from '{}'", GetTypeName(pType), GetTypeName(nsGetStaticRTTI<NonRefPtrT>()));
  }
  return static_cast<T>(ptr.m_pObject);
}

template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::TypedObject, int>>
const T nsVariant::Cast() const
{
  nsTypedObject obj;
  obj.m_pObject = GetData();
  obj.m_pType = GetReflectedType();
  return obj;
}

template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::CustomTypeCast, int>>
const T& nsVariant::Cast() const
{
  const nsRTTI* pType = GetReflectedType();
  NS_IGNORE_UNUSED(pType);
  using NonRefT = typename nsTypeTraits<T>::NonConstReferenceType;
  NS_ASSERT_DEV(IsDerivedFrom(pType, nsGetStaticRTTI<NonRefT>()), "Object of type '{0}' does not derive from '{}'", GetTypeName(pType), GetTypeName(nsGetStaticRTTI<NonRefT>()));

  return m_bIsShared ? *static_cast<const T*>(m_Data.shared->m_Ptr) : *reinterpret_cast<const T*>(&m_Data);
}

NS_ALWAYS_INLINE bool nsVariant::IsNumberStatic(nsUInt32 type)
{
  return type > Type::FirstStandardType && type <= Type::Double;
}

NS_ALWAYS_INLINE bool nsVariant::IsFloatingPointStatic(nsUInt32 type)
{
  return type == Type::Float || type == Type::Double;
}

NS_ALWAYS_INLINE bool nsVariant::IsStringStatic(nsUInt32 type)
{
  return type == Type::String || type == Type::StringView;
}

NS_ALWAYS_INLINE bool nsVariant::IsHashedStringStatic(nsUInt32 type)
{
  return type == Type::HashedString || type == Type::TempHashedString;
}

NS_ALWAYS_INLINE bool nsVariant::IsVector2Static(nsUInt32 type)
{
  return type == Type::Vector2 || type == Type::Vector2I || type == Type::Vector2U;
}

NS_ALWAYS_INLINE bool nsVariant::IsVector3Static(nsUInt32 type)
{
  return type == Type::Vector3 || type == Type::Vector3I || type == Type::Vector3U;
}

NS_ALWAYS_INLINE bool nsVariant::IsVector4Static(nsUInt32 type)
{
  return type == Type::Vector4 || type == Type::Vector4I || type == Type::Vector4U;
}

template <typename T>
T nsVariant::ConvertNumber() const
{
  switch (m_uiType)
  {
    case Type::Bool:
      return static_cast<T>(Cast<bool>());
    case Type::Int8:
      return static_cast<T>(Cast<nsInt8>());
    case Type::UInt8:
      return static_cast<T>(Cast<nsUInt8>());
    case Type::Int16:
      return static_cast<T>(Cast<nsInt16>());
    case Type::UInt16:
      return static_cast<T>(Cast<nsUInt16>());
    case Type::Int32:
      return static_cast<T>(Cast<nsInt32>());
    case Type::UInt32:
      return static_cast<T>(Cast<nsUInt32>());
    case Type::Int64:
      return static_cast<T>(Cast<nsInt64>());
    case Type::UInt64:
      return static_cast<T>(Cast<nsUInt64>());
    case Type::Float:
      return static_cast<T>(Cast<float>());
    case Type::Double:
      return static_cast<T>(Cast<double>());
  }

  NS_REPORT_FAILURE("Variant is not a number");
  return T(0);
}

template <>
struct nsHashHelper<nsVariant>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(const nsVariant& value)
  {
    nsUInt64 uiHash = value.ComputeHash(0);
    return (nsUInt32)uiHash;
  }

  NS_ALWAYS_INLINE static bool Equal(const nsVariant& a, const nsVariant& b)
  {
    return a.GetType() == b.GetType() && a == b;
  }
};

NS_WARNING_POP()
