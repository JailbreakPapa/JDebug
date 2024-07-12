#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Time/Timestamp.h>
#include <Foundation/Types/VarianceTypes.h>
#include <Foundation/Types/VariantTypeRegistry.h>

// nsAllocator::Stats

void operator<<(nsStreamWriter& inout_stream, const nsAllocator::Stats& rhs)
{
  inout_stream << rhs.m_uiNumAllocations;
  inout_stream << rhs.m_uiNumDeallocations;
  inout_stream << rhs.m_uiAllocationSize;
}

void operator>>(nsStreamReader& inout_stream, nsAllocator::Stats& rhs)
{
  inout_stream >> rhs.m_uiNumAllocations;
  inout_stream >> rhs.m_uiNumDeallocations;
  inout_stream >> rhs.m_uiAllocationSize;
}

// nsTime

void operator<<(nsStreamWriter& inout_stream, nsTime value)
{
  inout_stream << value.GetSeconds();
}

void operator>>(nsStreamReader& inout_stream, nsTime& ref_value)
{
  double d = 0;
  inout_stream.ReadQWordValue(&d).IgnoreResult();

  ref_value = nsTime::MakeFromSeconds(d);
}

// nsUuid

void operator<<(nsStreamWriter& inout_stream, const nsUuid& value)
{
  inout_stream << value.m_uiHigh;
  inout_stream << value.m_uiLow;
}

void operator>>(nsStreamReader& inout_stream, nsUuid& ref_value)
{
  inout_stream >> ref_value.m_uiHigh;
  inout_stream >> ref_value.m_uiLow;
}

// nsHashedString

void operator<<(nsStreamWriter& inout_stream, const nsHashedString& sValue)
{
  inout_stream.WriteString(sValue.GetView()).AssertSuccess();
}

void operator>>(nsStreamReader& inout_stream, nsHashedString& ref_sValue)
{
  nsStringBuilder sTemp;
  inout_stream >> sTemp;
  ref_sValue.Assign(sTemp);
}

// nsTempHashedString

void operator<<(nsStreamWriter& inout_stream, const nsTempHashedString& sValue)
{
  inout_stream << (nsUInt64)sValue.GetHash();
}

void operator>>(nsStreamReader& inout_stream, nsTempHashedString& ref_sValue)
{
  nsUInt64 hash;
  inout_stream >> hash;
  ref_sValue = nsTempHashedString(hash);
}

// nsVariant

struct WriteValueFunc
{
  template <typename T>
  NS_ALWAYS_INLINE void operator()()
  {
    (*m_pStream) << m_pValue->Get<T>();
  }

  nsStreamWriter* m_pStream;
  const nsVariant* m_pValue;
};

template <>
NS_FORCE_INLINE void WriteValueFunc::operator()<nsVariantArray>()
{
  const nsVariantArray& values = m_pValue->Get<nsVariantArray>();
  const nsUInt32 iCount = values.GetCount();
  (*m_pStream) << iCount;
  for (nsUInt32 i = 0; i < iCount; i++)
  {
    (*m_pStream) << values[i];
  }
}

template <>
NS_FORCE_INLINE void WriteValueFunc::operator()<nsVariantDictionary>()
{
  const nsVariantDictionary& values = m_pValue->Get<nsVariantDictionary>();
  const nsUInt32 iCount = values.GetCount();
  (*m_pStream) << iCount;
  for (auto it = values.GetIterator(); it.IsValid(); ++it)
  {
    (*m_pStream) << it.Key();
    (*m_pStream) << it.Value();
  }
}

template <>
inline void WriteValueFunc::operator()<nsTypedPointer>()
{
  NS_REPORT_FAILURE("Type 'nsReflectedClass*' not supported in serialization.");
}

template <>
inline void WriteValueFunc::operator()<nsTypedObject>()
{
  nsTypedObject obj = m_pValue->Get<nsTypedObject>();
  if (const nsVariantTypeInfo* pTypeInfo = nsVariantTypeRegistry::GetSingleton()->FindVariantTypeInfo(obj.m_pType))
  {
    (*m_pStream) << obj.m_pType->GetTypeName();
    pTypeInfo->Serialize(*m_pStream, obj.m_pObject);
  }
  else
  {
    NS_REPORT_FAILURE("The type '{0}' was declared but not defined, add NS_DEFINE_CUSTOM_VARIANT_TYPE({0}); to a cpp to enable serialization of this variant type.", obj.m_pType->GetTypeName());
  }
}

template <>
NS_FORCE_INLINE void WriteValueFunc::operator()<nsStringView>()
{
  nsStringBuilder s = m_pValue->Get<nsStringView>();
  (*m_pStream) << s;
}

template <>
NS_FORCE_INLINE void WriteValueFunc::operator()<nsDataBuffer>()
{
  const nsDataBuffer& data = m_pValue->Get<nsDataBuffer>();
  const nsUInt32 iCount = data.GetCount();
  (*m_pStream) << iCount;
  m_pStream->WriteBytes(data.GetData(), data.GetCount()).AssertSuccess();
}

struct ReadValueFunc
{
  template <typename T>
  NS_FORCE_INLINE void operator()()
  {
    T value;
    (*m_pStream) >> value;
    *m_pValue = value;
  }

  nsStreamReader* m_pStream;
  nsVariant* m_pValue;
};

template <>
NS_FORCE_INLINE void ReadValueFunc::operator()<nsVariantArray>()
{
  nsVariantArray values;
  nsUInt32 iCount;
  (*m_pStream) >> iCount;
  values.SetCount(iCount);
  for (nsUInt32 i = 0; i < iCount; i++)
  {
    (*m_pStream) >> values[i];
  }
  *m_pValue = values;
}

