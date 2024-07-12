#include <Foundation/FoundationPCH.h>

#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Serialization/ReflectionSerializer.h>
#include <Foundation/Types/VariantTypeRegistry.h>

#if NS_ENABLED(NS_PLATFORM_64BIT)
NS_CHECK_AT_COMPILETIME(sizeof(nsVariant) == 24);
#else
NS_CHECK_AT_COMPILETIME(sizeof(nsVariant) == 20);
#endif

/// constructors

nsVariant::nsVariant(const nsMat3& value)
{
  InitShared(value);
}

nsVariant::nsVariant(const nsMat4& value)
{
  InitShared(value);
}

nsVariant::nsVariant(const nsTransform& value)
{
  InitShared(value);
}

nsVariant::nsVariant(const char* value)
{
  InitShared(value);
}

nsVariant::nsVariant(const nsString& value)
{
  InitShared(value);
}

nsVariant::nsVariant(const nsStringView& value, bool bCopyString)
{
  if (bCopyString)
    InitShared(nsString(value));
  else
    InitInplace(value);
}

nsVariant::nsVariant(const nsUntrackedString& value)
{
  InitShared(value);
}

nsVariant::nsVariant(const nsDataBuffer& value)
{
  InitShared(value);
}

nsVariant::nsVariant(const nsVariantArray& value)
{
  using StorageType = typename TypeDeduction<nsVariantArray>::StorageType;
  m_Data.shared = NS_DEFAULT_NEW(TypedSharedData<StorageType>, value, nullptr);
  m_uiType = TypeDeduction<nsVariantArray>::value;
  m_bIsShared = true;
}

nsVariant::nsVariant(const nsVariantDictionary& value)
{
  using StorageType = typename TypeDeduction<nsVariantDictionary>::StorageType;
  m_Data.shared = NS_DEFAULT_NEW(TypedSharedData<StorageType>, value, nullptr);
  m_uiType = TypeDeduction<nsVariantDictionary>::value;
  m_bIsShared = true;
}

nsVariant::nsVariant(const nsTypedPointer& value)
{
  InitInplace(value);
}

nsVariant::nsVariant(const nsTypedObject& value)
{
  void* ptr = nsReflectionSerializer::Clone(value.m_pObject, value.m_pType);
  m_Data.shared = NS_DEFAULT_NEW(RTTISharedData, ptr, value.m_pType);
  m_uiType = Type::TypedObject;
  m_bIsShared = true;
}

void nsVariant::CopyTypedObject(const void* value, const nsRTTI* pType)
{
  Release();
  void* ptr = nsReflectionSerializer::Clone(value, pType);
  m_Data.shared = NS_DEFAULT_NEW(RTTISharedData, ptr, pType);
  m_uiType = Type::TypedObject;
  m_bIsShared = true;
}

void nsVariant::MoveTypedObject(void* value, const nsRTTI* pType)
{
  Release();
  m_Data.shared = NS_DEFAULT_NEW(RTTISharedData, value, pType);
  m_uiType = Type::TypedObject;
  m_bIsShared = true;
}

template <typename T>
NS_ALWAYS_INLINE void nsVariant::InitShared(const T& value)
{
  using StorageType = typename TypeDeduction<T>::StorageType;

  NS_CHECK_AT_COMPILETIME_MSG((sizeof(StorageType) > sizeof(Data)) || TypeDeduction<T>::forceSharing, "value of this type should be stored inplace");
  NS_CHECK_AT_COMPILETIME_MSG(TypeDeduction<T>::value != Type::Invalid, "value of this type cannot be stored in a Variant");
  const nsRTTI* pType = nsGetStaticRTTI<T>();

  m_Data.shared = NS_DEFAULT_NEW(TypedSharedData<StorageType>, value, pType);
  m_uiType = TypeDeduction<T>::value;
  m_bIsShared = true;
}

/// functors

struct ComputeHashFunc
{
  template <typename T>
  NS_FORCE_INLINE nsUInt64 operator()(const nsVariant& v, const void* pData, nsUInt64 uiSeed)
  {
    NS_CHECK_AT_COMPILETIME_MSG(sizeof(typename nsVariant::TypeDeduction<T>::StorageType) <= sizeof(float) * 4 &&
                                  !nsVariant::TypeDeduction<T>::forceSharing,
      "This type requires special handling! Add a specialization below.");
    return nsHashingUtils::xxHash64(pData, sizeof(T), uiSeed);
  }
};

