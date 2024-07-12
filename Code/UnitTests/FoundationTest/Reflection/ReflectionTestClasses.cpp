#include <FoundationTest/FoundationTestPCH.h>

#include <FoundationTest/Reflection/ReflectionTestClasses.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsExampleEnum, 1)
  NS_ENUM_CONSTANTS(nsExampleEnum::Value1, nsExampleEnum::Value2)
  NS_ENUM_CONSTANT(nsExampleEnum::Value3),
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_STATIC_REFLECTED_BITFLAGS(nsExampleBitflags, 1)
  NS_BITFLAGS_CONSTANTS(nsExampleBitflags::Value1, nsExampleBitflags::Value2)
  NS_BITFLAGS_CONSTANT(nsExampleBitflags::Value3),
NS_END_STATIC_REFLECTED_BITFLAGS;


NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAbstractTestClass, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;


NS_BEGIN_STATIC_REFLECTED_TYPE(nsAbstractTestStruct, nsNoBase, 1, nsRTTINoAllocator);
NS_END_STATIC_REFLECTED_TYPE;


NS_BEGIN_STATIC_REFLECTED_TYPE(nsTestStruct, nsNoBase, 7, nsRTTIDefaultAllocator<nsTestStruct>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Float", m_fFloat1)->AddAttributes(new nsDefaultValueAttribute(1.1f)),
    NS_MEMBER_PROPERTY_READ_ONLY("Vector", m_vProperty3)->AddAttributes(new nsDefaultValueAttribute(nsVec3(3.0f,4.0f,5.0f))),
    NS_ACCESSOR_PROPERTY("Int", GetInt, SetInt)->AddAttributes(new nsDefaultValueAttribute(2)),
    NS_MEMBER_PROPERTY("UInt8", m_UInt8)->AddAttributes(new nsDefaultValueAttribute(6)),
    NS_MEMBER_PROPERTY("Variant", m_variant)->AddAttributes(new nsDefaultValueAttribute("Test")),
    NS_MEMBER_PROPERTY("Angle", m_Angle)->AddAttributes(new nsDefaultValueAttribute(nsAngle::MakeFromDegree(0.5))),
    NS_MEMBER_PROPERTY("DataBuffer", m_DataBuffer)->AddAttributes(new nsDefaultValueAttribute(nsTestStruct::GetDefaultDataBuffer())),
    NS_MEMBER_PROPERTY("vVec3I", m_vVec3I)->AddAttributes(new nsDefaultValueAttribute(nsVec3I32(1,2,3))),
    NS_MEMBER_PROPERTY("VarianceAngle", m_VarianceAngle)->AddAttributes(new nsDefaultValueAttribute(nsVarianceTypeAngle{0.5f, nsAngle::MakeFromDegree(90.0f)})),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsTestStruct3, nsNoBase, 71, nsRTTIDefaultAllocator<nsTestStruct3>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Float", m_fFloat1)->AddAttributes(new nsDefaultValueAttribute(33.3f)),
    NS_ACCESSOR_PROPERTY("Int", GetInt, SetInt),
    NS_MEMBER_PROPERTY("UInt8", m_UInt8),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(),
    NS_CONSTRUCTOR_PROPERTY(double, nsInt16),
  }
  NS_END_FUNCTIONS;
}
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsTypedObjectStruct, nsNoBase, 1, nsRTTIDefaultAllocator<nsTypedObjectStruct>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Float", m_fFloat1)->AddAttributes(new nsDefaultValueAttribute(33.3f)),
    NS_MEMBER_PROPERTY("Int", m_iInt32),
    NS_MEMBER_PROPERTY("UInt8", m_UInt8),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsTestClass1, 11, nsRTTIDefaultAllocator<nsTestClass1>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("SubStruct", m_Struct),
    // NS_MEMBER_PROPERTY("MyVector", m_MyVector), Intentionally not reflected
    NS_MEMBER_PROPERTY("Color", m_Color),
    NS_ACCESSOR_PROPERTY_READ_ONLY("SubVector", GetVector)->AddAttributes(new nsDefaultValueAttribute(nsVec3(3, 4, 5)))
  }
    NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

