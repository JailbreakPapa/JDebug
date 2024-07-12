#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Reflection/ReflectionUtils.h>
#include <FoundationTest/Reflection/ReflectionTestClasses.h>

struct FunctionTest
{
  int StandardTypeFunction(int v, const nsVec2 vCv, nsVec3& ref_vRv, const nsVec4& vCrv, nsVec2U32* pPv, const nsVec3U32* pCpv)
  {
    NS_TEST_BOOL(m_values[0] == v);
    NS_TEST_BOOL(m_values[1] == vCv);
    NS_TEST_BOOL(m_values[2] == ref_vRv);
    NS_TEST_BOOL(m_values[3] == vCrv);
    if (m_bPtrAreNull)
    {
      NS_TEST_BOOL(!pPv);
      NS_TEST_BOOL(!pCpv);
    }
    else
    {
      NS_TEST_BOOL(m_values[4] == *pPv);
      NS_TEST_BOOL(m_values[5] == *pCpv);
    }
    ref_vRv.Set(1, 2, 3);
    if (pPv)
    {
      pPv->Set(1, 2);
    }
    return 5;
  }

  nsVarianceTypeAngle CustomTypeFunction(nsVarianceTypeAngle v, const nsVarianceTypeAngle cv, nsVarianceTypeAngle& ref_rv, const nsVarianceTypeAngle& crv, nsVarianceTypeAngle* pPv, const nsVarianceTypeAngle* pCpv)
  {
    NS_TEST_BOOL(m_values[0] == v);
    NS_TEST_BOOL(m_values[1] == cv);
    NS_TEST_BOOL(m_values[2] == ref_rv);
    NS_TEST_BOOL(m_values[3] == crv);
    if (m_bPtrAreNull)
    {
      NS_TEST_BOOL(!pPv);
      NS_TEST_BOOL(!pCpv);
    }
    else
    {
      NS_TEST_BOOL(m_values[4] == *pPv);
      NS_TEST_BOOL(m_values[5] == *pCpv);
    }
    ref_rv = {2.0f, nsAngle::MakeFromDegree(200.0f)};
    if (pPv)
    {
      *pPv = {4.0f, nsAngle::MakeFromDegree(400.0f)};
    }
    return {0.6f, nsAngle::MakeFromDegree(60.0f)};
  }

  nsVarianceTypeAngle CustomTypeFunction2(nsVarianceTypeAngle v, const nsVarianceTypeAngle cv, nsVarianceTypeAngle& ref_rv, const nsVarianceTypeAngle& crv, nsVarianceTypeAngle* pPv, const nsVarianceTypeAngle* pCpv)
  {
    NS_TEST_BOOL(*m_values[0].Get<nsVarianceTypeAngle*>() == v);
    NS_TEST_BOOL(*m_values[1].Get<nsVarianceTypeAngle*>() == cv);
    NS_TEST_BOOL(*m_values[2].Get<nsVarianceTypeAngle*>() == ref_rv);
    NS_TEST_BOOL(*m_values[3].Get<nsVarianceTypeAngle*>() == crv);
    if (m_bPtrAreNull)
    {
      NS_TEST_BOOL(!pPv);
      NS_TEST_BOOL(!pCpv);
    }
    else
    {
      NS_TEST_BOOL(*m_values[4].Get<nsVarianceTypeAngle*>() == *pPv);
      NS_TEST_BOOL(*m_values[5].Get<nsVarianceTypeAngle*>() == *pCpv);
    }
    ref_rv = {2.0f, nsAngle::MakeFromDegree(200.0f)};
    if (pPv)
    {
      *pPv = {4.0f, nsAngle::MakeFromDegree(400.0f)};
    }
    return {0.6f, nsAngle::MakeFromDegree(60.0f)};
  }

  const char* StringTypeFunction(const char* szString, nsString& ref_sString, nsStringView sView)
  {
    if (m_bPtrAreNull)
    {
      NS_TEST_BOOL(!szString);
    }
    else
    {
      NS_TEST_BOOL(m_values[0] == szString);
    }
    NS_TEST_BOOL(m_values[1] == ref_sString);
    NS_TEST_BOOL(m_values[2] == sView);
    return "StringRet";
  }

  nsEnum<nsExampleEnum> EnumFunction(
    nsEnum<nsExampleEnum> e, nsEnum<nsExampleEnum>& ref_re, const nsEnum<nsExampleEnum>& cre, nsEnum<nsExampleEnum>* pPe, const nsEnum<nsExampleEnum>* pCpe)
  {
    NS_TEST_BOOL(m_values[0].Get<nsInt64>() == e.GetValue());
    NS_TEST_BOOL(m_values[1].Get<nsInt64>() == ref_re.GetValue());
    NS_TEST_BOOL(m_values[2].Get<nsInt64>() == cre.GetValue());
    if (m_bPtrAreNull)
    {
      NS_TEST_BOOL(!pPe);
      NS_TEST_BOOL(!pCpe);
    }
    else
    {
      NS_TEST_BOOL(m_values[3].Get<nsInt64>() == pPe->GetValue());
      NS_TEST_BOOL(m_values[4].Get<nsInt64>() == pCpe->GetValue());
    }
    return nsExampleEnum::Value1;
  }

  nsBitflags<nsExampleBitflags> BitflagsFunction(nsBitflags<nsExampleBitflags> e, nsBitflags<nsExampleBitflags>& ref_re,
    const nsBitflags<nsExampleBitflags>& cre, nsBitflags<nsExampleBitflags>* pPe, const nsBitflags<nsExampleBitflags>* pCpe)
  {
    NS_TEST_BOOL(e == m_values[0].Get<nsInt64>());
    NS_TEST_BOOL(ref_re == m_values[1].Get<nsInt64>());
    NS_TEST_BOOL(cre == m_values[2].Get<nsInt64>());
    if (m_bPtrAreNull)
    {
      NS_TEST_BOOL(!pPe);
      NS_TEST_BOOL(!pCpe);
    }
    else
    {
      NS_TEST_BOOL(*pPe == m_values[3].Get<nsInt64>());
      NS_TEST_BOOL(*pCpe == m_values[4].Get<nsInt64>());
    }
    return nsExampleBitflags::Value1 | nsExampleBitflags::Value2;
  }

