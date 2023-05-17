
#define WD_MSVC_WARNING_NUMBER 4702 // Unreachable code for some reason
#include <Foundation/Basics/Compiler/MSVC/DisableWarning_MSVC.h>

WD_ALWAYS_INLINE wdVariant::wdVariant()
{
  m_uiType = Type::Invalid;
  m_bIsShared = false;
}

#include <Foundation/Basics/Compiler/MSVC/RestoreWarning_MSVC.h>

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdVariant& other)
{
  CopyFrom(other);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(wdVariant&& other) noexcept
{
  MoveFrom(std::move(other));
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const bool& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdInt8& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdUInt8& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdInt16& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdUInt16& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdInt32& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdUInt32& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdInt64& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdUInt64& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const float& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const double& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdColor& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdVec2& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdVec3& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdVec4& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdVec2I32& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdVec3I32& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdVec4I32& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdVec2U32& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdVec3U32& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdVec4U32& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdQuat& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdStringView& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdTime& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdUuid& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdAngle& value)
{
  InitInplace(value);
}

WD_ALWAYS_INLINE wdVariant::wdVariant(const wdColorGammaUB& value)
{
  InitInplace(value);
}

template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::CustomTypeCast, int>>
WD_ALWAYS_INLINE wdVariant::wdVariant(const T& value)
{
  const constexpr bool forceSharing = TypeDeduction<T>::forceSharing;
  const constexpr bool inlineSized = sizeof(T) <= InlinedStruct::DataSize;
  const constexpr bool isPOD = wdIsPodType<T>::value;
  InitTypedObject(value, wdTraitInt < (!forceSharing && inlineSized && isPOD) ? 1 : 0 > ());
}

template <typename T>
WD_ALWAYS_INLINE wdVariant::wdVariant(const T* value)
{
  constexpr bool bla = !std::is_same<T, void>::value;
  WD_CHECK_AT_COMPILETIME(bla);
  InitTypedPointer(const_cast<T*>(value), wdGetStaticRTTI<T>());
}

WD_ALWAYS_INLINE wdVariant::wdVariant(void* value, const wdRTTI* pType)
{
  InitTypedPointer(value, pType);
}

WD_ALWAYS_INLINE wdVariant::~wdVariant()
{
  Release();
}

WD_ALWAYS_INLINE void wdVariant::operator=(const wdVariant& other)
{
  if (this != &other)
  {
    Release();
    CopyFrom(other);
  }
}

WD_ALWAYS_INLINE void wdVariant::operator=(wdVariant&& other) noexcept
{
  if (this != &other)
  {
    Release();
    MoveFrom(std::move(other));
  }
}

template <typename T>
WD_ALWAYS_INLINE void wdVariant::operator=(const T& value)
{
  *this = wdVariant(value);
}

WD_ALWAYS_INLINE bool wdVariant::operator!=(const wdVariant& other) const
{
  return !(*this == other);
}

template <typename T>
WD_FORCE_INLINE bool wdVariant::operator==(const T& other) const
{
  using StorageType = typename TypeDeduction<T>::StorageType;
  struct TypeInfo
  {
    enum
    {
      isNumber = TypeDeduction<T>::value > Type::Invalid&& TypeDeduction<T>::value <= Type::Double
    };
  };

  if (IsFloatingPoint())
  {
    return wdVariantHelper::CompareFloat(*this, other, wdTraitInt<TypeInfo::isNumber>());
  }
  else if (IsNumber())
  {
    return wdVariantHelper::CompareNumber(*this, other, wdTraitInt<TypeInfo::isNumber>());
  }

  WD_ASSERT_DEV(IsA<StorageType>(), "Stored type '{0}' does not match comparison type '{1}'", m_uiType, TypeDeduction<T>::value);
  return Cast<StorageType>() == other;
}

template <typename T>
WD_ALWAYS_INLINE bool wdVariant::operator!=(const T& other) const
{
  return !(*this == other);
}

WD_ALWAYS_INLINE bool wdVariant::IsValid() const
{
  return m_uiType != Type::Invalid;
}

WD_ALWAYS_INLINE bool wdVariant::IsNumber() const
{
  return IsNumberStatic(m_uiType);
}

WD_ALWAYS_INLINE bool wdVariant::IsFloatingPoint() const
{
  return IsFloatingPointStatic(m_uiType);
}

WD_ALWAYS_INLINE bool wdVariant::IsString() const
{
  return IsStringStatic(m_uiType);
}

template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::DirectCast, int>>
WD_ALWAYS_INLINE bool wdVariant::IsA() const
{
  return m_uiType == TypeDeduction<T>::value;
}

template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::PointerCast, int>>
WD_ALWAYS_INLINE bool wdVariant::IsA() const
{
  if (m_uiType == TypeDeduction<T>::value)
  {
    const wdTypedPointer& ptr = *reinterpret_cast<const wdTypedPointer*>(&m_Data);
    // Always allow cast to void*.
    if constexpr (std::is_same<T, void*>::value || std::is_same<T, const void*>::value)
    {
      return true;
    }
    else if (ptr.m_pType)
    {
      typedef typename wdTypeTraits<T>::NonConstReferencePointerType NonPointerT;
      const wdRTTI* pType = wdGetStaticRTTI<NonPointerT>();
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

template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::TypedObject, int>>
WD_ALWAYS_INLINE bool wdVariant::IsA() const
{
  return m_uiType == TypeDeduction<T>::value;
}

template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::CustomTypeCast, int>>
WD_ALWAYS_INLINE bool wdVariant::IsA() const
{
  typedef typename wdTypeTraits<T>::NonConstReferenceType NonRefT;
  if (m_uiType == TypeDeduction<T>::value)
  {
    if (const wdRTTI* pType = GetReflectedType())
    {
      return IsDerivedFrom(pType, wdGetStaticRTTI<NonRefT>());
    }
  }
  return false;
}

WD_ALWAYS_INLINE wdVariant::Type::Enum wdVariant::GetType() const
{
  return static_cast<Type::Enum>(m_uiType);
}

template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::DirectCast, int>>
WD_ALWAYS_INLINE const T& wdVariant::Get() const
{
  WD_ASSERT_DEV(IsA<T>(), "Stored type '{0}' does not match requested type '{1}'", m_uiType, TypeDeduction<T>::value);
  return Cast<T>();
}

template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::PointerCast, int>>
WD_ALWAYS_INLINE T wdVariant::Get() const
{
  WD_ASSERT_DEV(IsA<T>(), "Stored type '{0}' does not match requested type '{1}'", m_uiType, TypeDeduction<T>::value);
  return Cast<T>();
}

template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::TypedObject, int>>
WD_ALWAYS_INLINE const T wdVariant::Get() const
{
  WD_ASSERT_DEV(IsA<T>(), "Stored type '{0}' does not match requested type '{1}'", m_uiType, TypeDeduction<T>::value);
  return Cast<T>();
}

template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::CustomTypeCast, int>>
WD_ALWAYS_INLINE const T& wdVariant::Get() const
{
  WD_ASSERT_DEV(m_uiType == TypeDeduction<T>::value, "Stored type '{0}' does not match requested type '{1}'", m_uiType, TypeDeduction<T>::value);
  return Cast<T>();
}

template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::DirectCast, int>>
WD_ALWAYS_INLINE T& wdVariant::GetWritable()
{
  GetWriteAccess();
  return const_cast<T&>(Get<T>());
}

template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::PointerCast, int>>
WD_ALWAYS_INLINE T wdVariant::GetWritable()
{
  GetWriteAccess();
  return const_cast<T>(Get<T>());
}

template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::CustomTypeCast, int>>
WD_ALWAYS_INLINE T& wdVariant::GetWritable()
{
  GetWriteAccess();
  return const_cast<T&>(Get<T>());
}

WD_ALWAYS_INLINE const void* wdVariant::GetData() const
{
  if (m_uiType == Type::TypedPointer)
  {
    return Cast<wdTypedPointer>().m_pObject;
  }
  return m_bIsShared ? m_Data.shared->m_Ptr : &m_Data;
}

template <typename T>
WD_ALWAYS_INLINE bool wdVariant::CanConvertTo() const
{
  return CanConvertTo(static_cast<Type::Enum>(TypeDeduction<T>::value));
}

template <typename T>
T wdVariant::ConvertTo(wdResult* out_pConversionStatus /* = nullptr*/) const
{
  if (!CanConvertTo<T>())
  {
    if (out_pConversionStatus != nullptr)
      *out_pConversionStatus = WD_FAILURE;

    return T();
  }

  if (m_uiType == TypeDeduction<T>::value)
  {
    if (out_pConversionStatus != nullptr)
      *out_pConversionStatus = WD_SUCCESS;

    return Cast<T>();
  }

  T result;
  bool bSuccessful = true;
  wdVariantHelper::To(*this, result, bSuccessful);

  if (out_pConversionStatus != nullptr)
    *out_pConversionStatus = bSuccessful ? WD_SUCCESS : WD_FAILURE;

  return result;
}


/// private methods

template <typename T>
WD_FORCE_INLINE void wdVariant::InitInplace(const T& value)
{
  WD_CHECK_AT_COMPILETIME_MSG(TypeDeduction<T>::value != Type::Invalid, "value of this type cannot be stored in a Variant");
  WD_CHECK_AT_COMPILETIME_MSG(wdIsPodType<T>::value, "in place data needs to be POD");
  wdMemoryUtils::CopyConstruct(reinterpret_cast<T*>(&m_Data), value, 1);

  m_uiType = TypeDeduction<T>::value;
  m_bIsShared = false;
}

template <typename T>
WD_FORCE_INLINE void wdVariant::InitTypedObject(const T& value, wdTraitInt<0>)
{
  typedef typename TypeDeduction<T>::StorageType StorageType;

  WD_CHECK_AT_COMPILETIME_MSG((sizeof(StorageType) > sizeof(InlinedStruct::DataSize)) || TypeDeduction<T>::forceSharing, "Value should be inplace instead.");
  WD_CHECK_AT_COMPILETIME_MSG(TypeDeduction<T>::value == Type::TypedObject, "value of this type cannot be stored in a Variant");
  const wdRTTI* pType = wdGetStaticRTTI<T>();
  m_Data.shared = WD_DEFAULT_NEW(TypedSharedData<StorageType>, value, pType);
  m_uiType = Type::TypedObject;
  m_bIsShared = true;
}

template <typename T>
WD_FORCE_INLINE void wdVariant::InitTypedObject(const T& value, wdTraitInt<1>)
{
  typedef typename TypeDeduction<T>::StorageType StorageType;
  WD_CHECK_AT_COMPILETIME_MSG((sizeof(StorageType) <= InlinedStruct::DataSize) && !TypeDeduction<T>::forceSharing, "Value can't be stored inplace.");
  WD_CHECK_AT_COMPILETIME_MSG(TypeDeduction<T>::value == Type::TypedObject, "value of this type cannot be stored in a Variant");
  WD_CHECK_AT_COMPILETIME_MSG(wdIsPodType<T>::value, "in place data needs to be POD");
  wdMemoryUtils::CopyConstruct(reinterpret_cast<T*>(&m_Data), value, 1);
  m_Data.inlined.m_pType = wdGetStaticRTTI<T>();
  m_uiType = Type::TypedObject;
  m_bIsShared = false;
}

inline void wdVariant::Release()
{
  if (m_bIsShared)
  {
    if (m_Data.shared->m_uiRef.Decrement() == 0)
    {
      WD_DEFAULT_DELETE(m_Data.shared);
    }
  }
}

inline void wdVariant::CopyFrom(const wdVariant& other)
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

WD_ALWAYS_INLINE void wdVariant::MoveFrom(wdVariant&& other)
{
  m_uiType = other.m_uiType;
  m_bIsShared = other.m_bIsShared;
  m_Data = other.m_Data;

  other.m_uiType = Type::Invalid;
  other.m_bIsShared = false;
  other.m_Data.shared = nullptr;
}

template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::DirectCast, int>>
const T& wdVariant::Cast() const
{
  const bool validType = wdConversionTest<T, typename TypeDeduction<T>::StorageType>::sameType;
  WD_CHECK_AT_COMPILETIME_MSG(validType, "Invalid Cast, can only cast to storage type");

  return m_bIsShared ? *static_cast<const T*>(m_Data.shared->m_Ptr) : *reinterpret_cast<const T*>(&m_Data);
}

template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::PointerCast, int>>
T wdVariant::Cast() const
{
  const wdTypedPointer& ptr = *reinterpret_cast<const wdTypedPointer*>(&m_Data);

  const wdRTTI* pType = GetReflectedType();
  typedef typename wdTypeTraits<T>::NonConstReferencePointerType NonRefPtrT;
  if constexpr (!std::is_same<T, void*>::value && !std::is_same<T, const void*>::value)
  {
    WD_ASSERT_DEV(pType == nullptr || IsDerivedFrom(pType, wdGetStaticRTTI<NonRefPtrT>()), "Object of type '{0}' does not derive from '{}'", GetTypeName(pType), GetTypeName(wdGetStaticRTTI<NonRefPtrT>()));
  }
  return static_cast<T>(ptr.m_pObject);
}

template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::TypedObject, int>>
const T wdVariant::Cast() const
{
  wdTypedObject obj;
  obj.m_pObject = GetData();
  obj.m_pType = GetReflectedType();
  return obj;
}

template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::CustomTypeCast, int>>
const T& wdVariant::Cast() const
{
  const wdRTTI* pType = GetReflectedType();
  typedef typename wdTypeTraits<T>::NonConstReferenceType NonRefT;
  WD_ASSERT_DEV(IsDerivedFrom(pType, wdGetStaticRTTI<NonRefT>()), "Object of type '{0}' does not derive from '{}'", GetTypeName(pType), GetTypeName(wdGetStaticRTTI<NonRefT>()));

  return m_bIsShared ? *static_cast<const T*>(m_Data.shared->m_Ptr) : *reinterpret_cast<const T*>(&m_Data);
}

WD_ALWAYS_INLINE bool wdVariant::IsNumberStatic(wdUInt32 type)
{
  return type > Type::Invalid && type <= Type::Double;
}

WD_ALWAYS_INLINE bool wdVariant::IsFloatingPointStatic(wdUInt32 type)
{
  return type == Type::Float || type == Type::Double;
}

WD_ALWAYS_INLINE bool wdVariant::IsStringStatic(wdUInt32 type)
{
  return type == Type::String || type == Type::StringView;
}

WD_ALWAYS_INLINE bool wdVariant::IsVector2Static(wdUInt32 type)
{
  return type == Type::Vector2 || type == Type::Vector2I || type == Type::Vector2U;
}

WD_ALWAYS_INLINE bool wdVariant::IsVector3Static(wdUInt32 type)
{
  return type == Type::Vector3 || type == Type::Vector3I || type == Type::Vector3U;
}

WD_ALWAYS_INLINE bool wdVariant::IsVector4Static(wdUInt32 type)
{
  return type == Type::Vector4 || type == Type::Vector4I || type == Type::Vector4U;
}

template <typename T>
T wdVariant::ConvertNumber() const
{
  switch (m_uiType)
  {
    case Type::Bool:
      return static_cast<T>(Cast<bool>());
    case Type::Int8:
      return static_cast<T>(Cast<wdInt8>());
    case Type::UInt8:
      return static_cast<T>(Cast<wdUInt8>());
    case Type::Int16:
      return static_cast<T>(Cast<wdInt16>());
    case Type::UInt16:
      return static_cast<T>(Cast<wdUInt16>());
    case Type::Int32:
      return static_cast<T>(Cast<wdInt32>());
    case Type::UInt32:
      return static_cast<T>(Cast<wdUInt32>());
    case Type::Int64:
      return static_cast<T>(Cast<wdInt64>());
    case Type::UInt64:
      return static_cast<T>(Cast<wdUInt64>());
    case Type::Float:
      return static_cast<T>(Cast<float>());
    case Type::Double:
      return static_cast<T>(Cast<double>());
  }

  WD_REPORT_FAILURE("Variant is not a number");
  return T(0);
}

template <>
struct wdHashHelper<wdVariant>
{
  WD_ALWAYS_INLINE static wdUInt32 Hash(const wdVariant& value)
  {
    wdUInt64 uiHash = value.ComputeHash(0);
    return (wdUInt32)uiHash;
  }

  WD_ALWAYS_INLINE static bool Equal(const wdVariant& a, const wdVariant& b) { return a == b; }
};
