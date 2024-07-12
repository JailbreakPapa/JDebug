#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/RangeView.h>
#include <Foundation/Types/VarianceTypes.h>

struct nsExampleEnum
{
  using StorageType = nsInt8;
  enum Enum
  {
    Value1 = 1,      // normal value
    Value2 = -2,     // normal value
    Value3 = 4,      // normal value
    Default = Value1 // Default initialization value (required)
  };
};

NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, nsExampleEnum);


struct nsExampleBitflags
{
  using StorageType = nsUInt64;
  enum Enum : nsUInt64
  {
    Value1 = NS_BIT(0),  // normal value
    Value2 = NS_BIT(31), // normal value
    Value3 = NS_BIT(63), // normal value
    Default = Value1     // Default initialization value (required)
  };

  struct Bits
  {
    StorageType Value1 : 1;
    StorageType Padding : 30;
    StorageType Value2 : 1;
    StorageType Padding2 : 31;
    StorageType Value3 : 1;
  };
};

NS_DECLARE_FLAGS_OPERATORS(nsExampleBitflags);

NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, nsExampleBitflags);


class nsAbstractTestClass : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsAbstractTestClass, nsReflectedClass);

  virtual void AbstractFunction() = 0;
};


struct nsAbstractTestStruct
{
  virtual void AbstractFunction() = 0;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, nsAbstractTestStruct);


struct nsTestStruct
{
  NS_ALLOW_PRIVATE_PROPERTIES(nsTestStruct);

public:
  static nsDataBuffer GetDefaultDataBuffer()
  {
    nsDataBuffer data;
    data.PushBack(255);
    data.PushBack(0);
    data.PushBack(127);
    return data;
  }

  nsTestStruct()
  {
    m_fFloat1 = 1.1f;
    m_iInt2 = 2;
    m_vProperty3.Set(3, 4, 5);
    m_UInt8 = 6;
    m_variant = "Test";
    m_Angle = nsAngle::MakeFromDegree(0.5);
    m_DataBuffer = GetDefaultDataBuffer();
    m_vVec3I = nsVec3I32(1, 2, 3);
    m_VarianceAngle.m_fVariance = 0.5f;
    m_VarianceAngle.m_Value = nsAngle::MakeFromDegree(90.0f);
  }



  bool operator==(const nsTestStruct& rhs) const
  {
    return m_fFloat1 == rhs.m_fFloat1 && m_UInt8 == rhs.m_UInt8 && m_variant == rhs.m_variant && m_iInt2 == rhs.m_iInt2 && m_vProperty3 == rhs.m_vProperty3 && m_Angle == rhs.m_Angle && m_DataBuffer == rhs.m_DataBuffer && m_vVec3I == rhs.m_vVec3I && m_VarianceAngle == rhs.m_VarianceAngle;
  }

  float m_fFloat1;
  nsUInt8 m_UInt8;
  nsVariant m_variant;
  nsAngle m_Angle;
  nsDataBuffer m_DataBuffer;
  nsVec3I32 m_vVec3I;
  nsVarianceTypeAngle m_VarianceAngle;

private:
  void SetInt(nsInt32 i) { m_iInt2 = i; }
  nsInt32 GetInt() const { return m_iInt2; }

  nsInt32 m_iInt2;
  nsVec3 m_vProperty3;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, nsTestStruct);


struct nsTestStruct3
{
  NS_ALLOW_PRIVATE_PROPERTIES(nsTestStruct3);

public:
  nsTestStruct3()
  {
    m_fFloat1 = 1.1f;
    m_UInt8 = 6;
    m_iInt32 = 2;
  }
  nsTestStruct3(double a, nsInt16 b)
  {
    m_fFloat1 = a;
    m_UInt8 = b;
    m_iInt32 = 32;
  }

  bool operator==(const nsTestStruct3& rhs) const { return m_fFloat1 == rhs.m_fFloat1 && m_iInt32 == rhs.m_iInt32 && m_UInt8 == rhs.m_UInt8; }

  bool operator!=(const nsTestStruct3& rhs) const { return !(*this == rhs); }

  double m_fFloat1;
  nsInt16 m_UInt8;

  nsUInt32 GetIntPublic() const { return m_iInt32; }

private:
  void SetInt(nsUInt32 i) { m_iInt32 = i; }
  nsUInt32 GetInt() const { return m_iInt32; }

  nsInt32 m_iInt32;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, nsTestStruct3);

