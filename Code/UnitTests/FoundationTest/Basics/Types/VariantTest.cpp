#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/VarianceTypes.h>
#include <Foundation/Types/Variant.h>
#include <FoundationTest/Reflection/ReflectionTestClasses.h>

// this file takes ages to compile in a Release build
// since we don't care for runtime performance, just disable all optimizations
#pragma optimize("", off)

class Blubb : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(Blubb, nsReflectedClass);

public:
  float u;
  float v;
};

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(Blubb, 1, nsRTTINoAllocator)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("u", u),
    NS_MEMBER_PROPERTY("v", v),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

template <typename T>
void TestVariant(nsVariant& v, nsVariantType::Enum type)
{
  NS_TEST_BOOL(v.IsValid());
  NS_TEST_BOOL(v.GetType() == type);
  NS_TEST_BOOL(v.CanConvertTo<T>());
  NS_TEST_BOOL(v.IsA<T>());
  NS_TEST_BOOL(v.GetReflectedType() == nsGetStaticRTTI<T>());

  nsTypedPointer ptr = v.GetWriteAccess();
  NS_TEST_BOOL(ptr.m_pObject == &v.Get<T>());
  NS_TEST_BOOL(ptr.m_pObject == &v.GetWritable<T>());
  NS_TEST_BOOL(ptr.m_pType == nsGetStaticRTTI<T>());

  NS_TEST_BOOL(ptr.m_pObject == v.GetData());

  nsVariant vCopy = v;
  nsTypedPointer ptr2 = vCopy.GetWriteAccess();
  NS_TEST_BOOL(ptr2.m_pObject == &vCopy.Get<T>());
  NS_TEST_BOOL(ptr2.m_pObject == &vCopy.GetWritable<T>());

  NS_TEST_BOOL(ptr2.m_pObject != ptr.m_pObject);
  NS_TEST_BOOL(ptr2.m_pType == nsGetStaticRTTI<T>());

  NS_TEST_BOOL(v.Get<T>() == vCopy.Get<T>());

  NS_TEST_BOOL(v.ComputeHash(0) != 0);
}

template <typename T>
inline void TestIntegerVariant(nsVariant::Type::Enum type)
{
  nsVariant b((T)23);
  TestVariant<T>(b, type);

  NS_TEST_BOOL(b.Get<T>() == 23);

  NS_TEST_BOOL(b == nsVariant(23));
  NS_TEST_BOOL(b != nsVariant(11));
  NS_TEST_BOOL(b == nsVariant((T)23));
  NS_TEST_BOOL(b != nsVariant((T)11));

  NS_TEST_BOOL(b == 23);
  NS_TEST_BOOL(b != 24);
  NS_TEST_BOOL(b == (T)23);
  NS_TEST_BOOL(b != (T)24);

  b = (T)17;
  NS_TEST_BOOL(b == (T)17);

  b = nsVariant((T)19);
  NS_TEST_BOOL(b == (T)19);

  NS_TEST_BOOL(b.IsNumber());
  NS_TEST_BOOL(b.IsFloatingPoint() == false);
  NS_TEST_BOOL(!b.IsString());
}

inline void TestNumberCanConvertTo(const nsVariant& v)
{
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Invalid) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Bool));
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Int8));
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::UInt8));
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Int16));
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::UInt16));
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Int32));
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::UInt32));
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Int64));
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::UInt64));
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Float));
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Double));
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Color) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector2) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector3) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector4) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector2I) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector3I) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector4I) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Quaternion) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Matrix3) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Matrix4) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Transform) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::String));
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::StringView) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::DataBuffer) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Time) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Uuid) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Angle) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::ColorGamma) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::HashedString));
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::TempHashedString));
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::VariantArray) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::VariantDictionary) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::TypedPointer) == false);
  NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::TypedObject) == false);

  nsResult conversionResult = NS_FAILURE;
  NS_TEST_BOOL(v.ConvertTo<bool>(&conversionResult) == true);
  NS_TEST_BOOL(conversionResult.Succeeded());

  NS_TEST_BOOL(v.ConvertTo<nsInt8>(&conversionResult) == 3);
  NS_TEST_BOOL(conversionResult.Succeeded());

  NS_TEST_BOOL(v.ConvertTo<nsUInt8>(&conversionResult) == 3);
  NS_TEST_BOOL(conversionResult.Succeeded());

  NS_TEST_BOOL(v.ConvertTo<nsInt16>(&conversionResult) == 3);
  NS_TEST_BOOL(conversionResult.Succeeded());

  NS_TEST_BOOL(v.ConvertTo<nsUInt16>(&conversionResult) == 3);
  NS_TEST_BOOL(conversionResult.Succeeded());

  NS_TEST_BOOL(v.ConvertTo<nsInt32>(&conversionResult) == 3);
  NS_TEST_BOOL(conversionResult.Succeeded());

  NS_TEST_BOOL(v.ConvertTo<nsUInt32>(&conversionResult) == 3);
  NS_TEST_BOOL(conversionResult.Succeeded());

  NS_TEST_BOOL(v.ConvertTo<nsInt64>(&conversionResult) == 3);
  NS_TEST_BOOL(conversionResult.Succeeded());

  NS_TEST_BOOL(v.ConvertTo<nsUInt64>(&conversionResult) == 3);
  NS_TEST_BOOL(conversionResult.Succeeded());

  NS_TEST_BOOL(v.ConvertTo<float>(&conversionResult) == 3.0f);
  NS_TEST_BOOL(conversionResult.Succeeded());

  NS_TEST_BOOL(v.ConvertTo<double>(&conversionResult) == 3.0);
  NS_TEST_BOOL(conversionResult.Succeeded());

  NS_TEST_BOOL(v.ConvertTo<nsString>(&conversionResult) == "3");
  NS_TEST_BOOL(conversionResult.Succeeded());

  NS_TEST_BOOL(v.ConvertTo<nsHashedString>(&conversionResult) == nsMakeHashedString("3"));
  NS_TEST_BOOL(conversionResult.Succeeded());

  NS_TEST_BOOL(v.ConvertTo<nsTempHashedString>(&conversionResult) == nsTempHashedString("3"));
  NS_TEST_BOOL(conversionResult.Succeeded());

  NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Bool).Get<bool>() == true);
  NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Int8).Get<nsInt8>() == 3);
  NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::UInt8).Get<nsUInt8>() == 3);
  NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Int16).Get<nsInt16>() == 3);
  NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::UInt16).Get<nsUInt16>() == 3);
  NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Int32).Get<nsInt32>() == 3);
  NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::UInt32).Get<nsUInt32>() == 3);
  NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Int64).Get<nsInt64>() == 3);
  NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::UInt64).Get<nsUInt64>() == 3);
  NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Float).Get<float>() == 3.0f);
  NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Double).Get<double>() == 3.0);
  NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::String).Get<nsString>() == "3");
  NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::HashedString).Get<nsHashedString>() == nsMakeHashedString("3"));
  NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::TempHashedString).Get<nsTempHashedString>() == nsTempHashedString("3"));
}

inline void TestCanOnlyConvertToID(const nsVariant& v, nsVariant::Type::Enum type)
{
  for (int iType = nsVariant::Type::FirstStandardType; iType < nsVariant::Type::LastExtendedType; ++iType)
  {
    if (iType == nsVariant::Type::LastStandardType)
      iType = nsVariant::Type::FirstExtendedType;

    if (iType == type)
    {
      NS_TEST_BOOL(v.CanConvertTo(type));
    }
    else
    {
      NS_TEST_BOOL(v.CanConvertTo((nsVariant::Type::Enum)iType) == false);
    }
  }
}

inline void TestCanOnlyConvertToStringAndID(const nsVariant& v, nsVariant::Type::Enum type, nsVariant::Type::Enum type2 = nsVariant::Type::Invalid,
  nsVariant::Type::Enum type3 = nsVariant::Type::Invalid)
{
  if (type2 == nsVariant::Type::Invalid)
    type2 = type;

  for (int iType = nsVariant::Type::FirstStandardType; iType < nsVariant::Type::LastExtendedType; ++iType)
  {
    if (iType == nsVariant::Type::LastStandardType)
      iType = nsVariant::Type::FirstExtendedType;

    if (iType == nsVariant::Type::String || iType == nsVariant::Type::HashedString || iType == nsVariant::Type::TempHashedString)
    {
      NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::String));
    }
    else if (iType == type || iType == type2 || iType == type3)
    {
      NS_TEST_BOOL(v.CanConvertTo(type));
    }
    else
    {
      NS_TEST_BOOL(v.CanConvertTo((nsVariant::Type::Enum)iType) == false);
    }
  }
}