template <>
NS_ALWAYS_INLINE nsUInt64 ComputeHashFunc::operator()<nsString>(const nsVariant& v, const void* pData, nsUInt64 uiSeed)
{
  auto pString = static_cast<const nsString*>(pData);

  return nsHashingUtils::xxHash64String(*pString, uiSeed);
}

template <>
NS_ALWAYS_INLINE nsUInt64 ComputeHashFunc::operator()<nsMat3>(const nsVariant& v, const void* pData, nsUInt64 uiSeed)
{
  return nsHashingUtils::xxHash64(pData, sizeof(nsMat3), uiSeed);
}

template <>
NS_ALWAYS_INLINE nsUInt64 ComputeHashFunc::operator()<nsMat4>(const nsVariant& v, const void* pData, nsUInt64 uiSeed)
{
  return nsHashingUtils::xxHash64(pData, sizeof(nsMat4), uiSeed);
}

template <>
NS_ALWAYS_INLINE nsUInt64 ComputeHashFunc::operator()<nsTransform>(const nsVariant& v, const void* pData, nsUInt64 uiSeed)
{
  return nsHashingUtils::xxHash64(pData, sizeof(nsTransform), uiSeed);
}

template <>
NS_ALWAYS_INLINE nsUInt64 ComputeHashFunc::operator()<nsDataBuffer>(const nsVariant& v, const void* pData, nsUInt64 uiSeed)
{
  auto pDataBuffer = static_cast<const nsDataBuffer*>(pData);

  return nsHashingUtils::xxHash64(pDataBuffer->GetData(), pDataBuffer->GetCount(), uiSeed);
}

template <>
NS_FORCE_INLINE nsUInt64 ComputeHashFunc::operator()<nsVariantArray>(const nsVariant& v, const void* pData, nsUInt64 uiSeed)
{
  auto pVariantArray = static_cast<const nsVariantArray*>(pData);

  nsUInt64 uiHash = uiSeed;
  for (const nsVariant& var : *pVariantArray)
  {
    uiHash = var.ComputeHash(uiHash);
  }

  return uiHash;
}

template <>
nsUInt64 ComputeHashFunc::operator()<nsVariantDictionary>(const nsVariant& v, const void* pData, nsUInt64 uiSeed)
{
  auto pVariantDictionary = static_cast<const nsVariantDictionary*>(pData);

  nsHybridArray<nsUInt64, 128> hashes;
  hashes.Reserve(pVariantDictionary->GetCount() * 2);

  for (auto& it : *pVariantDictionary)
  {
    hashes.PushBack(nsHashingUtils::xxHash64String(it.Key(), uiSeed));
    hashes.PushBack(it.Value().ComputeHash(uiSeed));
  }

  hashes.Sort();

  return nsHashingUtils::xxHash64(hashes.GetData(), hashes.GetCount() * sizeof(nsUInt64), uiSeed);
}

template <>
NS_FORCE_INLINE nsUInt64 ComputeHashFunc::operator()<nsTypedPointer>(const nsVariant& v, const void* pData, nsUInt64 uiSeed)
{
  NS_IGNORE_UNUSED(pData);

  NS_ASSERT_NOT_IMPLEMENTED;
  return 0;
}

template <>
NS_FORCE_INLINE nsUInt64 ComputeHashFunc::operator()<nsTypedObject>(const nsVariant& v, const void* pData, nsUInt64 uiSeed)
{
  auto pType = v.GetReflectedType();

  const nsVariantTypeInfo* pTypeInfo = nsVariantTypeRegistry::GetSingleton()->FindVariantTypeInfo(pType);
  NS_ASSERT_DEV(pTypeInfo, "The type '{0}' was declared but not defined, add NS_DEFINE_CUSTOM_VARIANT_TYPE({0}); to a cpp to enable comparing of this variant type.", pType->GetTypeName());
  NS_MSVC_ANALYSIS_ASSUME(pTypeInfo != nullptr);
  nsUInt32 uiHash32 = pTypeInfo->Hash(pData);

  return nsHashingUtils::xxHash64(&uiHash32, sizeof(nsUInt32), uiSeed);
}

struct CompareFunc
{
  template <typename T>
  NS_ALWAYS_INLINE void operator()()
  {
    m_bResult = m_pThis->Cast<T>() == m_pOther->Cast<T>();
  }

