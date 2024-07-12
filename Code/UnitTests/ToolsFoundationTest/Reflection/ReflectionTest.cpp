#include <ToolsFoundationTest/ToolsFoundationTestPCH.h>

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/VariantTypeRegistry.h>
#include <ToolsFoundation/Reflection/PhantomRttiManager.h>
#include <ToolsFoundation/Reflection/ReflectedTypeStorageAccessor.h>
#include <ToolsFoundation/Reflection/ToolsReflectionUtils.h>
#include <ToolsFoundationTest/Object/TestObjectManager.h>
#include <ToolsFoundationTest/Reflection/ReflectionTestClasses.h>

NS_CREATE_SIMPLE_TEST_GROUP(Reflection);


void VariantToPropertyTest(void* pIntStruct, const nsRTTI* pRttiInt, const char* szPropName, nsVariant::Type::Enum type)
{
  const nsAbstractMemberProperty* pProp = nsReflectionUtils::GetMemberProperty(pRttiInt, szPropName);
  NS_TEST_BOOL(pProp != nullptr);
  if (pProp)
  {
    nsVariant oldValue = nsReflectionUtils::GetMemberPropertyValue(pProp, pIntStruct);
    NS_TEST_BOOL(oldValue.IsValid());
    NS_TEST_BOOL(oldValue.GetType() == type);

    nsVariant defaultValue = nsReflectionUtils::GetDefaultValue(pProp);
    NS_TEST_BOOL(defaultValue.GetType() == type);
    nsReflectionUtils::SetMemberPropertyValue(pProp, pIntStruct, defaultValue);

    nsVariant newValue = nsReflectionUtils::GetMemberPropertyValue(pProp, pIntStruct);
    NS_TEST_BOOL(newValue.IsValid());
    NS_TEST_BOOL(newValue.GetType() == type);
    NS_TEST_BOOL(newValue == defaultValue);
    NS_TEST_BOOL(newValue != oldValue);
  }
}