template <>
NS_FORCE_INLINE void ReadValueFunc::operator()<nsVariantDictionary>()
{
  nsVariantDictionary values;
  nsUInt32 iCount;
  (*m_pStream) >> iCount;
  for (nsUInt32 i = 0; i < iCount; i++)
  {
    nsString key;
    nsVariant value;
    (*m_pStream) >> key;
    (*m_pStream) >> value;
    values.Insert(key, value);
  }
  *m_pValue = values;
}

template <>
inline void ReadValueFunc::operator()<nsTypedPointer>()
{
  NS_REPORT_FAILURE("Type 'nsTypedPointer' not supported in serialization.");
}

template <>
inline void ReadValueFunc::operator()<nsTypedObject>()
{
  nsStringBuilder sType;
  (*m_pStream) >> sType;
  const nsRTTI* pType = nsRTTI::FindTypeByName(sType);
  NS_ASSERT_DEV(pType, "The type '{0}' could not be found.", sType);
  const nsVariantTypeInfo* pTypeInfo = nsVariantTypeRegistry::GetSingleton()->FindVariantTypeInfo(pType);
  NS_ASSERT_DEV(pTypeInfo, "The type '{0}' was declared but not defined, add NS_DEFINE_CUSTOM_VARIANT_TYPE({0}); to a cpp to enable serialization of this variant type.", sType);
  NS_MSVC_ANALYSIS_ASSUME(pType != nullptr);
  NS_MSVC_ANALYSIS_ASSUME(pTypeInfo != nullptr);
  void* pObject = pType->GetAllocator()->Allocate<void>();
  pTypeInfo->Deserialize(*m_pStream, pObject);
  m_pValue->MoveTypedObject(pObject, pType);
}

template <>
inline void ReadValueFunc::operator()<nsStringView>()
{
  NS_REPORT_FAILURE("Type 'nsStringView' not supported in serialization.");
}

template <>
NS_FORCE_INLINE void ReadValueFunc::operator()<nsDataBuffer>()
{
  nsDataBuffer data;
  nsUInt32 iCount;
  (*m_pStream) >> iCount;
  data.SetCountUninitialized(iCount);

  m_pStream->ReadBytes(data.GetData(), iCount);
  *m_pValue = data;
}

void operator<<(nsStreamWriter& inout_stream, const nsVariant& value)
{
  nsUInt8 variantVersion = (nsUInt8)nsGetStaticRTTI<nsVariant>()->GetTypeVersion();
  inout_stream << variantVersion;
  nsVariant::Type::Enum type = value.GetType();
  nsUInt8 typeStorage = type;
  if (typeStorage == nsVariantType::StringView)
    typeStorage = nsVariantType::String;
  inout_stream << typeStorage;

  if (type != nsVariant::Type::Invalid)
  {
    WriteValueFunc func;
    func.m_pStream = &inout_stream;
    func.m_pValue = &value;

    nsVariant::DispatchTo(func, type);
  }
}

void operator>>(nsStreamReader& inout_stream, nsVariant& ref_value)
{
  nsUInt8 variantVersion;
  inout_stream >> variantVersion;
  NS_ASSERT_DEBUG(nsGetStaticRTTI<nsVariant>()->GetTypeVersion() == variantVersion, "Older variant serialization not supported!");

  nsUInt8 typeStorage;
  inout_stream >> typeStorage;
  nsVariant::Type::Enum type = (nsVariant::Type::Enum)typeStorage;

  if (type != nsVariant::Type::Invalid)
  {
    ReadValueFunc func;
    func.m_pStream = &inout_stream;
    func.m_pValue = &ref_value;

    nsVariant::DispatchTo(func, type);
  }
  else
  {
    ref_value = nsVariant();
  }
}

// nsTimestamp

void operator<<(nsStreamWriter& inout_stream, nsTimestamp value)
{
  inout_stream << value.GetInt64(nsSIUnitOfTime::Microsecond);
}

void operator>>(nsStreamReader& inout_stream, nsTimestamp& ref_value)
{
  nsInt64 value;
  inout_stream >> value;

  ref_value = nsTimestamp::MakeFromInt(value, nsSIUnitOfTime::Microsecond);
}

// nsVarianceTypeFloat

void operator<<(nsStreamWriter& inout_stream, const nsVarianceTypeFloat& value)
{
  inout_stream << value.m_fVariance;
  inout_stream << value.m_Value;
}
void operator>>(nsStreamReader& inout_stream, nsVarianceTypeFloat& ref_value)
{
  inout_stream >> ref_value.m_fVariance;
  inout_stream >> ref_value.m_Value;
}

// nsVarianceTypeTime

void operator<<(nsStreamWriter& inout_stream, const nsVarianceTypeTime& value)
{
  inout_stream << value.m_fVariance;
  inout_stream << value.m_Value;
}
void operator>>(nsStreamReader& inout_stream, nsVarianceTypeTime& ref_value)
{
  inout_stream >> ref_value.m_fVariance;
  inout_stream >> ref_value.m_Value;
}

// nsVarianceTypeAngle

void operator<<(nsStreamWriter& inout_stream, const nsVarianceTypeAngle& value)
{
  inout_stream << value.m_fVariance;
  inout_stream << value.m_Value;
}
void operator>>(nsStreamReader& inout_stream, nsVarianceTypeAngle& ref_value)
{
  inout_stream >> ref_value.m_fVariance;
  inout_stream >> ref_value.m_Value;
}