struct nsTypedObjectStruct
{
  NS_ALLOW_PRIVATE_PROPERTIES(nsTypedObjectStruct);

public:
  nsTypedObjectStruct()
  {
    m_fFloat1 = 1.1f;
    m_UInt8 = 6;
    m_iInt32 = 2;
  }
  nsTypedObjectStruct(double a, nsInt16 b)
  {
    m_fFloat1 = a;
    m_UInt8 = b;
    m_iInt32 = 32;
  }

  double m_fFloat1;
  nsInt16 m_UInt8;
  nsInt32 m_iInt32;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, nsTypedObjectStruct);
NS_DECLARE_CUSTOM_VARIANT_TYPE(nsTypedObjectStruct);

class nsTestClass1 : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsTestClass1, nsReflectedClass);

public:
  nsTestClass1()
  {
    m_MyVector.Set(3, 4, 5);

    m_Struct.m_fFloat1 = 33.3f;

    m_Color = nsColor::CornflowerBlue; // The Original!
  }

  nsTestClass1(const nsColor& c, const nsTestStruct& s)
  {
    m_Color = c;
    m_Struct = s;
    m_MyVector.Set(1, 2, 3);
  }

  bool operator==(const nsTestClass1& rhs) const { return m_Struct == rhs.m_Struct && m_MyVector == rhs.m_MyVector && m_Color == rhs.m_Color; }

  nsVec3 GetVector() const { return m_MyVector; }

  nsTestStruct m_Struct;
  nsVec3 m_MyVector;
  nsColor m_Color;
};


class nsTestClass2 : public nsTestClass1
{
  NS_ADD_DYNAMIC_REFLECTION(nsTestClass2, nsTestClass1);

public:
  nsTestClass2()
  {
    m_sCharPtr = "AAA";
    m_sString = "BBB";
    m_sStringView = "CCC";
  }

  bool operator==(const nsTestClass2& rhs) const { return m_Time == rhs.m_Time && m_enumClass == rhs.m_enumClass && m_bitflagsClass == rhs.m_bitflagsClass && m_array == rhs.m_array && m_Variant == rhs.m_Variant && m_sCharPtr == rhs.m_sCharPtr && m_sString == rhs.m_sString && m_sStringView == rhs.m_sStringView; }

  const char* GetCharPtr() const { return m_sCharPtr.GetData(); }
  void SetCharPtr(const char* szSz) { m_sCharPtr = szSz; }

  const nsString& GetString() const { return m_sString; }
  void SetString(const nsString& sStr) { m_sString = sStr; }

  nsStringView GetStringView() const { return m_sStringView.GetView(); }
  void SetStringView(nsStringView sStrView) { m_sStringView = sStrView; }

  nsTime m_Time;
  nsEnum<nsExampleEnum> m_enumClass;
  nsBitflags<nsExampleBitflags> m_bitflagsClass;
  nsHybridArray<float, 4> m_array;
  nsVariant m_Variant;

private:
  nsString m_sCharPtr;
  nsString m_sString;
  nsString m_sStringView;
};


struct nsTestClass2Allocator : public nsRTTIAllocator
{
  virtual nsInternal::NewInstance<void> AllocateInternal(nsAllocator* pAllocator) override
  {
    ++m_iAllocs;

    return NS_DEFAULT_NEW(nsTestClass2);
  }

  virtual void Deallocate(void* pObject, nsAllocator* pAllocator) override
  {
    ++m_iDeallocs;

    nsTestClass2* pPointer = (nsTestClass2*)pObject;
    NS_DEFAULT_DELETE(pPointer);
  }

  static nsInt32 m_iAllocs;
  static nsInt32 m_iDeallocs;
};


class nsTestClass2b : nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsTestClass2b, nsReflectedClass);

public:
  nsTestClass2b() { m_sText = "Tut"; }

  const char* GetText() const { return m_sText.GetData(); }
  void SetText(const char* szSz) { m_sText = szSz; }

  nsTestStruct3 m_Struct;
  nsColor m_Color;

private:
  nsString m_sText;
};


class nsTestArrays : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsTestArrays, nsReflectedClass);