nsInt32 nsTestClass2Allocator::m_iAllocs = 0;
nsInt32 nsTestClass2Allocator::m_iDeallocs = 0;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsTestClass2, 22, nsTestClass2Allocator)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("CharPtr", GetCharPtr, SetCharPtr)->AddAttributes(new nsDefaultValueAttribute("AAA")),
    NS_ACCESSOR_PROPERTY("String", GetString, SetString)->AddAttributes(new nsDefaultValueAttribute("BBB")),
    NS_ACCESSOR_PROPERTY("StringView", GetStringView, SetStringView)->AddAttributes(new nsDefaultValueAttribute("CCC")),
    NS_MEMBER_PROPERTY("Time", m_Time),
    NS_ENUM_MEMBER_PROPERTY("Enum", nsExampleEnum, m_enumClass),
    NS_BITFLAGS_MEMBER_PROPERTY("Bitflags", nsExampleBitflags, m_bitflagsClass),
    NS_ARRAY_MEMBER_PROPERTY("Array", m_array),
    NS_MEMBER_PROPERTY("Variant", m_Variant),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsTestClass2b, 24, nsRTTIDefaultAllocator<nsTestClass2b>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Text2b", GetText, SetText),
    NS_MEMBER_PROPERTY("SubStruct", m_Struct),
    NS_MEMBER_PROPERTY("Color", m_Color),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsTestArrays, 1, nsRTTIDefaultAllocator<nsTestArrays>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ARRAY_MEMBER_PROPERTY("Hybrid", m_Hybrid),
    NS_ARRAY_MEMBER_PROPERTY("HybridChar", m_HybridChar),
    NS_ARRAY_MEMBER_PROPERTY("Dynamic", m_Dynamic),
    NS_ARRAY_MEMBER_PROPERTY("Deque", m_Deque),
    NS_ARRAY_MEMBER_PROPERTY("Custom", m_CustomVariant),

    NS_ARRAY_MEMBER_PROPERTY_READ_ONLY("HybridRO", m_Hybrid),
    NS_ARRAY_MEMBER_PROPERTY_READ_ONLY("HybridCharRO", m_HybridChar),
    NS_ARRAY_MEMBER_PROPERTY_READ_ONLY("DynamicRO", m_Dynamic),
    NS_ARRAY_MEMBER_PROPERTY_READ_ONLY("DequeRO", m_Deque),
    NS_ARRAY_MEMBER_PROPERTY_READ_ONLY("CustomRO", m_CustomVariant),

    NS_ARRAY_ACCESSOR_PROPERTY("AcHybrid", GetCount, GetValue, SetValue, Insert, Remove),
    NS_ARRAY_ACCESSOR_PROPERTY_READ_ONLY("AcHybridRO", GetCount, GetValue),
    NS_ARRAY_ACCESSOR_PROPERTY("AcHybridChar", GetCountChar, GetValueChar, SetValueChar, InsertChar, RemoveChar),
    NS_ARRAY_ACCESSOR_PROPERTY_READ_ONLY("AcHybridCharRO", GetCountChar, GetValueChar),
    NS_ARRAY_ACCESSOR_PROPERTY("AcDynamic", GetCountDyn, GetValueDyn, SetValueDyn, InsertDyn, RemoveDyn),
    NS_ARRAY_ACCESSOR_PROPERTY_READ_ONLY("AcDynamicRO", GetCountDyn, GetValueDyn),
    NS_ARRAY_ACCESSOR_PROPERTY("AcDeque", GetCountDeq, GetValueDeq, SetValueDeq, InsertDeq, RemoveDeq),
    NS_ARRAY_ACCESSOR_PROPERTY_READ_ONLY("AcDequeRO", GetCountDeq, GetValueDeq),
    NS_ARRAY_ACCESSOR_PROPERTY("AcCustom", GetCountCustom, GetValueCustom, SetValueCustom, InsertCustom, RemoveCustom),
    NS_ARRAY_ACCESSOR_PROPERTY_READ_ONLY("AcCustomRO", GetCountCustom, GetValueCustom),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsUInt32 nsTestArrays::GetCount() const
{
  return m_Hybrid.GetCount();
}
double nsTestArrays::GetValue(nsUInt32 uiIndex) const
{
  return m_Hybrid[uiIndex];
}
void nsTestArrays::SetValue(nsUInt32 uiIndex, double value)
{
  m_Hybrid[uiIndex] = value;
}
void nsTestArrays::Insert(nsUInt32 uiIndex, double value)
{
  m_Hybrid.InsertAt(uiIndex, value);
}
void nsTestArrays::Remove(nsUInt32 uiIndex)
{
  m_Hybrid.RemoveAtAndCopy(uiIndex);
}