  nsTestStruct3 StructFunction(
    nsTestStruct3 s, const nsTestStruct3 cs, nsTestStruct3& ref_rs, const nsTestStruct3& crs, nsTestStruct3* pPs, const nsTestStruct3* pCps)
  {
    NS_TEST_BOOL(*static_cast<nsTestStruct3*>(m_values[0].Get<void*>()) == s);
    NS_TEST_BOOL(*static_cast<nsTestStruct3*>(m_values[1].Get<void*>()) == cs);
    NS_TEST_BOOL(*static_cast<nsTestStruct3*>(m_values[2].Get<void*>()) == ref_rs);
    NS_TEST_BOOL(*static_cast<nsTestStruct3*>(m_values[3].Get<void*>()) == crs);
    if (m_bPtrAreNull)
    {
      NS_TEST_BOOL(!pPs);
      NS_TEST_BOOL(!pCps);
    }
    else
    {
      NS_TEST_BOOL(*static_cast<nsTestStruct3*>(m_values[4].Get<void*>()) == *pPs);
      NS_TEST_BOOL(*static_cast<nsTestStruct3*>(m_values[5].Get<void*>()) == *pCps);
    }
    ref_rs.m_fFloat1 = 999.0f;
    ref_rs.m_UInt8 = 666;
    if (pPs)
    {
      pPs->m_fFloat1 = 666.0f;
      pPs->m_UInt8 = 999;
    }
    nsTestStruct3 retS;
    retS.m_fFloat1 = 42;
    retS.m_UInt8 = 42;
    return retS;
  }

  nsTestClass1 ReflectedClassFunction(
    nsTestClass1 s, const nsTestClass1 cs, nsTestClass1& ref_rs, const nsTestClass1& crs, nsTestClass1* pPs, const nsTestClass1* pCps)
  {
    NS_TEST_BOOL(*static_cast<nsTestClass1*>(m_values[0].ConvertTo<void*>()) == s);
    NS_TEST_BOOL(*static_cast<nsTestClass1*>(m_values[1].ConvertTo<void*>()) == cs);
    NS_TEST_BOOL(*static_cast<nsTestClass1*>(m_values[2].ConvertTo<void*>()) == ref_rs);
    NS_TEST_BOOL(*static_cast<nsTestClass1*>(m_values[3].ConvertTo<void*>()) == crs);
    if (m_bPtrAreNull)
    {
      NS_TEST_BOOL(!pPs);
      NS_TEST_BOOL(!pCps);
    }
    else
    {
      NS_TEST_BOOL(*static_cast<nsTestClass1*>(m_values[4].ConvertTo<void*>()) == *pPs);
      NS_TEST_BOOL(*static_cast<nsTestClass1*>(m_values[5].ConvertTo<void*>()) == *pCps);
    }
    ref_rs.m_Color.SetRGB(1, 2, 3);
    ref_rs.m_MyVector.Set(1, 2, 3);
    if (pPs)
    {
      pPs->m_Color.SetRGB(1, 2, 3);
      pPs->m_MyVector.Set(1, 2, 3);
    }
    nsTestClass1 retS;
    retS.m_Color.SetRGB(42, 42, 42);
    retS.m_MyVector.Set(42, 42, 42);
    return retS;
  }

  nsVariant VariantFunction(nsVariant v, const nsVariant cv, nsVariant& ref_rv, const nsVariant& crv, nsVariant* pPv, const nsVariant* pCpv)
  {
    NS_TEST_BOOL(m_values[0] == v);
    NS_TEST_BOOL(m_values[1] == cv);
    NS_TEST_BOOL(m_values[2] == ref_rv);
    NS_TEST_BOOL(m_values[3] == crv);
    if (m_bPtrAreNull)
    {
      // Can't have variant as nullptr as it must exist in the array and there is no further
      // way of distinguishing a between a nsVariant* and a nsVariant that is invalid.
      NS_TEST_BOOL(!pPv->IsValid());
      NS_TEST_BOOL(!pCpv->IsValid());
    }
    else
    {
      NS_TEST_BOOL(m_values[4] == *pPv);
      NS_TEST_BOOL(m_values[5] == *pCpv);
    }
    ref_rv = nsVec3(1, 2, 3);
    if (pPv)
    {
      *pPv = nsVec2U32(1, 2);
    }
    return 5;
  }

  nsVariantArray VariantArrayFunction(nsVariantArray a, const nsVariantArray ca, nsVariantArray& ref_a, const nsVariantArray& cra, nsVariantArray* pA, const nsVariantArray* pCa)
  {
    NS_TEST_BOOL(m_values[0].Get<nsVariantArray>() == a);
    NS_TEST_BOOL(m_values[1].Get<nsVariantArray>() == ca);
    NS_TEST_BOOL(m_values[2].Get<nsVariantArray>() == ref_a);
    NS_TEST_BOOL(m_values[3].Get<nsVariantArray>() == cra);
    if (m_bPtrAreNull)
    {
      NS_TEST_BOOL(!pA);
      NS_TEST_BOOL(!pCa);
    }
    else
    {
      NS_TEST_BOOL(m_values[4] == *pA);
      NS_TEST_BOOL(m_values[5] == *pCa);
    }
    ref_a.Clear();
    ref_a.PushBack(1.0f);
    ref_a.PushBack("Test");
    if (pA)
    {
      pA->Clear();
      pA->PushBack(2.0f);
      pA->PushBack("Test2");
    }

    nsVariantArray ret;
    ret.PushBack(3.0f);
    ret.PushBack("RetTest");
    return ret;
  }

