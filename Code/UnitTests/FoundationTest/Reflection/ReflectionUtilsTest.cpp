#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Reflection/Implementation/PropertyAttributes.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Serialization/ReflectionSerializer.h>
#include <FoundationTest/Reflection/ReflectionTestClasses.h>

template <typename T>
static void SetComponentTest(nsVec2Template<T> vVector, T value)
{
  nsVariant var = vVector;
  nsReflectionUtils::SetComponent(var, 0, value);
  NS_TEST_BOOL(var.Get<nsVec2Template<T>>().x == value);
  nsReflectionUtils::SetComponent(var, 1, value);
  NS_TEST_BOOL(var.Get<nsVec2Template<T>>().y == value);
}

template <typename T>
static void SetComponentTest(nsVec3Template<T> vVector, T value)
{
  nsVariant var = vVector;
  nsReflectionUtils::SetComponent(var, 0, value);
  NS_TEST_BOOL(var.Get<nsVec3Template<T>>().x == value);
  nsReflectionUtils::SetComponent(var, 1, value);
  NS_TEST_BOOL(var.Get<nsVec3Template<T>>().y == value);
  nsReflectionUtils::SetComponent(var, 2, value);
  NS_TEST_BOOL(var.Get<nsVec3Template<T>>().z == value);
}

template <typename T>
static void SetComponentTest(nsVec4Template<T> vVector, T value)
{
  nsVariant var = vVector;
  nsReflectionUtils::SetComponent(var, 0, value);
  NS_TEST_BOOL(var.Get<nsVec4Template<T>>().x == value);
  nsReflectionUtils::SetComponent(var, 1, value);
  NS_TEST_BOOL(var.Get<nsVec4Template<T>>().y == value);
  nsReflectionUtils::SetComponent(var, 2, value);
  NS_TEST_BOOL(var.Get<nsVec4Template<T>>().z == value);
  nsReflectionUtils::SetComponent(var, 3, value);
  NS_TEST_BOOL(var.Get<nsVec4Template<T>>().w == value);
}

template <class T>
static void ClampValueTest(T tooSmall, T tooBig, T min, T max)
{
  nsClampValueAttribute minClamp(min, {});
  nsClampValueAttribute maxClamp({}, max);
  nsClampValueAttribute bothClamp(min, max);

  nsVariant value = tooSmall;
  NS_TEST_BOOL(nsReflectionUtils::ClampValue(value, &minClamp).Succeeded());
  NS_TEST_BOOL(value == min);

  value = tooSmall;
  NS_TEST_BOOL(nsReflectionUtils::ClampValue(value, &bothClamp).Succeeded());
  NS_TEST_BOOL(value == min);

  value = tooBig;
  NS_TEST_BOOL(nsReflectionUtils::ClampValue(value, &maxClamp).Succeeded());
  NS_TEST_BOOL(value == max);

  value = tooBig;
  NS_TEST_BOOL(nsReflectionUtils::ClampValue(value, &bothClamp).Succeeded());
  NS_TEST_BOOL(value == max);
}


