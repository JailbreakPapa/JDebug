#pragma once

#include <Foundation/Math/Color8UNorm.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/VarianceTypes.h>

struct nsIntegerStruct
{
public:
  nsIntegerStruct()
  {
    m_iInt8 = 1;
    m_uiUInt8 = 1;
    m_iInt16 = 1;
    m_iUInt16 = 1;
    m_iInt32 = 1;
    m_uiUInt32 = 1;
    m_iInt64 = 1;
    m_iUInt64 = 1;
  }

  void SetInt8(nsInt8 i) { m_iInt8 = i; }
  nsInt8 GetInt8() const { return m_iInt8; }
  void SetUInt8(nsUInt8 i) { m_uiUInt8 = i; }
  nsUInt8 GetUInt8() const { return m_uiUInt8; }
  void SetInt32(nsInt32 i) { m_iInt32 = i; }
  nsInt32 GetInt32() const { return m_iInt32; }
  void SetUInt32(nsUInt32 i) { m_uiUInt32 = i; }
  nsUInt32 GetUInt32() const { return m_uiUInt32; }

  nsInt16 m_iInt16;
  nsUInt16 m_iUInt16;
  nsInt64 m_iInt64;
  nsUInt64 m_iUInt64;

private:
  nsInt8 m_iInt8;
  nsUInt8 m_uiUInt8;
  nsInt32 m_iInt32;
  nsUInt32 m_uiUInt32;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, nsIntegerStruct);


struct nsFloatStruct
{
public:
  nsFloatStruct()
  {
    m_fFloat = 1.0f;
    m_fDouble = 1.0;
    m_Time = nsTime::MakeFromSeconds(1.0);
    m_Angle = nsAngle::MakeFromDegree(45.0f);
  }

  void SetFloat(float f) { m_fFloat = f; }
  float GetFloat() const { return m_fFloat; }
  void SetDouble(double d) { m_fDouble = d; }
  double GetDouble() const { return m_fDouble; }
  void SetTime(nsTime t) { m_Time = t; }
  nsTime GetTime() const { return m_Time; }
  nsAngle GetAngle() const { return m_Angle; }
  void SetAngle(nsAngle t) { m_Angle = t; }

private:
  float m_fFloat;
  double m_fDouble;
  nsTime m_Time;
  nsAngle m_Angle;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, nsFloatStruct);


class nsPODClass : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsPODClass, nsReflectedClass);

public:
  nsPODClass()
  {
    m_bBool = true;
    m_Color = nsColor(1.0f, 0.0f, 0.0f, 0.0f);
    m_Color2 = nsColorGammaUB(255, 10, 1);
    m_sCharPtr = "Test";
    m_sString = "Test";
    m_sStringView = "Test";
    m_Buffer.PushBack(0xFF);
    m_Buffer.PushBack(0x0);
    m_Buffer.PushBack(0xCD);
    m_VarianceAngle = {0.1f, nsAngle::MakeFromDegree(90.0f)};
  }

  nsIntegerStruct m_IntegerStruct;
  nsFloatStruct m_FloatStruct;

  bool GetBool() const { return m_bBool; }
  void SetBool(bool b) { m_bBool = b; }

  nsColor GetColor() const { return m_Color; }
  void SetColor(nsColor c) { m_Color = c; }

  const char* GetCharPtr() const { return m_sCharPtr.GetData(); }
  void SetCharPtr(const char* szSz) { m_sCharPtr = szSz; }

  const nsString& GetString() const { return m_sString; }
  void SetString(const nsString& sStr) { m_sString = sStr; }

  nsStringView GetStringView() const { return m_sStringView.GetView(); }
  void SetStringView(nsStringView sStrView) { m_sStringView = sStrView; }

  const nsDataBuffer& GetBuffer() const { return m_Buffer; }
  void SetBuffer(const nsDataBuffer& data) { m_Buffer = data; }

  nsVarianceTypeAngle GetCustom() const { return m_VarianceAngle; }
  void SetCustom(nsVarianceTypeAngle value) { m_VarianceAngle = value; }

private:
  bool m_bBool;
  nsColor m_Color;
  nsColorGammaUB m_Color2;
  nsString m_sCharPtr;
  nsString m_sString;
  nsString m_sStringView;
  nsDataBuffer m_Buffer;
  nsVarianceTypeAngle m_VarianceAngle;
};


class nsMathClass : public nsPODClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsMathClass, nsPODClass);