NS_CREATE_SIMPLE_TEST(Reflection, ReflectionUtils)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Integer Properties")
  {
    nsIntegerStruct intStruct;
    const nsRTTI* pRttiInt = nsRTTI::FindTypeByName("nsIntegerStruct");
    NS_TEST_BOOL(pRttiInt != nullptr);

    VariantToPropertyTest(&intStruct, pRttiInt, "Int8", nsVariant::Type::Int8);
    NS_TEST_INT(0, intStruct.GetInt8());
    VariantToPropertyTest(&intStruct, pRttiInt, "UInt8", nsVariant::Type::UInt8);
    NS_TEST_INT(0, intStruct.GetUInt8());

    VariantToPropertyTest(&intStruct, pRttiInt, "Int16", nsVariant::Type::Int16);
    NS_TEST_INT(0, intStruct.m_iInt16);
    VariantToPropertyTest(&intStruct, pRttiInt, "UInt16", nsVariant::Type::UInt16);
    NS_TEST_INT(0, intStruct.m_iUInt16);

    VariantToPropertyTest(&intStruct, pRttiInt, "Int32", nsVariant::Type::Int32);
    NS_TEST_INT(0, intStruct.GetInt32());
    VariantToPropertyTest(&intStruct, pRttiInt, "UInt32", nsVariant::Type::UInt32);
    NS_TEST_INT(0, intStruct.GetUInt32());

    VariantToPropertyTest(&intStruct, pRttiInt, "Int64", nsVariant::Type::Int64);
    NS_TEST_INT(0, intStruct.m_iInt64);
    VariantToPropertyTest(&intStruct, pRttiInt, "UInt64", nsVariant::Type::UInt64);
    NS_TEST_INT(0, intStruct.m_iUInt64);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Float Properties")
  {
    nsFloatStruct floatStruct;
    const nsRTTI* pRttiFloat = nsRTTI::FindTypeByName("nsFloatStruct");
    NS_TEST_BOOL(pRttiFloat != nullptr);

    VariantToPropertyTest(&floatStruct, pRttiFloat, "Float", nsVariant::Type::Float);
    NS_TEST_FLOAT(0, floatStruct.GetFloat(), 0);
    VariantToPropertyTest(&floatStruct, pRttiFloat, "Double", nsVariant::Type::Double);
    NS_TEST_FLOAT(0, floatStruct.GetDouble(), 0);
    VariantToPropertyTest(&floatStruct, pRttiFloat, "Time", nsVariant::Type::Time);
    NS_TEST_FLOAT(0, floatStruct.GetTime().GetSeconds(), 0);
    VariantToPropertyTest(&floatStruct, pRttiFloat, "Angle", nsVariant::Type::Angle);
    NS_TEST_FLOAT(0, floatStruct.GetAngle().GetDegree(), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Misc Properties")
  {
    nsPODClass podClass;
    const nsRTTI* pRttiPOD = nsRTTI::FindTypeByName("nsPODClass");
    NS_TEST_BOOL(pRttiPOD != nullptr);

    VariantToPropertyTest(&podClass, pRttiPOD, "Bool", nsVariant::Type::Bool);
    NS_TEST_BOOL(podClass.GetBool() == false);
    VariantToPropertyTest(&podClass, pRttiPOD, "Color", nsVariant::Type::Color);
    NS_TEST_BOOL(podClass.GetColor() == nsColor(1.0f, 1.0f, 1.0f, 1.0f));

    VariantToPropertyTest(&podClass, pRttiPOD, "CharPtr", nsVariant::Type::String);
    NS_TEST_STRING(podClass.GetCharPtr(), "");

    VariantToPropertyTest(&podClass, pRttiPOD, "String", nsVariant::Type::String);
    NS_TEST_STRING(podClass.GetString(), "");

    // An nsStringView is special, nsReflectionUtils::GetMemberPropertyValue will return an nsString as that is the default assignment behaviour of nsStringView to nsVariant. However, nsReflectionUtils::GetDefaultValue will still return an nsStringView.
    {
      const nsAbstractMemberProperty* pProp = nsReflectionUtils::GetMemberProperty(pRttiPOD, "StringView");
      NS_TEST_BOOL(pProp != nullptr);
      if (pProp)
      {
        nsVariant oldValue = nsReflectionUtils::GetMemberPropertyValue(pProp, &podClass);
        NS_TEST_BOOL(oldValue.IsValid());
        NS_TEST_BOOL(oldValue.GetType() == nsVariant::Type::String);

        nsVariant defaultValue = nsReflectionUtils::GetDefaultValue(pProp);
        NS_TEST_BOOL(defaultValue.GetType() == nsVariant::Type::StringView);
        nsReflectionUtils::SetMemberPropertyValue(pProp, &podClass, defaultValue);

        nsVariant newValue = nsReflectionUtils::GetMemberPropertyValue(pProp, &podClass);
        NS_TEST_BOOL(newValue.IsValid());
        NS_TEST_BOOL(newValue.GetType() == nsVariant::Type::String);
        NS_TEST_BOOL(newValue == defaultValue);
        NS_TEST_BOOL(newValue != oldValue);
      }
      NS_TEST_STRING(podClass.GetStringView(), "");
    }

    VariantToPropertyTest(&podClass, pRttiPOD, "Buffer", nsVariant::Type::DataBuffer);
    NS_TEST_BOOL(podClass.GetBuffer() == nsDataBuffer());
    VariantToPropertyTest(&podClass, pRttiPOD, "VarianceAngle", nsVariant::Type::TypedObject);
    NS_TEST_BOOL(podClass.GetCustom() == nsVarianceTypeAngle{});
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Math Properties")
  {
    nsMathClass mathClass;
    const nsRTTI* pRttiMath = nsRTTI::FindTypeByName("nsMathClass");
    NS_TEST_BOOL(pRttiMath != nullptr);

    VariantToPropertyTest(&mathClass, pRttiMath, "Vec2", nsVariant::Type::Vector2);
    NS_TEST_BOOL(mathClass.GetVec2() == nsVec2(0.0f, 0.0f));
    VariantToPropertyTest(&mathClass, pRttiMath, "Vec3", nsVariant::Type::Vector3);
    NS_TEST_BOOL(mathClass.GetVec3() == nsVec3(0.0f, 0.0f, 0.0f));
    VariantToPropertyTest(&mathClass, pRttiMath, "Vec4", nsVariant::Type::Vector4);
    NS_TEST_BOOL(mathClass.GetVec4() == nsVec4(0.0f, 0.0f, 0.0f, 0.0f));
    VariantToPropertyTest(&mathClass, pRttiMath, "Vec2I", nsVariant::Type::Vector2I);
    NS_TEST_BOOL(mathClass.m_Vec2I == nsVec2I32(0, 0));
    VariantToPropertyTest(&mathClass, pRttiMath, "Vec3I", nsVariant::Type::Vector3I);
    NS_TEST_BOOL(mathClass.m_Vec3I == nsVec3I32(0, 0, 0));
    VariantToPropertyTest(&mathClass, pRttiMath, "Vec4I", nsVariant::Type::Vector4I);
    NS_TEST_BOOL(mathClass.m_Vec4I == nsVec4I32(0, 0, 0, 0));
    VariantToPropertyTest(&mathClass, pRttiMath, "Quat", nsVariant::Type::Quaternion);
    NS_TEST_BOOL(mathClass.GetQuat() == nsQuat(0.0f, 0.0f, 0.0f, 1.0f));
    VariantToPropertyTest(&mathClass, pRttiMath, "Mat3", nsVariant::Type::Matrix3);
    NS_TEST_BOOL(mathClass.GetMat3() == nsMat3::MakeIdentity());
    VariantToPropertyTest(&mathClass, pRttiMath, "Mat4", nsVariant::Type::Matrix4);
    NS_TEST_BOOL(mathClass.GetMat4() == nsMat4::MakeIdentity());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Enumeration Properties")
  {
    nsEnumerationsClass enumClass;
    const nsRTTI* pRttiEnum = nsRTTI::FindTypeByName("nsEnumerationsClass");
    NS_TEST_BOOL(pRttiEnum != nullptr);

    VariantToPropertyTest(&enumClass, pRttiEnum, "Enum", nsVariant::Type::Int64);
    NS_TEST_BOOL(enumClass.GetEnum() == nsExampleEnum::Value1);
    VariantToPropertyTest(&enumClass, pRttiEnum, "Bitflags", nsVariant::Type::Int64);
    NS_TEST_BOOL(enumClass.GetBitflags() == nsExampleBitflags::Value1);
  }
}

void AccessorPropertyTest(nsIReflectedTypeAccessor& ref_accessor, const char* szProperty, nsVariant::Type::Enum type)
{
  nsVariant oldValue = ref_accessor.GetValue(szProperty);
  NS_TEST_BOOL(oldValue.IsValid());
  NS_TEST_BOOL(oldValue.GetType() == type);

  const nsAbstractProperty* pProp = ref_accessor.GetType()->FindPropertyByName(szProperty);
  nsVariant defaultValue = nsToolsReflectionUtils::GetStorageDefault(pProp);
  NS_TEST_BOOL(defaultValue.GetType() == type);
  bool bSetSuccess = ref_accessor.SetValue(szProperty, defaultValue);
  NS_TEST_BOOL(bSetSuccess);

  nsVariant newValue = ref_accessor.GetValue(szProperty);
  NS_TEST_BOOL(newValue.IsValid());
  NS_TEST_BOOL(newValue.GetType() == type);
  NS_TEST_BOOL(newValue == defaultValue);
}

nsUInt32 AccessorPropertiesTest(nsIReflectedTypeAccessor& ref_accessor, const nsRTTI* pType)
{
  nsUInt32 uiPropertiesSet = 0;
  NS_TEST_BOOL(pType != nullptr);

  // Call for base class
  if (pType->GetParentType() != nullptr)
  {
    uiPropertiesSet += AccessorPropertiesTest(ref_accessor, pType->GetParentType());
  }

  // Test properties
  nsUInt32 uiPropCount = pType->GetProperties().GetCount();
  for (nsUInt32 i = 0; i < uiPropCount; ++i)
  {
    const nsAbstractProperty* pProp = pType->GetProperties()[i];
    const bool bIsValueType = nsReflectionUtils::IsValueType(pProp);

    switch (pProp->GetCategory())
    {
      case nsPropertyCategory::Member:
      {
        auto pProp3 = static_cast<const nsAbstractMemberProperty*>(pProp);
        if (pProp->GetFlags().IsSet(nsPropertyFlags::IsEnum))
        {
          AccessorPropertyTest(ref_accessor, pProp->GetPropertyName(), nsVariant::Type::Int64);
          uiPropertiesSet++;
        }
        else if (pProp->GetFlags().IsSet(nsPropertyFlags::Bitflags))
        {
          AccessorPropertyTest(ref_accessor, pProp->GetPropertyName(), nsVariant::Type::Int64);
          uiPropertiesSet++;
        }
        else if (bIsValueType)
        {
          nsVariantType::Enum storageType = nsToolsReflectionUtils::GetStorageType(pProp);
          AccessorPropertyTest(ref_accessor, pProp->GetPropertyName(), storageType);
          uiPropertiesSet++;
        }
        else // nsPropertyFlags::Class
        {
          // Recurs into sub-classes
          const nsUuid& subObjectGuid = ref_accessor.GetValue(pProp->GetPropertyName()).Get<nsUuid>();
          nsDocumentObject* pEmbeddedClassObject = const_cast<nsDocumentObject*>(ref_accessor.GetOwner()->GetChild(subObjectGuid));
          uiPropertiesSet += AccessorPropertiesTest(pEmbeddedClassObject->GetTypeAccessor(), pProp3->GetSpecificType());
        }
      }
      break;
      case nsPropertyCategory::Array:
      {
        // nsAbstractArrayProperty* pProp3 = static_cast<nsAbstractArrayProperty*>(pProp);
        // TODO
      }
      break;

      default:
        NS_ASSERT_NOT_IMPLEMENTED;
        break;
    }
  }
  return uiPropertiesSet;
}

nsUInt32 AccessorPropertiesTest(nsIReflectedTypeAccessor& ref_accessor)
{
  const nsRTTI* handle = ref_accessor.GetType();
  return AccessorPropertiesTest(ref_accessor, handle);
}

static nsUInt32 GetTypeCount()
{
  nsUInt32 uiCount = 0;
  nsRTTI::ForEachType([&](const nsRTTI* pRtti)
    { uiCount++; });
  return uiCount;
}

static const nsRTTI* RegisterType(const char* szTypeName)
{
  const nsRTTI* pRtti = nsRTTI::FindTypeByName(szTypeName);
  NS_TEST_BOOL(pRtti != nullptr);

  nsReflectedTypeDescriptor desc;
  nsToolsReflectionUtils::GetReflectedTypeDescriptorFromRtti(pRtti, desc);
  return nsPhantomRttiManager::RegisterType(desc);
}

NS_CREATE_SIMPLE_TEST(Reflection, ReflectedType)
{
  nsTestDocumentObjectManager manager;

  /*const nsRTTI* pRttiBase =*/RegisterType("nsReflectedClass");
  /*const nsRTTI* pRttiEnumBase =*/RegisterType("nsEnumBase");
  /*const nsRTTI* pRttiBitflagsBase =*/RegisterType("nsBitflagsBase");

  const nsRTTI* pRttiInt = RegisterType("nsIntegerStruct");
  const nsRTTI* pRttiFloat = RegisterType("nsFloatStruct");
  const nsRTTI* pRttiPOD = RegisterType("nsPODClass");
  const nsRTTI* pRttiMath = RegisterType("nsMathClass");
  /*const nsRTTI* pRttiEnum =*/RegisterType("nsExampleEnum");
  /*const nsRTTI* pRttiFlags =*/RegisterType("nsExampleBitflags");
  const nsRTTI* pRttiEnumerations = RegisterType("nsEnumerationsClass");

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsReflectedTypeStorageAccessor")
  {
    {
      nsDocumentObject* pObject = manager.CreateObject(pRttiInt);
      NS_TEST_INT(AccessorPropertiesTest(pObject->GetTypeAccessor()), 8);
      manager.DestroyObject(pObject);
    }
    {
      nsDocumentObject* pObject = manager.CreateObject(pRttiFloat);
      NS_TEST_INT(AccessorPropertiesTest(pObject->GetTypeAccessor()), 4);
      manager.DestroyObject(pObject);
    }
    {
      nsDocumentObject* pObject = manager.CreateObject(pRttiPOD);
      NS_TEST_INT(AccessorPropertiesTest(pObject->GetTypeAccessor()), 20);
      manager.DestroyObject(pObject);
    }
    {
      nsDocumentObject* pObject = manager.CreateObject(pRttiMath);
      NS_TEST_INT(AccessorPropertiesTest(pObject->GetTypeAccessor()), 29);
      manager.DestroyObject(pObject);
    }
    {
      nsDocumentObject* pObject = manager.CreateObject(pRttiEnumerations);
      NS_TEST_INT(AccessorPropertiesTest(pObject->GetTypeAccessor()), 2);
      manager.DestroyObject(pObject);
    }
  }
}


NS_CREATE_SIMPLE_TEST(Reflection, ReflectedTypeReloading)
{
  nsTestDocumentObjectManager manager;

  const nsRTTI* pRttiInner = nsRTTI::FindTypeByName("InnerStruct");
  const nsRTTI* pRttiInnerP = nullptr;
  nsReflectedTypeDescriptor descInner;

  const nsRTTI* pRttiOuter = nsRTTI::FindTypeByName("OuterClass");
  const nsRTTI* pRttiOuterP = nullptr;
  nsReflectedTypeDescriptor descOuter;

  nsUInt32 uiRegisteredBaseTypes = GetTypeCount();
  NS_TEST_BLOCK(nsTestBlock::Enabled, "RegisterType")
  {
    NS_TEST_BOOL(pRttiInner != nullptr);
    nsToolsReflectionUtils::GetReflectedTypeDescriptorFromRtti(pRttiInner, descInner);
    descInner.m_sTypeName = "InnerStructP";
    pRttiInnerP = nsPhantomRttiManager::RegisterType(descInner);
    NS_TEST_BOOL(pRttiInnerP != nullptr);

    NS_TEST_BOOL(pRttiOuter != nullptr);
    nsToolsReflectionUtils::GetReflectedTypeDescriptorFromRtti(pRttiOuter, descOuter);
    descOuter.m_sTypeName = "OuterClassP";
    descOuter.m_Properties[0].m_sType = "InnerStructP";
    pRttiOuterP = nsPhantomRttiManager::RegisterType(descOuter);
    NS_TEST_BOOL(pRttiOuterP != nullptr);
  }

  {
    nsDocumentObject* pInnerObject = manager.CreateObject(pRttiInnerP);
    manager.AddObject(pInnerObject, nullptr, "Children", -1);
    nsIReflectedTypeAccessor& innerAccessor = pInnerObject->GetTypeAccessor();

    nsDocumentObject* pOuterObject = manager.CreateObject(pRttiOuterP);
    manager.AddObject(pOuterObject, nullptr, "Children", -1);
    nsIReflectedTypeAccessor& outerAccessor = pOuterObject->GetTypeAccessor();

    nsUuid innerGuid = outerAccessor.GetValue("Inner").Get<nsUuid>();
    nsDocumentObject* pEmbeddedInnerObject = manager.GetObject(innerGuid);
    nsIReflectedTypeAccessor& embeddedInnerAccessor = pEmbeddedInnerObject->GetTypeAccessor();

    NS_TEST_BLOCK(nsTestBlock::Enabled, "SetValues")
    {
      // Just set a few values to make sure they don't get messed up by the following operations.
      NS_TEST_BOOL(innerAccessor.SetValue("IP1", 1.4f));
      NS_TEST_BOOL(outerAccessor.SetValue("OP1", 0.9f));
      NS_TEST_BOOL(embeddedInnerAccessor.SetValue("IP1", 1.4f));
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "AddProperty")
    {
      // Say we reload the engine and the InnerStruct now has a second property: IP2.
      descInner.m_Properties.PushBack(nsReflectedPropertyDescriptor(nsPropertyCategory::Member, "IP2", "nsVec4",
        nsBitflags<nsPropertyFlags>(nsPropertyFlags::StandardType), nsArrayPtr<nsPropertyAttribute* const>()));
      const nsRTTI* NewInnerHandle = nsPhantomRttiManager::RegisterType(descInner);
      NS_TEST_BOOL(NewInnerHandle == pRttiInnerP);

      // Check that the new property is present.
      AccessorPropertyTest(innerAccessor, "IP2", nsVariant::Type::Vector4);

      AccessorPropertyTest(embeddedInnerAccessor, "IP2", nsVariant::Type::Vector4);

      // Test that the old properties are still valid.
      NS_TEST_BOOL(innerAccessor.GetValue("IP1") == 1.4f);
      NS_TEST_BOOL(outerAccessor.GetValue("OP1") == 0.9f);
      NS_TEST_BOOL(embeddedInnerAccessor.GetValue("IP1") == 1.4f);
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "ChangeProperty")
    {
      // Out original inner float now is a Int32!
      descInner.m_Properties[0].m_sType = "nsInt32";
      const nsRTTI* NewInnerHandle = nsPhantomRttiManager::RegisterType(descInner);
      NS_TEST_BOOL(NewInnerHandle == pRttiInnerP);

      // Test if the previous value was converted correctly to its new type.
      nsVariant innerValue = innerAccessor.GetValue("IP1");
      NS_TEST_BOOL(innerValue.IsValid());
      NS_TEST_BOOL(innerValue.GetType() == nsVariant::Type::Int32);
      NS_TEST_INT(innerValue.Get<nsInt32>(), 1);

      nsVariant outerValue = embeddedInnerAccessor.GetValue("IP1");
      NS_TEST_BOOL(outerValue.IsValid());
      NS_TEST_BOOL(outerValue.GetType() == nsVariant::Type::Int32);
      NS_TEST_INT(outerValue.Get<nsInt32>(), 1);

      // Test that the old properties are still valid.
      NS_TEST_BOOL(outerAccessor.GetValue("OP1") == 0.9f);

      AccessorPropertyTest(innerAccessor, "IP2", nsVariant::Type::Vector4);
      AccessorPropertyTest(embeddedInnerAccessor, "IP2", nsVariant::Type::Vector4);
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "DeleteProperty")
    {
      // Lets now delete the original inner property IP1.
      descInner.m_Properties.RemoveAtAndCopy(0);
      const nsRTTI* NewInnerHandle = nsPhantomRttiManager::RegisterType(descInner);
      NS_TEST_BOOL(NewInnerHandle == pRttiInnerP);

      // Check that IP1 is really gone.
      NS_TEST_BOOL(!innerAccessor.GetValue("IP1").IsValid());
      NS_TEST_BOOL(!embeddedInnerAccessor.GetValue("IP1").IsValid());

      // Test that the old properties are still valid.
      NS_TEST_BOOL(outerAccessor.GetValue("OP1") == 0.9f);

      AccessorPropertyTest(innerAccessor, "IP2", nsVariant::Type::Vector4);
      AccessorPropertyTest(embeddedInnerAccessor, "IP2", nsVariant::Type::Vector4);
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "RevertProperties")
    {
      // Reset all classes to their initial state.
      nsToolsReflectionUtils::GetReflectedTypeDescriptorFromRtti(pRttiInner, descInner);
      descInner.m_sTypeName = "InnerStructP";
      nsPhantomRttiManager::RegisterType(descInner);

      nsToolsReflectionUtils::GetReflectedTypeDescriptorFromRtti(pRttiOuter, descOuter);
      descInner.m_sTypeName = "OuterStructP";
      descOuter.m_Properties[0].m_sType = "InnerStructP";
      nsPhantomRttiManager::RegisterType(descOuter);

      // Test that the old properties are back again.
      nsStringBuilder path = "IP1";
      nsVariant innerValue = innerAccessor.GetValue(path);
      NS_TEST_BOOL(innerValue.IsValid());
      NS_TEST_BOOL(innerValue.GetType() == nsVariant::Type::Float);
      NS_TEST_FLOAT(innerValue.Get<float>(), 1.0f, 0.0f);

      nsVariant outerValue = embeddedInnerAccessor.GetValue("IP1");
      NS_TEST_BOOL(outerValue.IsValid());
      NS_TEST_BOOL(outerValue.GetType() == nsVariant::Type::Float);
      NS_TEST_FLOAT(outerValue.Get<float>(), 1.0f, 0.0f);
      NS_TEST_BOOL(outerAccessor.GetValue("OP1") == 0.9f);
    }

    manager.RemoveObject(pInnerObject);
    manager.DestroyObject(pInnerObject);

    manager.RemoveObject(pOuterObject);
    manager.DestroyObject(pOuterObject);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "UnregisterType")
  {
    NS_TEST_INT(GetTypeCount(), uiRegisteredBaseTypes + 2);
    nsPhantomRttiManager::UnregisterType(pRttiOuterP);
    nsPhantomRttiManager::UnregisterType(pRttiInnerP);
    NS_TEST_INT(GetTypeCount(), uiRegisteredBaseTypes);
  }
}
