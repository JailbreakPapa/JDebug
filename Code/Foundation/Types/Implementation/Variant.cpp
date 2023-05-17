#include <Foundation/FoundationPCH.h>

#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Serialization/ReflectionSerializer.h>
#include <Foundation/Types/VariantTypeRegistry.h>

#if WD_ENABLED(WD_PLATFORM_64BIT)
WD_CHECK_AT_COMPILETIME(sizeof(wdVariant) == 24);
#else
WD_CHECK_AT_COMPILETIME(sizeof(wdVariant) == 20);
#endif

/// constructors

wdVariant::wdVariant(const wdMat3& value)
{
  InitShared(value);
}

wdVariant::wdVariant(const wdMat4& value)
{
  InitShared(value);
}

wdVariant::wdVariant(const wdTransform& value)
{
  InitShared(value);
}

wdVariant::wdVariant(const char* value)
{
  InitShared(value);
}

wdVariant::wdVariant(const wdString& value)
{
  InitShared(value);
}

wdVariant::wdVariant(const wdUntrackedString& value)
{
  InitShared(value);
}

wdVariant::wdVariant(const wdDataBuffer& value)
{
  InitShared(value);
}

wdVariant::wdVariant(const wdVariantArray& value)
{
  typedef typename TypeDeduction<wdVariantArray>::StorageType StorageType;
  m_Data.shared = WD_DEFAULT_NEW(TypedSharedData<StorageType>, value, nullptr);
  m_uiType = TypeDeduction<wdVariantArray>::value;
  m_bIsShared = true;
}

wdVariant::wdVariant(const wdVariantDictionary& value)
{
  typedef typename TypeDeduction<wdVariantDictionary>::StorageType StorageType;
  m_Data.shared = WD_DEFAULT_NEW(TypedSharedData<StorageType>, value, nullptr);
  m_uiType = TypeDeduction<wdVariantDictionary>::value;
  m_bIsShared = true;
}

wdVariant::wdVariant(const wdTypedPointer& value)
{
  InitInplace(value);
}

wdVariant::wdVariant(const wdTypedObject& value)
{
  void* ptr = wdReflectionSerializer::Clone(value.m_pObject, value.m_pType);
  m_Data.shared = WD_DEFAULT_NEW(RTTISharedData, ptr, value.m_pType);
  m_uiType = Type::TypedObject;
  m_bIsShared = true;
}

void wdVariant::CopyTypedObject(const void* value, const wdRTTI* pType)
{
  Release();
  void* ptr = wdReflectionSerializer::Clone(value, pType);
  m_Data.shared = WD_DEFAULT_NEW(RTTISharedData, ptr, pType);
  m_uiType = Type::TypedObject;
  m_bIsShared = true;
}

void wdVariant::MoveTypedObject(void* value, const wdRTTI* pType)
{
  Release();
  m_Data.shared = WD_DEFAULT_NEW(RTTISharedData, value, pType);
  m_uiType = Type::TypedObject;
  m_bIsShared = true;
}

template <typename T>
WD_ALWAYS_INLINE void wdVariant::InitShared(const T& value)
{
  typedef typename TypeDeduction<T>::StorageType StorageType;

  WD_CHECK_AT_COMPILETIME_MSG((sizeof(StorageType) > sizeof(Data)) || TypeDeduction<T>::forceSharing, "value of this type should be stored inplace");
  WD_CHECK_AT_COMPILETIME_MSG(TypeDeduction<T>::value != Type::Invalid, "value of this type cannot be stored in a Variant");
  const wdRTTI* pType = wdGetStaticRTTI<T>();

  m_Data.shared = WD_DEFAULT_NEW(TypedSharedData<StorageType>, value, pType);
  m_uiType = TypeDeduction<T>::value;
  m_bIsShared = true;
}

/// functors