public:
  nsMathClass()
  {
    m_vVec2 = nsVec2(1.0f, 1.0f);
    m_vVec3 = nsVec3(1.0f, 1.0f, 1.0f);
    m_vVec4 = nsVec4(1.0f, 1.0f, 1.0f, 1.0f);
    m_Vec2I = nsVec2I32(1, 1);
    m_Vec3I = nsVec3I32(1, 1, 1);
    m_Vec4I = nsVec4I32(1, 1, 1, 1);
    m_qQuat = nsQuat(1.0f, 1.0f, 1.0f, 1.0f);
    m_mMat3.SetZero();
    m_mMat4.SetZero();
  }

  void SetVec2(nsVec2 v) { m_vVec2 = v; }
  nsVec2 GetVec2() const { return m_vVec2; }
  void SetVec3(nsVec3 v) { m_vVec3 = v; }
  nsVec3 GetVec3() const { return m_vVec3; }
  void SetVec4(nsVec4 v) { m_vVec4 = v; }
  nsVec4 GetVec4() const { return m_vVec4; }
  void SetQuat(nsQuat q) { m_qQuat = q; }
  nsQuat GetQuat() const { return m_qQuat; }
  void SetMat3(nsMat3 m) { m_mMat3 = m; }
  nsMat3 GetMat3() const { return m_mMat3; }
  void SetMat4(nsMat4 m) { m_mMat4 = m; }
  nsMat4 GetMat4() const { return m_mMat4; }

  nsVec2I32 m_Vec2I;
  nsVec3I32 m_Vec3I;
  nsVec4I32 m_Vec4I;

private:
  nsVec2 m_vVec2;
  nsVec3 m_vVec3;
  nsVec4 m_vVec4;
  nsQuat m_qQuat;
  nsMat3 m_mMat3;
  nsMat4 m_mMat4;
};


struct nsExampleEnum
{
  using StorageType = nsInt8;
  enum Enum
  {
    Value1 = 0,      // normal value
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
NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, nsExampleBitflags);


class nsEnumerationsClass : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsEnumerationsClass, nsReflectedClass);

public:
  nsEnumerationsClass()
  {
    m_EnumClass = nsExampleEnum::Value2;
    m_BitflagsClass = nsExampleBitflags::Value2;
  }

  void SetEnum(nsExampleEnum::Enum e) { m_EnumClass = e; }
  nsExampleEnum::Enum GetEnum() const { return m_EnumClass; }
  void SetBitflags(nsBitflags<nsExampleBitflags> e) { m_BitflagsClass = e; }
  nsBitflags<nsExampleBitflags> GetBitflags() const { return m_BitflagsClass; }

private:
  nsEnum<nsExampleEnum> m_EnumClass;
  nsBitflags<nsExampleBitflags> m_BitflagsClass;
};


struct InnerStruct
{
  NS_DECLARE_POD_TYPE();

public:
  float m_fP1;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, InnerStruct);


class OuterClass : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(OuterClass, nsReflectedClass);

public:
  InnerStruct m_Inner1;
  float m_fP1;
};

class ExtendedOuterClass : public OuterClass
{
  NS_ADD_DYNAMIC_REFLECTION(ExtendedOuterClass, OuterClass);

public:
  nsString m_more;
};

class nsObjectTest : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsObjectTest, nsReflectedClass);

public:
  nsObjectTest() = default;
  ~nsObjectTest()
  {
    for (OuterClass* pTest : m_ClassPtrArray)
    {
      nsGetStaticRTTI<OuterClass>()->GetAllocator()->Deallocate(pTest);
    }
    for (nsObjectTest* pTest : m_SubObjectSet)
    {
      nsGetStaticRTTI<nsObjectTest>()->GetAllocator()->Deallocate(pTest);
    }
    for (auto it = m_ClassPtrMap.GetIterator(); it.IsValid(); ++it)
    {
      nsGetStaticRTTI<OuterClass>()->GetAllocator()->Deallocate(it.Value());
    }
  }

  nsArrayPtr<const nsString> GetStandardTypeSet() const;
  void StandardTypeSetInsert(const nsString& value);
  void StandardTypeSetRemove(const nsString& value);

  OuterClass m_MemberClass;

  nsDynamicArray<double> m_StandardTypeArray;
  nsDynamicArray<OuterClass> m_ClassArray;
  nsDeque<OuterClass*> m_ClassPtrArray;

  nsDynamicArray<nsString> m_StandardTypeSet;
  nsSet<nsObjectTest*> m_SubObjectSet;

  nsMap<nsString, double> m_StandardTypeMap;
  nsHashTable<nsString, OuterClass> m_ClassMap;
  nsMap<nsString, OuterClass*> m_ClassPtrMap;
};


class nsMirrorTest : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsMirrorTest, nsReflectedClass);

public:
  nsMirrorTest() = default;

  nsMathClass m_math;
  nsObjectTest m_object;
};