nsUInt32 nsTestArrays::GetCountChar() const
{
  return m_HybridChar.GetCount();
}
const char* nsTestArrays::GetValueChar(nsUInt32 uiIndex) const
{
  return m_HybridChar[uiIndex];
}
void nsTestArrays::SetValueChar(nsUInt32 uiIndex, const char* value)
{
  m_HybridChar[uiIndex] = value;
}
void nsTestArrays::InsertChar(nsUInt32 uiIndex, const char* value)
{
  m_HybridChar.InsertAt(uiIndex, value);
}
void nsTestArrays::RemoveChar(nsUInt32 uiIndex)
{
  m_HybridChar.RemoveAtAndCopy(uiIndex);
}

nsUInt32 nsTestArrays::GetCountDyn() const
{
  return m_Dynamic.GetCount();
}
const nsTestStruct3& nsTestArrays::GetValueDyn(nsUInt32 uiIndex) const
{
  return m_Dynamic[uiIndex];
}
void nsTestArrays::SetValueDyn(nsUInt32 uiIndex, const nsTestStruct3& value)
{
  m_Dynamic[uiIndex] = value;
}
void nsTestArrays::InsertDyn(nsUInt32 uiIndex, const nsTestStruct3& value)
{
  m_Dynamic.InsertAt(uiIndex, value);
}
void nsTestArrays::RemoveDyn(nsUInt32 uiIndex)
{
  m_Dynamic.RemoveAtAndCopy(uiIndex);
}

nsUInt32 nsTestArrays::GetCountDeq() const
{
  return m_Deque.GetCount();
}
const nsTestArrays& nsTestArrays::GetValueDeq(nsUInt32 uiIndex) const
{
  return m_Deque[uiIndex];
}
void nsTestArrays::SetValueDeq(nsUInt32 uiIndex, const nsTestArrays& value)
{
  m_Deque[uiIndex] = value;
}
void nsTestArrays::InsertDeq(nsUInt32 uiIndex, const nsTestArrays& value)
{
  m_Deque.InsertAt(uiIndex, value);
}
void nsTestArrays::RemoveDeq(nsUInt32 uiIndex)
{
  m_Deque.RemoveAtAndCopy(uiIndex);
}

nsUInt32 nsTestArrays::GetCountCustom() const
{
  return m_CustomVariant.GetCount();
}
nsVarianceTypeAngle nsTestArrays::GetValueCustom(nsUInt32 uiIndex) const
{
  return m_CustomVariant[uiIndex];
}
void nsTestArrays::SetValueCustom(nsUInt32 uiIndex, nsVarianceTypeAngle value)
{
  m_CustomVariant[uiIndex] = value;
}
void nsTestArrays::InsertCustom(nsUInt32 uiIndex, nsVarianceTypeAngle value)
{
  m_CustomVariant.InsertAt(uiIndex, value);
}
void nsTestArrays::RemoveCustom(nsUInt32 uiIndex)
{
  m_CustomVariant.RemoveAtAndCopy(uiIndex);
}

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsTestSets, 1, nsRTTIDefaultAllocator<nsTestSets>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_SET_MEMBER_PROPERTY("Set", m_SetMember),
    NS_SET_MEMBER_PROPERTY_READ_ONLY("SetRO", m_SetMember),
    NS_SET_ACCESSOR_PROPERTY("AcSet", GetSet, Insert, Remove),
    NS_SET_ACCESSOR_PROPERTY_READ_ONLY("AcSetRO", GetSet),
    NS_SET_MEMBER_PROPERTY("HashSet", m_HashSetMember),
    NS_SET_MEMBER_PROPERTY_READ_ONLY("HashSetRO", m_HashSetMember),
    NS_SET_ACCESSOR_PROPERTY("HashAcSet", GetHashSet, HashInsert, HashRemove),
    NS_SET_ACCESSOR_PROPERTY_READ_ONLY("HashAcSetRO", GetHashSet),
    NS_SET_ACCESSOR_PROPERTY("AcPseudoSet", GetPseudoSet, PseudoInsert, PseudoRemove),
    NS_SET_ACCESSOR_PROPERTY_READ_ONLY("AcPseudoSetRO", GetPseudoSet),
    NS_SET_ACCESSOR_PROPERTY("AcPseudoSet2", GetPseudoSet2, PseudoInsert2, PseudoRemove2),
    NS_SET_ACCESSOR_PROPERTY_READ_ONLY("AcPseudoSet2RO", GetPseudoSet2),
    NS_SET_ACCESSOR_PROPERTY("AcPseudoSet2b", GetPseudoSet2, PseudoInsert2b, PseudoRemove2b),
    NS_SET_MEMBER_PROPERTY("CustomHashSet", m_CustomVariant),
    NS_SET_MEMBER_PROPERTY_READ_ONLY("CustomHashSetRO", m_CustomVariant),
    NS_SET_ACCESSOR_PROPERTY("CustomHashAcSet", GetCustomHashSet, CustomHashInsert, CustomHashRemove),
    NS_SET_ACCESSOR_PROPERTY_READ_ONLY("CustomHashAcSetRO", GetCustomHashSet),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