public:
  nsTestArrays() = default;

  bool operator==(const nsTestArrays& rhs) const
  {
    return m_Hybrid == rhs.m_Hybrid && m_Dynamic == rhs.m_Dynamic && m_Deque == rhs.m_Deque && m_HybridChar == rhs.m_HybridChar && m_CustomVariant == rhs.m_CustomVariant;
  }

  bool operator!=(const nsTestArrays& rhs) const { return !(*this == rhs); }

  nsUInt32 GetCount() const;
  double GetValue(nsUInt32 uiIndex) const;
  void SetValue(nsUInt32 uiIndex, double value);
  void Insert(nsUInt32 uiIndex, double value);
  void Remove(nsUInt32 uiIndex);

  nsUInt32 GetCountChar() const;
  const char* GetValueChar(nsUInt32 uiIndex) const;
  void SetValueChar(nsUInt32 uiIndex, const char* value);
  void InsertChar(nsUInt32 uiIndex, const char* value);
  void RemoveChar(nsUInt32 uiIndex);

  nsUInt32 GetCountDyn() const;
  const nsTestStruct3& GetValueDyn(nsUInt32 uiIndex) const;
  void SetValueDyn(nsUInt32 uiIndex, const nsTestStruct3& value);
  void InsertDyn(nsUInt32 uiIndex, const nsTestStruct3& value);
  void RemoveDyn(nsUInt32 uiIndex);

  nsUInt32 GetCountDeq() const;
  const nsTestArrays& GetValueDeq(nsUInt32 uiIndex) const;
  void SetValueDeq(nsUInt32 uiIndex, const nsTestArrays& value);
  void InsertDeq(nsUInt32 uiIndex, const nsTestArrays& value);
  void RemoveDeq(nsUInt32 uiIndex);

  nsUInt32 GetCountCustom() const;
  nsVarianceTypeAngle GetValueCustom(nsUInt32 uiIndex) const;
  void SetValueCustom(nsUInt32 uiIndex, nsVarianceTypeAngle value);
  void InsertCustom(nsUInt32 uiIndex, nsVarianceTypeAngle value);
  void RemoveCustom(nsUInt32 uiIndex);

  nsHybridArray<double, 5> m_Hybrid;
  nsHybridArray<nsString, 2> m_HybridChar;
  nsDynamicArray<nsTestStruct3> m_Dynamic;
  nsDeque<nsTestArrays> m_Deque;
  nsHybridArray<nsVarianceTypeAngle, 1> m_CustomVariant;
};


class nsTestSets : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsTestSets, nsReflectedClass);

public:
  nsTestSets() = default;

  bool operator==(const nsTestSets& rhs) const
  {
    return m_SetMember == rhs.m_SetMember && m_SetAccessor == rhs.m_SetAccessor && m_Deque == rhs.m_Deque && m_Array == rhs.m_Array && m_CustomVariant == rhs.m_CustomVariant;
  }

  bool operator!=(const nsTestSets& rhs) const { return !(*this == rhs); }

  const nsSet<double>& GetSet() const;
  void Insert(double value);
  void Remove(double value);

  const nsHashSet<nsInt64>& GetHashSet() const;
  void HashInsert(nsInt64 value);
  void HashRemove(nsInt64 value);

  const nsDeque<int>& GetPseudoSet() const;
  void PseudoInsert(int value);
  void PseudoRemove(int value);

  nsArrayPtr<const nsString> GetPseudoSet2() const;
  void PseudoInsert2(const nsString& value);
  void PseudoRemove2(const nsString& value);

  void PseudoInsert2b(const char* value);
  void PseudoRemove2b(const char* value);

  const nsHashSet<nsVarianceTypeAngle>& GetCustomHashSet() const;
  void CustomHashInsert(nsVarianceTypeAngle value);
  void CustomHashRemove(nsVarianceTypeAngle value);

  nsSet<nsInt8> m_SetMember;
  nsSet<double> m_SetAccessor;

  nsHashSet<nsInt32> m_HashSetMember;
  nsHashSet<nsInt64> m_HashSetAccessor;

  nsDeque<int> m_Deque;
  nsDynamicArray<nsString> m_Array;
  nsHashSet<nsVarianceTypeAngle> m_CustomVariant;
};


class nsTestMaps : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsTestMaps, nsReflectedClass);

public:
  nsTestMaps() = default;

  bool operator==(const nsTestMaps& rhs) const;

  const nsMap<nsString, nsInt64>& GetContainer() const;
  void Insert(const char* szKey, nsInt64 value);
  void Remove(const char* szKey);

  const nsHashTable<nsString, nsString>& GetContainer2() const;
  void Insert2(const char* szKey, const nsString& value);
  void Remove2(const char* szKey);

  const nsRangeView<const char*, nsUInt32> GetKeys3() const;
  void Insert3(const char* szKey, const nsVariant& value);
  void Remove3(const char* szKey);
  bool GetValue3(const char* szKey, nsVariant& out_value) const;

  nsMap<nsString, int> m_MapMember;
  nsMap<nsString, nsInt64> m_MapAccessor;

  nsHashTable<nsString, double> m_HashTableMember;
  nsHashTable<nsString, nsString> m_HashTableAccessor;

  nsMap<nsString, nsVarianceTypeAngle> m_CustomVariant;

  struct Tuple
  {
    nsString m_Key;
    nsVariant m_Value;
  };
  nsHybridArray<Tuple, 2> m_Accessor3;
};