struct ComputeHashFunc
{
  template <typename T>
  WD_FORCE_INLINE wdUInt64 operator()(const wdVariant& v, const void* pData, wdUInt64 uiSeed)
  {
    WD_CHECK_AT_COMPILETIME_MSG(sizeof(typename wdVariant::TypeDeduction<T>::StorageType) <= sizeof(float) * 4 &&
                                  !wdVariant::TypeDeduction<T>::forceSharing,
      "This type requires special handling! Add a specialization below.");
    return wdHashingUtils::xxHash64(pData, sizeof(T), uiSeed);
  }
};

template <>
WD_ALWAYS_INLINE wdUInt64 ComputeHashFunc::operator()<wdString>(const wdVariant& v, const void* pData, wdUInt64 uiSeed)
{
  wdString* pString = (wdString*)pData;

  return wdHashingUtils::xxHash64(pString->GetData(), pString->GetElementCount(), uiSeed);
}

template <>
WD_ALWAYS_INLINE wdUInt64 ComputeHashFunc::operator()<wdMat3>(const wdVariant& v, const void* pData, wdUInt64 uiSeed)
{
  return wdHashingUtils::xxHash64(pData, sizeof(wdMat3), uiSeed);
}

template <>
WD_ALWAYS_INLINE wdUInt64 ComputeHashFunc::operator()<wdMat4>(const wdVariant& v, const void* pData, wdUInt64 uiSeed)
{
  return wdHashingUtils::xxHash64(pData, sizeof(wdMat4), uiSeed);
}

template <>
WD_ALWAYS_INLINE wdUInt64 ComputeHashFunc::operator()<wdTransform>(const wdVariant& v, const void* pData, wdUInt64 uiSeed)
{
  return wdHashingUtils::xxHash64(pData, sizeof(wdTransform), uiSeed);
}

template <>
WD_ALWAYS_INLINE wdUInt64 ComputeHashFunc::operator()<wdDataBuffer>(const wdVariant& v, const void* pData, wdUInt64 uiSeed)
{
  wdDataBuffer* pDataBuffer = (wdDataBuffer*)pData;

  return wdHashingUtils::xxHash64(pDataBuffer->GetData(), pDataBuffer->GetCount(), uiSeed);
}

template <>
WD_FORCE_INLINE wdUInt64 ComputeHashFunc::operator()<wdVariantArray>(const wdVariant& v, const void* pData, wdUInt64 uiSeed)
{
  WD_IGNORE_UNUSED(pData);

  WD_ASSERT_NOT_IMPLEMENTED;
  return 0;
}

template <>
WD_FORCE_INLINE wdUInt64 ComputeHashFunc::operator()<wdVariantDictionary>(const wdVariant& v, const void* pData, wdUInt64 uiSeed)
{
  WD_IGNORE_UNUSED(pData);

  WD_ASSERT_NOT_IMPLEMENTED;
  return 0;
}

template <>
WD_FORCE_INLINE wdUInt64 ComputeHashFunc::operator()<wdTypedPointer>(const wdVariant& v, const void* pData, wdUInt64 uiSeed)
{
  WD_IGNORE_UNUSED(pData);

  WD_ASSERT_NOT_IMPLEMENTED;
  return 0;
}

template <>
WD_FORCE_INLINE wdUInt64 ComputeHashFunc::operator()<wdTypedObject>(const wdVariant& v, const void* pData, wdUInt64 uiSeed)
{
  auto pType = v.GetReflectedType();

  const wdVariantTypeInfo* pTypeInfo = wdVariantTypeRegistry::GetSingleton()->FindVariantTypeInfo(pType);
  WD_ASSERT_DEV(pTypeInfo, "The type '{0}' was declared but not defined, add WD_DEFINE_CUSTOM_VARIANT_TYPE({0}); to a cpp to enable comparing of this variant type.", pType->GetTypeName());
  WD_MSVC_ANALYSIS_ASSUME(pTypeInfo != nullptr);
  wdUInt32 uiHash32 = pTypeInfo->Hash(pData);

  return wdHashingUtils::xxHash64(&uiHash32, sizeof(wdUInt32), uiSeed);
}

struct CompareFunc
{
  template <typename T>
  WD_ALWAYS_INLINE void operator()()
  {
    m_bResult = m_pThis->Cast<T>() == m_pOther->Cast<T>();
  }

