#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Serialization/ReflectionSerializer.h>
#include <FoundationTest/Reflection/ReflectionTestClasses.h>


template <typename T>
void TestSerialization(const T& source)
{
  nsDefaultMemoryStreamStorage StreamStorage;

  NS_TEST_BLOCK(nsTestBlock::Enabled, "WriteObjectToDDL")
  {
    nsMemoryStreamWriter FileOut(&StreamStorage);

    nsReflectionSerializer::WriteObjectToDDL(FileOut, nsGetStaticRTTI<T>(), &source, false, nsOpenDdlWriter::TypeStringMode::Compliant);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReadObjectPropertiesFromDDL")
  {
    nsMemoryStreamReader FileIn(&StreamStorage);
    T data;
    nsReflectionSerializer::ReadObjectPropertiesFromDDL(FileIn, *nsGetStaticRTTI<T>(), &data);

    NS_TEST_BOOL(data == source);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReadObjectFromDDL")
  {
    nsMemoryStreamReader FileIn(&StreamStorage);

    const nsRTTI* pRtti;
    void* pObject = nsReflectionSerializer::ReadObjectFromDDL(FileIn, pRtti);

    T& c2 = *((T*)pObject);

    NS_TEST_BOOL(c2 == source);

    if (pObject)
    {
      pRtti->GetAllocator()->Deallocate(pObject);
    }
  }

  nsDefaultMemoryStreamStorage StreamStorageBinary;
  NS_TEST_BLOCK(nsTestBlock::Enabled, "WriteObjectToBinary")
  {
    nsMemoryStreamWriter FileOut(&StreamStorageBinary);

    nsReflectionSerializer::WriteObjectToBinary(FileOut, nsGetStaticRTTI<T>(), &source);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReadObjectPropertiesFromBinary")
  {
    nsMemoryStreamReader FileIn(&StreamStorageBinary);
    T data;
    nsReflectionSerializer::ReadObjectPropertiesFromBinary(FileIn, *nsGetStaticRTTI<T>(), &data);

    NS_TEST_BOOL(data == source);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReadObjectFromBinary")
  {
    nsMemoryStreamReader FileIn(&StreamStorageBinary);

    const nsRTTI* pRtti;
    void* pObject = nsReflectionSerializer::ReadObjectFromBinary(FileIn, pRtti);

    T& c2 = *((T*)pObject);

    NS_TEST_BOOL(c2 == source);

    if (pObject)
    {
      pRtti->GetAllocator()->Deallocate(pObject);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Clone")
  {
    {
      T clone;
      nsReflectionSerializer::Clone(&source, &clone, nsGetStaticRTTI<T>());
      NS_TEST_BOOL(clone == source);
      NS_TEST_BOOL(nsReflectionUtils::IsEqual(&clone, &source, nsGetStaticRTTI<T>()));
    }

    {
      T* pClone = nsReflectionSerializer::Clone(&source);
      NS_TEST_BOOL(*pClone == source);
      NS_TEST_BOOL(nsReflectionUtils::IsEqual(pClone, &source));
      nsGetStaticRTTI<T>()->GetAllocator()->Deallocate(pClone);
    }
  }
}


NS_CREATE_SIMPLE_TEST_GROUP(Reflection);

NS_CREATE_SIMPLE_TEST(Reflection, Types)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Iterate All")
  {
    bool bFoundStruct = false;
    bool bFoundClass1 = false;
    bool bFoundClass2 = false;

    nsRTTI::ForEachType([&](const nsRTTI* pRtti)
      {
        if (pRtti->GetTypeName() == "nsTestStruct")
          bFoundStruct = true;
        if (pRtti->GetTypeName() == "nsTestClass1")
          bFoundClass1 = true;
        if (pRtti->GetTypeName() == "nsTestClass2")
          bFoundClass2 = true;

        NS_TEST_STRING(pRtti->GetPluginName(), "Static"); });

    NS_TEST_BOOL(bFoundStruct);
    NS_TEST_BOOL(bFoundClass1);
    NS_TEST_BOOL(bFoundClass2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsDerivedFrom")
  {
    nsDynamicArray<const nsRTTI*> allTypes;
    nsRTTI::ForEachType([&](const nsRTTI* pRtti)
      { allTypes.PushBack(pRtti); });

    // ground truth - traversing up the parent list
    auto ManualIsDerivedFrom = [](const nsRTTI* t, const nsRTTI* pBaseType) -> bool
    {
      while (t != nullptr)
      {
        if (t == pBaseType)
          return true;

        t = t->GetParentType();
      }

      return false;
    };

    // test each type against every other:
    for (const nsRTTI* typeA : allTypes)
    {
      for (const nsRTTI* typeB : allTypes)
      {
        bool derived = typeA->IsDerivedFrom(typeB);
        bool manualCheck = ManualIsDerivedFrom(typeA, typeB);
        NS_TEST_BOOL(derived == manualCheck);
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "PropertyFlags")
  {
    NS_TEST_BOOL(nsPropertyFlags::GetParameterFlags<void>() == (nsPropertyFlags::Void));
    NS_TEST_BOOL(nsPropertyFlags::GetParameterFlags<const char*>() == (nsPropertyFlags::StandardType | nsPropertyFlags::Const));
    NS_TEST_BOOL(nsPropertyFlags::GetParameterFlags<int>() == nsPropertyFlags::StandardType);
    NS_TEST_BOOL(nsPropertyFlags::GetParameterFlags<int&>() == (nsPropertyFlags::StandardType | nsPropertyFlags::Reference));
    NS_TEST_BOOL(nsPropertyFlags::GetParameterFlags<int*>() == (nsPropertyFlags::StandardType | nsPropertyFlags::Pointer));

    NS_TEST_BOOL(nsPropertyFlags::GetParameterFlags<const int>() == (nsPropertyFlags::StandardType | nsPropertyFlags::Const));
    NS_TEST_BOOL(
      nsPropertyFlags::GetParameterFlags<const int&>() == (nsPropertyFlags::StandardType | nsPropertyFlags::Reference | nsPropertyFlags::Const));
    NS_TEST_BOOL(
      nsPropertyFlags::GetParameterFlags<const int*>() == (nsPropertyFlags::StandardType | nsPropertyFlags::Pointer | nsPropertyFlags::Const));

    NS_TEST_BOOL(nsPropertyFlags::GetParameterFlags<nsVariant>() == (nsPropertyFlags::StandardType));

    NS_TEST_BOOL(nsPropertyFlags::GetParameterFlags<nsExampleEnum::Enum>() == nsPropertyFlags::IsEnum);
    NS_TEST_BOOL(nsPropertyFlags::GetParameterFlags<nsEnum<nsExampleEnum>>() == nsPropertyFlags::IsEnum);
    NS_TEST_BOOL(nsPropertyFlags::GetParameterFlags<nsBitflags<nsExampleBitflags>>() == nsPropertyFlags::Bitflags);

    NS_TEST_BOOL(nsPropertyFlags::GetParameterFlags<nsTestStruct3>() == nsPropertyFlags::Class);
    NS_TEST_BOOL(nsPropertyFlags::GetParameterFlags<nsTestClass2>() == nsPropertyFlags::Class);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TypeFlags")
  {
    NS_TEST_INT(nsGetStaticRTTI<bool>()->GetTypeFlags().GetValue(), nsTypeFlags::StandardType);
    NS_TEST_INT(nsGetStaticRTTI<nsUuid>()->GetTypeFlags().GetValue(), nsTypeFlags::StandardType);
    NS_TEST_INT(nsGetStaticRTTI<const char*>()->GetTypeFlags().GetValue(), nsTypeFlags::StandardType);
    NS_TEST_INT(nsGetStaticRTTI<nsString>()->GetTypeFlags().GetValue(), nsTypeFlags::StandardType);
    NS_TEST_INT(nsGetStaticRTTI<nsMat4>()->GetTypeFlags().GetValue(), nsTypeFlags::StandardType);
    NS_TEST_INT(nsGetStaticRTTI<nsVariant>()->GetTypeFlags().GetValue(), nsTypeFlags::StandardType);

    NS_TEST_INT(nsGetStaticRTTI<nsAbstractTestClass>()->GetTypeFlags().GetValue(), (nsTypeFlags::Class | nsTypeFlags::Abstract).GetValue());
    NS_TEST_INT(nsGetStaticRTTI<nsAbstractTestStruct>()->GetTypeFlags().GetValue(), (nsTypeFlags::Class | nsTypeFlags::Abstract).GetValue());

    NS_TEST_INT(nsGetStaticRTTI<nsTestStruct3>()->GetTypeFlags().GetValue(), nsTypeFlags::Class);
    NS_TEST_INT(nsGetStaticRTTI<nsTestClass2>()->GetTypeFlags().GetValue(), nsTypeFlags::Class);

    NS_TEST_INT(nsGetStaticRTTI<nsExampleEnum>()->GetTypeFlags().GetValue(), nsTypeFlags::IsEnum);
    NS_TEST_INT(nsGetStaticRTTI<nsExampleBitflags>()->GetTypeFlags().GetValue(), nsTypeFlags::Bitflags);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FindTypeByName")
  {
    const nsRTTI* pFloat = nsRTTI::FindTypeByName("float");
    NS_TEST_BOOL(pFloat != nullptr);
    NS_TEST_STRING(pFloat->GetTypeName(), "float");

    const nsRTTI* pStruct = nsRTTI::FindTypeByName("nsTestStruct");
    NS_TEST_BOOL(pStruct != nullptr);
    NS_TEST_STRING(pStruct->GetTypeName(), "nsTestStruct");

    const nsRTTI* pClass2 = nsRTTI::FindTypeByName("nsTestClass2");
    NS_TEST_BOOL(pClass2 != nullptr);
    NS_TEST_STRING(pClass2->GetTypeName(), "nsTestClass2");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FindTypeByNameHash")
  {
    const nsRTTI* pFloat = nsRTTI::FindTypeByName("float");
    const nsRTTI* pFloat2 = nsRTTI::FindTypeByNameHash(pFloat->GetTypeNameHash());
    NS_TEST_BOOL(pFloat == pFloat2);

    const nsRTTI* pStruct = nsRTTI::FindTypeByName("nsTestStruct");
    const nsRTTI* pStruct2 = nsRTTI::FindTypeByNameHash(pStruct->GetTypeNameHash());
    NS_TEST_BOOL(pStruct == pStruct2);

    const nsRTTI* pClass = nsRTTI::FindTypeByName("nsTestClass2");
    const nsRTTI* pClass2 = nsRTTI::FindTypeByNameHash(pClass->GetTypeNameHash());
    NS_TEST_BOOL(pClass == pClass2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetProperties")
  {
    {
      const nsRTTI* pType = nsRTTI::FindTypeByName("nsTestStruct");

      auto Props = pType->GetProperties();
      NS_TEST_INT(Props.GetCount(), 9);
      NS_TEST_STRING(Props[0]->GetPropertyName(), "Float");
      NS_TEST_STRING(Props[1]->GetPropertyName(), "Vector");
      NS_TEST_STRING(Props[2]->GetPropertyName(), "Int");
      NS_TEST_STRING(Props[3]->GetPropertyName(), "UInt8");
      NS_TEST_STRING(Props[4]->GetPropertyName(), "Variant");
      NS_TEST_STRING(Props[5]->GetPropertyName(), "Angle");
      NS_TEST_STRING(Props[6]->GetPropertyName(), "DataBuffer");
      NS_TEST_STRING(Props[7]->GetPropertyName(), "vVec3I");
      NS_TEST_STRING(Props[8]->GetPropertyName(), "VarianceAngle");
    }

    {
      const nsRTTI* pType = nsRTTI::FindTypeByName("nsTestClass2");

      auto Props = pType->GetProperties();
      NS_TEST_INT(Props.GetCount(), 8);
      NS_TEST_STRING(Props[0]->GetPropertyName(), "CharPtr");
      NS_TEST_STRING(Props[1]->GetPropertyName(), "String");
      NS_TEST_STRING(Props[2]->GetPropertyName(), "StringView");
      NS_TEST_STRING(Props[3]->GetPropertyName(), "Time");
      NS_TEST_STRING(Props[4]->GetPropertyName(), "Enum");
      NS_TEST_STRING(Props[5]->GetPropertyName(), "Bitflags");
      NS_TEST_STRING(Props[6]->GetPropertyName(), "Array");
      NS_TEST_STRING(Props[7]->GetPropertyName(), "Variant");

      nsHybridArray<const nsAbstractProperty*, 32> AllProps;
      pType->GetAllProperties(AllProps);

      NS_TEST_INT(AllProps.GetCount(), 11);
      NS_TEST_STRING(AllProps[0]->GetPropertyName(), "SubStruct");
      NS_TEST_STRING(AllProps[1]->GetPropertyName(), "Color");
      NS_TEST_STRING(AllProps[2]->GetPropertyName(), "SubVector");
      NS_TEST_STRING(AllProps[3]->GetPropertyName(), "CharPtr");
      NS_TEST_STRING(AllProps[4]->GetPropertyName(), "String");
      NS_TEST_STRING(AllProps[5]->GetPropertyName(), "StringView");
      NS_TEST_STRING(AllProps[6]->GetPropertyName(), "Time");
      NS_TEST_STRING(AllProps[7]->GetPropertyName(), "Enum");
      NS_TEST_STRING(AllProps[8]->GetPropertyName(), "Bitflags");
      NS_TEST_STRING(AllProps[9]->GetPropertyName(), "Array");
      NS_TEST_STRING(AllProps[10]->GetPropertyName(), "Variant");
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Casts")
  {
    nsTestClass2 test;
    nsTestClass1* pTestClass1 = &test;
    const nsTestClass1* pConstTestClass1 = &test;

    nsTestClass2* pTestClass2 = nsStaticCast<nsTestClass2*>(pTestClass1);
    const nsTestClass2* pConstTestClass2 = nsStaticCast<const nsTestClass2*>(pConstTestClass1);

    pTestClass2 = nsDynamicCast<nsTestClass2*>(pTestClass1);
    pConstTestClass2 = nsDynamicCast<const nsTestClass2*>(pConstTestClass1);
    NS_TEST_BOOL(pTestClass2 != nullptr);
    NS_TEST_BOOL(pConstTestClass2 != nullptr);

    nsTestClass1 otherTest;
    pTestClass1 = &otherTest;
    pConstTestClass1 = &otherTest;

    pTestClass2 = nsDynamicCast<nsTestClass2*>(pTestClass1);
    pConstTestClass2 = nsDynamicCast<const nsTestClass2*>(pConstTestClass1);
    NS_TEST_BOOL(pTestClass2 == nullptr);
    NS_TEST_BOOL(pConstTestClass2 == nullptr);
  }

#if NS_ENABLED(NS_SUPPORTS_DYNAMIC_PLUGINS) && NS_ENABLED(NS_COMPILE_ENGINE_AS_DLL)

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Types From Plugin")
  {
    nsResult loadPlugin = nsPlugin::LoadPlugin(nsFoundationTest_Plugin1);
    NS_TEST_BOOL(loadPlugin == NS_SUCCESS);

    if (loadPlugin.Failed())
      return;

    const nsRTTI* pStruct2 = nsRTTI::FindTypeByName("nsTestStruct2");
    NS_TEST_BOOL(pStruct2 != nullptr);

    if (pStruct2)
    {
      NS_TEST_STRING(pStruct2->GetTypeName(), "nsTestStruct2");
    }

    bool bFoundStruct2 = false;

    nsRTTI::ForEachType(
      [&](const nsRTTI* pRtti)
      {
        if (pRtti->GetTypeName() == "nsTestStruct2")
        {
          bFoundStruct2 = true;

          NS_TEST_STRING(pRtti->GetPluginName(), nsFoundationTest_Plugin1);

          void* pInstance = pRtti->GetAllocator()->Allocate<void>();
          NS_TEST_BOOL(pInstance != nullptr);

          const nsAbstractProperty* pProp = pRtti->FindPropertyByName("Float2");

          NS_TEST_BOOL(pProp != nullptr);

          NS_TEST_BOOL(pProp->GetCategory() == nsPropertyCategory::Member);
          auto pAbsMember = static_cast<const nsAbstractMemberProperty*>(pProp);

          NS_TEST_BOOL(pAbsMember->GetSpecificType() == nsGetStaticRTTI<float>());

          auto pMember = static_cast<const nsTypedMemberProperty<float>*>(pAbsMember);

          NS_TEST_FLOAT(pMember->GetValue(pInstance), 42.0f, 0);
          pMember->SetValue(pInstance, 43.0f);
          NS_TEST_FLOAT(pMember->GetValue(pInstance), 43.0f, 0);

          pRtti->GetAllocator()->Deallocate(pInstance);
        }
        else
        {
          NS_TEST_STRING(pRtti->GetPluginName(), "Static");
        }
      });

    NS_TEST_BOOL(bFoundStruct2);

    nsPlugin::UnloadAllPlugins();
  }
#endif
}


NS_CREATE_SIMPLE_TEST(Reflection, Hierarchies)
{
  nsTestClass2Allocator::m_iAllocs = 0;
  nsTestClass2Allocator::m_iDeallocs = 0;

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsTestStruct")
  {
    const nsRTTI* pRtti = nsGetStaticRTTI<nsTestStruct>();

    NS_TEST_STRING(pRtti->GetTypeName(), "nsTestStruct");
    NS_TEST_INT(pRtti->GetTypeSize(), sizeof(nsTestStruct));
    NS_TEST_BOOL(pRtti->GetVariantType() == nsVariant::Type::Invalid);

    NS_TEST_BOOL(pRtti->GetParentType() == nullptr);

    NS_TEST_BOOL(pRtti->GetAllocator()->CanAllocate());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsTestClass1")
  {
    const nsRTTI* pRtti = nsGetStaticRTTI<nsTestClass1>();

    NS_TEST_STRING(pRtti->GetTypeName(), "nsTestClass1");
    NS_TEST_INT(pRtti->GetTypeSize(), sizeof(nsTestClass1));
    NS_TEST_BOOL(pRtti->GetVariantType() == nsVariant::Type::Invalid);

    NS_TEST_BOOL(pRtti->GetParentType() == nsGetStaticRTTI<nsReflectedClass>());

    NS_TEST_BOOL(pRtti->GetAllocator()->CanAllocate());

    nsTestClass1* pInstance = pRtti->GetAllocator()->Allocate<nsTestClass1>();
    NS_TEST_BOOL(pInstance != nullptr);

    NS_TEST_BOOL(pInstance->GetDynamicRTTI() == nsGetStaticRTTI<nsTestClass1>());
    pInstance->GetDynamicRTTI()->GetAllocator()->Deallocate(pInstance);

    NS_TEST_BOOL(pRtti->IsDerivedFrom<nsReflectedClass>());
    NS_TEST_BOOL(pRtti->IsDerivedFrom(nsGetStaticRTTI<nsReflectedClass>()));

    NS_TEST_BOOL(pRtti->IsDerivedFrom<nsTestClass1>());
    NS_TEST_BOOL(pRtti->IsDerivedFrom(nsGetStaticRTTI<nsTestClass1>()));

    NS_TEST_BOOL(!pRtti->IsDerivedFrom<nsVec3>());
    NS_TEST_BOOL(!pRtti->IsDerivedFrom(nsGetStaticRTTI<nsVec3>()));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsTestClass2")
  {
    const nsRTTI* pRtti = nsGetStaticRTTI<nsTestClass2>();

    NS_TEST_STRING(pRtti->GetTypeName(), "nsTestClass2");
    NS_TEST_INT(pRtti->GetTypeSize(), sizeof(nsTestClass2));
    NS_TEST_BOOL(pRtti->GetVariantType() == nsVariant::Type::Invalid);

    NS_TEST_BOOL(pRtti->GetParentType() == nsGetStaticRTTI<nsTestClass1>());

    NS_TEST_BOOL(pRtti->GetAllocator()->CanAllocate());

    NS_TEST_INT(nsTestClass2Allocator::m_iAllocs, 0);
    NS_TEST_INT(nsTestClass2Allocator::m_iDeallocs, 0);

    nsTestClass2* pInstance = pRtti->GetAllocator()->Allocate<nsTestClass2>();
    NS_TEST_BOOL(pInstance != nullptr);

    NS_TEST_BOOL(pInstance->GetDynamicRTTI() == nsGetStaticRTTI<nsTestClass2>());

    NS_TEST_INT(nsTestClass2Allocator::m_iAllocs, 1);
    NS_TEST_INT(nsTestClass2Allocator::m_iDeallocs, 0);

    pInstance->GetDynamicRTTI()->GetAllocator()->Deallocate(pInstance);

    NS_TEST_INT(nsTestClass2Allocator::m_iAllocs, 1);
    NS_TEST_INT(nsTestClass2Allocator::m_iDeallocs, 1);

    NS_TEST_BOOL(pRtti->IsDerivedFrom<nsTestClass1>());
    NS_TEST_BOOL(pRtti->IsDerivedFrom(nsGetStaticRTTI<nsTestClass1>()));

    NS_TEST_BOOL(pRtti->IsDerivedFrom<nsTestClass2>());
    NS_TEST_BOOL(pRtti->IsDerivedFrom(nsGetStaticRTTI<nsTestClass2>()));

    NS_TEST_BOOL(pRtti->IsDerivedFrom<nsReflectedClass>());
    NS_TEST_BOOL(pRtti->IsDerivedFrom(nsGetStaticRTTI<nsReflectedClass>()));

    NS_TEST_BOOL(!pRtti->IsDerivedFrom<nsVec3>());
    NS_TEST_BOOL(!pRtti->IsDerivedFrom(nsGetStaticRTTI<nsVec3>()));
  }
}


template <typename T, typename T2>
void TestMemberProperty(const char* szPropName, void* pObject, const nsRTTI* pRtti, nsBitflags<nsPropertyFlags> expectedFlags, T2 expectedValue, T2 testValue, bool bTestDefaultValue = true)
{
  const nsAbstractProperty* pProp = pRtti->FindPropertyByName(szPropName);
  NS_TEST_BOOL(pProp != nullptr);

  NS_TEST_BOOL(pProp->GetCategory() == nsPropertyCategory::Member);

  NS_TEST_BOOL(pProp->GetSpecificType() == nsGetStaticRTTI<T>());
  auto pMember = static_cast<const nsTypedMemberProperty<T>*>(pProp);

  NS_TEST_INT(pMember->GetFlags().GetValue(), expectedFlags.GetValue());

  T value = pMember->GetValue(pObject);
  NS_TEST_BOOL(expectedValue == value);

  if (bTestDefaultValue)
  {
    // Default value
    nsVariant defaultValue = nsReflectionUtils::GetDefaultValue(pProp);
    NS_TEST_BOOL(nsVariant(expectedValue) == defaultValue);
  }

  if (!pMember->GetFlags().IsSet(nsPropertyFlags::ReadOnly))
  {
    pMember->SetValue(pObject, testValue);

    NS_TEST_BOOL(testValue == pMember->GetValue(pObject));

    nsReflectionUtils::SetMemberPropertyValue(pMember, pObject, nsVariant(expectedValue));
    nsVariant res = nsReflectionUtils::GetMemberPropertyValue(pMember, pObject);

    NS_TEST_BOOL(res == nsVariant(expectedValue));
    NS_TEST_BOOL(res != nsVariant(testValue));

    nsReflectionUtils::SetMemberPropertyValue(pMember, pObject, nsVariant(testValue));
    res = nsReflectionUtils::GetMemberPropertyValue(pMember, pObject);

    NS_TEST_BOOL(res != nsVariant(expectedValue));
    NS_TEST_BOOL(res == nsVariant(testValue));
  }
}

NS_CREATE_SIMPLE_TEST(Reflection, MemberProperties)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsTestStruct")
  {
    nsTestStruct data;
    const nsRTTI* pRtti = nsGetStaticRTTI<nsTestStruct>();

    TestMemberProperty<float>("Float", &data, pRtti, nsPropertyFlags::StandardType, 1.1f, 5.0f);
    TestMemberProperty<nsInt32>("Int", &data, pRtti, nsPropertyFlags::StandardType, 2, -8);
    TestMemberProperty<nsVec3>("Vector", &data, pRtti, nsPropertyFlags::StandardType | nsPropertyFlags::ReadOnly, nsVec3(3, 4, 5),
      nsVec3(0, -1.0f, 3.14f));
    TestMemberProperty<nsVariant>("Variant", &data, pRtti, nsPropertyFlags::StandardType, nsVariant("Test"),
      nsVariant(nsVec3(0, -1.0f, 3.14f)));
    TestMemberProperty<nsAngle>("Angle", &data, pRtti, nsPropertyFlags::StandardType, nsAngle::MakeFromDegree(0.5f), nsAngle::MakeFromDegree(1.0f));
    nsVarianceTypeAngle expectedVA = {0.5f, nsAngle::MakeFromDegree(90.0f)};
    nsVarianceTypeAngle testVA = {0.1f, nsAngle::MakeFromDegree(45.0f)};
    TestMemberProperty<nsVarianceTypeAngle>("VarianceAngle", &data, pRtti, nsPropertyFlags::Class, expectedVA, testVA);

    nsDataBuffer expected;
    expected.PushBack(255);
    expected.PushBack(0);
    expected.PushBack(127);

    nsDataBuffer newValue;
    newValue.PushBack(1);
    newValue.PushBack(2);

    TestMemberProperty<nsDataBuffer>("DataBuffer", &data, pRtti, nsPropertyFlags::StandardType, expected, newValue);
    TestMemberProperty<nsVec3I32>("vVec3I", &data, pRtti, nsPropertyFlags::StandardType, nsVec3I32(1, 2, 3), nsVec3I32(5, 6, 7));

    TestSerialization<nsTestStruct>(data);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsTestClass2")
  {
    nsTestClass2 Instance;
    const nsRTTI* pRtti = nsGetStaticRTTI<nsTestClass2>();

    {
      TestMemberProperty<const char*>("CharPtr", &Instance, pRtti, nsPropertyFlags::StandardType | nsPropertyFlags::Const, nsString("AAA"), nsString("aaaa"));

      TestMemberProperty<nsString>("String", &Instance, pRtti, nsPropertyFlags::StandardType, nsString("BBB"), nsString("bbbb"));

      TestMemberProperty<nsStringView>("StringView", &Instance, pRtti, nsPropertyFlags::StandardType, "CCC"_nssv, "cccc"_nssv);

      Instance.SetStringView("CCC");
      TestMemberProperty<nsStringView>("StringView", &Instance, pRtti, nsPropertyFlags::StandardType, nsString("CCC"), nsString("cccc"));

      const nsAbstractProperty* pProp = pRtti->FindPropertyByName("SubVector", false);
      NS_TEST_BOOL(pProp == nullptr);
    }

    {
      TestMemberProperty<nsVec3>("SubVector", &Instance, pRtti, nsPropertyFlags::StandardType | nsPropertyFlags::ReadOnly, nsVec3(3, 4, 5), nsVec3(3, 4, 5));
      const nsAbstractProperty* pProp = pRtti->FindPropertyByName("SubStruct", false);
      NS_TEST_BOOL(pProp == nullptr);
    }

    {
      const nsAbstractProperty* pProp = pRtti->FindPropertyByName("SubStruct");
      NS_TEST_BOOL(pProp != nullptr);

      NS_TEST_BOOL(pProp->GetCategory() == nsPropertyCategory::Member);
      nsAbstractMemberProperty* pAbs = (nsAbstractMemberProperty*)pProp;

      const nsRTTI* pStruct = pAbs->GetSpecificType();
      void* pSubStruct = pAbs->GetPropertyPointer(&Instance);

      NS_TEST_BOOL(pSubStruct != nullptr);

      TestMemberProperty<float>("Float", pSubStruct, pStruct, nsPropertyFlags::StandardType, 33.3f, 44.4f, false);
    }

    TestSerialization<nsTestClass2>(Instance);
  }
}


NS_CREATE_SIMPLE_TEST(Reflection, Enum)
{
  const nsRTTI* pEnumRTTI = nsGetStaticRTTI<nsExampleEnum>();
  const nsRTTI* pRTTI = nsGetStaticRTTI<nsTestEnumStruct>();

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Enum Constants")
  {
    NS_TEST_BOOL(pEnumRTTI->IsDerivedFrom<nsEnumBase>());
    auto props = pEnumRTTI->GetProperties();
    NS_TEST_INT(props.GetCount(), 4); // Default + 3

    for (auto pProp : props)
    {
      NS_TEST_BOOL(pProp->GetCategory() == nsPropertyCategory::Constant);
      NS_TEST_BOOL(pProp->GetSpecificType() == nsGetStaticRTTI<nsInt8>());
    }
    NS_TEST_INT(nsExampleEnum::Default, nsReflectionUtils::DefaultEnumerationValue(pEnumRTTI));

    NS_TEST_STRING(props[0]->GetPropertyName(), "nsExampleEnum::Default");
    NS_TEST_STRING(props[1]->GetPropertyName(), "nsExampleEnum::Value1");
    NS_TEST_STRING(props[2]->GetPropertyName(), "nsExampleEnum::Value2");
    NS_TEST_STRING(props[3]->GetPropertyName(), "nsExampleEnum::Value3");

    auto pTypedConstantProp0 = static_cast<const nsTypedConstantProperty<nsInt8>*>(props[0]);
    auto pTypedConstantProp1 = static_cast<const nsTypedConstantProperty<nsInt8>*>(props[1]);
    auto pTypedConstantProp2 = static_cast<const nsTypedConstantProperty<nsInt8>*>(props[2]);
    auto pTypedConstantProp3 = static_cast<const nsTypedConstantProperty<nsInt8>*>(props[3]);
    NS_TEST_INT(pTypedConstantProp0->GetValue(), nsExampleEnum::Default);
    NS_TEST_INT(pTypedConstantProp1->GetValue(), nsExampleEnum::Value1);
    NS_TEST_INT(pTypedConstantProp2->GetValue(), nsExampleEnum::Value2);
    NS_TEST_INT(pTypedConstantProp3->GetValue(), nsExampleEnum::Value3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Enum Property")
  {
    nsTestEnumStruct data;
    auto props = pRTTI->GetProperties();
    NS_TEST_INT(props.GetCount(), 4);

    for (auto pProp : props)
    {
      NS_TEST_BOOL(pProp->GetCategory() == nsPropertyCategory::Member);
      NS_TEST_INT(pProp->GetFlags().GetValue(), nsPropertyFlags::IsEnum);
      NS_TEST_BOOL(pProp->GetSpecificType() == pEnumRTTI);
      auto pEnumProp = static_cast<const nsAbstractEnumerationProperty*>(pProp);
      NS_TEST_BOOL(pEnumProp->GetValue(&data) == nsExampleEnum::Value1);

      const nsRTTI* pEnumPropertyRTTI = pEnumProp->GetSpecificType();
      // Set and get all valid enum values.
      for (auto pProp2 : pEnumPropertyRTTI->GetProperties().GetSubArray(1))
      {
        auto pConstantProp = static_cast<const nsTypedConstantProperty<nsInt8>*>(pProp2);
        pEnumProp->SetValue(&data, pConstantProp->GetValue());
        NS_TEST_INT(pEnumProp->GetValue(&data), pConstantProp->GetValue());

        // Enum <-> string
        nsStringBuilder sValue;
        NS_TEST_BOOL(nsReflectionUtils::EnumerationToString(pEnumPropertyRTTI, pConstantProp->GetValue(), sValue));
        NS_TEST_STRING(sValue, pConstantProp->GetPropertyName());

        // Setting the value via a string also works.
        pEnumProp->SetValue(&data, nsExampleEnum::Value1);
        nsReflectionUtils::SetMemberPropertyValue(pEnumProp, &data, sValue.GetData());
        NS_TEST_INT(pEnumProp->GetValue(&data), pConstantProp->GetValue());

        nsInt64 iValue = 0;
        NS_TEST_BOOL(nsReflectionUtils::StringToEnumeration(pEnumPropertyRTTI, sValue, iValue));
        NS_TEST_INT(iValue, pConstantProp->GetValue());

        // Testing the short enum name version
        NS_TEST_BOOL(nsReflectionUtils::EnumerationToString(
          pEnumPropertyRTTI, pConstantProp->GetValue(), sValue, nsReflectionUtils::EnumConversionMode::ValueNameOnly));
        NS_TEST_BOOL(sValue.IsEqual(pConstantProp->GetPropertyName()) ||
                     sValue.IsEqual(nsStringUtils::FindLastSubString(pConstantProp->GetPropertyName(), "::") + 2));

        NS_TEST_BOOL(nsReflectionUtils::StringToEnumeration(pEnumPropertyRTTI, sValue, iValue));
        NS_TEST_INT(iValue, pConstantProp->GetValue());

        // Testing the short enum name version
        NS_TEST_BOOL(nsReflectionUtils::EnumerationToString(
          pEnumPropertyRTTI, pConstantProp->GetValue(), sValue, nsReflectionUtils::EnumConversionMode::ValueNameOnly));
        NS_TEST_BOOL(sValue.IsEqual(pConstantProp->GetPropertyName()) ||
                     sValue.IsEqual(nsStringUtils::FindLastSubString(pConstantProp->GetPropertyName(), "::") + 2));

        NS_TEST_BOOL(nsReflectionUtils::StringToEnumeration(pEnumPropertyRTTI, sValue, iValue));
        NS_TEST_INT(iValue, pConstantProp->GetValue());

        NS_TEST_INT(iValue, nsReflectionUtils::MakeEnumerationValid(pEnumPropertyRTTI, iValue));
        NS_TEST_INT(nsExampleEnum::Default, nsReflectionUtils::MakeEnumerationValid(pEnumPropertyRTTI, iValue + 666));
      }
    }

    NS_TEST_BOOL(data.m_enum == nsExampleEnum::Value3);
    NS_TEST_BOOL(data.m_enumClass == nsExampleEnum::Value3);

    NS_TEST_BOOL(data.GetEnum() == nsExampleEnum::Value3);
    NS_TEST_BOOL(data.GetEnumClass() == nsExampleEnum::Value3);

    TestSerialization<nsTestEnumStruct>(data);
  }
}


NS_CREATE_SIMPLE_TEST(Reflection, Bitflags)
{
  const nsRTTI* pBitflagsRTTI = nsGetStaticRTTI<nsExampleBitflags>();
  const nsRTTI* pRTTI = nsGetStaticRTTI<nsTestBitflagsStruct>();

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Bitflags Constants")
  {
    NS_TEST_BOOL(pBitflagsRTTI->IsDerivedFrom<nsBitflagsBase>());
    auto props = pBitflagsRTTI->GetProperties();
    NS_TEST_INT(props.GetCount(), 4); // Default + 3

    for (auto pProp : props)
    {
      NS_TEST_BOOL(pProp->GetCategory() == nsPropertyCategory::Constant);
      NS_TEST_BOOL(pProp->GetSpecificType() == nsGetStaticRTTI<nsUInt64>());
    }
    NS_TEST_INT(nsExampleBitflags::Default, nsReflectionUtils::DefaultEnumerationValue(pBitflagsRTTI));

    NS_TEST_STRING(props[0]->GetPropertyName(), "nsExampleBitflags::Default");
    NS_TEST_STRING(props[1]->GetPropertyName(), "nsExampleBitflags::Value1");
    NS_TEST_STRING(props[2]->GetPropertyName(), "nsExampleBitflags::Value2");
    NS_TEST_STRING(props[3]->GetPropertyName(), "nsExampleBitflags::Value3");

    auto pTypedConstantProp0 = static_cast<const nsTypedConstantProperty<nsUInt64>*>(props[0]);
    auto pTypedConstantProp1 = static_cast<const nsTypedConstantProperty<nsUInt64>*>(props[1]);
    auto pTypedConstantProp2 = static_cast<const nsTypedConstantProperty<nsUInt64>*>(props[2]);
    auto pTypedConstantProp3 = static_cast<const nsTypedConstantProperty<nsUInt64>*>(props[3]);
    NS_TEST_BOOL(pTypedConstantProp0->GetValue() == nsExampleBitflags::Default);
    NS_TEST_BOOL(pTypedConstantProp1->GetValue() == nsExampleBitflags::Value1);
    NS_TEST_BOOL(pTypedConstantProp2->GetValue() == nsExampleBitflags::Value2);
    NS_TEST_BOOL(pTypedConstantProp3->GetValue() == nsExampleBitflags::Value3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Bitflags Property")
  {
    nsTestBitflagsStruct data;
    auto props = pRTTI->GetProperties();
    NS_TEST_INT(props.GetCount(), 2);

    for (auto pProp : props)
    {
      NS_TEST_BOOL(pProp->GetCategory() == nsPropertyCategory::Member);
      NS_TEST_BOOL(pProp->GetSpecificType() == pBitflagsRTTI);
      NS_TEST_INT(pProp->GetFlags().GetValue(), nsPropertyFlags::Bitflags);
      auto pBitflagsProp = static_cast<const nsAbstractEnumerationProperty*>(pProp);
      NS_TEST_BOOL(pBitflagsProp->GetValue(&data) == nsExampleBitflags::Value1);

      const nsRTTI* pBitflagsPropertyRTTI = pBitflagsProp->GetSpecificType();

      // Set and get all valid bitflags values. (skip default value)
      nsUInt64 constants[] = {
        static_cast<const nsTypedConstantProperty<nsUInt64>*>(pBitflagsPropertyRTTI->GetProperties()[1])->GetValue(),
        static_cast<const nsTypedConstantProperty<nsUInt64>*>(pBitflagsPropertyRTTI->GetProperties()[2])->GetValue(),
        static_cast<const nsTypedConstantProperty<nsUInt64>*>(pBitflagsPropertyRTTI->GetProperties()[3])->GetValue(),
      };

      const char* stringValues[] = {"",
        "nsExampleBitflags::Value1",
        "nsExampleBitflags::Value2",
        "nsExampleBitflags::Value1|nsExampleBitflags::Value2",
        "nsExampleBitflags::Value3",
        "nsExampleBitflags::Value1|nsExampleBitflags::Value3",
        "nsExampleBitflags::Value2|nsExampleBitflags::Value3",
        "nsExampleBitflags::Value1|nsExampleBitflags::Value2|nsExampleBitflags::Value3"};

      const char* stringValuesShort[] = {"",
        "Value1",
        "Value2",
        "Value1|Value2",
        "Value3",
        "Value1|Value3",
        "Value2|Value3",
        "Value1|Value2|Value3"};
      for (nsInt32 i = 0; i < 8; ++i)
      {
        nsUInt64 uiBitflagValue = 0;
        uiBitflagValue |= (i & NS_BIT(0)) != 0 ? constants[0] : 0;
        uiBitflagValue |= (i & NS_BIT(1)) != 0 ? constants[1] : 0;
        uiBitflagValue |= (i & NS_BIT(2)) != 0 ? constants[2] : 0;

        pBitflagsProp->SetValue(&data, uiBitflagValue);
        NS_TEST_INT(pBitflagsProp->GetValue(&data), uiBitflagValue);

        // Bitflags <-> string
        nsStringBuilder sValue;
        NS_TEST_BOOL(nsReflectionUtils::EnumerationToString(pBitflagsPropertyRTTI, uiBitflagValue, sValue));
        NS_TEST_STRING(sValue, stringValues[i]);

        // Setting the value via a string also works.
        pBitflagsProp->SetValue(&data, 0);
        nsReflectionUtils::SetMemberPropertyValue(pBitflagsProp, &data, sValue.GetData());
        NS_TEST_INT(pBitflagsProp->GetValue(&data), uiBitflagValue);

        nsInt64 iValue = 0;
        NS_TEST_BOOL(nsReflectionUtils::StringToEnumeration(pBitflagsPropertyRTTI, sValue, iValue));
        NS_TEST_INT(iValue, uiBitflagValue);

        // Testing the short enum name version
        NS_TEST_BOOL(nsReflectionUtils::EnumerationToString(
          pBitflagsPropertyRTTI, uiBitflagValue, sValue, nsReflectionUtils::EnumConversionMode::ValueNameOnly));
        NS_TEST_BOOL(sValue.IsEqual(stringValuesShort[i]));

        NS_TEST_BOOL(nsReflectionUtils::StringToEnumeration(pBitflagsPropertyRTTI, sValue, iValue));
        NS_TEST_INT(iValue, uiBitflagValue);

        // Testing the short enum name version
        NS_TEST_BOOL(nsReflectionUtils::EnumerationToString(
          pBitflagsPropertyRTTI, uiBitflagValue, sValue, nsReflectionUtils::EnumConversionMode::ValueNameOnly));
        NS_TEST_BOOL(sValue.IsEqual(stringValuesShort[i]));

        NS_TEST_BOOL(nsReflectionUtils::StringToEnumeration(pBitflagsPropertyRTTI, sValue, iValue));
        NS_TEST_INT(iValue, uiBitflagValue);

        NS_TEST_INT(iValue, nsReflectionUtils::MakeEnumerationValid(pBitflagsPropertyRTTI, iValue));
        NS_TEST_INT(iValue, nsReflectionUtils::MakeEnumerationValid(pBitflagsPropertyRTTI, iValue | NS_BIT(16)));
      }
    }

    NS_TEST_BOOL(data.m_bitflagsClass == (nsExampleBitflags::Value1 | nsExampleBitflags::Value2 | nsExampleBitflags::Value3));
    NS_TEST_BOOL(data.GetBitflagsClass() == (nsExampleBitflags::Value1 | nsExampleBitflags::Value2 | nsExampleBitflags::Value3));
    TestSerialization<nsTestBitflagsStruct>(data);
  }
}


template <typename T>
void TestArrayPropertyVariant(const nsAbstractArrayProperty* pArrayProp, void* pObject, const nsRTTI* pRtti, T& value)
{
  T temp = {};

  // Reflection Utils
  nsVariant value0 = nsReflectionUtils::GetArrayPropertyValue(pArrayProp, pObject, 0);
  NS_TEST_BOOL(value0 == nsVariant(value));
  // insert
  nsReflectionUtils::InsertArrayPropertyValue(pArrayProp, pObject, nsVariant(temp), 2);
  NS_TEST_INT(pArrayProp->GetCount(pObject), 3);
  nsVariant value2 = nsReflectionUtils::GetArrayPropertyValue(pArrayProp, pObject, 2);
  NS_TEST_BOOL(value0 != value2);
  nsReflectionUtils::SetArrayPropertyValue(pArrayProp, pObject, 2, value);
  value2 = nsReflectionUtils::GetArrayPropertyValue(pArrayProp, pObject, 2);
  NS_TEST_BOOL(value0 == value2);
  // remove again
  nsReflectionUtils::RemoveArrayPropertyValue(pArrayProp, pObject, 2);
  NS_TEST_INT(pArrayProp->GetCount(pObject), 2);
}

template <>
void TestArrayPropertyVariant<nsTestArrays>(const nsAbstractArrayProperty* pArrayProp, void* pObject, const nsRTTI* pRtti, nsTestArrays& value)
{
}

template <>
void TestArrayPropertyVariant<nsTestStruct3>(const nsAbstractArrayProperty* pArrayProp, void* pObject, const nsRTTI* pRtti, nsTestStruct3& value)
{
}

template <typename T>
void TestArrayProperty(const char* szPropName, void* pObject, const nsRTTI* pRtti, T& value)
{
  const nsAbstractProperty* pProp = pRtti->FindPropertyByName(szPropName);
  NS_TEST_BOOL(pProp != nullptr);
  NS_TEST_BOOL(pProp->GetCategory() == nsPropertyCategory::Array);
  auto pArrayProp = static_cast<const nsAbstractArrayProperty*>(pProp);
  const nsRTTI* pElemRtti = pProp->GetSpecificType();
  NS_TEST_BOOL(pElemRtti == nsGetStaticRTTI<T>());
  if (!pArrayProp->GetFlags().IsSet(nsPropertyFlags::ReadOnly))
  {
    // If we don't know the element type T but we can allocate it, we can handle it anyway.
    if (pElemRtti->GetAllocator()->CanAllocate())
    {
      void* pData = pElemRtti->GetAllocator()->Allocate<void>();

      pArrayProp->SetCount(pObject, 2);
      NS_TEST_INT(pArrayProp->GetCount(pObject), 2);
      // Push default constructed object in both slots.
      pArrayProp->SetValue(pObject, 0, pData);
      pArrayProp->SetValue(pObject, 1, pData);

      // Retrieve it again and compare to function parameter, they should be different.
      pArrayProp->GetValue(pObject, 0, pData);
      NS_TEST_BOOL(*static_cast<T*>(pData) != value);
      pArrayProp->GetValue(pObject, 1, pData);
      NS_TEST_BOOL(*static_cast<T*>(pData) != value);

      pElemRtti->GetAllocator()->Deallocate(pData);
    }

    pArrayProp->Clear(pObject);
    NS_TEST_INT(pArrayProp->GetCount(pObject), 0);
    pArrayProp->SetCount(pObject, 2);
    pArrayProp->SetValue(pObject, 0, &value);
    pArrayProp->SetValue(pObject, 1, &value);

    // Insert default init values
    T temp = {};
    pArrayProp->Insert(pObject, 2, &temp);
    NS_TEST_INT(pArrayProp->GetCount(pObject), 3);
    pArrayProp->Insert(pObject, 0, &temp);
    NS_TEST_INT(pArrayProp->GetCount(pObject), 4);

    // Remove them again
    pArrayProp->Remove(pObject, 3);
    NS_TEST_INT(pArrayProp->GetCount(pObject), 3);
    pArrayProp->Remove(pObject, 0);
    NS_TEST_INT(pArrayProp->GetCount(pObject), 2);

    TestArrayPropertyVariant<T>(pArrayProp, pObject, pRtti, value);
  }

  // Assumes this function gets called first by a writeable property, and then immediately by the same data as a read-only property.
  // So the checks are valid for the read-only version, too.
  NS_TEST_INT(pArrayProp->GetCount(pObject), 2);

  T v1 = {};
  pArrayProp->GetValue(pObject, 0, &v1);
  if constexpr (std::is_same<const char*, T>::value)
  {
    NS_TEST_BOOL(nsStringUtils::IsEqual(v1, value));
  }
  else
  {
    NS_TEST_BOOL(v1 == value);
  }

  T v2 = {};
  pArrayProp->GetValue(pObject, 1, &v2);
  if constexpr (std::is_same<const char*, T>::value)
  {
    NS_TEST_BOOL(nsStringUtils::IsEqual(v2, value));
  }
  else
  {
    NS_TEST_BOOL(v2 == value);
  }

  if (pElemRtti->GetAllocator()->CanAllocate())
  {
    // Current values should be different from default constructed version.
    void* pData = pElemRtti->GetAllocator()->Allocate<void>();

    NS_TEST_BOOL(*static_cast<T*>(pData) != v1);
    NS_TEST_BOOL(*static_cast<T*>(pData) != v2);

    pElemRtti->GetAllocator()->Deallocate(pData);
  }
}

NS_CREATE_SIMPLE_TEST(Reflection, Arrays)
{
  nsTestArrays containers;
  const nsRTTI* pRtti = nsGetStaticRTTI<nsTestArrays>();
  NS_TEST_BOOL(pRtti != nullptr);

  NS_TEST_BLOCK(nsTestBlock::Enabled, "POD Array")
  {
    double fValue = 5;
    TestArrayProperty<double>("Hybrid", &containers, pRtti, fValue);
    TestArrayProperty<double>("HybridRO", &containers, pRtti, fValue);

    TestArrayProperty<double>("AcHybrid", &containers, pRtti, fValue);
    TestArrayProperty<double>("AcHybridRO", &containers, pRtti, fValue);

    const char* szValue = "Bla";
    const char* szValue2 = "LongString------------------------------------------------------------------------------------";
    nsString sValue = szValue;
    nsString sValue2 = szValue2;

    TestArrayProperty<nsString>("HybridChar", &containers, pRtti, sValue);
    TestArrayProperty<nsString>("HybridCharRO", &containers, pRtti, sValue);

    TestArrayProperty<const char*>("AcHybridChar", &containers, pRtti, szValue);
    TestArrayProperty<const char*>("AcHybridCharRO", &containers, pRtti, szValue);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Struct Array")
  {
    nsTestStruct3 data;
    data.m_fFloat1 = 99.0f;
    data.m_UInt8 = 127;

    TestArrayProperty<nsTestStruct3>("Dynamic", &containers, pRtti, data);
    TestArrayProperty<nsTestStruct3>("DynamicRO", &containers, pRtti, data);

    TestArrayProperty<nsTestStruct3>("AcDynamic", &containers, pRtti, data);
    TestArrayProperty<nsTestStruct3>("AcDynamicRO", &containers, pRtti, data);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsReflectedClass Array")
  {
    nsTestArrays data;
    data.m_Hybrid.PushBack(42.0);

    TestArrayProperty<nsTestArrays>("Deque", &containers, pRtti, data);
    TestArrayProperty<nsTestArrays>("DequeRO", &containers, pRtti, data);

    TestArrayProperty<nsTestArrays>("AcDeque", &containers, pRtti, data);
    TestArrayProperty<nsTestArrays>("AcDequeRO", &containers, pRtti, data);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Custom Variant Array")
  {
    nsVarianceTypeAngle data{0.1f, nsAngle::MakeFromDegree(45.0f)};

    TestArrayProperty<nsVarianceTypeAngle>("Custom", &containers, pRtti, data);
    TestArrayProperty<nsVarianceTypeAngle>("CustomRO", &containers, pRtti, data);

    TestArrayProperty<nsVarianceTypeAngle>("AcCustom", &containers, pRtti, data);
    TestArrayProperty<nsVarianceTypeAngle>("AcCustomRO", &containers, pRtti, data);
  }

  TestSerialization<nsTestArrays>(containers);
}

/// \brief Determines whether a type is a pointer.
template <typename T>
struct nsIsPointer
{
  static constexpr bool value = false;
};

template <typename T>
struct nsIsPointer<T*>
{
  static constexpr bool value = true;
};

template <typename T>
void TestSetProperty(const char* szPropName, void* pObject, const nsRTTI* pRtti, T& ref_value1, T& ref_value2)
{
  const nsAbstractProperty* pProp = pRtti->FindPropertyByName(szPropName);
  if (!NS_TEST_BOOL(pProp != nullptr))
    return;

  NS_TEST_BOOL(pProp->GetCategory() == nsPropertyCategory::Set);
  auto pSetProp = static_cast<const nsAbstractSetProperty*>(pProp);
  const nsRTTI* pElemRtti = pProp->GetSpecificType();
  NS_TEST_BOOL(pElemRtti == nsGetStaticRTTI<T>());

  if (!pSetProp->GetFlags().IsSet(nsPropertyFlags::ReadOnly))
  {
    pSetProp->Clear(pObject);
    NS_TEST_BOOL(pSetProp->IsEmpty(pObject));
    pSetProp->Insert(pObject, &ref_value1);
    NS_TEST_BOOL(!pSetProp->IsEmpty(pObject));
    NS_TEST_BOOL(pSetProp->Contains(pObject, &ref_value1));
    NS_TEST_BOOL(!pSetProp->Contains(pObject, &ref_value2));
    pSetProp->Insert(pObject, &ref_value2);
    NS_TEST_BOOL(!pSetProp->IsEmpty(pObject));
    NS_TEST_BOOL(pSetProp->Contains(pObject, &ref_value1));
    NS_TEST_BOOL(pSetProp->Contains(pObject, &ref_value2));

    // Insert default init value
    if (!nsIsPointer<T>::value)
    {
      T temp = T{};
      pSetProp->Insert(pObject, &temp);
      NS_TEST_BOOL(!pSetProp->IsEmpty(pObject));
      NS_TEST_BOOL(pSetProp->Contains(pObject, &ref_value1));
      NS_TEST_BOOL(pSetProp->Contains(pObject, &ref_value2));
      NS_TEST_BOOL(pSetProp->Contains(pObject, &temp));

      // Remove it again
      pSetProp->Remove(pObject, &temp);
      NS_TEST_BOOL(!pSetProp->IsEmpty(pObject));
      NS_TEST_BOOL(!pSetProp->Contains(pObject, &temp));
    }
  }

  // Assumes this function gets called first by a writeable property, and then immediately by the same data as a read-only property.
  // So the checks are valid for the read-only version, too.
  NS_TEST_BOOL(!pSetProp->IsEmpty(pObject));
  NS_TEST_BOOL(pSetProp->Contains(pObject, &ref_value1));
  NS_TEST_BOOL(pSetProp->Contains(pObject, &ref_value2));


  nsHybridArray<nsVariant, 16> keys;
  pSetProp->GetValues(pObject, keys);
  NS_TEST_INT(keys.GetCount(), 2);
}

NS_CREATE_SIMPLE_TEST(Reflection, Sets)
{
  nsTestSets containers;
  const nsRTTI* pRtti = nsGetStaticRTTI<nsTestSets>();
  NS_TEST_BOOL(pRtti != nullptr);

  // Disabled because MSVC 2017 has code generation issues in Release builds
  NS_TEST_BLOCK(nsTestBlock::Disabled, "nsSet")
  {
    nsInt8 iValue1 = -5;
    nsInt8 iValue2 = 127;
    TestSetProperty<nsInt8>("Set", &containers, pRtti, iValue1, iValue2);
    TestSetProperty<nsInt8>("SetRO", &containers, pRtti, iValue1, iValue2);

    double fValue1 = 5;
    double fValue2 = -3;
    TestSetProperty<double>("AcSet", &containers, pRtti, fValue1, fValue2);
    TestSetProperty<double>("AcSetRO", &containers, pRtti, fValue1, fValue2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsHashSet")
  {
    nsInt32 iValue1 = -5;
    nsInt32 iValue2 = 127;
    TestSetProperty<nsInt32>("HashSet", &containers, pRtti, iValue1, iValue2);
    TestSetProperty<nsInt32>("HashSetRO", &containers, pRtti, iValue1, iValue2);

    nsInt64 fValue1 = 5;
    nsInt64 fValue2 = -3;
    TestSetProperty<nsInt64>("HashAcSet", &containers, pRtti, fValue1, fValue2);
    TestSetProperty<nsInt64>("HashAcSetRO", &containers, pRtti, fValue1, fValue2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsDeque Pseudo Set")
  {
    int iValue1 = -5;
    int iValue2 = 127;

    TestSetProperty<int>("AcPseudoSet", &containers, pRtti, iValue1, iValue2);
    TestSetProperty<int>("AcPseudoSetRO", &containers, pRtti, iValue1, iValue2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsSetPtr Pseudo Set")
  {
    nsString sValue1 = "TestString1";
    nsString sValue2 = "Test String Deus";

    TestSetProperty<nsString>("AcPseudoSet2", &containers, pRtti, sValue1, sValue2);
    TestSetProperty<nsString>("AcPseudoSet2RO", &containers, pRtti, sValue1, sValue2);

    const char* szValue1 = "TestString1";
    const char* szValue2 = "Test String Deus";
    TestSetProperty<const char*>("AcPseudoSet2b", &containers, pRtti, szValue1, szValue2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Custom Variant HashSet")
  {
    nsVarianceTypeAngle value1{-0.1f, nsAngle::MakeFromDegree(-45.0f)};
    nsVarianceTypeAngle value2{0.1f, nsAngle::MakeFromDegree(45.0f)};

    TestSetProperty<nsVarianceTypeAngle>("CustomHashSet", &containers, pRtti, value1, value2);
    TestSetProperty<nsVarianceTypeAngle>("CustomHashSetRO", &containers, pRtti, value1, value2);

    nsVarianceTypeAngle value3{-0.2f, nsAngle::MakeFromDegree(-90.0f)};
    nsVarianceTypeAngle value4{0.2f, nsAngle::MakeFromDegree(90.0f)};
    TestSetProperty<nsVarianceTypeAngle>("CustomHashAcSet", &containers, pRtti, value3, value4);
    TestSetProperty<nsVarianceTypeAngle>("CustomHashAcSetRO", &containers, pRtti, value3, value4);
  }
  TestSerialization<nsTestSets>(containers);
}

template <typename T>
void TestMapProperty(const char* szPropName, void* pObject, const nsRTTI* pRtti, T& ref_value1, T& ref_value2)
{
  const nsAbstractProperty* pProp = pRtti->FindPropertyByName(szPropName);
  NS_TEST_BOOL(pProp != nullptr);
  NS_TEST_BOOL(pProp->GetCategory() == nsPropertyCategory::Map);
  auto pMapProp = static_cast<const nsAbstractMapProperty*>(pProp);
  const nsRTTI* pElemRtti = pProp->GetSpecificType();
  NS_TEST_BOOL(pElemRtti == nsGetStaticRTTI<T>());
  NS_TEST_BOOL(nsReflectionUtils::IsBasicType(pElemRtti) || pElemRtti == nsGetStaticRTTI<nsVariant>() || pElemRtti == nsGetStaticRTTI<nsVarianceTypeAngle>());

  if (!pMapProp->GetFlags().IsSet(nsPropertyFlags::ReadOnly))
  {
    pMapProp->Clear(pObject);
    NS_TEST_BOOL(pMapProp->IsEmpty(pObject));
    pMapProp->Insert(pObject, "value1", &ref_value1);
    NS_TEST_BOOL(!pMapProp->IsEmpty(pObject));
    NS_TEST_BOOL(pMapProp->Contains(pObject, "value1"));
    NS_TEST_BOOL(!pMapProp->Contains(pObject, "value2"));
    T getValue;
    NS_TEST_BOOL(!pMapProp->GetValue(pObject, "value2", &getValue));
    NS_TEST_BOOL(pMapProp->GetValue(pObject, "value1", &getValue));
    NS_TEST_BOOL(getValue == ref_value1);

    pMapProp->Insert(pObject, "value2", &ref_value2);
    NS_TEST_BOOL(!pMapProp->IsEmpty(pObject));
    NS_TEST_BOOL(pMapProp->Contains(pObject, "value1"));
    NS_TEST_BOOL(pMapProp->Contains(pObject, "value2"));
    NS_TEST_BOOL(pMapProp->GetValue(pObject, "value1", &getValue));
    NS_TEST_BOOL(getValue == ref_value1);
    NS_TEST_BOOL(pMapProp->GetValue(pObject, "value2", &getValue));
    NS_TEST_BOOL(getValue == ref_value2);
  }

  // Assumes this function gets called first by a writeable property, and then immediately by the same data as a read-only property.
  // So the checks are valid for the read-only version, too.
  T getValue2;
  NS_TEST_BOOL(!pMapProp->IsEmpty(pObject));
  NS_TEST_BOOL(pMapProp->Contains(pObject, "value1"));
  NS_TEST_BOOL(pMapProp->Contains(pObject, "value2"));
  NS_TEST_BOOL(pMapProp->GetValue(pObject, "value1", &getValue2));
  NS_TEST_BOOL(getValue2 == ref_value1);
  NS_TEST_BOOL(pMapProp->GetValue(pObject, "value2", &getValue2));
  NS_TEST_BOOL(getValue2 == ref_value2);

  nsHybridArray<nsString, 16> keys;
  pMapProp->GetKeys(pObject, keys);
  NS_TEST_INT(keys.GetCount(), 2);
  keys.Sort();
  NS_TEST_BOOL(keys[0] == "value1");
  NS_TEST_BOOL(keys[1] == "value2");
}

NS_CREATE_SIMPLE_TEST(Reflection, Maps)
{
  nsTestMaps containers;
  const nsRTTI* pRtti = nsGetStaticRTTI<nsTestMaps>();
  NS_TEST_BOOL(pRtti != nullptr);

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsMap")
  {
    int iValue1 = -5;
    int iValue2 = 127;
    TestMapProperty<int>("Map", &containers, pRtti, iValue1, iValue2);
    TestMapProperty<int>("MapRO", &containers, pRtti, iValue1, iValue2);

    nsInt64 iValue1b = 5;
    nsInt64 iValue2b = -3;
    TestMapProperty<nsInt64>("AcMap", &containers, pRtti, iValue1b, iValue2b);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsHashMap")
  {
    double fValue1 = -5;
    double fValue2 = 127;
    TestMapProperty<double>("HashTable", &containers, pRtti, fValue1, fValue2);
    TestMapProperty<double>("HashTableRO", &containers, pRtti, fValue1, fValue2);

    nsString sValue1 = "Bla";
    nsString sValue2 = "Test";
    TestMapProperty<nsString>("AcHashTable", &containers, pRtti, sValue1, sValue2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Accessor")
  {
    nsVariant sValue1 = "Test";
    nsVariant sValue2 = nsVec4(1, 2, 3, 4);
    TestMapProperty<nsVariant>("Accessor", &containers, pRtti, sValue1, sValue2);
    TestMapProperty<nsVariant>("AccessorRO", &containers, pRtti, sValue1, sValue2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CustomVariant")
  {
    nsVarianceTypeAngle value1{-0.1f, nsAngle::MakeFromDegree(-45.0f)};
    nsVarianceTypeAngle value2{0.1f, nsAngle::MakeFromDegree(45.0f)};

    TestMapProperty<nsVarianceTypeAngle>("CustomVariant", &containers, pRtti, value1, value2);
    TestMapProperty<nsVarianceTypeAngle>("CustomVariantRO", &containers, pRtti, value1, value2);
  }
  TestSerialization<nsTestMaps>(containers);
}


template <typename T>
void TestPointerMemberProperty(const char* szPropName, void* pObject, const nsRTTI* pRtti, nsBitflags<nsPropertyFlags> expectedFlags, T* pExpectedValue)
{
  const nsAbstractProperty* pProp = pRtti->FindPropertyByName(szPropName);
  NS_TEST_BOOL(pProp != nullptr);
  NS_TEST_BOOL(pProp->GetCategory() == nsPropertyCategory::Member);
  auto pAbsMember = static_cast<const nsAbstractMemberProperty*>(pProp);
  NS_TEST_INT(pProp->GetFlags().GetValue(), expectedFlags.GetValue());
  NS_TEST_BOOL(pProp->GetSpecificType() == nsGetStaticRTTI<T>());
  void* pData = nullptr;
  pAbsMember->GetValuePtr(pObject, &pData);
  NS_TEST_BOOL(pData == pExpectedValue);

  // Set value to null.
  {
    void* pDataNull = nullptr;
    pAbsMember->SetValuePtr(pObject, &pDataNull);
    void* pDataNull2 = nullptr;
    pAbsMember->GetValuePtr(pObject, &pDataNull2);
    NS_TEST_BOOL(pDataNull == pDataNull2);
  }

  // Set value to new instance.
  {
    void* pNewData = pAbsMember->GetSpecificType()->GetAllocator()->Allocate<void>();
    pAbsMember->SetValuePtr(pObject, &pNewData);
    void* pData2 = nullptr;
    pAbsMember->GetValuePtr(pObject, &pData2);
    NS_TEST_BOOL(pNewData == pData2);
  }

  // Delete old value
  pAbsMember->GetSpecificType()->GetAllocator()->Deallocate(pData);
}

NS_CREATE_SIMPLE_TEST(Reflection, Pointer)
{
  const nsRTTI* pRtti = nsGetStaticRTTI<nsTestPtr>();
  NS_TEST_BOOL(pRtti != nullptr);

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Member Property Ptr")
  {
    nsTestPtr containers;
    {
      const nsAbstractProperty* pProp = pRtti->FindPropertyByName("ConstCharPtr");
      NS_TEST_BOOL(pProp != nullptr);
      NS_TEST_BOOL(pProp->GetCategory() == nsPropertyCategory::Member);
      NS_TEST_INT(pProp->GetFlags().GetValue(), (nsPropertyFlags::StandardType | nsPropertyFlags::Const).GetValue());
      NS_TEST_BOOL(pProp->GetSpecificType() == nsGetStaticRTTI<const char*>());
    }

    TestPointerMemberProperty<nsTestArrays>(
      "ArraysPtr", &containers, pRtti, nsPropertyFlags::Class | nsPropertyFlags::Pointer | nsPropertyFlags::PointerOwner, containers.m_pArrays);
    TestPointerMemberProperty<nsTestArrays>("ArraysPtrDirect", &containers, pRtti,
      nsPropertyFlags::Class | nsPropertyFlags::Pointer | nsPropertyFlags::PointerOwner, containers.m_pArraysDirect);
  }

  nsTestPtr containers;
  nsDefaultMemoryStreamStorage StreamStorage;

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Serialize Property Ptr")
  {
    containers.m_sString = "Test";

    containers.m_pArrays = NS_DEFAULT_NEW(nsTestArrays);
    containers.m_pArrays->m_Deque.PushBack(nsTestArrays());

    containers.m_ArrayPtr.PushBack(NS_DEFAULT_NEW(nsTestArrays));
    containers.m_ArrayPtr[0]->m_Hybrid.PushBack(5.0);

    containers.m_SetPtr.Insert(NS_DEFAULT_NEW(nsTestSets));
    containers.m_SetPtr.GetIterator().Key()->m_Array.PushBack("BLA");
  }

  TestSerialization<nsTestPtr>(containers);
}