class nsTestPtr : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsTestPtr, nsReflectedClass);

public:
  nsTestPtr()
  {
    m_pArrays = nullptr;
    m_pArraysDirect = nullptr;
  }

  ~nsTestPtr()
  {
    NS_DEFAULT_DELETE(m_pArrays);
    NS_DEFAULT_DELETE(m_pArraysDirect);
    for (auto ptr : m_ArrayPtr)
    {
      NS_DEFAULT_DELETE(ptr);
    }
    m_ArrayPtr.Clear();
    for (auto ptr : m_SetPtr)
    {
      NS_DEFAULT_DELETE(ptr);
    }
    m_SetPtr.Clear();
  }

  bool operator==(const nsTestPtr& rhs) const
  {
    if (m_sString != rhs.m_sString || (m_pArrays != rhs.m_pArrays && *m_pArrays != *rhs.m_pArrays))
      return false;

    if (m_ArrayPtr.GetCount() != rhs.m_ArrayPtr.GetCount())
      return false;

    for (nsUInt32 i = 0; i < m_ArrayPtr.GetCount(); i++)
    {
      if (!(*m_ArrayPtr[i] == *rhs.m_ArrayPtr[i]))
        return false;
    }

    // only works for the test data if the test.
    if (m_SetPtr.IsEmpty() && rhs.m_SetPtr.IsEmpty())
      return true;

    if (m_SetPtr.GetCount() != 1 || rhs.m_SetPtr.GetCount() != 1)
      return true;

    return *m_SetPtr.GetIterator().Key() == *rhs.m_SetPtr.GetIterator().Key();
  }

  void SetString(const char* szValue) { m_sString = szValue; }
  const char* GetString() const { return m_sString; }

  void SetArrays(nsTestArrays* pValue) { m_pArrays = pValue; }
  nsTestArrays* GetArrays() const { return m_pArrays; }


  nsString m_sString;
  nsTestArrays* m_pArrays;
  nsTestArrays* m_pArraysDirect;
  nsDeque<nsTestArrays*> m_ArrayPtr;
  nsSet<nsTestSets*> m_SetPtr;
};


struct nsTestEnumStruct
{
  NS_ALLOW_PRIVATE_PROPERTIES(nsTestEnumStruct);

public:
  nsTestEnumStruct()
  {
    m_enum = nsExampleEnum::Value1;
    m_enumClass = nsExampleEnum::Value1;
    m_Enum2 = nsExampleEnum::Value1;
    m_EnumClass2 = nsExampleEnum::Value1;
  }

  bool operator==(const nsTestEnumStruct& rhs) const { return m_Enum2 == rhs.m_Enum2 && m_enum == rhs.m_enum && m_enumClass == rhs.m_enumClass && m_EnumClass2 == rhs.m_EnumClass2; }

  nsExampleEnum::Enum m_enum;
  nsEnum<nsExampleEnum> m_enumClass;

  void SetEnum(nsExampleEnum::Enum e) { m_Enum2 = e; }
  nsExampleEnum::Enum GetEnum() const { return m_Enum2; }
  void SetEnumClass(nsEnum<nsExampleEnum> e) { m_EnumClass2 = e; }
  nsEnum<nsExampleEnum> GetEnumClass() const { return m_EnumClass2; }

private:
  nsExampleEnum::Enum m_Enum2;
  nsEnum<nsExampleEnum> m_EnumClass2;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, nsTestEnumStruct);


struct nsTestBitflagsStruct
{
  NS_ALLOW_PRIVATE_PROPERTIES(nsTestBitflagsStruct);

public:
  nsTestBitflagsStruct()
  {
    m_bitflagsClass = nsExampleBitflags::Value1;
    m_BitflagsClass2 = nsExampleBitflags::Value1;
  }

  bool operator==(const nsTestBitflagsStruct& rhs) const { return m_bitflagsClass == rhs.m_bitflagsClass && m_BitflagsClass2 == rhs.m_BitflagsClass2; }

  nsBitflags<nsExampleBitflags> m_bitflagsClass;

  void SetBitflagsClass(nsBitflags<nsExampleBitflags> e) { m_BitflagsClass2 = e; }
  nsBitflags<nsExampleBitflags> GetBitflagsClass() const { return m_BitflagsClass2; }

private:
  nsBitflags<nsExampleBitflags> m_BitflagsClass2;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, nsTestBitflagsStruct);