NS_CREATE_SIMPLE_TEST(Reflection, Utils)
{
  nsDefaultMemoryStreamStorage StreamStorage;

  NS_TEST_BLOCK(nsTestBlock::Enabled, "WriteObjectToDDL")
  {
    nsMemoryStreamWriter FileOut(&StreamStorage);

    nsTestClass2 c2;
    c2.SetCharPtr("Hallo");
    c2.SetString("World");
    c2.SetStringView("!!!");
    c2.m_MyVector.Set(14, 16, 18);
    c2.m_Struct.m_fFloat1 = 128;
    c2.m_Struct.m_UInt8 = 234;
    c2.m_Struct.m_Angle = nsAngle::MakeFromDegree(360);
    c2.m_Struct.m_vVec3I = nsVec3I32(9, 8, 7);
    c2.m_Struct.m_DataBuffer.Clear();
    c2.m_Color = nsColor(0.1f, 0.2f, 0.3f);
    c2.m_Time = nsTime::MakeFromSeconds(91.0f);
    c2.m_enumClass = nsExampleEnum::Value3;
    c2.m_bitflagsClass = nsExampleBitflags::Value1 | nsExampleBitflags::Value2 | nsExampleBitflags::Value3;
    c2.m_array.PushBack(5.0f);
    c2.m_array.PushBack(10.0f);
    c2.m_Variant = nsVec3(1.0f, 2.0f, 3.0f);

    nsReflectionSerializer::WriteObjectToDDL(FileOut, c2.GetDynamicRTTI(), &c2, false, nsOpenDdlWriter::TypeStringMode::Compliant);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReadObjectPropertiesFromDDL")
  {
    nsMemoryStreamReader FileIn(&StreamStorage);

    nsTestClass2 c2;

    nsReflectionSerializer::ReadObjectPropertiesFromDDL(FileIn, *c2.GetDynamicRTTI(), &c2);

    NS_TEST_STRING(c2.GetCharPtr(), "Hallo");
    NS_TEST_STRING(c2.GetString(), "World");
    NS_TEST_STRING(c2.GetStringView(), "!!!");
    NS_TEST_VEC3(c2.m_MyVector, nsVec3(3, 4, 5), 0.0f);
    NS_TEST_FLOAT(c2.m_Time.GetSeconds(), 91.0f, 0.0f);
    NS_TEST_FLOAT(c2.m_Color.r, 0.1f, 0.0f);
    NS_TEST_FLOAT(c2.m_Color.g, 0.2f, 0.0f);
    NS_TEST_FLOAT(c2.m_Color.b, 0.3f, 0.0f);
    NS_TEST_FLOAT(c2.m_Struct.m_fFloat1, 128, 0.0f);
    NS_TEST_INT(c2.m_Struct.m_UInt8, 234);
    NS_TEST_BOOL(c2.m_Struct.m_Angle == nsAngle::MakeFromDegree(360));
    NS_TEST_BOOL(c2.m_Struct.m_vVec3I == nsVec3I32(9, 8, 7));
    NS_TEST_BOOL(c2.m_Struct.m_DataBuffer == nsDataBuffer());
    NS_TEST_BOOL(c2.m_enumClass == nsExampleEnum::Value3);
    NS_TEST_BOOL(c2.m_bitflagsClass == (nsExampleBitflags::Value1 | nsExampleBitflags::Value2 | nsExampleBitflags::Value3));
    NS_TEST_INT(c2.m_array.GetCount(), 2);
    if (c2.m_array.GetCount() == 2)
    {
      NS_TEST_FLOAT(c2.m_array[0], 5.0f, 0.0f);
      NS_TEST_FLOAT(c2.m_array[1], 10.0f, 0.0f);
    }
    NS_TEST_VEC3(c2.m_Variant.Get<nsVec3>(), nsVec3(1.0f, 2.0f, 3.0f), 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReadObjectPropertiesFromDDL (different type)")
  {
    // here we restore the same properties into a different type of object which has properties that are named the same
    // but may have slightly different types (but which are compatible)

    nsMemoryStreamReader FileIn(&StreamStorage);

    nsTestClass2b c2;

    nsReflectionSerializer::ReadObjectPropertiesFromDDL(FileIn, *c2.GetDynamicRTTI(), &c2);

    NS_TEST_STRING(c2.GetText(), "Tut"); // not restored, different property name
    NS_TEST_FLOAT(c2.m_Color.r, 0.1f, 0.0f);
    NS_TEST_FLOAT(c2.m_Color.g, 0.2f, 0.0f);
    NS_TEST_FLOAT(c2.m_Color.b, 0.3f, 0.0f);
    NS_TEST_FLOAT(c2.m_Struct.m_fFloat1, 128, 0.0f);
    NS_TEST_INT(c2.m_Struct.m_UInt8, 234);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReadObjectFromDDL")
  {
    nsMemoryStreamReader FileIn(&StreamStorage);

    const nsRTTI* pRtti;
    void* pObject = nsReflectionSerializer::ReadObjectFromDDL(FileIn, pRtti);

    nsTestClass2& c2 = *((nsTestClass2*)pObject);

    NS_TEST_STRING(c2.GetCharPtr(), "Hallo");
    NS_TEST_STRING(c2.GetString(), "World");
    NS_TEST_STRING(c2.GetStringView(), "!!!");
    NS_TEST_VEC3(c2.m_MyVector, nsVec3(3, 4, 5), 0.0f);
    NS_TEST_FLOAT(c2.m_Time.GetSeconds(), 91.0f, 0.0f);
    NS_TEST_FLOAT(c2.m_Color.r, 0.1f, 0.0f);
    NS_TEST_FLOAT(c2.m_Color.g, 0.2f, 0.0f);
    NS_TEST_FLOAT(c2.m_Color.b, 0.3f, 0.0f);
    NS_TEST_FLOAT(c2.m_Struct.m_fFloat1, 128, 0.0f);
    NS_TEST_INT(c2.m_Struct.m_UInt8, 234);
    NS_TEST_BOOL(c2.m_Struct.m_Angle == nsAngle::MakeFromDegree(360));
    NS_TEST_BOOL(c2.m_Struct.m_vVec3I == nsVec3I32(9, 8, 7));
    NS_TEST_BOOL(c2.m_Struct.m_DataBuffer == nsDataBuffer());
    NS_TEST_BOOL(c2.m_enumClass == nsExampleEnum::Value3);
    NS_TEST_BOOL(c2.m_bitflagsClass == (nsExampleBitflags::Value1 | nsExampleBitflags::Value2 | nsExampleBitflags::Value3));
    NS_TEST_INT(c2.m_array.GetCount(), 2);
    if (c2.m_array.GetCount() == 2)
    {
      NS_TEST_FLOAT(c2.m_array[0], 5.0f, 0.0f);
      NS_TEST_FLOAT(c2.m_array[1], 10.0f, 0.0f);
    }
    NS_TEST_VEC3(c2.m_Variant.Get<nsVec3>(), nsVec3(1.0f, 2.0f, 3.0f), 0.0f);

    if (pObject)
    {
      pRtti->GetAllocator()->Deallocate(pObject);
    }
  }

  nsFileSystem::ClearAllDataDirectories();

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetComponent")
  {
    SetComponentTest(nsVec2(0.0f, 0.1f), -0.5f);
    SetComponentTest(nsVec3(0.0f, 0.1f, 0.2f), -0.5f);
    SetComponentTest(nsVec4(0.0f, 0.1f, 0.2f, 0.3f), -0.5f);
    SetComponentTest(nsVec2I32(0, 1), -4);
    SetComponentTest(nsVec3I32(0, 1, 2), -4);
    SetComponentTest(nsVec4I32(0, 1, 2, 3), -4);
    SetComponentTest(nsVec2U32(0, 1), 4u);
    SetComponentTest(nsVec3U32(0, 1, 2), 4u);
    SetComponentTest(nsVec4U32(0, 1, 2, 3), 4u);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ClampValue")
  {
    ClampValueTest<float>(-1, 1000, 2, 4);
    ClampValueTest<double>(-1, 1000, 2, 4);
    ClampValueTest<nsInt32>(-1, 1000, 2, 4);
    ClampValueTest<nsUInt64>(1, 1000, 2, 4);
    ClampValueTest<nsTime>(nsTime::MakeFromMilliseconds(1), nsTime::MakeFromMilliseconds(1000), nsTime::MakeFromMilliseconds(2), nsTime::MakeFromMilliseconds(4));
    ClampValueTest<nsAngle>(nsAngle::MakeFromDegree(1), nsAngle::MakeFromDegree(1000), nsAngle::MakeFromDegree(2), nsAngle::MakeFromDegree(4));
    ClampValueTest<nsVec3>(nsVec3(1), nsVec3(1000), nsVec3(2), nsVec3(4));
    ClampValueTest<nsVec4I32>(nsVec4I32(1), nsVec4I32(1000), nsVec4I32(2), nsVec4I32(4));
    ClampValueTest<nsVec4U32>(nsVec4U32(1), nsVec4U32(1000), nsVec4U32(2), nsVec4U32(4));

    nsVarianceTypeFloat vf = {1.0f, 2.0f};
    nsVariant variance = vf;
    NS_TEST_BOOL(nsReflectionUtils::ClampValue(variance, nullptr).Succeeded());

    nsVarianceTypeFloat clamp = {2.0f, 3.0f};
    nsClampValueAttribute minClamp(clamp, {});
    NS_TEST_BOOL(nsReflectionUtils::ClampValue(variance, &minClamp).Failed());
  }
}