  const wdVariant* m_pThis;
  const wdVariant* m_pOther;
  bool m_bResult;
};

template <>
WD_FORCE_INLINE void CompareFunc::operator()<wdTypedObject>()
{
  m_bResult = false;
  wdTypedObject A = m_pThis->Get<wdTypedObject>();
  wdTypedObject B = m_pOther->Get<wdTypedObject>();
  if (A.m_pType == B.m_pType)
  {
    const wdVariantTypeInfo* pTypeInfo = wdVariantTypeRegistry::GetSingleton()->FindVariantTypeInfo(A.m_pType);
    WD_ASSERT_DEV(pTypeInfo, "The type '{0}' was declared but not defined, add WD_DEFINE_CUSTOM_VARIANT_TYPE({0}); to a cpp to enable comparing of this variant type.", A.m_pType->GetTypeName());
    WD_MSVC_ANALYSIS_ASSUME(pTypeInfo != nullptr);
    m_bResult = pTypeInfo->Equal(A.m_pObject, B.m_pObject);
  }
}

struct IndexFunc
{
  template <typename T>
  WD_FORCE_INLINE wdVariant Impl(wdTraitInt<1>)
  {
    const wdRTTI* pRtti = m_pThis->GetReflectedType();
    wdAbstractMemberProperty* pProp = wdReflectionUtils::GetMemberProperty(pRtti, m_uiIndex);
    if (!pProp)
      return wdVariant();

    if (m_pThis->GetType() == wdVariantType::TypedPointer)
    {
      const wdTypedPointer& ptr = m_pThis->Get<wdTypedPointer>();
      if (ptr.m_pObject)
        return wdReflectionUtils::GetMemberPropertyValue(pProp, ptr.m_pObject);
      else
        return wdVariant();
    }
    return wdReflectionUtils::GetMemberPropertyValue(pProp, m_pThis->GetData());
  }

  template <typename T>
  WD_ALWAYS_INLINE wdVariant Impl(wdTraitInt<0>)
  {
    return wdVariant();
  }

  template <typename T>
  WD_FORCE_INLINE void operator()()
  {
    m_Result = Impl<T>(wdTraitInt<wdVariant::TypeDeduction<T>::hasReflectedMembers>());
  }

  const wdVariant* m_pThis;
  wdVariant m_Result;
  wdUInt32 m_uiIndex;
};

struct KeyFunc
{
  template <typename T>
  WD_FORCE_INLINE wdVariant Impl(wdTraitInt<1>)
  {
    const wdRTTI* pRtti = m_pThis->GetReflectedType();
    wdAbstractMemberProperty* pProp = wdReflectionUtils::GetMemberProperty(pRtti, m_szKey);
    if (!pProp)
      return wdVariant();
    if (m_pThis->GetType() == wdVariantType::TypedPointer)
    {
      const wdTypedPointer& ptr = m_pThis->Get<wdTypedPointer>();
      if (ptr.m_pObject)
        return wdReflectionUtils::GetMemberPropertyValue(pProp, ptr.m_pObject);
      else
        return wdVariant();
    }
    return wdReflectionUtils::GetMemberPropertyValue(pProp, m_pThis->GetData());
  }

  template <typename T>
  WD_ALWAYS_INLINE wdVariant Impl(wdTraitInt<0>)
  {
    return wdVariant();
  }

  template <typename T>
  WD_ALWAYS_INLINE void operator()()
  {
    m_Result = Impl<T>(wdTraitInt<wdVariant::TypeDeduction<T>::hasReflectedMembers>());
  }

  const wdVariant* m_pThis;
  wdVariant m_Result;
  const char* m_szKey;
};

struct ConvertFunc
{
  template <typename T>
  WD_ALWAYS_INLINE void operator()()
  {
    T result;
    wdVariantHelper::To(*m_pThis, result, m_bSuccessful);
    m_Result = result;
  }

  const wdVariant* m_pThis;
  wdVariant m_Result;
  bool m_bSuccessful;
};

/// public methods