  const nsVariant* m_pThis;
  const nsVariant* m_pOther;
  bool m_bResult;
};

template <>
NS_FORCE_INLINE void CompareFunc::operator()<nsTypedObject>()
{
  m_bResult = false;
  nsTypedObject A = m_pThis->Get<nsTypedObject>();
  nsTypedObject B = m_pOther->Get<nsTypedObject>();
  if (A.m_pType == B.m_pType)
  {
    const nsVariantTypeInfo* pTypeInfo = nsVariantTypeRegistry::GetSingleton()->FindVariantTypeInfo(A.m_pType);
    NS_ASSERT_DEV(pTypeInfo, "The type '{0}' was declared but not defined, add NS_DEFINE_CUSTOM_VARIANT_TYPE({0}); to a cpp to enable comparing of this variant type.", A.m_pType->GetTypeName());
    NS_MSVC_ANALYSIS_ASSUME(pTypeInfo != nullptr);
    m_bResult = pTypeInfo->Equal(A.m_pObject, B.m_pObject);
  }
}

struct IndexFunc
{
  template <typename T>
  NS_FORCE_INLINE nsVariant Impl(nsTraitInt<1>)
  {
    const nsRTTI* pRtti = m_pThis->GetReflectedType();
    const nsAbstractMemberProperty* pProp = nsReflectionUtils::GetMemberProperty(pRtti, m_uiIndex);
    if (!pProp)
      return nsVariant();

    if (m_pThis->GetType() == nsVariantType::TypedPointer)
    {
      const nsTypedPointer& ptr = m_pThis->Get<nsTypedPointer>();
      if (ptr.m_pObject)
        return nsReflectionUtils::GetMemberPropertyValue(pProp, ptr.m_pObject);
      else
        return nsVariant();
    }
    return nsReflectionUtils::GetMemberPropertyValue(pProp, m_pThis->GetData());
  }

  template <typename T>
  NS_ALWAYS_INLINE nsVariant Impl(nsTraitInt<0>)
  {
    return nsVariant();
  }

  template <typename T>
  NS_FORCE_INLINE void operator()()
  {
    m_Result = Impl<T>(nsTraitInt<nsVariant::TypeDeduction<T>::hasReflectedMembers>());
  }

  const nsVariant* m_pThis;
  nsVariant m_Result;
  nsUInt32 m_uiIndex;
};

struct KeyFunc
{
  template <typename T>
  NS_FORCE_INLINE nsVariant Impl(nsTraitInt<1>)
  {
    const nsRTTI* pRtti = m_pThis->GetReflectedType();
    const nsAbstractMemberProperty* pProp = nsReflectionUtils::GetMemberProperty(pRtti, m_szKey);
    if (!pProp)
      return nsVariant();
    if (m_pThis->GetType() == nsVariantType::TypedPointer)
    {
      const nsTypedPointer& ptr = m_pThis->Get<nsTypedPointer>();
      if (ptr.m_pObject)
        return nsReflectionUtils::GetMemberPropertyValue(pProp, ptr.m_pObject);
      else
        return nsVariant();
    }
    return nsReflectionUtils::GetMemberPropertyValue(pProp, m_pThis->GetData());
  }

  template <typename T>
  NS_ALWAYS_INLINE nsVariant Impl(nsTraitInt<0>)
  {
    return nsVariant();
  }

  template <typename T>
  NS_ALWAYS_INLINE void operator()()
  {
    m_Result = Impl<T>(nsTraitInt<nsVariant::TypeDeduction<T>::hasReflectedMembers>());
  }

  const nsVariant* m_pThis;
  nsVariant m_Result;
  const char* m_szKey;
};

struct ConvertFunc
{
  template <typename T>
  NS_ALWAYS_INLINE void operator()()
  {
    T result = {};
    nsVariantHelper::To(*m_pThis, result, m_bSuccessful);

    if constexpr (std::is_same_v<T, nsStringView>)
    {
      m_Result = nsVariant(result, false);
    }
    else
    {
      m_Result = result;
    }
  }

  const nsVariant* m_pThis;
  nsVariant m_Result;
  bool m_bSuccessful;
};

/// public methods