const nsSet<double>& nsTestSets::GetSet() const
{
  return m_SetAccessor;
}

void nsTestSets::Insert(double value)
{
  m_SetAccessor.Insert(value);
}

void nsTestSets::Remove(double value)
{
  m_SetAccessor.Remove(value);
}


const nsHashSet<nsInt64>& nsTestSets::GetHashSet() const
{
  return m_HashSetAccessor;
}

void nsTestSets::HashInsert(nsInt64 value)
{
  m_HashSetAccessor.Insert(value);
}

void nsTestSets::HashRemove(nsInt64 value)
{
  m_HashSetAccessor.Remove(value);
}

const nsDeque<int>& nsTestSets::GetPseudoSet() const
{
  return m_Deque;
}

void nsTestSets::PseudoInsert(int value)
{
  if (!m_Deque.Contains(value))
    m_Deque.PushBack(value);
}

void nsTestSets::PseudoRemove(int value)
{
  m_Deque.RemoveAndCopy(value);
}


nsArrayPtr<const nsString> nsTestSets::GetPseudoSet2() const
{
  return m_Array;
}

void nsTestSets::PseudoInsert2(const nsString& value)
{
  if (!m_Array.Contains(value))
    m_Array.PushBack(value);
}

void nsTestSets::PseudoRemove2(const nsString& value)
{
  m_Array.RemoveAndCopy(value);
}

void nsTestSets::PseudoInsert2b(const char* value)
{
  if (!m_Array.Contains(value))
    m_Array.PushBack(value);
}

void nsTestSets::PseudoRemove2b(const char* value)
{
  m_Array.RemoveAndCopy(value);
}

const nsHashSet<nsVarianceTypeAngle>& nsTestSets::GetCustomHashSet() const
{
  return m_CustomVariant;
}

void nsTestSets::CustomHashInsert(nsVarianceTypeAngle value)
{
  m_CustomVariant.Insert(value);
}

void nsTestSets::CustomHashRemove(nsVarianceTypeAngle value)
{
  m_CustomVariant.Remove(value);
}

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsTestMaps, 1, nsRTTIDefaultAllocator<nsTestMaps>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MAP_MEMBER_PROPERTY("Map", m_MapMember),
    NS_MAP_MEMBER_PROPERTY_READ_ONLY("MapRO", m_MapMember),
    NS_MAP_WRITE_ACCESSOR_PROPERTY("AcMap", GetContainer, Insert, Remove),
    NS_MAP_MEMBER_PROPERTY("HashTable", m_HashTableMember),
    NS_MAP_MEMBER_PROPERTY_READ_ONLY("HashTableRO", m_HashTableMember),
    NS_MAP_WRITE_ACCESSOR_PROPERTY("AcHashTable", GetContainer2, Insert2, Remove2),
    NS_MAP_ACCESSOR_PROPERTY("Accessor", GetKeys3, GetValue3, Insert3, Remove3),
    NS_MAP_ACCESSOR_PROPERTY_READ_ONLY("AccessorRO", GetKeys3, GetValue3),
    NS_MAP_MEMBER_PROPERTY("CustomVariant", m_CustomVariant),
    NS_MAP_MEMBER_PROPERTY_READ_ONLY("CustomVariantRO", m_CustomVariant),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

bool nsTestMaps::operator==(const nsTestMaps& rhs) const
{
  for (nsUInt32 i = 0; i < m_Accessor3.GetCount(); i++)
  {
    bool bRes = false;
    for (nsUInt32 j = 0; j < rhs.m_Accessor3.GetCount(); j++)
    {
      if (m_Accessor3[i].m_Key == rhs.m_Accessor3[j].m_Key)
      {
        if (m_Accessor3[i].m_Value == rhs.m_Accessor3[j].m_Value)
          bRes = true;
      }
    }
    if (!bRes)
      return false;
  }
  return m_MapMember == rhs.m_MapMember && m_MapAccessor == rhs.m_MapAccessor && m_HashTableMember == rhs.m_HashTableMember && m_HashTableAccessor == rhs.m_HashTableAccessor && m_CustomVariant == rhs.m_CustomVariant;
}