  nsVariantDictionary VariantDictionaryFunction(nsVariantDictionary a, const nsVariantDictionary ca, nsVariantDictionary& ref_a, const nsVariantDictionary& cra, nsVariantDictionary* pA, const nsVariantDictionary* pCa)
  {
    NS_TEST_BOOL(m_values[0].Get<nsVariantDictionary>() == a);
    NS_TEST_BOOL(m_values[1].Get<nsVariantDictionary>() == ca);
    NS_TEST_BOOL(m_values[2].Get<nsVariantDictionary>() == ref_a);
    NS_TEST_BOOL(m_values[3].Get<nsVariantDictionary>() == cra);
    if (m_bPtrAreNull)
    {
      NS_TEST_BOOL(!pA);
      NS_TEST_BOOL(!pCa);
    }
    else
    {
      NS_TEST_BOOL(m_values[4] == *pA);
      NS_TEST_BOOL(m_values[5] == *pCa);
    }
    ref_a.Clear();
    ref_a.Insert("f", 1.0f);
    ref_a.Insert("s", "Test");
    if (pA)
    {
      pA->Clear();
      pA->Insert("f", 2.0f);
      pA->Insert("s", "Test2");
    }

    nsVariantDictionary ret;
    ret.Insert("f", 3.0f);
    ret.Insert("s", "RetTest");
    return ret;
  }

  static void StaticFunction(bool b, nsVariant v)
  {
    NS_TEST_BOOL(b == true);
    NS_TEST_BOOL(v == 4.0f);
  }

  static int StaticFunction2() { return 42; }

  bool m_bPtrAreNull = false;
  nsDynamicArray<nsVariant> m_values;
};

using ParamSig = std::tuple<const nsRTTI*, nsBitflags<nsPropertyFlags>>;

void VerifyFunctionSignature(const nsAbstractFunctionProperty* pFunc, nsArrayPtr<ParamSig> params, ParamSig ret)
{
  NS_TEST_INT(params.GetCount(), pFunc->GetArgumentCount());
  for (nsUInt32 i = 0; i < nsMath::Min(params.GetCount(), pFunc->GetArgumentCount()); i++)
  {
    NS_TEST_BOOL(pFunc->GetArgumentType(i) == std::get<0>(params[i]));
    NS_TEST_BOOL(pFunc->GetArgumentFlags(i) == std::get<1>(params[i]));
  }
  NS_TEST_BOOL(pFunc->GetReturnType() == std::get<0>(ret));
  NS_TEST_BOOL(pFunc->GetReturnFlags() == std::get<1>(ret));
}