bool wdVariant::operator==(const wdVariant& other) const
{
  if (m_uiType == Type::Invalid && other.m_uiType == Type::Invalid)
  {
    return true;
  }
  else if ((IsFloatingPoint() && other.IsNumber()) || (other.IsFloatingPoint() && IsNumber()))
  {
    // if either of them is a floating point number, compare them as doubles

    return ConvertNumber<double>() == other.ConvertNumber<double>();
  }
  else if (IsNumber() && other.IsNumber())
  {
    return ConvertNumber<wdInt64>() == other.ConvertNumber<wdInt64>();
  }
  else if (IsString() && other.IsString())
  {
    const wdStringView a = IsA<wdStringView>() ? Get<wdStringView>() : wdStringView(Get<wdString>().GetData());
    const wdStringView b = other.IsA<wdStringView>() ? other.Get<wdStringView>() : wdStringView(other.Get<wdString>().GetData());
    return a.IsEqual(b);
  }
  else if (m_uiType == other.m_uiType)
  {
    CompareFunc compareFunc;
    compareFunc.m_pThis = this;
    compareFunc.m_pOther = &other;

    DispatchTo(compareFunc, GetType());

    return compareFunc.m_bResult;
  }

  return false;
}

wdTypedPointer wdVariant::GetWriteAccess()
{
  wdTypedPointer obj;
  obj.m_pType = GetReflectedType();
  if (m_bIsShared)
  {
    if (m_Data.shared->m_uiRef > 1)
    {
      // We need to make sure we hold the only reference to the shared data to be able to edit it.
      SharedData* pData = m_Data.shared->Clone();
      Release();
      m_Data.shared = pData;
    }
    obj.m_pObject = m_Data.shared->m_Ptr;
  }
  else
  {
    obj.m_pObject = m_uiType == Type::TypedPointer ? Cast<wdTypedPointer>().m_pObject : &m_Data;
  }
  return obj;
}

const wdVariant wdVariant::operator[](wdUInt32 uiIndex) const
{
  if (m_uiType == Type::VariantArray)
  {
    const wdVariantArray& a = Cast<wdVariantArray>();
    if (uiIndex < a.GetCount())
      return a[uiIndex];
  }
  else if (IsValid())
  {
    IndexFunc func;
    func.m_pThis = this;
    func.m_uiIndex = uiIndex;

    DispatchTo(func, GetType());

    return func.m_Result;
  }

  return wdVariant();
}

const wdVariant wdVariant::operator[](StringWrapper key) const
{
  if (m_uiType == Type::VariantDictionary)
  {
    wdVariant result;
    Cast<wdVariantDictionary>().TryGetValue(key.m_str, result);
    return result;
  }
  else if (IsValid())
  {
    KeyFunc func;
    func.m_pThis = this;
    func.m_szKey = key.m_str;

    DispatchTo(func, GetType());

    return func.m_Result;
  }

  return wdVariant();
}

bool wdVariant::CanConvertTo(Type::Enum type) const
{
  if (m_uiType == type)
    return true;

  if (!IsValid() || type == Type::Invalid)
    return false;

  if (IsNumberStatic(type) && (IsNumber() || m_uiType == Type::String))
    return true;

  if (IsVector2Static(type) && (IsVector2Static(m_uiType)))
    return true;

  if (IsVector3Static(type) && (IsVector3Static(m_uiType)))
    return true;

  if (IsVector4Static(type) && (IsVector4Static(m_uiType)))
    return true;

  if (type == Type::String && m_uiType < Type::LastStandardType && m_uiType != Type::DataBuffer)
    return true;
  if (type == Type::String && m_uiType == Type::VariantArray)
    return true;
  if (type == Type::Color && m_uiType == Type::ColorGamma)
    return true;
  if (type == Type::ColorGamma && m_uiType == Type::Color)
    return true;

  if (type == Type::TypedPointer && m_uiType == Type::TypedPointer)
    return true;
  if (type == Type::TypedObject && m_uiType == Type::TypedObject)
    return true;

  return false;
}

