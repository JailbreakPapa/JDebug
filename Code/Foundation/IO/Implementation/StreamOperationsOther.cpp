#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Time/Timestamp.h>
#include <Foundation/Types/VarianceTypes.h>
#include <Foundation/Types/VariantTypeRegistry.h>

// wdAllocatorBase::Stats

void operator<<(wdStreamWriter& inout_stream, const wdAllocatorBase::Stats& rhs)
{
  inout_stream << rhs.m_uiNumAllocations;
  inout_stream << rhs.m_uiNumDeallocations;
  inout_stream << rhs.m_uiAllocationSize;
}

void operator>>(wdStreamReader& inout_stream, wdAllocatorBase::Stats& rhs)
{
  inout_stream >> rhs.m_uiNumAllocations;
  inout_stream >> rhs.m_uiNumDeallocations;
  inout_stream >> rhs.m_uiAllocationSize;
}

// wdTime

void operator<<(wdStreamWriter& inout_stream, wdTime value)
{
  inout_stream << value.GetSeconds();
}

void operator>>(wdStreamReader& inout_stream, wdTime& ref_value)
{
  double d = 0;
  inout_stream.ReadQWordValue(&d).IgnoreResult();

  ref_value = wdTime::Seconds(d);
}

// wdUuid

void operator<<(wdStreamWriter& inout_stream, const wdUuid& value)
{
  inout_stream << value.m_uiHigh;
  inout_stream << value.m_uiLow;
}

void operator>>(wdStreamReader& inout_stream, wdUuid& ref_value)
{
  inout_stream >> ref_value.m_uiHigh;
  inout_stream >> ref_value.m_uiLow;
}

// wdHashedString

void operator<<(wdStreamWriter& inout_stream, const wdHashedString& sValue)
{
  inout_stream.WriteString(sValue.GetView()).AssertSuccess();
}

void operator>>(wdStreamReader& inout_stream, wdHashedString& ref_sValue)
{
  wdStringBuilder sTemp;
  inout_stream >> sTemp;
  ref_sValue.Assign(sTemp);
}

// wdTempHashedString

void operator<<(wdStreamWriter& inout_stream, const wdTempHashedString& sValue)
{
  inout_stream << (wdUInt64)sValue.GetHash();
}

void operator>>(wdStreamReader& inout_stream, wdTempHashedString& ref_sValue)
{
  wdUInt64 hash;
  inout_stream >> hash;
  ref_sValue = wdTempHashedString(hash);
}

// wdVariant

struct WriteValueFunc
{
  template <typename T>
  WD_ALWAYS_INLINE void operator()()
  {
    (*m_pStream) << m_pValue->Get<T>();
  }

  wdStreamWriter* m_pStream;
  const wdVariant* m_pValue;
};

template <>
WD_FORCE_INLINE void WriteValueFunc::operator()<wdVariantArray>()
{
  const wdVariantArray& values = m_pValue->Get<wdVariantArray>();
  const wdUInt32 iCount = values.GetCount();
  (*m_pStream) << iCount;
  for (wdUInt32 i = 0; i < iCount; i++)
  {
    (*m_pStream) << values[i];
  }
}

template <>
WD_FORCE_INLINE void WriteValueFunc::operator()<wdVariantDictionary>()
{
  const wdVariantDictionary& values = m_pValue->Get<wdVariantDictionary>();
  const wdUInt32 iCount = values.GetCount();
  (*m_pStream) << iCount;
  for (auto it = values.GetIterator(); it.IsValid(); ++it)
  {
    (*m_pStream) << it.Key();
    (*m_pStream) << it.Value();
  }
}

template <>
inline void WriteValueFunc::operator()<wdTypedPointer>()
{
  WD_REPORT_FAILURE("Type 'wdReflectedClass*' not supported in serialization.");
}

template <>
inline void WriteValueFunc::operator()<wdTypedObject>()
{
  wdTypedObject obj = m_pValue->Get<wdTypedObject>();
  if (const wdVariantTypeInfo* pTypeInfo = wdVariantTypeRegistry::GetSingleton()->FindVariantTypeInfo(obj.m_pType))
  {
    (*m_pStream) << obj.m_pType->GetTypeName();
    pTypeInfo->Serialize(*m_pStream, obj.m_pObject);
  }
  else
  {
    WD_REPORT_FAILURE("The type '{0}' was declared but not defined, add WD_DEFINE_CUSTOM_VARIANT_TYPE({0}); to a cpp to enable serialization of this variant type.", obj.m_pType->GetTypeName());
  }
}

template <>
WD_FORCE_INLINE void WriteValueFunc::operator()<wdStringView>()
{
  wdStringBuilder s = m_pValue->Get<wdStringView>();
  (*m_pStream) << s;
}

template <>
WD_FORCE_INLINE void WriteValueFunc::operator()<wdDataBuffer>()
{
  const wdDataBuffer& data = m_pValue->Get<wdDataBuffer>();
  const wdUInt32 iCount = data.GetCount();
  (*m_pStream) << iCount;
  m_pStream->WriteBytes(data.GetData(), data.GetCount()).AssertSuccess();
}

struct ReadValueFunc
{
  template <typename T>
  WD_FORCE_INLINE void operator()()
  {
    T value;
    (*m_pStream) >> value;
    *m_pValue = value;
  }

  wdStreamReader* m_pStream;
  wdVariant* m_pValue;
};

template <>
WD_FORCE_INLINE void ReadValueFunc::operator()<wdVariantArray>()
{
  wdVariantArray values;
  wdUInt32 iCount;
  (*m_pStream) >> iCount;
  values.SetCount(iCount);
  for (wdUInt32 i = 0; i < iCount; i++)
  {
    (*m_pStream) >> values[i];
  }
  *m_pValue = values;
}