NS_CREATE_SIMPLE_TEST(Basics, Variant)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Invalid")
  {
    nsVariant b;
    NS_TEST_BOOL(b.GetType() == nsVariant::Type::Invalid);
    NS_TEST_BOOL(b == nsVariant());
    NS_TEST_BOOL(b != nsVariant(0));
    NS_TEST_BOOL(!b.IsValid());
    NS_TEST_BOOL(!b[0].IsValid());
    NS_TEST_BOOL(!b["x"].IsValid());
    NS_TEST_BOOL(b.GetReflectedType() == nullptr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "bool")
  {
    nsVariant b(true);
    TestVariant<bool>(b, nsVariantType::Bool);

    NS_TEST_BOOL(b.Get<bool>() == true);

    NS_TEST_BOOL(b == nsVariant(true));
    NS_TEST_BOOL(b != nsVariant(false));

    NS_TEST_BOOL(b == true);
    NS_TEST_BOOL(b != false);

    b = false;
    NS_TEST_BOOL(b == false);

    b = nsVariant(true);
    NS_TEST_BOOL(b == true);
    NS_TEST_BOOL(!b[0].IsValid());

    NS_TEST_BOOL(b.IsNumber());
    NS_TEST_BOOL(!b.IsString());
    NS_TEST_BOOL(b.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsInt8")
  {
    TestIntegerVariant<nsInt8>(nsVariant::Type::Int8);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsUInt8")
  {
    TestIntegerVariant<nsUInt8>(nsVariant::Type::UInt8);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsInt16")
  {
    TestIntegerVariant<nsInt16>(nsVariant::Type::Int16);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsUInt16")
  {
    TestIntegerVariant<nsUInt16>(nsVariant::Type::UInt16);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsInt32")
  {
    TestIntegerVariant<nsInt32>(nsVariant::Type::Int32);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsUInt32")
  {
    TestIntegerVariant<nsUInt32>(nsVariant::Type::UInt32);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsInt64")
  {
    TestIntegerVariant<nsInt64>(nsVariant::Type::Int64);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsUInt64")
  {
    TestIntegerVariant<nsUInt64>(nsVariant::Type::UInt64);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "float")
  {
    nsVariant b(42.0f);
    TestVariant<float>(b, nsVariantType::Float);

    NS_TEST_BOOL(b.Get<float>() == 42.0f);

    NS_TEST_BOOL(b == nsVariant(42));
    NS_TEST_BOOL(b != nsVariant(11));
    NS_TEST_BOOL(b == nsVariant(42.0));
    NS_TEST_BOOL(b != nsVariant(11.0));
    NS_TEST_BOOL(b == nsVariant(42.0f));
    NS_TEST_BOOL(b != nsVariant(11.0f));

    NS_TEST_BOOL(b == 42);
    NS_TEST_BOOL(b != 41);
    NS_TEST_BOOL(b == 42.0);
    NS_TEST_BOOL(b != 41.0);
    NS_TEST_BOOL(b == 42.0f);
    NS_TEST_BOOL(b != 41.0f);

    b = 17.0f;
    NS_TEST_BOOL(b == 17.0f);

    b = nsVariant(19.0f);
    NS_TEST_BOOL(b == 19.0f);

    NS_TEST_BOOL(b.IsNumber());
    NS_TEST_BOOL(!b.IsString());
    NS_TEST_BOOL(b.IsFloatingPoint());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "double")
  {
    nsVariant b(42.0);
    TestVariant<double>(b, nsVariantType::Double);
    NS_TEST_BOOL(b.Get<double>() == 42.0);

    NS_TEST_BOOL(b == nsVariant(42));
    NS_TEST_BOOL(b != nsVariant(11));
    NS_TEST_BOOL(b == nsVariant(42.0));
    NS_TEST_BOOL(b != nsVariant(11.0));
    NS_TEST_BOOL(b == nsVariant(42.0f));
    NS_TEST_BOOL(b != nsVariant(11.0f));

    NS_TEST_BOOL(b == 42);
    NS_TEST_BOOL(b != 41);
    NS_TEST_BOOL(b == 42.0);
    NS_TEST_BOOL(b != 41.0);
    NS_TEST_BOOL(b == 42.0f);
    NS_TEST_BOOL(b != 41.0f);

    b = 17.0;
    NS_TEST_BOOL(b == 17.0);

    b = nsVariant(19.0);
    NS_TEST_BOOL(b == 19.0);

    NS_TEST_BOOL(b.IsNumber());
    NS_TEST_BOOL(!b.IsString());
    NS_TEST_BOOL(b.IsFloatingPoint());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsColor")
  {
    nsVariant v(nsColor(1, 2, 3, 1));
    TestVariant<nsColor>(v, nsVariantType::Color);

    NS_TEST_BOOL(v.CanConvertTo<nsColorGammaUB>());
    NS_TEST_BOOL(v.ConvertTo<nsColorGammaUB>() == static_cast<nsColorGammaUB>(nsColor(1, 2, 3, 1)));
    NS_TEST_BOOL(v.Get<nsColor>() == nsColor(1, 2, 3, 1));

    NS_TEST_BOOL(v == nsVariant(nsColor(1, 2, 3)));
    NS_TEST_BOOL(v != nsVariant(nsColor(1, 1, 1)));

    NS_TEST_BOOL(v == nsColor(1, 2, 3));
    NS_TEST_BOOL(v != nsColor(1, 4, 3));

    v = nsColor(5, 8, 9);
    NS_TEST_BOOL(v == nsColor(5, 8, 9));

    v = nsVariant(nsColor(7, 9, 4));
    NS_TEST_BOOL(v == nsColor(7, 9, 4));
    NS_TEST_BOOL(v[0] == 7);
    NS_TEST_BOOL(v[1] == 9);
    NS_TEST_BOOL(v[2] == 4);
    NS_TEST_BOOL(v[3] == 1);
    NS_TEST_BOOL(v[4] == nsVariant());
    NS_TEST_BOOL(!v[4].IsValid());
    NS_TEST_BOOL(v["r"] == 7);
    NS_TEST_BOOL(v["g"] == 9);
    NS_TEST_BOOL(v["b"] == 4);
    NS_TEST_BOOL(v["a"] == 1);
    NS_TEST_BOOL(v["x"] == nsVariant());
    NS_TEST_BOOL(!v["x"].IsValid());

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(!v.IsString());
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsColorGammaUB")
  {
    nsVariant v(nsColorGammaUB(64, 128, 255, 255));
    TestVariant<nsColorGammaUB>(v, nsVariantType::ColorGamma);

    NS_TEST_BOOL(v.CanConvertTo<nsColor>());
    NS_TEST_BOOL(v.Get<nsColorGammaUB>() == nsColorGammaUB(64, 128, 255, 255));

    NS_TEST_BOOL(v == nsVariant(nsColorGammaUB(64, 128, 255, 255)));
    NS_TEST_BOOL(v != nsVariant(nsColorGammaUB(255, 128, 255, 255)));

    NS_TEST_BOOL(v == nsColorGammaUB(64, 128, 255, 255));
    NS_TEST_BOOL(v != nsColorGammaUB(64, 42, 255, 255));

    v = nsColorGammaUB(10, 50, 200);
    NS_TEST_BOOL(v == nsColorGammaUB(10, 50, 200));

    v = nsVariant(nsColorGammaUB(17, 120, 200));
    NS_TEST_BOOL(v == nsColorGammaUB(17, 120, 200));
    NS_TEST_BOOL(v[0] == 17);
    NS_TEST_BOOL(v[1] == 120);
    NS_TEST_BOOL(v[2] == 200);
    NS_TEST_BOOL(v[3] == 255);
    NS_TEST_BOOL(v[4] == nsVariant());
    NS_TEST_BOOL(!v[4].IsValid());
    NS_TEST_BOOL(v["r"] == 17);
    NS_TEST_BOOL(v["g"] == 120);
    NS_TEST_BOOL(v["b"] == 200);
    NS_TEST_BOOL(v["a"] == 255);
    NS_TEST_BOOL(v["x"] == nsVariant());
    NS_TEST_BOOL(!v["x"].IsValid());

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(!v.IsString());
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsVec2")
  {
    nsVariant v(nsVec2(1, 2));
    TestVariant<nsVec2>(v, nsVariantType::Vector2);

    NS_TEST_BOOL(v.Get<nsVec2>() == nsVec2(1, 2));

    NS_TEST_BOOL(v == nsVariant(nsVec2(1, 2)));
    NS_TEST_BOOL(v != nsVariant(nsVec2(1, 1)));

    NS_TEST_BOOL(v == nsVec2(1, 2));
    NS_TEST_BOOL(v != nsVec2(1, 4));

    v = nsVec2(5, 8);
    NS_TEST_BOOL(v == nsVec2(5, 8));

    v = nsVariant(nsVec2(7, 9));
    NS_TEST_BOOL(v == nsVec2(7, 9));
    NS_TEST_BOOL(v[0] == 7);
    NS_TEST_BOOL(v[1] == 9);
    NS_TEST_BOOL(v["x"] == 7);
    NS_TEST_BOOL(v["y"] == 9);

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(!v.IsString());
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsVec3")
  {
    nsVariant v(nsVec3(1, 2, 3));
    TestVariant<nsVec3>(v, nsVariantType::Vector3);

    NS_TEST_BOOL(v.Get<nsVec3>() == nsVec3(1, 2, 3));

    NS_TEST_BOOL(v == nsVariant(nsVec3(1, 2, 3)));
    NS_TEST_BOOL(v != nsVariant(nsVec3(1, 1, 3)));

    NS_TEST_BOOL(v == nsVec3(1, 2, 3));
    NS_TEST_BOOL(v != nsVec3(1, 4, 3));

    v = nsVec3(5, 8, 9);
    NS_TEST_BOOL(v == nsVec3(5, 8, 9));

    v = nsVariant(nsVec3(7, 9, 8));
    NS_TEST_BOOL(v == nsVec3(7, 9, 8));
    NS_TEST_BOOL(v[0] == 7);
    NS_TEST_BOOL(v[1] == 9);
    NS_TEST_BOOL(v[2] == 8);
    NS_TEST_BOOL(v["x"] == 7);
    NS_TEST_BOOL(v["y"] == 9);
    NS_TEST_BOOL(v["z"] == 8);

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(!v.IsString());
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsVec4")
  {
    nsVariant v(nsVec4(1, 2, 3, 4));
    TestVariant<nsVec4>(v, nsVariantType::Vector4);

    NS_TEST_BOOL(v.Get<nsVec4>() == nsVec4(1, 2, 3, 4));

    NS_TEST_BOOL(v == nsVariant(nsVec4(1, 2, 3, 4)));
    NS_TEST_BOOL(v != nsVariant(nsVec4(1, 1, 3, 4)));

    NS_TEST_BOOL(v == nsVec4(1, 2, 3, 4));
    NS_TEST_BOOL(v != nsVec4(1, 4, 3, 4));

    v = nsVec4(5, 8, 9, 3);
    NS_TEST_BOOL(v == nsVec4(5, 8, 9, 3));

    v = nsVariant(nsVec4(7, 9, 8, 4));
    NS_TEST_BOOL(v == nsVec4(7, 9, 8, 4));
    NS_TEST_BOOL(v[0] == 7);
    NS_TEST_BOOL(v[1] == 9);
    NS_TEST_BOOL(v[2] == 8);
    NS_TEST_BOOL(v[3] == 4);
    NS_TEST_BOOL(v["x"] == 7);
    NS_TEST_BOOL(v["y"] == 9);
    NS_TEST_BOOL(v["z"] == 8);
    NS_TEST_BOOL(v["w"] == 4);

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(!v.IsString());
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsVec2I32")
  {
    nsVariant v(nsVec2I32(1, 2));
    TestVariant<nsVec2I32>(v, nsVariantType::Vector2I);

    NS_TEST_BOOL(v.Get<nsVec2I32>() == nsVec2I32(1, 2));

    NS_TEST_BOOL(v == nsVariant(nsVec2I32(1, 2)));
    NS_TEST_BOOL(v != nsVariant(nsVec2I32(1, 1)));

    NS_TEST_BOOL(v == nsVec2I32(1, 2));
    NS_TEST_BOOL(v != nsVec2I32(1, 4));

    v = nsVec2I32(5, 8);
    NS_TEST_BOOL(v == nsVec2I32(5, 8));

    v = nsVariant(nsVec2I32(7, 9));
    NS_TEST_BOOL(v == nsVec2I32(7, 9));
    NS_TEST_BOOL(v[0] == 7);
    NS_TEST_BOOL(v[1] == 9);
    NS_TEST_BOOL(v["x"] == 7);
    NS_TEST_BOOL(v["y"] == 9);

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(!v.IsString());
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsVec3I32")
  {
    nsVariant v(nsVec3I32(1, 2, 3));
    TestVariant<nsVec3I32>(v, nsVariantType::Vector3I);

    NS_TEST_BOOL(v.Get<nsVec3I32>() == nsVec3I32(1, 2, 3));

    NS_TEST_BOOL(v == nsVariant(nsVec3I32(1, 2, 3)));
    NS_TEST_BOOL(v != nsVariant(nsVec3I32(1, 1, 3)));

    NS_TEST_BOOL(v == nsVec3I32(1, 2, 3));
    NS_TEST_BOOL(v != nsVec3I32(1, 4, 3));

    v = nsVec3I32(5, 8, 9);
    NS_TEST_BOOL(v == nsVec3I32(5, 8, 9));

    v = nsVariant(nsVec3I32(7, 9, 8));
    NS_TEST_BOOL(v == nsVec3I32(7, 9, 8));
    NS_TEST_BOOL(v[0] == 7);
    NS_TEST_BOOL(v[1] == 9);
    NS_TEST_BOOL(v[2] == 8);
    NS_TEST_BOOL(v["x"] == 7);
    NS_TEST_BOOL(v["y"] == 9);
    NS_TEST_BOOL(v["z"] == 8);

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(!v.IsString());
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsVec4I32")
  {
    nsVariant v(nsVec4I32(1, 2, 3, 4));
    TestVariant<nsVec4I32>(v, nsVariantType::Vector4I);

    NS_TEST_BOOL(v.Get<nsVec4I32>() == nsVec4I32(1, 2, 3, 4));

    NS_TEST_BOOL(v == nsVariant(nsVec4I32(1, 2, 3, 4)));
    NS_TEST_BOOL(v != nsVariant(nsVec4I32(1, 1, 3, 4)));

    NS_TEST_BOOL(v == nsVec4I32(1, 2, 3, 4));
    NS_TEST_BOOL(v != nsVec4I32(1, 4, 3, 4));

    v = nsVec4I32(5, 8, 9, 3);
    NS_TEST_BOOL(v == nsVec4I32(5, 8, 9, 3));

    v = nsVariant(nsVec4I32(7, 9, 8, 4));
    NS_TEST_BOOL(v == nsVec4I32(7, 9, 8, 4));
    NS_TEST_BOOL(v[0] == 7);
    NS_TEST_BOOL(v[1] == 9);
    NS_TEST_BOOL(v[2] == 8);
    NS_TEST_BOOL(v[3] == 4);
    NS_TEST_BOOL(v["x"] == 7);
    NS_TEST_BOOL(v["y"] == 9);
    NS_TEST_BOOL(v["z"] == 8);
    NS_TEST_BOOL(v["w"] == 4);

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(!v.IsString());
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsQuat")
  {
    nsVariant v(nsQuat(1, 2, 3, 4));
    TestVariant<nsQuat>(v, nsVariantType::Quaternion);

    NS_TEST_BOOL(v.Get<nsQuat>() == nsQuat(1, 2, 3, 4));

    NS_TEST_BOOL(v == nsQuat(1, 2, 3, 4));
    NS_TEST_BOOL(v != nsQuat(1, 2, 3, 5));

    NS_TEST_BOOL(v == nsQuat(1, 2, 3, 4));
    NS_TEST_BOOL(v != nsQuat(1, 4, 3, 4));

    v = nsQuat(5, 8, 9, 3);
    NS_TEST_BOOL(v == nsQuat(5, 8, 9, 3));

    v = nsVariant(nsQuat(7, 9, 8, 4));
    NS_TEST_BOOL(v == nsQuat(7, 9, 8, 4));
    NS_TEST_BOOL(v[0] == 7);
    NS_TEST_BOOL(v[1] == 9);
    NS_TEST_BOOL(v[2] == 8);
    NS_TEST_BOOL(v[3] == 4);
    NS_TEST_BOOL(v["x"] == 7);
    NS_TEST_BOOL(v["y"] == 9);
    NS_TEST_BOOL(v["z"] == 8);
    NS_TEST_BOOL(v["w"] == 4);

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(!v.IsString());
    NS_TEST_BOOL(v.IsFloatingPoint() == false);

    nsTypedPointer ptr = v.GetWriteAccess();
    NS_TEST_BOOL(ptr.m_pObject == &v.Get<nsQuat>());
    NS_TEST_BOOL(ptr.m_pObject == &v.GetWritable<nsQuat>());
    NS_TEST_BOOL(ptr.m_pType == nsGetStaticRTTI<nsQuat>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsMat3")
  {
    nsVariant v(nsMat3::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9));
    TestVariant<nsMat3>(v, nsVariantType::Matrix3);

    NS_TEST_BOOL(v.Get<nsMat3>() == nsMat3::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9));

    NS_TEST_BOOL(v == nsVariant(nsMat3::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9)));
    NS_TEST_BOOL(v != nsVariant(nsMat3::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 8)));

    NS_TEST_BOOL(v == nsMat3::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9));
    NS_TEST_BOOL(v != nsMat3::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 8));

    v = nsMat3::MakeFromValues(5, 8, 9, 3, 1, 2, 3, 4, 5);
    NS_TEST_BOOL(v == nsMat3::MakeFromValues(5, 8, 9, 3, 1, 2, 3, 4, 5));

    v = nsVariant(nsMat3::MakeFromValues(5, 8, 9, 3, 1, 2, 3, 4, 4));
    NS_TEST_BOOL(v == nsMat3::MakeFromValues(5, 8, 9, 3, 1, 2, 3, 4, 4));

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(!v.IsString());
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsMat4")
  {
    nsVariant v(nsMat4::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16));
    TestVariant<nsMat4>(v, nsVariantType::Matrix4);

    NS_TEST_BOOL(v.Get<nsMat4>() == nsMat4::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16));

    NS_TEST_BOOL(v == nsVariant(nsMat4::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)));
    NS_TEST_BOOL(v != nsVariant(nsMat4::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15)));

    NS_TEST_BOOL(v == nsMat4::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16));
    NS_TEST_BOOL(v != nsMat4::MakeFromValues(1, 2, 3, 4, 5, 6, 2, 8, 9, 10, 11, 12, 13, 14, 15, 16));

    v = nsMat4::MakeFromValues(5, 8, 9, 3, 1, 2, 3, 4, 5, 3, 7, 3, 6, 8, 6, 8);
    NS_TEST_BOOL(v == nsMat4::MakeFromValues(5, 8, 9, 3, 1, 2, 3, 4, 5, 3, 7, 3, 6, 8, 6, 8));

    v = nsVariant(nsMat4::MakeFromValues(5, 8, 9, 3, 1, 2, 1, 4, 5, 3, 7, 3, 6, 8, 6, 8));
    NS_TEST_BOOL(v == nsMat4::MakeFromValues(5, 8, 9, 3, 1, 2, 1, 4, 5, 3, 7, 3, 6, 8, 6, 8));

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(!v.IsString());
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsTransform")
  {
    nsVariant v(nsTransform(nsVec3(1, 2, 3), nsQuat(4, 5, 6, 7), nsVec3(8, 9, 10)));
    TestVariant<nsTransform>(v, nsVariantType::Transform);

    NS_TEST_BOOL(v.Get<nsTransform>() == nsTransform(nsVec3(1, 2, 3), nsQuat(4, 5, 6, 7), nsVec3(8, 9, 10)));

    NS_TEST_BOOL(v == nsTransform(nsVec3(1, 2, 3), nsQuat(4, 5, 6, 7), nsVec3(8, 9, 10)));
    NS_TEST_BOOL(v != nsTransform(nsVec3(1, 2, 3), nsQuat(4, 5, 6, 7), nsVec3(8, 9, 11)));

    v = nsTransform(nsVec3(5, 8, 9), nsQuat(3, 1, 2, 3), nsVec3(4, 5, 3));
    NS_TEST_BOOL(v == nsTransform(nsVec3(5, 8, 9), nsQuat(3, 1, 2, 3), nsVec3(4, 5, 3)));

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(!v.IsString());
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "const char*")
  {
    nsVariant v("This is a const char array");
    TestVariant<nsString>(v, nsVariantType::String);

    NS_TEST_BOOL(v.IsA<const char*>());
    NS_TEST_BOOL(v.IsA<char*>());
    NS_TEST_BOOL(v.Get<nsString>() == nsString("This is a const char array"));

    NS_TEST_BOOL(v == nsVariant("This is a const char array"));
    NS_TEST_BOOL(v != nsVariant("This is something else"));

    NS_TEST_BOOL(v == nsString("This is a const char array"));
    NS_TEST_BOOL(v != nsString("This is another string"));

    NS_TEST_BOOL(v == "This is a const char array");
    NS_TEST_BOOL(v != "This is another string");

    NS_TEST_BOOL(v == (const char*)"This is a const char array");
    NS_TEST_BOOL(v != (const char*)"This is another string");

    v = "blurg!";
    NS_TEST_BOOL(v == nsString("blurg!"));

    v = nsVariant("blärg!");
    NS_TEST_BOOL(v == nsString("blärg!"));

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(v.IsString());
    NS_TEST_BOOL(v.CanConvertTo<nsStringView>());
    nsStringView view = v.ConvertTo<nsStringView>();
    NS_TEST_BOOL(view == v.Get<nsString>());
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsString")
  {
    nsVariant v(nsString("This is an nsString"));
    TestVariant<nsString>(v, nsVariantType::String);

    NS_TEST_BOOL(v.Get<nsString>() == nsString("This is an nsString"));

    NS_TEST_BOOL(v == nsVariant(nsString("This is an nsString")));
    NS_TEST_BOOL(v == nsVariant(nsStringView("This is an nsString"), false));
    NS_TEST_BOOL(v != nsVariant(nsString("This is something else")));

    NS_TEST_BOOL(v == nsString("This is an nsString"));
    NS_TEST_BOOL(v != nsString("This is another nsString"));

    v = nsString("blurg!");
    NS_TEST_BOOL(v == nsString("blurg!"));

    v = nsVariant(nsString("blärg!"));
    NS_TEST_BOOL(v == nsString("blärg!"));

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(v.IsString());
    NS_TEST_BOOL(v.CanConvertTo<nsStringView>());
    nsStringView view = v.ConvertTo<nsStringView>();
    NS_TEST_BOOL(view == v.Get<nsString>());
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsStringView")
  {
    const char* szTemp = "This is an nsStringView";
    nsStringView bla(szTemp);
    nsVariant v(bla, false);
    TestVariant<nsStringView>(v, nsVariantType::StringView);

    const nsString sCopy = szTemp;
    NS_TEST_BOOL(v.Get<nsStringView>() == sCopy);

    NS_TEST_BOOL(v == nsVariant(nsStringView(sCopy.GetData()), false));
    NS_TEST_BOOL(v == nsVariant(nsString("This is an nsStringView")));
    NS_TEST_BOOL(v != nsVariant(nsStringView("This is something else"), false));

    NS_TEST_BOOL(v == nsStringView(sCopy.GetData()));
    NS_TEST_BOOL(v != nsStringView("This is something else"));

    v = nsVariant(nsStringView("blurg!"), false);
    NS_TEST_BOOL(v == nsStringView("blurg!"));

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(v.IsString());
    NS_TEST_BOOL(v.CanConvertTo<nsString>());
    nsString sString = v.ConvertTo<nsString>();
    NS_TEST_BOOL(sString == v.Get<nsStringView>());
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsDataBuffer")
  {
    nsDataBuffer a, a2;
    a.PushBack(nsUInt8(1));
    a.PushBack(nsUInt8(2));
    a.PushBack(nsUInt8(255));

    nsVariant va(a);
    TestVariant<nsDataBuffer>(va, nsVariantType::DataBuffer);

    const nsDataBuffer& b = va.Get<nsDataBuffer>();
    nsArrayPtr<const nsUInt8> b2 = va.Get<nsDataBuffer>();

    NS_TEST_BOOL(a == b);
    NS_TEST_BOOL(a == b2);

    NS_TEST_BOOL(a != a2);

    NS_TEST_BOOL(va == a);
    NS_TEST_BOOL(va != a2);

    NS_TEST_BOOL(va.IsNumber() == false);
    NS_TEST_BOOL(!va.IsString());
    NS_TEST_BOOL(va.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsTime")
  {
    nsVariant v(nsTime::MakeFromSeconds(1337));
    TestVariant<nsTime>(v, nsVariantType::Time);

    NS_TEST_BOOL(v.Get<nsTime>() == nsTime::MakeFromSeconds(1337));

    NS_TEST_BOOL(v == nsVariant(nsTime::MakeFromSeconds(1337)));
    NS_TEST_BOOL(v != nsVariant(nsTime::MakeFromSeconds(1336)));

    NS_TEST_BOOL(v == nsTime::MakeFromSeconds(1337));
    NS_TEST_BOOL(v != nsTime::MakeFromSeconds(1338));

    v = nsTime::MakeFromSeconds(8472);
    NS_TEST_BOOL(v == nsTime::MakeFromSeconds(8472));

    v = nsVariant(nsTime::MakeFromSeconds(13));
    NS_TEST_BOOL(v == nsTime::MakeFromSeconds(13));

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(!v.IsString());
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsUuid")
  {
    nsUuid id;
    nsVariant v(id);
    TestVariant<nsUuid>(v, nsVariantType::Uuid);

    NS_TEST_BOOL(v.Get<nsUuid>() == nsUuid());

    const nsUuid uuid = nsUuid::MakeUuid();
    NS_TEST_BOOL(v != nsVariant(uuid));
    NS_TEST_BOOL(nsVariant(uuid).Get<nsUuid>() == uuid);

    const nsUuid uuid2 = nsUuid::MakeUuid();
    NS_TEST_BOOL(nsVariant(uuid) != nsVariant(uuid2));

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(!v.IsString());
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsAngle")
  {
    nsVariant v(nsAngle::MakeFromDegree(1337));
    TestVariant<nsAngle>(v, nsVariantType::Angle);

    NS_TEST_BOOL(v.Get<nsAngle>() == nsAngle::MakeFromDegree(1337));

    NS_TEST_BOOL(v == nsVariant(nsAngle::MakeFromDegree(1337)));
    NS_TEST_BOOL(v != nsVariant(nsAngle::MakeFromDegree(1336)));

    NS_TEST_BOOL(v == nsAngle::MakeFromDegree(1337));
    NS_TEST_BOOL(v != nsAngle::MakeFromDegree(1338));

    v = nsAngle::MakeFromDegree(8472);
    NS_TEST_BOOL(v == nsAngle::MakeFromDegree(8472));

    v = nsVariant(nsAngle::MakeFromDegree(13));
    NS_TEST_BOOL(v == nsAngle::MakeFromDegree(13));

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(!v.IsString());
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsHashedString")
  {
    nsVariant v(nsMakeHashedString("ABCDE"));
    TestVariant<nsHashedString>(v, nsVariantType::HashedString);

    NS_TEST_BOOL(v.Get<nsHashedString>() == nsMakeHashedString("ABCDE"));

    NS_TEST_BOOL(v == nsVariant(nsMakeHashedString("ABCDE")));
    NS_TEST_BOOL(v != nsVariant(nsMakeHashedString("ABCDK")));
    NS_TEST_BOOL(v == nsVariant(nsTempHashedString("ABCDE")));
    NS_TEST_BOOL(v != nsVariant(nsTempHashedString("ABCDK")));

    NS_TEST_BOOL(v == nsMakeHashedString("ABCDE"));
    NS_TEST_BOOL(v != nsMakeHashedString("ABCDK"));
    NS_TEST_BOOL(v == nsTempHashedString("ABCDE"));
    NS_TEST_BOOL(v != nsTempHashedString("ABCDK"));

    v = nsMakeHashedString("HHH");
    NS_TEST_BOOL(v == nsMakeHashedString("HHH"));

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(v.IsString() == false);
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsTempHashedString")
  {
    nsVariant v(nsTempHashedString("ABCDE"));
    TestVariant<nsTempHashedString>(v, nsVariantType::TempHashedString);

    NS_TEST_BOOL(v.Get<nsTempHashedString>() == nsTempHashedString("ABCDE"));

    NS_TEST_BOOL(v == nsVariant(nsTempHashedString("ABCDE")));
    NS_TEST_BOOL(v != nsVariant(nsTempHashedString("ABCDK")));
    NS_TEST_BOOL(v == nsVariant(nsMakeHashedString("ABCDE")));
    NS_TEST_BOOL(v != nsVariant(nsMakeHashedString("ABCDK")));

    NS_TEST_BOOL(v == nsTempHashedString("ABCDE"));
    NS_TEST_BOOL(v != nsTempHashedString("ABCDK"));
    NS_TEST_BOOL(v == nsMakeHashedString("ABCDE"));
    NS_TEST_BOOL(v != nsMakeHashedString("ABCDK"));

    v = nsTempHashedString("HHH");
    NS_TEST_BOOL(v == nsTempHashedString("HHH"));

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(v.IsString() == false);
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsVariantArray")
  {
    nsVariantArray a, a2;
    a.PushBack("This");
    a.PushBack("is a");
    a.PushBack("test");

    nsVariant va(a);
    NS_TEST_BOOL(va.IsValid());
    NS_TEST_BOOL(va.GetType() == nsVariant::Type::VariantArray);
    NS_TEST_BOOL(va.IsA<nsVariantArray>());
    NS_TEST_BOOL(va.GetReflectedType() == nullptr);

    const nsArrayPtr<const nsVariant>& b = va.Get<nsVariantArray>();
    nsArrayPtr<const nsVariant> b2 = va.Get<nsVariantArray>();

    NS_TEST_BOOL(a == b);
    NS_TEST_BOOL(a == b2);

    NS_TEST_BOOL(a != a2);

    NS_TEST_BOOL(va == a);
    NS_TEST_BOOL(va != a2);

    NS_TEST_BOOL(va[0] == nsString("This"));
    NS_TEST_BOOL(va[1] == nsString("is a"));
    NS_TEST_BOOL(va[2] == nsString("test"));
    NS_TEST_BOOL(va[4] == nsVariant());
    NS_TEST_BOOL(!va[4].IsValid());

    NS_TEST_BOOL(va.IsNumber() == false);
    NS_TEST_BOOL(!va.IsString());
    NS_TEST_BOOL(va.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsVariantDictionary")
  {
    nsVariantDictionary a, a2;
    a["my"] = true;
    a["luv"] = 4;
    a["pon"] = "ies";

    nsVariant va(a);
    NS_TEST_BOOL(va.IsValid());
    NS_TEST_BOOL(va.GetType() == nsVariant::Type::VariantDictionary);
    NS_TEST_BOOL(va.IsA<nsVariantDictionary>());
    NS_TEST_BOOL(va.GetReflectedType() == nullptr);

    const nsVariantDictionary& d1 = va.Get<nsVariantDictionary>();
    nsVariantDictionary d2 = va.Get<nsVariantDictionary>();

    NS_TEST_BOOL(a == d1);
    NS_TEST_BOOL(a == d2);
    NS_TEST_BOOL(d1 == d2);

    NS_TEST_BOOL(va == a);
    NS_TEST_BOOL(va != a2);

    NS_TEST_BOOL(va["my"] == true);
    NS_TEST_BOOL(va["luv"] == 4);
    NS_TEST_BOOL(va["pon"] == nsString("ies"));
    NS_TEST_BOOL(va["x"] == nsVariant());
    NS_TEST_BOOL(!va["x"].IsValid());

    NS_TEST_BOOL(va.IsNumber() == false);
    NS_TEST_BOOL(!va.IsString());
    NS_TEST_BOOL(va.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsTypedPointer")
  {
    Blubb blubb;
    blubb.u = 1.0f;
    blubb.v = 2.0f;

    Blubb blubb2;

    nsVariant v(&blubb);

    NS_TEST_BOOL(v.IsValid());
    NS_TEST_BOOL(v.GetType() == nsVariant::Type::TypedPointer);
    NS_TEST_BOOL(v.IsA<Blubb*>());
    NS_TEST_BOOL(v.Get<Blubb*>() == &blubb);
    NS_TEST_BOOL(v.IsA<nsReflectedClass*>());
    NS_TEST_BOOL(v.Get<nsReflectedClass*>() == &blubb);
    NS_TEST_BOOL(v.Get<nsReflectedClass*>() != &blubb2);
    NS_TEST_BOOL(nsDynamicCast<Blubb*>(v) == &blubb);
    NS_TEST_BOOL(nsDynamicCast<nsVec3*>(v) == nullptr);
    NS_TEST_BOOL(v.IsA<void*>());
    NS_TEST_BOOL(v.Get<void*>() == &blubb);
    NS_TEST_BOOL(v.IsA<const void*>());
    NS_TEST_BOOL(v.Get<const void*>() == &blubb);
    NS_TEST_BOOL(v.GetData() == &blubb);
    NS_TEST_BOOL(v.IsA<nsTypedPointer>());
    NS_TEST_BOOL(v.GetReflectedType() == nsGetStaticRTTI<Blubb>());
    NS_TEST_BOOL(!v.IsA<nsVec3*>());

    nsTypedPointer ptr = v.Get<nsTypedPointer>();
    NS_TEST_BOOL(ptr.m_pObject == &blubb);
    NS_TEST_BOOL(ptr.m_pType == nsGetStaticRTTI<Blubb>());

    nsTypedPointer ptr2 = v.GetWriteAccess();
    NS_TEST_BOOL(ptr2.m_pObject == &blubb);
    NS_TEST_BOOL(ptr2.m_pType == nsGetStaticRTTI<Blubb>());

    NS_TEST_BOOL(v[0] == 1.0f);
    NS_TEST_BOOL(v[1] == 2.0f);
    NS_TEST_BOOL(v["u"] == 1.0f);
    NS_TEST_BOOL(v["v"] == 2.0f);
    nsVariant v2 = &blubb;
    NS_TEST_BOOL(v == v2);
    nsVariant v3 = ptr;
    NS_TEST_BOOL(v == v3);

    NS_TEST_BOOL(v.IsNumber() == false);
    NS_TEST_BOOL(!v.IsString());
    NS_TEST_BOOL(v.IsFloatingPoint() == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsTypedPointer nullptr")
  {
    nsTypedPointer ptr = {nullptr, nsGetStaticRTTI<Blubb>()};
    nsVariant v = ptr;
    NS_TEST_BOOL(v.IsValid());
    NS_TEST_BOOL(v.GetType() == nsVariant::Type::TypedPointer);
    NS_TEST_BOOL(v.IsA<Blubb*>());
    NS_TEST_BOOL(v.Get<Blubb*>() == nullptr);
    NS_TEST_BOOL(v.IsA<nsReflectedClass*>());
    NS_TEST_BOOL(v.Get<nsReflectedClass*>() == nullptr);
    NS_TEST_BOOL(nsDynamicCast<Blubb*>(v) == nullptr);
    NS_TEST_BOOL(nsDynamicCast<nsVec3*>(v) == nullptr);
    NS_TEST_BOOL(v.IsA<void*>());
    NS_TEST_BOOL(v.Get<void*>() == nullptr);
    NS_TEST_BOOL(v.IsA<const void*>());
    NS_TEST_BOOL(v.Get<const void*>() == nullptr);
    NS_TEST_BOOL(v.IsA<nsTypedPointer>());
    NS_TEST_BOOL(v.GetReflectedType() == nsGetStaticRTTI<Blubb>());
    NS_TEST_BOOL(!v.IsA<nsVec3*>());

    nsTypedPointer ptr2 = v.Get<nsTypedPointer>();
    NS_TEST_BOOL(ptr2.m_pObject == nullptr);
    NS_TEST_BOOL(ptr2.m_pType == nsGetStaticRTTI<Blubb>());

    NS_TEST_BOOL(!v[0].IsValid());
    NS_TEST_BOOL(!v["u"].IsValid());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsTypedObject inline")
  {
    // nsAngle::MakeFromDegree(90.0f) was replaced with radian as release builds generate a different float then debug.
    nsVarianceTypeAngle value = {0.1f, nsAngle::MakeFromRadian(1.57079637f)};
    nsVarianceTypeAngle value2 = {0.2f, nsAngle::MakeFromRadian(1.57079637f)};

    nsVariant v(value);
    TestVariant<nsVarianceTypeAngle>(v, nsVariantType::TypedObject);

    NS_TEST_BOOL(v.IsA<nsTypedObject>());
    NS_TEST_BOOL(!v.IsA<void*>());
    NS_TEST_BOOL(!v.IsA<const void*>());
    NS_TEST_BOOL(!v.IsA<nsVec3*>());
    NS_TEST_BOOL(nsDynamicCast<nsVec3*>(v) == nullptr);

    const nsVarianceTypeAngle& valueGet = v.Get<nsVarianceTypeAngle>();
    NS_TEST_BOOL(value == valueGet);

    nsVariant va = value;
    NS_TEST_BOOL(v == va);

    nsVariant v2 = value2;
    NS_TEST_BOOL(v != v2);

    nsUInt64 uiHash = v.ComputeHash(0);
    NS_TEST_INT(uiHash, 8527525522777555267ul);

    nsVarianceTypeAngle* pTypedAngle = NS_DEFAULT_NEW(nsVarianceTypeAngle, {0.1f, nsAngle::MakeFromRadian(1.57079637f)});
    nsVariant copy;
    copy.CopyTypedObject(pTypedAngle, nsGetStaticRTTI<nsVarianceTypeAngle>());
    nsVariant move;
    move.MoveTypedObject(pTypedAngle, nsGetStaticRTTI<nsVarianceTypeAngle>());
    NS_TEST_BOOL(v == copy);
    NS_TEST_BOOL(v == move);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsTypedObject shared")
  {
    nsTypedObjectStruct data;
    nsVariant v = data;
    NS_TEST_BOOL(v.IsValid());
    NS_TEST_BOOL(v.GetType() == nsVariant::Type::TypedObject);
    NS_TEST_BOOL(v.IsA<nsTypedObject>());
    NS_TEST_BOOL(v.IsA<nsTypedObjectStruct>());
    NS_TEST_BOOL(!v.IsA<void*>());
    NS_TEST_BOOL(!v.IsA<const void*>());
    NS_TEST_BOOL(!v.IsA<nsVec3*>());
    NS_TEST_BOOL(nsDynamicCast<nsVec3*>(v) == nullptr);
    NS_TEST_BOOL(v.GetReflectedType() == nsGetStaticRTTI<nsTypedObjectStruct>());

    nsVariant v2 = v;

    nsTypedPointer ptr = v.GetWriteAccess();
    NS_TEST_BOOL(ptr.m_pObject == &v.Get<nsTypedObjectStruct>());
    NS_TEST_BOOL(ptr.m_pObject == &v.GetWritable<nsTypedObjectStruct>());
    NS_TEST_BOOL(ptr.m_pObject != &v2.Get<nsTypedObjectStruct>());
    NS_TEST_BOOL(ptr.m_pType == nsGetStaticRTTI<nsTypedObjectStruct>());

    NS_TEST_BOOL(nsReflectionUtils::IsEqual(ptr.m_pObject, &v2.Get<nsTypedObjectStruct>(), nsGetStaticRTTI<nsTypedObjectStruct>()));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (bool)")
  {
    nsVariant v(true);

    NS_TEST_BOOL(v.CanConvertTo<bool>());
    NS_TEST_BOOL(v.CanConvertTo<nsInt32>());

    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Invalid) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Bool));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Int8));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::UInt8));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Int16));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::UInt16));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Int32));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::UInt32));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Int64));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::UInt64));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Float));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Double));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Color) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector2) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector3) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector4) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector2I) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector3I) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector4I) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Quaternion) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Matrix3) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Matrix4) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::String));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::StringView) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::DataBuffer) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Time) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Angle) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::VariantArray) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::VariantDictionary) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::TypedPointer) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::TypedObject) == false);

    NS_TEST_BOOL(v.ConvertTo<bool>() == true);
    NS_TEST_BOOL(v.ConvertTo<nsInt8>() == 1);
    NS_TEST_BOOL(v.ConvertTo<nsUInt8>() == 1);
    NS_TEST_BOOL(v.ConvertTo<nsInt16>() == 1);
    NS_TEST_BOOL(v.ConvertTo<nsUInt16>() == 1);
    NS_TEST_BOOL(v.ConvertTo<nsInt32>() == 1);
    NS_TEST_BOOL(v.ConvertTo<nsUInt32>() == 1);
    NS_TEST_BOOL(v.ConvertTo<nsInt64>() == 1);
    NS_TEST_BOOL(v.ConvertTo<nsUInt64>() == 1);
    NS_TEST_BOOL(v.ConvertTo<float>() == 1.0f);
    NS_TEST_BOOL(v.ConvertTo<double>() == 1.0);
    NS_TEST_BOOL(v.ConvertTo<nsString>() == "true");
    NS_TEST_BOOL(v.ConvertTo<nsHashedString>() == nsMakeHashedString("true"));
    NS_TEST_BOOL(v.ConvertTo<nsTempHashedString>() == nsTempHashedString("true"));

    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Bool).Get<bool>() == true);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Int8).Get<nsInt8>() == 1);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::UInt8).Get<nsUInt8>() == 1);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Int16).Get<nsInt16>() == 1);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::UInt16).Get<nsUInt16>() == 1);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Int32).Get<nsInt32>() == 1);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::UInt32).Get<nsUInt32>() == 1);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Int64).Get<nsInt64>() == 1);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::UInt64).Get<nsUInt64>() == 1);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Float).Get<float>() == 1.0f);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Double).Get<double>() == 1.0);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::String).Get<nsString>() == "true");
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::HashedString).Get<nsHashedString>() == nsMakeHashedString("true"));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::TempHashedString).Get<nsTempHashedString>() == nsTempHashedString("true"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsInt8)")
  {
    nsVariant v((nsInt8)3);
    TestNumberCanConvertTo(v);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsUInt8)")
  {
    nsVariant v((nsUInt8)3);
    TestNumberCanConvertTo(v);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsInt16)")
  {
    nsVariant v((nsInt16)3);
    TestNumberCanConvertTo(v);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsUInt16)")
  {
    nsVariant v((nsUInt16)3);
    TestNumberCanConvertTo(v);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsInt32)")
  {
    nsVariant v((nsInt32)3);
    TestNumberCanConvertTo(v);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsUInt32)")
  {
    nsVariant v((nsUInt32)3);
    TestNumberCanConvertTo(v);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsInt64)")
  {
    nsVariant v((nsInt64)3);
    TestNumberCanConvertTo(v);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsUInt64)")
  {
    nsVariant v((nsUInt64)3);
    TestNumberCanConvertTo(v);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (float)")
  {
    nsVariant v((float)3.0f);
    TestNumberCanConvertTo(v);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (double)")
  {
    nsVariant v((double)3.0f);
    TestNumberCanConvertTo(v);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (Color)")
  {
    nsColor c(3, 3, 4, 0);
    nsVariant v(c);

    TestCanOnlyConvertToStringAndID(v, nsVariant::Type::Color, nsVariant::Type::ColorGamma);

    nsResult conversionResult = NS_FAILURE;
    NS_TEST_BOOL(v.ConvertTo<nsColor>(&conversionResult) == c);
    NS_TEST_BOOL(conversionResult.Succeeded());

    NS_TEST_BOOL(v.ConvertTo<nsString>(&conversionResult) == "{ r=3, g=3, b=4, a=0 }");
    NS_TEST_BOOL(conversionResult.Succeeded());

    NS_TEST_BOOL(v.ConvertTo<nsHashedString>() == nsMakeHashedString("{ r=3, g=3, b=4, a=0 }"));
    NS_TEST_BOOL(v.ConvertTo<nsTempHashedString>() == nsTempHashedString("{ r=3, g=3, b=4, a=0 }"));

    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Color).Get<nsColor>() == c);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::String).Get<nsString>() == "{ r=3, g=3, b=4, a=0 }");
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::HashedString).Get<nsHashedString>() == nsMakeHashedString("{ r=3, g=3, b=4, a=0 }"));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::TempHashedString).Get<nsTempHashedString>() == nsTempHashedString("{ r=3, g=3, b=4, a=0 }"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (ColorGamma)")
  {
    nsColorGammaUB c(0, 128, 64, 255);
    nsVariant v(c);

    TestCanOnlyConvertToStringAndID(v, nsVariant::Type::ColorGamma, nsVariant::Type::Color);

    nsResult conversionResult = NS_FAILURE;
    NS_TEST_BOOL(v.ConvertTo<nsColorGammaUB>(&conversionResult) == c);
    NS_TEST_BOOL(conversionResult.Succeeded());

    nsString val = v.ConvertTo<nsString>(&conversionResult);
    NS_TEST_BOOL(val == "{ r=0, g=128, b=64, a=255 }");
    NS_TEST_BOOL(conversionResult.Succeeded());

    NS_TEST_BOOL(v.ConvertTo<nsHashedString>() == nsMakeHashedString("{ r=0, g=128, b=64, a=255 }"));
    NS_TEST_BOOL(v.ConvertTo<nsTempHashedString>() == nsTempHashedString("{ r=0, g=128, b=64, a=255 }"));

    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::ColorGamma).Get<nsColorGammaUB>() == c);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::String).Get<nsString>() == "{ r=0, g=128, b=64, a=255 }");
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::HashedString).Get<nsHashedString>() == nsMakeHashedString("{ r=0, g=128, b=64, a=255 }"));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::TempHashedString).Get<nsTempHashedString>() == nsTempHashedString("{ r=0, g=128, b=64, a=255 }"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsVec2)")
  {
    nsVec2 vec(3.0f, 4.0f);
    nsVariant v(vec);

    TestCanOnlyConvertToStringAndID(v, nsVariant::Type::Vector2, nsVariant::Type::Vector2I, nsVariant::Type::Vector2U);

    NS_TEST_BOOL(v.ConvertTo<nsVec2>() == vec);
    NS_TEST_BOOL(v.ConvertTo<nsVec2I32>() == nsVec2I32(3, 4));
    NS_TEST_BOOL(v.ConvertTo<nsVec2U32>() == nsVec2U32(3, 4));
    NS_TEST_BOOL(v.ConvertTo<nsString>() == "{ x=3, y=4 }");
    NS_TEST_BOOL(v.ConvertTo<nsHashedString>() == nsMakeHashedString("{ x=3, y=4 }"));
    NS_TEST_BOOL(v.ConvertTo<nsTempHashedString>() == nsTempHashedString("{ x=3, y=4 }"));

    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Vector2).Get<nsVec2>() == vec);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Vector2I).Get<nsVec2I32>() == nsVec2I32(3, 4));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Vector2U).Get<nsVec2U32>() == nsVec2U32(3, 4));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::String).Get<nsString>() == "{ x=3, y=4 }");
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::HashedString).Get<nsHashedString>() == nsMakeHashedString("{ x=3, y=4 }"));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::TempHashedString).Get<nsTempHashedString>() == nsTempHashedString("{ x=3, y=4 }"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsVec3)")
  {
    nsVec3 vec(3.0f, 4.0f, 6.0f);
    nsVariant v(vec);

    TestCanOnlyConvertToStringAndID(v, nsVariant::Type::Vector3, nsVariant::Type::Vector3I, nsVariant::Type::Vector3U);

    NS_TEST_BOOL(v.ConvertTo<nsVec3>() == vec);
    NS_TEST_BOOL(v.ConvertTo<nsVec3I32>() == nsVec3I32(3, 4, 6));
    NS_TEST_BOOL(v.ConvertTo<nsVec3U32>() == nsVec3U32(3, 4, 6));
    NS_TEST_BOOL(v.ConvertTo<nsString>() == "{ x=3, y=4, z=6 }");
    NS_TEST_BOOL(v.ConvertTo<nsHashedString>() == nsMakeHashedString("{ x=3, y=4, z=6 }"));
    NS_TEST_BOOL(v.ConvertTo<nsTempHashedString>() == nsTempHashedString("{ x=3, y=4, z=6 }"));

    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Vector3).Get<nsVec3>() == vec);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Vector3I).Get<nsVec3I32>() == nsVec3I32(3, 4, 6));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Vector3U).Get<nsVec3U32>() == nsVec3U32(3, 4, 6));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::String).Get<nsString>() == "{ x=3, y=4, z=6 }");
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::HashedString).Get<nsHashedString>() == nsMakeHashedString("{ x=3, y=4, z=6 }"));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::TempHashedString).Get<nsTempHashedString>() == nsTempHashedString("{ x=3, y=4, z=6 }"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsVec4)")
  {
    nsVec4 vec(3.0f, 4.0f, 3, 56);
    nsVariant v(vec);

    TestCanOnlyConvertToStringAndID(v, nsVariant::Type::Vector4, nsVariant::Type::Vector4I, nsVariant::Type::Vector4U);

    NS_TEST_BOOL(v.ConvertTo<nsVec4>() == vec);
    NS_TEST_BOOL(v.ConvertTo<nsVec4I32>() == nsVec4I32(3, 4, 3, 56));
    NS_TEST_BOOL(v.ConvertTo<nsVec4U32>() == nsVec4U32(3, 4, 3, 56));
    NS_TEST_BOOL(v.ConvertTo<nsString>() == "{ x=3, y=4, z=3, w=56 }");
    NS_TEST_BOOL(v.ConvertTo<nsHashedString>() == nsMakeHashedString("{ x=3, y=4, z=3, w=56 }"));
    NS_TEST_BOOL(v.ConvertTo<nsTempHashedString>() == nsTempHashedString("{ x=3, y=4, z=3, w=56 }"));

    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Vector4).Get<nsVec4>() == vec);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Vector4I).Get<nsVec4I32>() == nsVec4I32(3, 4, 3, 56));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Vector4U).Get<nsVec4U32>() == nsVec4U32(3, 4, 3, 56));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::String).Get<nsString>() == "{ x=3, y=4, z=3, w=56 }");
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::HashedString).Get<nsHashedString>() == nsMakeHashedString("{ x=3, y=4, z=3, w=56 }"));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::TempHashedString).Get<nsTempHashedString>() == nsTempHashedString("{ x=3, y=4, z=3, w=56 }"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsVec2I32)")
  {
    nsVec2I32 vec(3, 4);
    nsVariant v(vec);

    TestCanOnlyConvertToStringAndID(v, nsVariant::Type::Vector2I, nsVariant::Type::Vector2U, nsVariant::Type::Vector2);

    NS_TEST_BOOL(v.ConvertTo<nsVec2I32>() == vec);
    NS_TEST_BOOL(v.ConvertTo<nsVec2>() == nsVec2(3, 4));
    NS_TEST_BOOL(v.ConvertTo<nsVec2U32>() == nsVec2U32(3, 4));
    NS_TEST_BOOL(v.ConvertTo<nsString>() == "{ x=3, y=4 }");
    NS_TEST_BOOL(v.ConvertTo<nsHashedString>() == nsMakeHashedString("{ x=3, y=4 }"));
    NS_TEST_BOOL(v.ConvertTo<nsTempHashedString>() == nsTempHashedString("{ x=3, y=4 }"));

    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Vector2I).Get<nsVec2I32>() == vec);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Vector2).Get<nsVec2>() == nsVec2(3, 4));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Vector2U).Get<nsVec2U32>() == nsVec2U32(3, 4));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::String).Get<nsString>() == "{ x=3, y=4 }");
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::HashedString).Get<nsHashedString>() == nsMakeHashedString("{ x=3, y=4 }"));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::TempHashedString).Get<nsTempHashedString>() == nsTempHashedString("{ x=3, y=4 }"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsVec3I32)")
  {
    nsVec3I32 vec(3, 4, 6);
    nsVariant v(vec);

    TestCanOnlyConvertToStringAndID(v, nsVariant::Type::Vector3I, nsVariant::Type::Vector3U, nsVariant::Type::Vector3);

    NS_TEST_BOOL(v.ConvertTo<nsVec3I32>() == vec);
    NS_TEST_BOOL(v.ConvertTo<nsVec3>() == nsVec3(3, 4, 6));
    NS_TEST_BOOL(v.ConvertTo<nsVec3U32>() == nsVec3U32(3, 4, 6));
    NS_TEST_BOOL(v.ConvertTo<nsString>() == "{ x=3, y=4, z=6 }");
    NS_TEST_BOOL(v.ConvertTo<nsHashedString>() == nsMakeHashedString("{ x=3, y=4, z=6 }"));
    NS_TEST_BOOL(v.ConvertTo<nsTempHashedString>() == nsTempHashedString("{ x=3, y=4, z=6 }"));

    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Vector3I).Get<nsVec3I32>() == vec);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Vector3).Get<nsVec3>() == nsVec3(3, 4, 6));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Vector3U).Get<nsVec3U32>() == nsVec3U32(3, 4, 6));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::String).Get<nsString>() == "{ x=3, y=4, z=6 }");
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::HashedString).Get<nsHashedString>() == nsMakeHashedString("{ x=3, y=4, z=6 }"));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::TempHashedString).Get<nsTempHashedString>() == nsTempHashedString("{ x=3, y=4, z=6 }"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsVec4I32)")
  {
    nsVec4I32 vec(3, 4, 3, 56);
    nsVariant v(vec);

    TestCanOnlyConvertToStringAndID(v, nsVariant::Type::Vector4I, nsVariant::Type::Vector4U, nsVariant::Type::Vector4);

    NS_TEST_BOOL(v.ConvertTo<nsVec4I32>() == vec);
    NS_TEST_BOOL(v.ConvertTo<nsVec4>() == nsVec4(3, 4, 3, 56));
    NS_TEST_BOOL(v.ConvertTo<nsVec4U32>() == nsVec4U32(3, 4, 3, 56));
    NS_TEST_BOOL(v.ConvertTo<nsString>() == "{ x=3, y=4, z=3, w=56 }");
    NS_TEST_BOOL(v.ConvertTo<nsHashedString>() == nsMakeHashedString("{ x=3, y=4, z=3, w=56 }"));
    NS_TEST_BOOL(v.ConvertTo<nsTempHashedString>() == nsTempHashedString("{ x=3, y=4, z=3, w=56 }"));

    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Vector4I).Get<nsVec4I32>() == vec);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Vector4).Get<nsVec4>() == nsVec4(3, 4, 3, 56));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Vector4U).Get<nsVec4U32>() == nsVec4U32(3, 4, 3, 56));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::String).Get<nsString>() == "{ x=3, y=4, z=3, w=56 }");
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::HashedString).Get<nsHashedString>() == nsMakeHashedString("{ x=3, y=4, z=3, w=56 }"));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::TempHashedString).Get<nsTempHashedString>() == nsTempHashedString("{ x=3, y=4, z=3, w=56 }"));
  }
  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsQuat)")
  {
    nsQuat q(3.0f, 4.0f, 3, 56);
    nsVariant v(q);

    TestCanOnlyConvertToStringAndID(v, nsVariant::Type::Quaternion);

    NS_TEST_BOOL(v.ConvertTo<nsQuat>() == q);
    NS_TEST_BOOL(v.ConvertTo<nsString>() == "{ x=3, y=4, z=3, w=56 }");
    NS_TEST_BOOL(v.ConvertTo<nsHashedString>() == nsMakeHashedString("{ x=3, y=4, z=3, w=56 }"));
    NS_TEST_BOOL(v.ConvertTo<nsTempHashedString>() == nsTempHashedString("{ x=3, y=4, z=3, w=56 }"));

    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Quaternion).Get<nsQuat>() == q);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::String).Get<nsString>() == "{ x=3, y=4, z=3, w=56 }");
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::HashedString).Get<nsHashedString>() == nsMakeHashedString("{ x=3, y=4, z=3, w=56 }"));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::TempHashedString).Get<nsTempHashedString>() == nsTempHashedString("{ x=3, y=4, z=3, w=56 }"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsMat3)")
  {
    nsMat3 m = nsMat3::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9);
    nsVariant v(m);

    TestCanOnlyConvertToStringAndID(v, nsVariant::Type::Matrix3);

    NS_TEST_BOOL(v.ConvertTo<nsMat3>() == m);
    NS_TEST_BOOL(v.ConvertTo<nsString>() == "{ c1r1=1, c2r1=2, c3r1=3, c1r2=4, c2r2=5, c3r2=6, c1r3=7, c2r3=8, c3r3=9 }");
    NS_TEST_BOOL(v.ConvertTo<nsHashedString>() == nsMakeHashedString("{ c1r1=1, c2r1=2, c3r1=3, c1r2=4, c2r2=5, c3r2=6, c1r3=7, c2r3=8, c3r3=9 }"));
    NS_TEST_BOOL(v.ConvertTo<nsTempHashedString>() == nsTempHashedString("{ c1r1=1, c2r1=2, c3r1=3, c1r2=4, c2r2=5, c3r2=6, c1r3=7, c2r3=8, c3r3=9 }"));

    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Matrix3).Get<nsMat3>() == m);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::String).Get<nsString>() == "{ c1r1=1, c2r1=2, c3r1=3, c1r2=4, c2r2=5, c3r2=6, c1r3=7, c2r3=8, c3r3=9 }");
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::HashedString).Get<nsHashedString>() == nsMakeHashedString("{ c1r1=1, c2r1=2, c3r1=3, c1r2=4, c2r2=5, c3r2=6, c1r3=7, c2r3=8, c3r3=9 }"));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::TempHashedString).Get<nsTempHashedString>() == nsTempHashedString("{ c1r1=1, c2r1=2, c3r1=3, c1r2=4, c2r2=5, c3r2=6, c1r3=7, c2r3=8, c3r3=9 }"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsMat4)")
  {
    nsMat4 m = nsMat4::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6);
    nsVariant v(m);

    TestCanOnlyConvertToStringAndID(v, nsVariant::Type::Matrix4);

    NS_TEST_BOOL(v.ConvertTo<nsMat4>() == m);
    NS_TEST_BOOL(v.ConvertTo<nsString>() == "{ c1r1=1, c2r1=2, c3r1=3, c4r1=4, "
                                            "c1r2=5, c2r2=6, c3r2=7, c4r2=8, "
                                            "c1r3=9, c2r3=0, c3r3=1, c4r3=2, "
                                            "c1r4=3, c2r4=4, c3r4=5, c4r4=6 }");
    NS_TEST_BOOL(v.ConvertTo<nsHashedString>() == nsMakeHashedString("{ c1r1=1, c2r1=2, c3r1=3, c4r1=4, "
                                                                     "c1r2=5, c2r2=6, c3r2=7, c4r2=8, "
                                                                     "c1r3=9, c2r3=0, c3r3=1, c4r3=2, "
                                                                     "c1r4=3, c2r4=4, c3r4=5, c4r4=6 }"));
    NS_TEST_BOOL(v.ConvertTo<nsTempHashedString>() == nsTempHashedString("{ c1r1=1, c2r1=2, c3r1=3, c4r1=4, "
                                                                         "c1r2=5, c2r2=6, c3r2=7, c4r2=8, "
                                                                         "c1r3=9, c2r3=0, c3r3=1, c4r3=2, "
                                                                         "c1r4=3, c2r4=4, c3r4=5, c4r4=6 }"));

    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Matrix4).Get<nsMat4>() == m);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::String).Get<nsString>() == "{ c1r1=1, c2r1=2, c3r1=3, c4r1=4, "
                                                                         "c1r2=5, c2r2=6, c3r2=7, c4r2=8, "
                                                                         "c1r3=9, c2r3=0, c3r3=1, c4r3=2, "
                                                                         "c1r4=3, c2r4=4, c3r4=5, c4r4=6 }");
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::HashedString).Get<nsHashedString>() == nsMakeHashedString("{ c1r1=1, c2r1=2, c3r1=3, c4r1=4, "
                                                                                                        "c1r2=5, c2r2=6, c3r2=7, c4r2=8, "
                                                                                                        "c1r3=9, c2r3=0, c3r3=1, c4r3=2, "
                                                                                                        "c1r4=3, c2r4=4, c3r4=5, c4r4=6 }"));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::TempHashedString).Get<nsTempHashedString>() == nsTempHashedString("{ c1r1=1, c2r1=2, c3r1=3, c4r1=4, "
                                                                                                                "c1r2=5, c2r2=6, c3r2=7, c4r2=8, "
                                                                                                                "c1r3=9, c2r3=0, c3r3=1, c4r3=2, "
                                                                                                                "c1r4=3, c2r4=4, c3r4=5, c4r4=6 }"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsString)")
  {
    nsVariant v("ich hab keine Lust mehr");

    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Invalid) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Bool));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Int8));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::UInt8));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Int16));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::UInt16));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Int32));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::UInt32));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Int64));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::UInt64));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Float));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Double));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Color) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector2) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector3) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector4) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector2I) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector3I) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector4I) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Quaternion) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Matrix3) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Matrix4) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::String));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::StringView));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::DataBuffer) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Time) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Angle) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::ColorGamma) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::HashedString));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::TempHashedString));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::VariantArray) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::VariantDictionary) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::TypedPointer) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::TypedObject) == false);

    {
      nsResult ConversionStatus = NS_SUCCESS;
      NS_TEST_BOOL(v.ConvertTo<bool>(&ConversionStatus) == false);
      NS_TEST_BOOL(ConversionStatus == NS_FAILURE);

      ConversionStatus = NS_SUCCESS;
      NS_TEST_BOOL(v.ConvertTo<nsInt8>(&ConversionStatus) == 0);
      NS_TEST_BOOL(ConversionStatus == NS_FAILURE);

      ConversionStatus = NS_SUCCESS;
      NS_TEST_BOOL(v.ConvertTo<nsUInt8>(&ConversionStatus) == 0);
      NS_TEST_BOOL(ConversionStatus == NS_FAILURE);

      ConversionStatus = NS_SUCCESS;
      NS_TEST_BOOL(v.ConvertTo<nsInt16>(&ConversionStatus) == 0);
      NS_TEST_BOOL(ConversionStatus == NS_FAILURE);

      ConversionStatus = NS_SUCCESS;
      NS_TEST_BOOL(v.ConvertTo<nsUInt16>(&ConversionStatus) == 0);
      NS_TEST_BOOL(ConversionStatus == NS_FAILURE);

      ConversionStatus = NS_SUCCESS;
      NS_TEST_BOOL(v.ConvertTo<nsInt32>(&ConversionStatus) == 0);
      NS_TEST_BOOL(ConversionStatus == NS_FAILURE);

      ConversionStatus = NS_SUCCESS;
      NS_TEST_BOOL(v.ConvertTo<nsUInt32>(&ConversionStatus) == 0);
      NS_TEST_BOOL(ConversionStatus == NS_FAILURE);

      ConversionStatus = NS_SUCCESS;
      NS_TEST_BOOL(v.ConvertTo<nsInt64>(&ConversionStatus) == 0);
      NS_TEST_BOOL(ConversionStatus == NS_FAILURE);

      ConversionStatus = NS_SUCCESS;
      NS_TEST_BOOL(v.ConvertTo<nsUInt64>(&ConversionStatus) == 0);
      NS_TEST_BOOL(ConversionStatus == NS_FAILURE);

      ConversionStatus = NS_SUCCESS;
      NS_TEST_BOOL(v.ConvertTo<float>(&ConversionStatus) == 0.0f);
      NS_TEST_BOOL(ConversionStatus == NS_FAILURE);

      ConversionStatus = NS_SUCCESS;
      NS_TEST_BOOL(v.ConvertTo<double>(&ConversionStatus) == 0.0);
      NS_TEST_BOOL(ConversionStatus == NS_FAILURE);

      ConversionStatus = NS_SUCCESS;
      NS_TEST_BOOL(v.ConvertTo<nsHashedString>(&ConversionStatus) == nsMakeHashedString("ich hab keine Lust mehr"));
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

      ConversionStatus = NS_SUCCESS;
      NS_TEST_BOOL(v.ConvertTo<nsTempHashedString>(&ConversionStatus) == nsTempHashedString("ich hab keine Lust mehr"));
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);
    }

    {
      v = "true";
      nsResult ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo<bool>(&ConversionStatus) == true);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

      ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Bool, &ConversionStatus).Get<bool>() == true);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);
    }

    {
      v = "-128";
      nsResult ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo<nsInt8>(&ConversionStatus) == -128);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

      ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Int8, &ConversionStatus).Get<nsInt8>() == -128);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);
    }

    {
      v = "255";
      nsResult ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo<nsUInt8>(&ConversionStatus) == 255);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

      ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::UInt8, &ConversionStatus).Get<nsUInt8>() == 255);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);
    }

    {
      v = "-5643";
      nsResult ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo<nsInt16>(&ConversionStatus) == -5643);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

      ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Int16, &ConversionStatus).Get<nsInt16>() == -5643);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);
    }

    {
      v = "9001";
      nsResult ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo<nsUInt16>(&ConversionStatus) == 9001);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

      ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::UInt16, &ConversionStatus).Get<nsUInt16>() == 9001);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);
    }

    {
      v = "46";
      nsResult ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo<nsInt32>(&ConversionStatus) == 46);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

      ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Int32, &ConversionStatus).Get<nsInt32>() == 46);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);
    }

    {
      v = "356";
      nsResult ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo<nsUInt32>(&ConversionStatus) == 356);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

      ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::UInt32, &ConversionStatus).Get<nsUInt32>() == 356);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);
    }

    {
      v = "64";
      nsResult ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo<nsInt64>(&ConversionStatus) == 64);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

      ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Int64, &ConversionStatus).Get<nsInt64>() == 64);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);
    }

    {
      v = "6464";
      nsResult ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo<nsUInt64>(&ConversionStatus) == 6464);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

      ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::UInt64, &ConversionStatus).Get<nsUInt64>() == 6464);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);
    }

    {
      v = "0.07564f";
      nsResult ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo<float>(&ConversionStatus) == 0.07564f);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

      ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Float, &ConversionStatus).Get<float>() == 0.07564f);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);
    }

    {
      v = "0.4453";
      nsResult ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo<double>(&ConversionStatus) == 0.4453);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

      ConversionStatus = NS_FAILURE;
      NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Double, &ConversionStatus).Get<double>() == 0.4453);
      NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsStringView)")
  {
    nsStringView va0("Test String");
    nsVariant v(va0, false);

    TestCanOnlyConvertToStringAndID(v, nsVariant::Type::StringView);

    NS_TEST_BOOL(v.ConvertTo<nsStringView>() == va0);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::StringView).Get<nsStringView>() == va0);

    {
      nsVariant va, va2;

      va = "Bla";
      NS_TEST_BOOL(va.IsA<nsString>());
      NS_TEST_BOOL(va.CanConvertTo<nsString>());
      NS_TEST_BOOL(va.CanConvertTo<nsStringView>());

      va = nsVariant("Bla"_nssv, false);
      NS_TEST_BOOL(va.IsA<nsStringView>());
      NS_TEST_BOOL(va.CanConvertTo<nsString>());
      NS_TEST_BOOL(va.CanConvertTo<nsStringView>());

      va2 = va;
      NS_TEST_BOOL(va2.IsA<nsStringView>());
      NS_TEST_BOOL(va2.CanConvertTo<nsString>());
      NS_TEST_BOOL(va2.CanConvertTo<nsStringView>());
      NS_TEST_BOOL(va2.ConvertTo<nsStringView>() == "Bla");
      NS_TEST_BOOL(va2.ConvertTo<nsString>() == "Bla");

      nsVariant va3 = va2.ConvertTo(nsVariantType::StringView);
      NS_TEST_BOOL(va3.IsA<nsStringView>());
      NS_TEST_BOOL(va3.ConvertTo<nsString>() == "Bla");

      va = "Blub";
      NS_TEST_BOOL(va.IsA<nsString>());

      nsVariant va4 = va.ConvertTo(nsVariantType::StringView);
      NS_TEST_BOOL(va4.IsA<nsStringView>());
      NS_TEST_BOOL(va4.ConvertTo<nsString>() == "Blub");
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsDataBuffer)")
  {
    nsDataBuffer va;
    va.PushBack(255);
    va.PushBack(4);
    nsVariant v(va);

    TestCanOnlyConvertToID(v, nsVariant::Type::DataBuffer);

    NS_TEST_BOOL(v.ConvertTo<nsDataBuffer>() == va);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::DataBuffer).Get<nsDataBuffer>() == va);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsTime)")
  {
    nsTime t = nsTime::MakeFromSeconds(123.0);
    nsVariant v(t);

    TestCanOnlyConvertToStringAndID(v, nsVariant::Type::Time);

    NS_TEST_BOOL(v.ConvertTo<nsTime>() == t);
    // NS_TEST_BOOL(v.ConvertTo<nsString>() == "");

    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Time).Get<nsTime>() == t);
    // NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::String).Get<nsString>() == "");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsUuid)")
  {
    const nsUuid uuid = nsUuid::MakeUuid();
    nsVariant v(uuid);

    TestCanOnlyConvertToStringAndID(v, nsVariant::Type::Uuid);

    NS_TEST_BOOL(v.ConvertTo<nsUuid>() == uuid);
    // NS_TEST_BOOL(v.ConvertTo<nsString>() == "");

    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Uuid).Get<nsUuid>() == uuid);
    // NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::String).Get<nsString>() == "");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsAngle)")
  {
    nsAngle t = nsAngle::MakeFromDegree(123.0);
    nsVariant v(t);

    TestCanOnlyConvertToStringAndID(v, nsVariant::Type::Angle);

    NS_TEST_BOOL(v.ConvertTo<nsAngle>() == t);
    NS_TEST_BOOL(v.ConvertTo<nsString>() == "123.0°");
    // NS_TEST_BOOL(v.ConvertTo<nsHashedString>() == nsMakeHashedString("123.0°")); // For some reason the compiler stumbles upon the degree sign, encoding weirdness most likely
    // NS_TEST_BOOL(v.ConvertTo<nsTempHashedString>() == nsTempHashedString("123.0°"));

    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::Angle).Get<nsAngle>() == t);
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::String).Get<nsString>() == "123.0°");
    // NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::HashedString).Get<nsHashedString>() == nsMakeHashedString("123.0°"));
    // NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::TempHashedString).Get<nsTempHashedString>() == nsTempHashedString("123.0°"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsHashedString)")
  {
    nsVariant v(nsMakeHashedString("78"));

    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Invalid) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Bool));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Int8));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::UInt8));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Int16));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::UInt16));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Int32));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::UInt32));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Int64));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::UInt64));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Float));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Double));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Color) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector2) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector3) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector4) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector2I) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector3I) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Vector4I) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Quaternion) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Matrix3) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Matrix4) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::String));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::StringView));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::DataBuffer) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Time) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::Angle) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::ColorGamma) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::HashedString));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::TempHashedString));
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::VariantArray) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::VariantDictionary) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::TypedPointer) == false);
    NS_TEST_BOOL(v.CanConvertTo(nsVariant::Type::TypedObject) == false);

    nsResult ConversionStatus = NS_SUCCESS;
    NS_TEST_BOOL(v.ConvertTo<bool>(&ConversionStatus) == false);
    NS_TEST_BOOL(ConversionStatus == NS_FAILURE);

    ConversionStatus = NS_FAILURE;
    NS_TEST_INT(v.ConvertTo<nsInt8>(&ConversionStatus), 78);
    NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

    ConversionStatus = NS_FAILURE;
    NS_TEST_INT(v.ConvertTo<nsUInt8>(&ConversionStatus), 78);
    NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

    ConversionStatus = NS_FAILURE;
    NS_TEST_INT(v.ConvertTo<nsInt16>(&ConversionStatus), 78);
    NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

    ConversionStatus = NS_FAILURE;
    NS_TEST_INT(v.ConvertTo<nsUInt16>(&ConversionStatus), 78);
    NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

    ConversionStatus = NS_FAILURE;
    NS_TEST_INT(v.ConvertTo<nsInt32>(&ConversionStatus), 78);
    NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

    ConversionStatus = NS_FAILURE;
    NS_TEST_INT(v.ConvertTo<nsUInt32>(&ConversionStatus), 78);
    NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

    ConversionStatus = NS_FAILURE;
    NS_TEST_INT(v.ConvertTo<nsInt64>(&ConversionStatus), 78);
    NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

    ConversionStatus = NS_FAILURE;
    NS_TEST_INT(v.ConvertTo<nsUInt64>(&ConversionStatus), 78);
    NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

    ConversionStatus = NS_FAILURE;
    NS_TEST_BOOL(v.ConvertTo<float>(&ConversionStatus) == 78.0f);
    NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

    ConversionStatus = NS_FAILURE;
    NS_TEST_BOOL(v.ConvertTo<double>(&ConversionStatus) == 78.0);
    NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

    ConversionStatus = NS_FAILURE;
    NS_TEST_STRING(v.ConvertTo<nsString>(&ConversionStatus), "78");
    NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

    ConversionStatus = NS_FAILURE;
    NS_TEST_BOOL(v.ConvertTo<nsStringView>(&ConversionStatus) == "78"_nssv);
    NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);

    ConversionStatus = NS_FAILURE;
    NS_TEST_BOOL(v.ConvertTo<nsTempHashedString>(&ConversionStatus) == nsTempHashedString("78"));
    NS_TEST_BOOL(ConversionStatus == NS_SUCCESS);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsTempHashedString)")
  {
    nsTempHashedString s("VVVV");
    nsVariant v(s);

    TestCanOnlyConvertToStringAndID(v, nsVariant::Type::TempHashedString);

    NS_TEST_BOOL(v.ConvertTo<nsTempHashedString>() == nsTempHashedString("VVVV"));
    NS_TEST_BOOL(v.ConvertTo<nsString>() == "0x69d489c8b7fa5f47");

    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::TempHashedString).Get<nsTempHashedString>() == nsTempHashedString("VVVV"));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::String).Get<nsString>() == "0x69d489c8b7fa5f47");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (VariantArray)")
  {
    nsVariantArray va;
    va.PushBack(2.5);
    va.PushBack("ABC");
    va.PushBack(nsVariant());
    nsVariant v(va);

    TestCanOnlyConvertToStringAndID(v, nsVariant::Type::VariantArray);

    NS_TEST_BOOL(v.ConvertTo<nsVariantArray>() == va);
    NS_TEST_STRING(v.ConvertTo<nsString>(), "[2.5, ABC, <Invalid>]");
    NS_TEST_BOOL(v.ConvertTo<nsHashedString>() == nsMakeHashedString("[2.5, ABC, <Invalid>]"));
    NS_TEST_BOOL(v.ConvertTo<nsTempHashedString>() == nsTempHashedString("[2.5, ABC, <Invalid>]"));

    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::VariantArray).Get<nsVariantArray>() == va);
    NS_TEST_STRING(v.ConvertTo(nsVariant::Type::String).Get<nsString>(), "[2.5, ABC, <Invalid>]");
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::HashedString).Get<nsHashedString>() == nsMakeHashedString("[2.5, ABC, <Invalid>]"));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::TempHashedString).Get<nsTempHashedString>() == nsTempHashedString("[2.5, ABC, <Invalid>]"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "(Can)ConvertTo (nsVariantDictionary)")
  {
    nsVariantDictionary va;
    va.Insert("A", 2.5);
    va.Insert("B", "ABC");
    va.Insert("C", nsVariant());
    nsVariant v(va);

    TestCanOnlyConvertToStringAndID(v, nsVariant::Type::VariantDictionary);

    NS_TEST_BOOL(v.ConvertTo<nsVariantDictionary>() == va);
    NS_TEST_STRING(v.ConvertTo<nsString>(), "{A=2.5, C=<Invalid>, B=ABC}");
    NS_TEST_BOOL(v.ConvertTo<nsHashedString>() == nsMakeHashedString("{A=2.5, C=<Invalid>, B=ABC}"));
    NS_TEST_BOOL(v.ConvertTo<nsTempHashedString>() == nsTempHashedString("{A=2.5, C=<Invalid>, B=ABC}"));

    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::VariantDictionary).Get<nsVariantDictionary>() == va);
    NS_TEST_STRING(v.ConvertTo(nsVariant::Type::String).Get<nsString>(), "{A=2.5, C=<Invalid>, B=ABC}");
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::HashedString).Get<nsHashedString>() == nsMakeHashedString("{A=2.5, C=<Invalid>, B=ABC}"));
    NS_TEST_BOOL(v.ConvertTo(nsVariant::Type::TempHashedString).Get<nsTempHashedString>() == nsTempHashedString("{A=2.5, C=<Invalid>, B=ABC}"));
  }
}

#pragma optimize("", on)