wdVariant wdVariant::ConvertTo(Type::Enum type, wdResult* out_pConversionStatus /* = nullptr*/) const
{
  if (!CanConvertTo(type))
  {
    if (out_pConversionStatus != nullptr)
      *out_pConversionStatus = WD_FAILURE;

    return wdVariant(); // creates an invalid variant
  }

  if (m_uiType == type)
  {
    if (out_pConversionStatus != nullptr)
      *out_pConversionStatus = WD_SUCCESS;

    return *this;
  }

  ConvertFunc convertFunc;
  convertFunc.m_pThis = this;
  convertFunc.m_bSuccessful = true;

  DispatchTo(convertFunc, type);

  if (out_pConversionStatus != nullptr)
    *out_pConversionStatus = convertFunc.m_bSuccessful ? WD_SUCCESS : WD_FAILURE;

  return convertFunc.m_Result;
}

wdUInt64 wdVariant::ComputeHash(wdUInt64 uiSeed) const
{
  if (!IsValid())
    return uiSeed;

  ComputeHashFunc obj;
  return DispatchTo<ComputeHashFunc>(obj, GetType(), *this, GetData(), uiSeed);
}


inline wdVariant::RTTISharedData::RTTISharedData(void* pData, const wdRTTI* pType)
  : SharedData(pData, pType)
{
  WD_ASSERT_DEBUG(pType != nullptr && pType->GetAllocator()->CanAllocate(), "");
}

inline wdVariant::RTTISharedData::~RTTISharedData()
{
  m_pType->GetAllocator()->Deallocate(m_Ptr);
}


wdVariant::wdVariant::SharedData* wdVariant::RTTISharedData::Clone() const
{
  void* ptr = wdReflectionSerializer::Clone(m_Ptr, m_pType);
  return WD_DEFAULT_NEW(RTTISharedData, ptr, m_pType);
}

struct GetTypeFromVariantFunc
{
  template <typename T>
  WD_ALWAYS_INLINE void operator()()
  {
    m_pType = wdGetStaticRTTI<T>();
  }

  const wdVariant* m_pVariant;
  const wdRTTI* m_pType;
};

template <>
WD_ALWAYS_INLINE void GetTypeFromVariantFunc::operator()<wdVariantArray>()
{
  m_pType = nullptr;
}
template <>
WD_ALWAYS_INLINE void GetTypeFromVariantFunc::operator()<wdVariantDictionary>()
{
  m_pType = nullptr;
}
template <>
WD_ALWAYS_INLINE void GetTypeFromVariantFunc::operator()<wdTypedPointer>()
{
  m_pType = m_pVariant->Cast<wdTypedPointer>().m_pType;
}
template <>
WD_ALWAYS_INLINE void GetTypeFromVariantFunc::operator()<wdTypedObject>()
{
  m_pType = m_pVariant->m_bIsShared ? m_pVariant->m_Data.shared->m_pType : m_pVariant->m_Data.inlined.m_pType;
}

const wdRTTI* wdVariant::GetReflectedType() const
{
  if (m_uiType != Type::Invalid)
  {
    GetTypeFromVariantFunc func;
    func.m_pVariant = this;
    func.m_pType = nullptr;
    wdVariant::DispatchTo(func, GetType());
    return func.m_pType;
  }
  return nullptr;
}

void wdVariant::InitTypedPointer(void* value, const wdRTTI* pType)
{
  wdTypedPointer ptr;
  ptr.m_pObject = value;
  ptr.m_pType = pType;

  wdMemoryUtils::CopyConstruct(reinterpret_cast<wdTypedPointer*>(&m_Data), ptr, 1);

  m_uiType = TypeDeduction<wdTypedPointer>::value;
  m_bIsShared = false;
}

bool wdVariant::IsDerivedFrom(const wdRTTI* pType1, const wdRTTI* pType2)
{
  return pType1->IsDerivedFrom(pType2);
}

const char* wdVariant::GetTypeName(const wdRTTI* pType)
{
  return pType->GetTypeName();
}

WD_STATICLINK_FILE(Foundation, Foundation_Types_Implementation_Variant);