const nsMap<nsString, nsInt64>& nsTestMaps::GetContainer() const
{
  return m_MapAccessor;
}

void nsTestMaps::Insert(const char* szKey, nsInt64 value)
{
  m_MapAccessor.Insert(szKey, value);
}

void nsTestMaps::Remove(const char* szKey)
{
  m_MapAccessor.Remove(szKey);
}

const nsHashTable<nsString, nsString>& nsTestMaps::GetContainer2() const
{
  return m_HashTableAccessor;
}

void nsTestMaps::Insert2(const char* szKey, const nsString& value)
{
  m_HashTableAccessor.Insert(szKey, value);
}


void nsTestMaps::Remove2(const char* szKey)
{
  m_HashTableAccessor.Remove(szKey);
}

const nsRangeView<const char*, nsUInt32> nsTestMaps::GetKeys3() const
{
  return nsRangeView<const char*, nsUInt32>([this]() -> nsUInt32
    { return 0; },
    [this]() -> nsUInt32
    { return m_Accessor3.GetCount(); },
    [this](nsUInt32& ref_uiIt)
    { ++ref_uiIt; },
    [this](const nsUInt32& uiIt) -> const char*
    { return m_Accessor3[uiIt].m_Key; });
}

void nsTestMaps::Insert3(const char* szKey, const nsVariant& value)
{
  for (auto&& t : m_Accessor3)
  {
    if (t.m_Key == szKey)
    {
      t.m_Value = value;
      return;
    }
  }
  auto&& t = m_Accessor3.ExpandAndGetRef();
  t.m_Key = szKey;
  t.m_Value = value;
}

void nsTestMaps::Remove3(const char* szKey)
{
  for (nsUInt32 i = 0; i < m_Accessor3.GetCount(); i++)
  {
    const Tuple& t = m_Accessor3[i];
    if (t.m_Key == szKey)
    {
      m_Accessor3.RemoveAtAndSwap(i);
      break;
    }
  }
}

bool nsTestMaps::GetValue3(const char* szKey, nsVariant& out_value) const
{
  for (const auto& t : m_Accessor3)
  {
    if (t.m_Key == szKey)
    {
      out_value = t.m_Value;
      return true;
    }
  }
  return false;
}

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsTestPtr, 1, nsRTTIDefaultAllocator<nsTestPtr>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("ConstCharPtr", GetString, SetString),
    NS_ACCESSOR_PROPERTY("ArraysPtr", GetArrays, SetArrays)->AddFlags(nsPropertyFlags::PointerOwner),
    NS_MEMBER_PROPERTY("ArraysPtrDirect", m_pArraysDirect)->AddFlags(nsPropertyFlags::PointerOwner),
    NS_ARRAY_MEMBER_PROPERTY("PtrArray", m_ArrayPtr)->AddFlags(nsPropertyFlags::PointerOwner),
    NS_SET_MEMBER_PROPERTY("PtrSet", m_SetPtr)->AddFlags(nsPropertyFlags::PointerOwner),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;


NS_BEGIN_STATIC_REFLECTED_TYPE(nsTestEnumStruct, nsNoBase, 1, nsRTTIDefaultAllocator<nsTestEnumStruct>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ENUM_MEMBER_PROPERTY("m_enum", nsExampleEnum, m_enum),
    NS_ENUM_MEMBER_PROPERTY("m_enumClass", nsExampleEnum, m_enumClass),
    NS_ENUM_ACCESSOR_PROPERTY("m_enum2", nsExampleEnum, GetEnum, SetEnum),
    NS_ENUM_ACCESSOR_PROPERTY("m_enumClass2", nsExampleEnum,  GetEnumClass, SetEnumClass),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsTestBitflagsStruct, nsNoBase, 1, nsRTTIDefaultAllocator<nsTestBitflagsStruct>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_BITFLAGS_MEMBER_PROPERTY("m_bitflagsClass", nsExampleBitflags, m_bitflagsClass),
    NS_BITFLAGS_ACCESSOR_PROPERTY("m_bitflagsClass2", nsExampleBitflags, GetBitflagsClass, SetBitflagsClass),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on