template <>
WD_FORCE_INLINE void ReadValueFunc::operator()<wdVariantDictionary>()
{
  wdVariantDictionary values;
  wdUInt32 iCount;
  (*m_pStream) >> iCount;
  for (wdUInt32 i = 0; i < iCount; i++)
  {
    wdString key;
    wdVariant value;
    (*m_pStream) >> key;
    (*m_pStream) >> value;
    values.Insert(key, value);
  }
  *m_pValue = values;
}

template <>
inline void ReadValueFunc::operator()<wdTypedPointer>()
{
  WD_REPORT_FAILURE("Type 'wdTypedPointer' not supported in serialization.");
}

template <>
inline void ReadValueFunc::operator()<wdTypedObject>()
{
  wdStringBuilder sType;
  (*m_pStream) >> sType;
  const wdRTTI* pType = wdRTTI::FindTypeByName(sType);
  WD_ASSERT_DEV(pType, "The type '{0}' could not be found.", sType);
  const wdVariantTypeInfo* pTypeInfo = wdVariantTypeRegistry::GetSingleton()->FindVariantTypeInfo(pType);
  WD_ASSERT_DEV(pTypeInfo, "The type '{0}' was declared but not defined, add WD_DEFINE_CUSTOM_VARIANT_TYPE({0}); to a cpp to enable serialization of this variant type.", sType);
  WD_MSVC_ANALYSIS_ASSUME(pType != nullptr);
  WD_MSVC_ANALYSIS_ASSUME(pTypeInfo != nullptr);
  void* pObject = pType->GetAllocator()->Allocate<void>();
  pTypeInfo->Deserialize(*m_pStream, pObject);
  m_pValue->MoveTypedObject(pObject, pType);
}

template <>
inline void ReadValueFunc::operator()<wdStringView>()
{
  WD_REPORT_FAILURE("Type 'wdStringView' not supported in serialization.");
}

template <>
WD_FORCE_INLINE void ReadValueFunc::operator()<wdDataBuffer>()
{
  wdDataBuffer data;
  wdUInt32 iCount;
  (*m_pStream) >> iCount;
  data.SetCountUninitialized(iCount);

  m_pStream->ReadBytes(data.GetData(), iCount);
  *m_pValue = data;
}

void operator<<(wdStreamWriter& inout_stream, const wdVariant& value)
{
  wdUInt8 variantVersion = (wdUInt8)wdGetStaticRTTI<wdVariant>()->GetTypeVersion();
  inout_stream << variantVersion;
  wdVariant::Type::Enum type = value.GetType();
  wdUInt8 typeStorage = type;
  if (typeStorage == wdVariantType::StringView)
    typeStorage = wdVariantType::String;
  inout_stream << typeStorage;

  if (type != wdVariant::Type::Invalid)
  {
    WriteValueFunc func;
    func.m_pStream = &inout_stream;
    func.m_pValue = &value;

    wdVariant::DispatchTo(func, type);
  }
}

void operator>>(wdStreamReader& inout_stream, wdVariant& ref_value)
{
  wdUInt8 variantVersion;
  inout_stream >> variantVersion;
  WD_ASSERT_DEBUG(wdGetStaticRTTI<wdVariant>()->GetTypeVersion() == variantVersion, "Older variant serialization not supported!");

  wdUInt8 typeStorage;
  inout_stream >> typeStorage;
  wdVariant::Type::Enum type = (wdVariant::Type::Enum)typeStorage;

  if (type != wdVariant::Type::Invalid)
  {
    ReadValueFunc func;
    func.m_pStream = &inout_stream;
    func.m_pValue = &ref_value;

    wdVariant::DispatchTo(func, type);
  }
  else
  {
    ref_value = wdVariant();
  }
}

// wdTimestamp

void operator<<(wdStreamWriter& inout_stream, wdTimestamp value)
{
  inout_stream << value.GetInt64(wdSIUnitOfTime::Microsecond);
}

void operator>>(wdStreamReader& inout_stream, wdTimestamp& ref_value)
{
  wdInt64 value;
  inout_stream >> value;

  ref_value.SetInt64(value, wdSIUnitOfTime::Microsecond);
}

// wdVarianceTypeFloat

void operator<<(wdStreamWriter& inout_stream, const wdVarianceTypeFloat& value)
{
  inout_stream << value.m_fVariance;
  inout_stream << value.m_Value;
}
void operator>>(wdStreamReader& inout_stream, wdVarianceTypeFloat& ref_value)
{
  inout_stream >> ref_value.m_fVariance;
  inout_stream >> ref_value.m_Value;
}

// wdVarianceTypeTime

void operator<<(wdStreamWriter& inout_stream, const wdVarianceTypeTime& value)
{
  inout_stream << value.m_fVariance;
  inout_stream << value.m_Value;
}
void operator>>(wdStreamReader& inout_stream, wdVarianceTypeTime& ref_value)
{
  inout_stream >> ref_value.m_fVariance;
  inout_stream >> ref_value.m_Value;
}

// wdVarianceTypeAngle

void operator<<(wdStreamWriter& inout_stream, const wdVarianceTypeAngle& value)
{
  inout_stream << value.m_fVariance;
  inout_stream << value.m_Value;
}
void operator>>(wdStreamReader& inout_stream, wdVarianceTypeAngle& ref_value)
{
  inout_stream >> ref_value.m_fVariance;
  inout_stream >> ref_value.m_Value;
}
WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_StreamOperationsOther);