NS_CREATE_SIMPLE_TEST(Reflection, Functions)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Member Functions - StandardTypes")
  {
    nsFunctionProperty<decltype(&FunctionTest::StandardTypeFunction)> funccall("", &FunctionTest::StandardTypeFunction);
    ParamSig testSet[] = {
      ParamSig(nsGetStaticRTTI<int>(), nsPropertyFlags::StandardType),
      ParamSig(nsGetStaticRTTI<nsVec2>(), nsPropertyFlags::StandardType),
      ParamSig(nsGetStaticRTTI<nsVec3>(), nsPropertyFlags::StandardType | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsVec4>(), nsPropertyFlags::StandardType | nsPropertyFlags::Const | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsVec2U32>(), nsPropertyFlags::StandardType | nsPropertyFlags::Pointer),
      ParamSig(nsGetStaticRTTI<nsVec3U32>(), nsPropertyFlags::StandardType | nsPropertyFlags::Const | nsPropertyFlags::Pointer),
    };
    VerifyFunctionSignature(&funccall, nsArrayPtr<ParamSig>(testSet), ParamSig(nsGetStaticRTTI<int>(), nsPropertyFlags::StandardType));
    NS_TEST_BOOL(funccall.GetFunctionType() == nsFunctionType::Member);

    FunctionTest test;
    test.m_values.PushBack(1);
    test.m_values.PushBack(nsVec2(2));
    test.m_values.PushBack(nsVec3(3));
    test.m_values.PushBack(nsVec4(4));
    test.m_values.PushBack(nsVec2U32(5));
    test.m_values.PushBack(nsVec3U32(6));

    nsVariant ret;
    funccall.Execute(&test, test.m_values, ret);
    NS_TEST_BOOL(ret.GetType() == nsVariantType::Int32);
    NS_TEST_BOOL(ret == 5);
    NS_TEST_BOOL(test.m_values[2] == nsVec3(1, 2, 3));
    NS_TEST_BOOL(test.m_values[4] == nsVec2U32(1, 2));

    test.m_bPtrAreNull = true;
    test.m_values[4] = nsVariant();
    test.m_values[5] = nsVariant();
    ret = nsVariant();
    funccall.Execute(&test, test.m_values, ret);
    NS_TEST_BOOL(ret.GetType() == nsVariantType::Int32);
    NS_TEST_BOOL(ret == 5);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Member Functions - CustomType")
  {
    nsFunctionProperty<decltype(&FunctionTest::CustomTypeFunction)> funccall("", &FunctionTest::CustomTypeFunction);
    ParamSig testSet[] = {
      ParamSig(nsGetStaticRTTI<nsVarianceTypeAngle>(), nsPropertyFlags::Class),
      ParamSig(nsGetStaticRTTI<nsVarianceTypeAngle>(), nsPropertyFlags::Class),
      ParamSig(nsGetStaticRTTI<nsVarianceTypeAngle>(), nsPropertyFlags::Class | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsVarianceTypeAngle>(), nsPropertyFlags::Class | nsPropertyFlags::Const | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsVarianceTypeAngle>(), nsPropertyFlags::Class | nsPropertyFlags::Pointer),
      ParamSig(nsGetStaticRTTI<nsVarianceTypeAngle>(), nsPropertyFlags::Class | nsPropertyFlags::Const | nsPropertyFlags::Pointer),
    };
    VerifyFunctionSignature(&funccall, nsArrayPtr<ParamSig>(testSet), ParamSig(nsGetStaticRTTI<nsVarianceTypeAngle>(), nsPropertyFlags::Class));
    NS_TEST_BOOL(funccall.GetFunctionType() == nsFunctionType::Member);

    {
      FunctionTest test;
      test.m_values.PushBack(nsVarianceTypeAngle{0.0f, nsAngle::MakeFromDegree(0.0f)});
      test.m_values.PushBack(nsVarianceTypeAngle{0.1f, nsAngle::MakeFromDegree(10.0f)});
      test.m_values.PushBack(nsVarianceTypeAngle{0.2f, nsAngle::MakeFromDegree(20.0f)});
      test.m_values.PushBack(nsVarianceTypeAngle{0.3f, nsAngle::MakeFromDegree(30.0f)});
      test.m_values.PushBack(nsVarianceTypeAngle{0.4f, nsAngle::MakeFromDegree(40.0f)});
      test.m_values.PushBack(nsVarianceTypeAngle{0.5f, nsAngle::MakeFromDegree(50.0f)});

      nsVariant ret;
      funccall.Execute(&test, test.m_values, ret);
      NS_TEST_BOOL(ret.GetType() == nsVariantType::TypedObject);
      NS_TEST_BOOL(ret == nsVariant(nsVarianceTypeAngle{0.6f, nsAngle::MakeFromDegree(60.0f)}));
      NS_TEST_BOOL(test.m_values[2] == nsVariant(nsVarianceTypeAngle{2.0f, nsAngle::MakeFromDegree(200.0f)}));
      NS_TEST_BOOL(test.m_values[4] == nsVariant(nsVarianceTypeAngle{4.0f, nsAngle::MakeFromDegree(400.0f)}));

      test.m_bPtrAreNull = true;
      test.m_values[4] = nsVariant();
      test.m_values[5] = nsVariant();
      ret = nsVariant();
      funccall.Execute(&test, test.m_values, ret);
      NS_TEST_BOOL(ret.GetType() == nsVariantType::TypedObject);
      NS_TEST_BOOL(ret == nsVariant(nsVarianceTypeAngle{0.6f, nsAngle::MakeFromDegree(60.0f)}));
    }

    {
      nsFunctionProperty<decltype(&FunctionTest::CustomTypeFunction2)> funccall2("", &FunctionTest::CustomTypeFunction2);

      FunctionTest test;
      nsVarianceTypeAngle v0{0.0f, nsAngle::MakeFromDegree(0.0f)};
      nsVarianceTypeAngle v1{0.1f, nsAngle::MakeFromDegree(10.0f)};
      nsVarianceTypeAngle v2{0.2f, nsAngle::MakeFromDegree(20.0f)};
      nsVarianceTypeAngle v3{0.3f, nsAngle::MakeFromDegree(30.0f)};
      nsVarianceTypeAngle v4{0.4f, nsAngle::MakeFromDegree(40.0f)};
      nsVarianceTypeAngle v5{0.5f, nsAngle::MakeFromDegree(50.0f)};
      test.m_values.PushBack(&v0);
      test.m_values.PushBack(&v1);
      test.m_values.PushBack(&v2);
      test.m_values.PushBack(&v3);
      test.m_values.PushBack(&v4);
      test.m_values.PushBack(&v5);

      nsVariant ret;
      funccall2.Execute(&test, test.m_values, ret);
      NS_TEST_BOOL(ret.GetType() == nsVariantType::TypedObject);
      NS_TEST_BOOL(ret == nsVariant(nsVarianceTypeAngle{0.6f, nsAngle::MakeFromDegree(60.0f)}));
      NS_TEST_BOOL((*test.m_values[2].Get<nsVarianceTypeAngle*>() == nsVarianceTypeAngle{2.0f, nsAngle::MakeFromDegree(200.0f)}));
      NS_TEST_BOOL((*test.m_values[4].Get<nsVarianceTypeAngle*>() == nsVarianceTypeAngle{4.0f, nsAngle::MakeFromDegree(400.0f)}));

      test.m_bPtrAreNull = true;
      test.m_values[4] = nsVariant();
      test.m_values[5] = nsVariant();
      ret = nsVariant();
      funccall2.Execute(&test, test.m_values, ret);
      NS_TEST_BOOL(ret.GetType() == nsVariantType::TypedObject);
      NS_TEST_BOOL(ret == nsVariant(nsVarianceTypeAngle{0.6f, nsAngle::MakeFromDegree(60.0f)}));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Member Functions - Strings")
  {
    nsFunctionProperty<decltype(&FunctionTest::StringTypeFunction)> funccall("", &FunctionTest::StringTypeFunction);
    ParamSig testSet[] = {
      ParamSig(nsGetStaticRTTI<const char*>(), nsPropertyFlags::StandardType | nsPropertyFlags::Const),
      ParamSig(nsGetStaticRTTI<nsString>(), nsPropertyFlags::StandardType | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsStringView>(), nsPropertyFlags::StandardType),
    };
    VerifyFunctionSignature(
      &funccall, nsArrayPtr<ParamSig>(testSet), ParamSig(nsGetStaticRTTI<const char*>(), nsPropertyFlags::StandardType | nsPropertyFlags::Const));
    NS_TEST_BOOL(funccall.GetFunctionType() == nsFunctionType::Member);

    FunctionTest test;
    test.m_values.PushBack(nsVariant(nsString("String0")));
    test.m_values.PushBack(nsVariant(nsString("String1")));
    test.m_values.PushBack(nsVariant(nsStringView("String2"), false));

    {
      // Exact types
      nsVariant ret;
      funccall.Execute(&test, test.m_values, ret);
      NS_TEST_BOOL(ret.GetType() == nsVariantType::String);
      NS_TEST_BOOL(ret == nsString("StringRet"));
    }

    {
      // Using nsString instead of nsStringView
      test.m_values[2] = nsString("String2");
      nsVariant ret;
      funccall.Execute(&test, test.m_values, ret);
      NS_TEST_BOOL(ret.GetType() == nsVariantType::String);
      NS_TEST_BOOL(ret == nsString("StringRet"));
      test.m_values[2] = nsVariant(nsStringView("String2"), false);
    }

    {
      // Using nullptr instead of const char*
      test.m_bPtrAreNull = true;
      test.m_values[0] = nsVariant();
      nsVariant ret;
      funccall.Execute(&test, test.m_values, ret);
      NS_TEST_BOOL(ret.GetType() == nsVariantType::String);
      NS_TEST_BOOL(ret == nsString("StringRet"));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Member Functions - Enum")
  {
    nsFunctionProperty<decltype(&FunctionTest::EnumFunction)> funccall("", &FunctionTest::EnumFunction);
    ParamSig testSet[] = {
      ParamSig(nsGetStaticRTTI<nsExampleEnum>(), nsPropertyFlags::IsEnum),
      ParamSig(nsGetStaticRTTI<nsExampleEnum>(), nsPropertyFlags::IsEnum | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsExampleEnum>(), nsPropertyFlags::IsEnum | nsPropertyFlags::Const | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsExampleEnum>(), nsPropertyFlags::IsEnum | nsPropertyFlags::Pointer),
      ParamSig(nsGetStaticRTTI<nsExampleEnum>(), nsPropertyFlags::IsEnum | nsPropertyFlags::Const | nsPropertyFlags::Pointer),
    };
    VerifyFunctionSignature(&funccall, nsArrayPtr<ParamSig>(testSet), ParamSig(nsGetStaticRTTI<nsExampleEnum>(), nsPropertyFlags::IsEnum));
    NS_TEST_BOOL(funccall.GetFunctionType() == nsFunctionType::Member);

    FunctionTest test;
    test.m_values.PushBack((nsInt64)nsExampleEnum::Value1);
    test.m_values.PushBack((nsInt64)nsExampleEnum::Value2);
    test.m_values.PushBack((nsInt64)nsExampleEnum::Value3);
    test.m_values.PushBack((nsInt64)nsExampleEnum::Default);
    test.m_values.PushBack((nsInt64)nsExampleEnum::Value3);

    nsVariant ret;
    funccall.Execute(&test, test.m_values, ret);
    NS_TEST_BOOL(ret.GetType() == nsVariantType::Int64);
    NS_TEST_BOOL(ret == (nsInt64)nsExampleEnum::Value1);

    test.m_bPtrAreNull = true;
    test.m_values[3] = nsVariant();
    test.m_values[4] = nsVariant();
    ret = nsVariant();
    funccall.Execute(&test, test.m_values, ret);
    NS_TEST_BOOL(ret.GetType() == nsVariantType::Int64);
    NS_TEST_BOOL(ret == (nsInt64)nsExampleEnum::Value1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Member Functions - Bitflags")
  {
    nsFunctionProperty<decltype(&FunctionTest::BitflagsFunction)> funccall("", &FunctionTest::BitflagsFunction);
    ParamSig testSet[] = {
      ParamSig(nsGetStaticRTTI<nsExampleBitflags>(), nsPropertyFlags::Bitflags),
      ParamSig(nsGetStaticRTTI<nsExampleBitflags>(), nsPropertyFlags::Bitflags | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsExampleBitflags>(), nsPropertyFlags::Bitflags | nsPropertyFlags::Const | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsExampleBitflags>(), nsPropertyFlags::Bitflags | nsPropertyFlags::Pointer),
      ParamSig(nsGetStaticRTTI<nsExampleBitflags>(), nsPropertyFlags::Bitflags | nsPropertyFlags::Const | nsPropertyFlags::Pointer),
    };
    VerifyFunctionSignature(&funccall, nsArrayPtr<ParamSig>(testSet), ParamSig(nsGetStaticRTTI<nsExampleBitflags>(), nsPropertyFlags::Bitflags));
    NS_TEST_BOOL(funccall.GetFunctionType() == nsFunctionType::Member);

    FunctionTest test;
    test.m_values.PushBack((nsInt64)(0));
    test.m_values.PushBack((nsInt64)(nsExampleBitflags::Value2));
    test.m_values.PushBack((nsInt64)(nsExampleBitflags::Value3 | nsExampleBitflags::Value2).GetValue());
    test.m_values.PushBack((nsInt64)(nsExampleBitflags::Value1 | nsExampleBitflags::Value2 | nsExampleBitflags::Value3).GetValue());
    test.m_values.PushBack((nsInt64)(nsExampleBitflags::Value3));

    nsVariant ret;
    funccall.Execute(&test, test.m_values, ret);
    NS_TEST_BOOL(ret.GetType() == nsVariantType::Int64);
    NS_TEST_BOOL(ret == (nsInt64)(nsExampleBitflags::Value1 | nsExampleBitflags::Value2).GetValue());

    test.m_bPtrAreNull = true;
    test.m_values[3] = nsVariant();
    test.m_values[4] = nsVariant();
    ret = nsVariant();
    funccall.Execute(&test, test.m_values, ret);
    NS_TEST_BOOL(ret.GetType() == nsVariantType::Int64);
    NS_TEST_BOOL(ret == (nsInt64)(nsExampleBitflags::Value1 | nsExampleBitflags::Value2).GetValue());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Member Functions - Structs")
  {
    nsFunctionProperty<decltype(&FunctionTest::StructFunction)> funccall("", &FunctionTest::StructFunction);
    ParamSig testSet[] = {
      ParamSig(nsGetStaticRTTI<nsTestStruct3>(), nsPropertyFlags::Class),
      ParamSig(nsGetStaticRTTI<nsTestStruct3>(), nsPropertyFlags::Class),
      ParamSig(nsGetStaticRTTI<nsTestStruct3>(), nsPropertyFlags::Class | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsTestStruct3>(), nsPropertyFlags::Class | nsPropertyFlags::Const | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsTestStruct3>(), nsPropertyFlags::Class | nsPropertyFlags::Pointer),
      ParamSig(nsGetStaticRTTI<nsTestStruct3>(), nsPropertyFlags::Class | nsPropertyFlags::Const | nsPropertyFlags::Pointer),
    };
    VerifyFunctionSignature(&funccall, nsArrayPtr<ParamSig>(testSet), ParamSig(nsGetStaticRTTI<nsTestStruct3>(), nsPropertyFlags::Class));
    NS_TEST_BOOL(funccall.GetFunctionType() == nsFunctionType::Member);

    FunctionTest test;
    nsTestStruct3 retS;
    retS.m_fFloat1 = 0;
    retS.m_UInt8 = 0;
    nsTestStruct3 value;
    value.m_fFloat1 = 0;
    value.m_UInt8 = 0;
    nsTestStruct3 rs;
    rs.m_fFloat1 = 42;
    nsTestStruct3 ps;
    ps.m_fFloat1 = 18;

    test.m_values.PushBack(nsVariant(&value));
    test.m_values.PushBack(nsVariant(&value));
    test.m_values.PushBack(nsVariant(&rs));
    test.m_values.PushBack(nsVariant(&value));
    test.m_values.PushBack(nsVariant(&ps));
    test.m_values.PushBack(nsVariant(&value));

    // nsVariantAdapter<nsTestStruct3 const*> aa(nsVariant(&value));
    // auto bla = nsIsStandardType<nsTestStruct3 const*>::value;

    nsVariant ret(&retS);
    funccall.Execute(&test, test.m_values, ret);
    NS_TEST_FLOAT(retS.m_fFloat1, 42, 0);
    NS_TEST_INT(retS.m_UInt8, 42);

    NS_TEST_FLOAT(rs.m_fFloat1, 999, 0);
    NS_TEST_INT(rs.m_UInt8, 666);

    NS_TEST_DOUBLE(ps.m_fFloat1, 666, 0);
    NS_TEST_INT(ps.m_UInt8, 999);

    test.m_bPtrAreNull = true;
    test.m_values[4] = nsVariant();
    test.m_values[5] = nsVariant();
    funccall.Execute(&test, test.m_values, ret);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Member Functions - Reflected Classes")
  {
    nsFunctionProperty<decltype(&FunctionTest::ReflectedClassFunction)> funccall("", &FunctionTest::ReflectedClassFunction);
    ParamSig testSet[] = {
      ParamSig(nsGetStaticRTTI<nsTestClass1>(), nsPropertyFlags::Class),
      ParamSig(nsGetStaticRTTI<nsTestClass1>(), nsPropertyFlags::Class),
      ParamSig(nsGetStaticRTTI<nsTestClass1>(), nsPropertyFlags::Class | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsTestClass1>(), nsPropertyFlags::Class | nsPropertyFlags::Const | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsTestClass1>(), nsPropertyFlags::Class | nsPropertyFlags::Pointer),
      ParamSig(nsGetStaticRTTI<nsTestClass1>(), nsPropertyFlags::Class | nsPropertyFlags::Const | nsPropertyFlags::Pointer),
    };
    VerifyFunctionSignature(&funccall, nsArrayPtr<ParamSig>(testSet), ParamSig(nsGetStaticRTTI<nsTestClass1>(), nsPropertyFlags::Class));
    NS_TEST_BOOL(funccall.GetFunctionType() == nsFunctionType::Member);

    FunctionTest test;
    nsTestClass1 retS;
    retS.m_Color = nsColor::Chocolate;
    nsTestClass1 value;
    value.m_Color = nsColor::AliceBlue;
    nsTestClass1 rs;
    rs.m_Color = nsColor::Beige;
    nsTestClass1 ps;
    ps.m_Color = nsColor::DarkBlue;

    test.m_values.PushBack(nsVariant(&value));
    test.m_values.PushBack(nsVariant(&value));
    test.m_values.PushBack(nsVariant(&rs));
    test.m_values.PushBack(nsVariant(&value));
    test.m_values.PushBack(nsVariant(&ps));
    test.m_values.PushBack(nsVariant(&value));

    rs.m_Color.SetRGB(1, 2, 3);
    rs.m_MyVector.Set(1, 2, 3);


    nsVariant ret(&retS);
    funccall.Execute(&test, test.m_values, ret);
    NS_TEST_BOOL(retS.m_Color == nsColor(42, 42, 42));
    NS_TEST_BOOL(retS.m_MyVector == nsVec3(42, 42, 42));

    NS_TEST_BOOL(rs.m_Color == nsColor(1, 2, 3));
    NS_TEST_BOOL(rs.m_MyVector == nsVec3(1, 2, 3));

    NS_TEST_BOOL(ps.m_Color == nsColor(1, 2, 3));
    NS_TEST_BOOL(ps.m_MyVector == nsVec3(1, 2, 3));

    test.m_bPtrAreNull = true;
    test.m_values[4] = nsVariant();
    test.m_values[5] = nsVariant();
    funccall.Execute(&test, test.m_values, ret);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Member Functions - Variant")
  {
    nsFunctionProperty<decltype(&FunctionTest::VariantFunction)> funccall("", &FunctionTest::VariantFunction);
    ParamSig testSet[] = {
      ParamSig(nsGetStaticRTTI<nsVariant>(), nsPropertyFlags::StandardType),
      ParamSig(nsGetStaticRTTI<nsVariant>(), nsPropertyFlags::StandardType),
      ParamSig(nsGetStaticRTTI<nsVariant>(), nsPropertyFlags::StandardType | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsVariant>(), nsPropertyFlags::StandardType | nsPropertyFlags::Const | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsVariant>(), nsPropertyFlags::StandardType | nsPropertyFlags::Pointer),
      ParamSig(nsGetStaticRTTI<nsVariant>(), nsPropertyFlags::StandardType | nsPropertyFlags::Const | nsPropertyFlags::Pointer),
    };
    VerifyFunctionSignature(&funccall, nsArrayPtr<ParamSig>(testSet), ParamSig(nsGetStaticRTTI<nsVariant>(), nsPropertyFlags::StandardType));
    NS_TEST_BOOL(funccall.GetFunctionType() == nsFunctionType::Member);

    FunctionTest test;
    test.m_values.PushBack(1);
    test.m_values.PushBack(nsVec2(2));
    test.m_values.PushBack(nsVec3(3));
    test.m_values.PushBack(nsVec4(4));
    test.m_values.PushBack(nsVec2U32(5));
    test.m_values.PushBack(nsVec3U32(6));

    nsVariant ret;
    funccall.Execute(&test, test.m_values, ret);
    NS_TEST_BOOL(ret.GetType() == nsVariantType::Int32);
    NS_TEST_BOOL(ret == 5);
    NS_TEST_BOOL(test.m_values[2] == nsVec3(1, 2, 3));
    NS_TEST_BOOL(test.m_values[4] == nsVec2U32(1, 2));

    test.m_bPtrAreNull = true;
    test.m_values[4] = nsVariant();
    test.m_values[5] = nsVariant();
    ret = nsVariant();
    funccall.Execute(&test, test.m_values, ret);
    NS_TEST_BOOL(ret.GetType() == nsVariantType::Int32);
    NS_TEST_BOOL(ret == 5);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Member Functions - VariantArray")
  {
    nsFunctionProperty<decltype(&FunctionTest::VariantArrayFunction)> funccall("", &FunctionTest::VariantArrayFunction);
    ParamSig testSet[] = {
      ParamSig(nsGetStaticRTTI<nsVariantArray>(), nsPropertyFlags::Class),
      ParamSig(nsGetStaticRTTI<nsVariantArray>(), nsPropertyFlags::Class),
      ParamSig(nsGetStaticRTTI<nsVariantArray>(), nsPropertyFlags::Class | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsVariantArray>(), nsPropertyFlags::Class | nsPropertyFlags::Const | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsVariantArray>(), nsPropertyFlags::Class | nsPropertyFlags::Pointer),
      ParamSig(nsGetStaticRTTI<nsVariantArray>(), nsPropertyFlags::Class | nsPropertyFlags::Const | nsPropertyFlags::Pointer),
    };
    VerifyFunctionSignature(&funccall, nsArrayPtr<ParamSig>(testSet), ParamSig(nsGetStaticRTTI<nsVariantArray>(), nsPropertyFlags::Class));
    NS_TEST_BOOL(funccall.GetFunctionType() == nsFunctionType::Member);

    nsVariantArray testA;
    testA.PushBack(nsVec3(3));
    testA.PushBack(nsTime::MakeFromHours(22));
    testA.PushBack("Hello");

    FunctionTest test;
    for (nsUInt32 i = 0; i < 6; ++i)
    {
      test.m_values.PushBack(testA);
      testA.PushBack(i);
    }

    nsVariantArray expectedOutRef;
    expectedOutRef.PushBack(1.0f);
    expectedOutRef.PushBack("Test");

    nsVariantArray expectedOutPtr;
    expectedOutPtr.PushBack(2.0f);
    expectedOutPtr.PushBack("Test2");

    nsVariantArray expectedRet;
    expectedRet.PushBack(3.0f);
    expectedRet.PushBack("RetTest");

    nsVariant ret;
    funccall.Execute(&test, test.m_values, ret);
    NS_TEST_BOOL(ret.GetType() == nsVariantType::VariantArray);
    NS_TEST_BOOL(ret.Get<nsVariantArray>() == expectedRet);
    NS_TEST_BOOL(test.m_values[2] == expectedOutRef);
    NS_TEST_BOOL(test.m_values[4] == expectedOutPtr);

    test.m_bPtrAreNull = true;
    test.m_values[4] = nsVariant();
    test.m_values[5] = nsVariant();
    ret = nsVariant();
    funccall.Execute(&test, test.m_values, ret);
    NS_TEST_BOOL(ret.GetType() == nsVariantType::VariantArray);
    NS_TEST_BOOL(ret.Get<nsVariantArray>() == expectedRet);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Member Functions - VariantDictionary")
  {
    nsFunctionProperty<decltype(&FunctionTest::VariantDictionaryFunction)> funccall("", &FunctionTest::VariantDictionaryFunction);
    ParamSig testSet[] = {
      ParamSig(nsGetStaticRTTI<nsVariantDictionary>(), nsPropertyFlags::Class),
      ParamSig(nsGetStaticRTTI<nsVariantDictionary>(), nsPropertyFlags::Class),
      ParamSig(nsGetStaticRTTI<nsVariantDictionary>(), nsPropertyFlags::Class | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsVariantDictionary>(), nsPropertyFlags::Class | nsPropertyFlags::Const | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsVariantDictionary>(), nsPropertyFlags::Class | nsPropertyFlags::Pointer),
      ParamSig(nsGetStaticRTTI<nsVariantDictionary>(), nsPropertyFlags::Class | nsPropertyFlags::Const | nsPropertyFlags::Pointer),
    };
    VerifyFunctionSignature(&funccall, nsArrayPtr<ParamSig>(testSet), ParamSig(nsGetStaticRTTI<nsVariantDictionary>(), nsPropertyFlags::Class));
    NS_TEST_BOOL(funccall.GetFunctionType() == nsFunctionType::Member);

    nsVariantDictionary testA;
    testA.Insert("v", nsVec3(3));
    testA.Insert("t", nsTime::MakeFromHours(22));
    testA.Insert("s", "Hello");

    nsStringBuilder tmp;
    FunctionTest test;
    for (nsUInt32 i = 0; i < 6; ++i)
    {
      test.m_values.PushBack(testA);
      testA.Insert(nsConversionUtils::ToString(i, tmp), i);
    }

    nsVariantDictionary expectedOutRef;
    expectedOutRef.Insert("f", 1.0f);
    expectedOutRef.Insert("s", "Test");

    nsVariantDictionary expectedOutPtr;
    expectedOutPtr.Insert("f", 2.0f);
    expectedOutPtr.Insert("s", "Test2");

    nsVariantDictionary expectedRet;
    expectedRet.Insert("f", 3.0f);
    expectedRet.Insert("s", "RetTest");

    nsVariant ret;
    funccall.Execute(&test, test.m_values, ret);
    NS_TEST_BOOL(ret.GetType() == nsVariantType::VariantDictionary);
    NS_TEST_BOOL(ret.Get<nsVariantDictionary>() == expectedRet);
    NS_TEST_BOOL(test.m_values[2] == expectedOutRef);
    NS_TEST_BOOL(test.m_values[4] == expectedOutPtr);

    test.m_bPtrAreNull = true;
    test.m_values[4] = nsVariant();
    test.m_values[5] = nsVariant();
    ret = nsVariant();
    funccall.Execute(&test, test.m_values, ret);
    NS_TEST_BOOL(ret.GetType() == nsVariantType::VariantDictionary);
    NS_TEST_BOOL(ret.Get<nsVariantDictionary>() == expectedRet);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Static Functions")
  {
    // Void return
    nsFunctionProperty<decltype(&FunctionTest::StaticFunction)> funccall("", &FunctionTest::StaticFunction);
    ParamSig testSet[] = {
      ParamSig(nsGetStaticRTTI<bool>(), nsPropertyFlags::StandardType),
      ParamSig(nsGetStaticRTTI<nsVariant>(), nsPropertyFlags::StandardType),
    };
    VerifyFunctionSignature(&funccall, nsArrayPtr<ParamSig>(testSet), ParamSig(nsGetStaticRTTI<void>(), nsPropertyFlags::Void));
    NS_TEST_BOOL(funccall.GetFunctionType() == nsFunctionType::StaticMember);

    nsDynamicArray<nsVariant> values;
    values.PushBack(true);
    values.PushBack(4.0f);
    nsVariant ret;
    funccall.Execute(nullptr, values, ret);
    NS_TEST_BOOL(ret.GetType() == nsVariantType::Invalid);

    // Zero parameter
    nsFunctionProperty<decltype(&FunctionTest::StaticFunction2)> funccall2("", &FunctionTest::StaticFunction2);
    VerifyFunctionSignature(&funccall2, nsArrayPtr<ParamSig>(), ParamSig(nsGetStaticRTTI<int>(), nsPropertyFlags::StandardType));
    NS_TEST_BOOL(funccall.GetFunctionType() == nsFunctionType::StaticMember);
    values.Clear();
    funccall2.Execute(nullptr, values, ret);
    NS_TEST_BOOL(ret.GetType() == nsVariantType::Int32);
    NS_TEST_BOOL(ret == 42);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor Functions - StandardTypes")
  {
    nsConstructorFunctionProperty<nsVec4, float, float, float, float> funccall;
    ParamSig testSet[] = {
      ParamSig(nsGetStaticRTTI<float>(), nsPropertyFlags::StandardType),
      ParamSig(nsGetStaticRTTI<float>(), nsPropertyFlags::StandardType),
      ParamSig(nsGetStaticRTTI<float>(), nsPropertyFlags::StandardType),
      ParamSig(nsGetStaticRTTI<float>(), nsPropertyFlags::StandardType),
    };
    VerifyFunctionSignature(
      &funccall, nsArrayPtr<ParamSig>(testSet), ParamSig(nsGetStaticRTTI<nsVec4>(), nsPropertyFlags::StandardType | nsPropertyFlags::Pointer));
    NS_TEST_BOOL(funccall.GetFunctionType() == nsFunctionType::Constructor);

    nsDynamicArray<nsVariant> values;
    values.PushBack(1.0f);
    values.PushBack(2.0f);
    values.PushBack(3.0f);
    values.PushBack(4.0f);
    nsVariant ret;
    funccall.Execute(nullptr, values, ret);
    NS_TEST_BOOL(ret.GetType() == nsVariantType::Vector4);
    NS_TEST_BOOL(ret == nsVec4(1.0f, 2.0f, 3.0f, 4.0f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor Functions - Struct")
  {
    nsConstructorFunctionProperty<nsTestStruct3, double, nsInt16> funccall;
    ParamSig testSet[] = {
      ParamSig(nsGetStaticRTTI<double>(), nsPropertyFlags::StandardType),
      ParamSig(nsGetStaticRTTI<nsInt16>(), nsPropertyFlags::StandardType),
    };
    VerifyFunctionSignature(
      &funccall, nsArrayPtr<ParamSig>(testSet), ParamSig(nsGetStaticRTTI<nsTestStruct3>(), nsPropertyFlags::Class | nsPropertyFlags::Pointer));
    NS_TEST_BOOL(funccall.GetFunctionType() == nsFunctionType::Constructor);

    nsDynamicArray<nsVariant> values;
    values.PushBack(59.0);
    values.PushBack((nsInt16)666);
    nsVariant ret;
    funccall.Execute(nullptr, values, ret);
    NS_TEST_BOOL(ret.GetType() == nsVariantType::TypedPointer);
    nsTestStruct3* pRet = static_cast<nsTestStruct3*>(ret.ConvertTo<void*>());
    NS_TEST_BOOL(pRet != nullptr);

    NS_TEST_FLOAT(pRet->m_fFloat1, 59.0, 0);
    NS_TEST_INT(pRet->m_UInt8, 666);
    NS_TEST_INT(pRet->GetIntPublic(), 32);

    NS_DEFAULT_DELETE(pRet);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor Functions - Reflected Classes")
  {
    // The function signature does not actually need to match the ctor 100% as long as implicit conversion is possible.
    nsConstructorFunctionProperty<nsTestClass1, const nsColor&, const nsTestStruct&> funccall;
    ParamSig testSet[] = {
      ParamSig(nsGetStaticRTTI<nsColor>(), nsPropertyFlags::StandardType | nsPropertyFlags::Const | nsPropertyFlags::Reference),
      ParamSig(nsGetStaticRTTI<nsTestStruct>(), nsPropertyFlags::Class | nsPropertyFlags::Const | nsPropertyFlags::Reference),
    };
    VerifyFunctionSignature(
      &funccall, nsArrayPtr<ParamSig>(testSet), ParamSig(nsGetStaticRTTI<nsTestClass1>(), nsPropertyFlags::Class | nsPropertyFlags::Pointer));
    NS_TEST_BOOL(funccall.GetFunctionType() == nsFunctionType::Constructor);

    nsDynamicArray<nsVariant> values;
    nsTestStruct s;
    s.m_fFloat1 = 1.0f;
    s.m_UInt8 = 255;
    values.PushBack(nsColor::CornflowerBlue);
    values.PushBack(nsVariant(&s));
    nsVariant ret;
    funccall.Execute(nullptr, values, ret);
    NS_TEST_BOOL(ret.GetType() == nsVariantType::TypedPointer);
    nsTestClass1* pRet = static_cast<nsTestClass1*>(ret.ConvertTo<void*>());
    NS_TEST_BOOL(pRet != nullptr);

    NS_TEST_BOOL(pRet->m_Color == nsColor::CornflowerBlue);
    NS_TEST_BOOL(pRet->m_Struct == s);
    NS_TEST_BOOL(pRet->m_MyVector == nsVec3(1, 2, 3));

    NS_DEFAULT_DELETE(pRet);
  }
}