bool nsVariant::operator==(const nsVariant& other) const
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
    return ConvertNumber<nsInt64>() == other.ConvertNumber<nsInt64>();
  }
  else if (IsString() && other.IsString())
  {
    const nsStringView a = IsA<nsStringView>() ? Get<nsStringView>() : nsStringView(Get<nsString>().GetData());
    const nsStringView b = other.IsA<nsStringView>() ? other.Get<nsStringView>() : nsStringView(other.Get<nsString>().GetData());
    return a.IsEqual(b);
  }
  else if (IsHashedString() && other.IsHashedString())
  {
    const nsTempHashedString a = IsA<nsTempHashedString>() ? Get<nsTempHashedString>() : nsTempHashedString(Get<nsHashedString>());
    const nsTempHashedString b = other.IsA<nsTempHashedString>() ? other.Get<nsTempHashedString>() : nsTempHashedString(other.Get<nsHashedString>());
    return a == b;
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

nsTypedPointer nsVariant::GetWriteAccess()
{
  nsTypedPointer obj;
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
    obj.m_pObject = m_uiType == Type::TypedPointer ? Cast<nsTypedPointer>().m_pObject : &m_Data;
  }
  return obj;
}

const nsVariant nsVariant::operator[](nsUInt32 uiIndex) const
{
  if (m_uiType == Type::VariantArray)
  {
    const nsVariantArray& a = Cast<nsVariantArray>();
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

  return nsVariant();
}

const nsVariant nsVariant::operator[](StringWrapper key) const
{
  if (m_uiType == Type::VariantDictionary)
  {
    nsVariant result;
    Cast<nsVariantDictionary>().TryGetValue(key.m_str, result);
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

  return nsVariant();
}

bool nsVariant::CanConvertTo(Type::Enum type) const
{
  if (m_uiType == type)
    return true;

  if (type == Type::Invalid)
    return false;

  const bool bTargetIsString = (type == Type::String) || (type == Type::HashedString) || (type == Type::TempHashedString);

  if (bTargetIsString && m_uiType == Type::Invalid)
    return true;

  if (bTargetIsString && (m_uiType > Type::FirstStandardType && m_uiType < Type::LastStandardType && m_uiType != Type::DataBuffer))
    return true;
  if (bTargetIsString && (m_uiType == Type::VariantArray || m_uiType == Type::VariantDictionary))
    return true;
  if (type == Type::StringView && (m_uiType == Type::String || m_uiType == Type::HashedString))
    return true;
  if (type == Type::TempHashedString && m_uiType == Type::HashedString)
    return true;

  if (!IsValid())
    return false;

  if (IsNumberStatic(type) && (IsNumber() || m_uiType == Type::String || m_uiType == Type::HashedString))
    return true;

  if (IsVector2Static(type) && (IsVector2Static(m_uiType)))
    return true;

  if (IsVector3Static(type) && (IsVector3Static(m_uiType)))
    return true;

  if (IsVector4Static(type) && (IsVector4Static(m_uiType)))
    return true;

  if (type == Type::Color && m_uiType == Type::ColorGamma)
    return true;
  if (type == Type::ColorGamma && m_uiType == Type::Color)
    return true;

  return false;
}

nsVariant nsVariant::ConvertTo(Type::Enum type, nsResult* out_pConversionStatus /* = nullptr*/) const
{
  if (!CanConvertTo(type))
  {
    if (out_pConversionStatus != nullptr)
      *out_pConversionStatus = NS_FAILURE;

    return nsVariant(); // creates an invalid variant
  }

  if (m_uiType == type)
  {
    if (out_pConversionStatus != nullptr)
      *out_pConversionStatus = NS_SUCCESS;

    return *this;
  }

  ConvertFunc convertFunc;
  convertFunc.m_pThis = this;
  convertFunc.m_bSuccessful = true;

  DispatchTo(convertFunc, type);

  if (out_pConversionStatus != nullptr)
    *out_pConversionStatus = convertFunc.m_bSuccessful ? NS_SUCCESS : NS_FAILURE;

  return convertFunc.m_Result;
}

nsUInt64 nsVariant::ComputeHash(nsUInt64 uiSeed) const
{
  if (!IsValid())
    return uiSeed;

  ComputeHashFunc obj;
  return DispatchTo<ComputeHashFunc>(obj, GetType(), *this, GetData(), uiSeed + GetType());
}


inline nsVariant::RTTISharedData::RTTISharedData(void* pData, const nsRTTI* pType)
  : SharedData(pData, pType)
{
  NS_ASSERT_DEBUG(pType != nullptr && pType->GetAllocator()->CanAllocate(), "");
}

inline nsVariant::RTTISharedData::~RTTISharedData()
{
  m_pType->GetAllocator()->Deallocate(m_Ptr);
}


nsVariant::nsVariant::SharedData* nsVariant::RTTISharedData::Clone() const
{
  void* ptr = nsReflectionSerializer::Clone(m_Ptr, m_pType);
  return NS_DEFAULT_NEW(RTTISharedData, ptr, m_pType);
}

struct GetTypeFromVariantFunc
{
  template <typename T>
  NS_ALWAYS_INLINE void operator()()
  {
    m_pType = nsGetStaticRTTI<T>();
  }

  const nsVariant* m_pVariant;
  const nsRTTI* m_pType;
};

template <>
NS_ALWAYS_INLINE void GetTypeFromVariantFunc::operator()<nsVariantArray>()
{
  m_pType = nullptr;
}
template <>
NS_ALWAYS_INLINE void GetTypeFromVariantFunc::operator()<nsVariantDictionary>()
{
  m_pType = nullptr;
}
template <>
NS_ALWAYS_INLINE void GetTypeFromVariantFunc::operator()<nsTypedPointer>()
{
  m_pType = m_pVariant->Cast<nsTypedPointer>().m_pType;
}
template <>
NS_ALWAYS_INLINE void GetTypeFromVariantFunc::operator()<nsTypedObject>()
{
  m_pType = m_pVariant->m_bIsShared ? m_pVariant->m_Data.shared->m_pType : m_pVariant->m_Data.inlined.m_pType;
}

const nsRTTI* nsVariant::GetReflectedType() const
{
  if (m_uiType != Type::Invalid)
  {
    GetTypeFromVariantFunc func;
    func.m_pVariant = this;
    func.m_pType = nullptr;
    nsVariant::DispatchTo(func, GetType());
    return func.m_pType;
  }
  return nullptr;
}

void nsVariant::InitTypedPointer(void* value, const nsRTTI* pType)
{
  nsTypedPointer ptr;
  ptr.m_pObject = value;
  ptr.m_pType = pType;

  nsMemoryUtils::CopyConstruct(reinterpret_cast<nsTypedPointer*>(&m_Data), ptr, 1);

  m_uiType = TypeDeduction<nsTypedPointer>::value;
  m_bIsShared = false;
}

bool nsVariant::IsDerivedFrom(const nsRTTI* pType1, const nsRTTI* pType2)
{
  return pType1->IsDerivedFrom(pType2);
}

nsStringView nsVariant::GetTypeName(const nsRTTI* pType)
{
  return pType->GetTypeName();
}

//////////////////////////////////////////////////////////////////////////

struct AddFunc
{
  template <typename T>
  NS_ALWAYS_INLINE void operator()(const nsVariant& a, const nsVariant& b, nsVariant& out_res)
  {
    if constexpr (std::is_same_v<T, nsInt8> || std::is_same_v<T, nsUInt8> ||
                  std::is_same_v<T, nsInt16> || std::is_same_v<T, nsUInt16> ||
                  std::is_same_v<T, nsInt32> || std::is_same_v<T, nsUInt32> ||
                  std::is_same_v<T, nsInt64> || std::is_same_v<T, nsUInt64> ||
                  std::is_same_v<T, float> || std::is_same_v<T, double> ||
                  std::is_same_v<T, nsColor> ||
                  std::is_same_v<T, nsVec2> || std::is_same_v<T, nsVec3> || std::is_same_v<T, nsVec4> ||
                  std::is_same_v<T, nsVec2I32> || std::is_same_v<T, nsVec3I32> || std::is_same_v<T, nsVec4I32> ||
                  std::is_same_v<T, nsVec2U32> || std::is_same_v<T, nsVec3U32> || std::is_same_v<T, nsVec4U32> ||
                  std::is_same_v<T, nsTime> ||
                  std::is_same_v<T, nsAngle>)
    {
      out_res = a.Get<T>() + b.Get<T>();
    }
    else if constexpr (std::is_same_v<T, nsString> || std::is_same_v<T, nsStringView>)
    {
      nsStringBuilder s;
      s.Set(a.Get<T>(), b.Get<T>());
      out_res = nsString(s.GetView());
    }
    else if constexpr (std::is_same_v<T, nsHashedString>)
    {
      nsStringBuilder s;
      s.Set(a.Get<T>(), b.Get<T>());

      nsHashedString hashedS;
      hashedS.Assign(s);
      out_res = hashedS;
    }
  }
};

nsVariant operator+(const nsVariant& a, const nsVariant& b)
{
  if (a.IsNumber() && b.IsNumber())
  {
    auto biggerType = nsMath::Max(a.GetType(), b.GetType());

    AddFunc func;
    nsVariant result;
    nsVariant::DispatchTo(func, biggerType, a.ConvertTo(biggerType), b.ConvertTo(biggerType), result);
    return result;
  }
  else if (a.GetType() == b.GetType())
  {
    AddFunc func;
    nsVariant result;
    nsVariant::DispatchTo(func, a.GetType(), a, b, result);
    return result;
  }

  return nsVariant();
}

//////////////////////////////////////////////////////////////////////////

struct SubFunc
{
  template <typename T>
  NS_ALWAYS_INLINE void operator()(const nsVariant& a, const nsVariant& b, nsVariant& out_res)
  {
    if constexpr (std::is_same_v<T, nsInt8> || std::is_same_v<T, nsUInt8> ||
                  std::is_same_v<T, nsInt16> || std::is_same_v<T, nsUInt16> ||
                  std::is_same_v<T, nsInt32> || std::is_same_v<T, nsUInt32> ||
                  std::is_same_v<T, nsInt64> || std::is_same_v<T, nsUInt64> ||
                  std::is_same_v<T, float> || std::is_same_v<T, double> ||
                  std::is_same_v<T, nsColor> ||
                  std::is_same_v<T, nsVec2> || std::is_same_v<T, nsVec3> || std::is_same_v<T, nsVec4> ||
                  std::is_same_v<T, nsVec2I32> || std::is_same_v<T, nsVec3I32> || std::is_same_v<T, nsVec4I32> ||
                  std::is_same_v<T, nsVec2U32> || std::is_same_v<T, nsVec3U32> || std::is_same_v<T, nsVec4U32> ||
                  std::is_same_v<T, nsTime> ||
                  std::is_same_v<T, nsAngle>)
    {
      out_res = a.Get<T>() - b.Get<T>();
    }
  }
};

nsVariant operator-(const nsVariant& a, const nsVariant& b)
{
  if (a.IsNumber() && b.IsNumber())
  {
    auto biggerType = nsMath::Max(a.GetType(), b.GetType());

    SubFunc func;
    nsVariant result;
    nsVariant::DispatchTo(func, biggerType, a.ConvertTo(biggerType), b.ConvertTo(biggerType), result);
    return result;
  }
  else if (a.GetType() == b.GetType())
  {
    SubFunc func;
    nsVariant result;
    nsVariant::DispatchTo(func, a.GetType(), a, b, result);
    return result;
  }

  return nsVariant();
}

//////////////////////////////////////////////////////////////////////////

struct MulFunc
{
  template <typename T>
  NS_ALWAYS_INLINE void operator()(const nsVariant& a, const nsVariant& b, nsVariant& out_res)
  {
    if constexpr (std::is_same_v<T, nsInt8> || std::is_same_v<T, nsUInt8> ||
                  std::is_same_v<T, nsInt16> || std::is_same_v<T, nsUInt16> ||
                  std::is_same_v<T, nsInt32> || std::is_same_v<T, nsUInt32> ||
                  std::is_same_v<T, nsInt64> || std::is_same_v<T, nsUInt64> ||
                  std::is_same_v<T, float> || std::is_same_v<T, double> ||
                  std::is_same_v<T, nsColor> ||
                  std::is_same_v<T, nsTime>)
    {
      out_res = a.Get<T>() * b.Get<T>();
    }
    else if constexpr (std::is_same_v<T, nsVec2> || std::is_same_v<T, nsVec3> || std::is_same_v<T, nsVec4> ||
                       std::is_same_v<T, nsVec2I32> || std::is_same_v<T, nsVec3I32> || std::is_same_v<T, nsVec4I32> ||
                       std::is_same_v<T, nsVec2U32> || std::is_same_v<T, nsVec3U32> || std::is_same_v<T, nsVec4U32>)
    {
      out_res = a.Get<T>().CompMul(b.Get<T>());
    }
    else if constexpr (std::is_same_v<T, nsAngle>)
    {
      out_res = nsAngle(a.Get<T>() * b.Get<T>().GetRadian());
    }
  }
};

nsVariant operator*(const nsVariant& a, const nsVariant& b)
{
  if (a.IsNumber() && b.IsNumber())
  {
    auto biggerType = nsMath::Max(a.GetType(), b.GetType());

    MulFunc func;
    nsVariant result;
    nsVariant::DispatchTo(func, biggerType, a.ConvertTo(biggerType), b.ConvertTo(biggerType), result);
    return result;
  }
  else if (a.GetType() == b.GetType())
  {
    MulFunc func;
    nsVariant result;
    nsVariant::DispatchTo(func, a.GetType(), a, b, result);
    return result;
  }

  return nsVariant();
}

//////////////////////////////////////////////////////////////////////////

struct DivFunc
{
  template <typename T>
  NS_ALWAYS_INLINE void operator()(const nsVariant& a, const nsVariant& b, nsVariant& out_res)
  {
    if constexpr (std::is_same_v<T, nsInt8> || std::is_same_v<T, nsUInt8> ||
                  std::is_same_v<T, nsInt16> || std::is_same_v<T, nsUInt16> ||
                  std::is_same_v<T, nsInt32> || std::is_same_v<T, nsUInt32> ||
                  std::is_same_v<T, nsInt64> || std::is_same_v<T, nsUInt64> ||
                  std::is_same_v<T, float> || std::is_same_v<T, double> ||
                  std::is_same_v<T, nsTime>)
    {
      out_res = a.Get<T>() / b.Get<T>();
    }
    else if constexpr (std::is_same_v<T, nsVec2> || std::is_same_v<T, nsVec3> || std::is_same_v<T, nsVec4> ||
                       std::is_same_v<T, nsVec2I32> || std::is_same_v<T, nsVec3I32> || std::is_same_v<T, nsVec4I32> ||
                       std::is_same_v<T, nsVec2U32> || std::is_same_v<T, nsVec3U32> || std::is_same_v<T, nsVec4U32>)
    {
      out_res = a.Get<T>().CompDiv(b.Get<T>());
    }
    else if constexpr (std::is_same_v<T, nsAngle>)
    {
      out_res = nsAngle(a.Get<T>() / b.Get<T>().GetRadian());
    }
  }
};

nsVariant operator/(const nsVariant& a, const nsVariant& b)
{
  if (a.IsNumber() && b.IsNumber())
  {
    auto biggerType = nsMath::Max(a.GetType(), b.GetType());

    DivFunc func;
    nsVariant result;
    nsVariant::DispatchTo(func, biggerType, a.ConvertTo(biggerType), b.ConvertTo(biggerType), result);
    return result;
  }
  else if (a.GetType() == b.GetType())
  {
    DivFunc func;
    nsVariant result;
    nsVariant::DispatchTo(func, a.GetType(), a, b, result);
    return result;
  }

  return nsVariant();
}

//////////////////////////////////////////////////////////////////////////

struct LerpFunc
{
  constexpr static bool CanInterpolate(nsVariantType::Enum variantType)
  {
    return variantType >= nsVariantType::Int8 && variantType <= nsVariantType::Vector4;
  }

  template <typename T>
  NS_ALWAYS_INLINE void operator()(const nsVariant& a, const nsVariant& b, double x, nsVariant& out_res)
  {
    if constexpr (std::is_same_v<T, nsQuat>)
    {
      nsQuat q = nsQuat::MakeSlerp(a.Get<nsQuat>(), b.Get<nsQuat>(), static_cast<float>(x));
      out_res = q;
    }
    else if constexpr (CanInterpolate(static_cast<nsVariantType::Enum>(nsVariantTypeDeduction<T>::value)))
    {
      out_res = nsMath::Lerp(a.Get<T>(), b.Get<T>(), static_cast<float>(x));
    }
    else
    {
      out_res = (x < 0.5) ? a : b;
    }
  }
};

namespace nsMath
{
  nsVariant Lerp(const nsVariant& a, const nsVariant& b, double fFactor)
  {
    LerpFunc func;
    nsVariant result;
    nsVariant::DispatchTo(func, a.GetType(), a, b, fFactor, result);
    return result;
  }
} // namespace nsMath
