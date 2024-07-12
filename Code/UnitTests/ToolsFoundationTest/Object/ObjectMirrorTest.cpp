#include <ToolsFoundationTest/ToolsFoundationTestPCH.h>

#include <ToolsFoundation/Object/ObjectAccessorBase.h>
#include <ToolsFoundationTest/Object/TestObjectManager.h>
#include <ToolsFoundationTest/Reflection/ReflectionTestClasses.h>

void MirrorCheck(nsTestDocument* pDoc, const nsDocumentObject* pObject)
{
  // Create native object graph
  nsAbstractObjectGraph graph;
  nsAbstractObjectNode* pRootNode = nullptr;
  {
    nsRttiConverterWriter rttiConverter(&graph, &pDoc->m_Context, true, true);
    pRootNode = rttiConverter.AddObjectToGraph(pObject->GetType(), pDoc->m_ObjectMirror.GetNativeObjectPointer(pObject), "Object");
  }

  // Create object manager graph
  nsAbstractObjectGraph origGraph;
  nsAbstractObjectNode* pOrigRootNode = nullptr;
  {
    nsDocumentObjectConverterWriter writer(&origGraph, pDoc->GetObjectManager());
    pOrigRootNode = writer.AddObjectToGraph(pObject);
  }

  // Remap native guids so they match the object manager (stuff like embedded classes will not have a guid on the native side).
  graph.ReMapNodeGuidsToMatchGraph(pRootNode, origGraph, pOrigRootNode);
  nsDeque<nsAbstractGraphDiffOperation> diffResult;

  graph.CreateDiffWithBaseGraph(origGraph, diffResult);

  NS_TEST_BOOL(diffResult.GetCount() == 0);
}


nsVariant GetVariantFromType(nsVariant::Type::Enum type)
{
  switch (type)
  {
    case nsVariant::Type::Invalid:
      return nsVariant();
    case nsVariant::Type::Bool:
      return nsVariant(true);
    case nsVariant::Type::Int8:
      return nsVariant((nsInt8)-55);
    case nsVariant::Type::UInt8:
      return nsVariant((nsUInt8)44);
    case nsVariant::Type::Int16:
      return nsVariant((nsInt16)-444);
    case nsVariant::Type::UInt16:
      return nsVariant((nsUInt16)666);
    case nsVariant::Type::Int32:
      return nsVariant((nsInt32)-88880);
    case nsVariant::Type::UInt32:
      return nsVariant((nsUInt32)123445);
    case nsVariant::Type::Int64:
      return nsVariant((nsInt64)-888800000);
    case nsVariant::Type::UInt64:
      return nsVariant((nsUInt64)123445000);
    case nsVariant::Type::Float:
      return nsVariant(1024.0f);
    case nsVariant::Type::Double:
      return nsVariant(-2048.0f);
    case nsVariant::Type::Color:
      return nsVariant(nsColor(0.5f, 33.0f, 2.0f, 0.3f));
    case nsVariant::Type::ColorGamma:
      return nsVariant(nsColorGammaUB(nsColor(0.5f, 33.0f, 2.0f, 0.3f)));
    case nsVariant::Type::Vector2:
      return nsVariant(nsVec2(2.0f, 4.0f));
    case nsVariant::Type::Vector3:
      return nsVariant(nsVec3(2.0f, 4.0f, -8.0f));
    case nsVariant::Type::Vector4:
      return nsVariant(nsVec4(1.0f, 7.0f, 8.0f, -10.0f));
    case nsVariant::Type::Vector2I:
      return nsVariant(nsVec2I32(1, 2));
    case nsVariant::Type::Vector3I:
      return nsVariant(nsVec3I32(3, 4, 5));
    case nsVariant::Type::Vector4I:
      return nsVariant(nsVec4I32(6, 7, 8, 9));
    case nsVariant::Type::Quaternion:
    {
      nsQuat quat;
      quat = nsQuat::MakeFromEulerAngles(nsAngle::MakeFromDegree(30), nsAngle::MakeFromDegree(-15), nsAngle::MakeFromDegree(20));
      return nsVariant(quat);
    }
    case nsVariant::Type::Matrix3:
    {
      nsMat3 mat = nsMat3::MakeIdentity();

      mat = nsMat3::MakeAxisRotation(nsVec3(1.0f, 0.0f, 0.0f), nsAngle::MakeFromDegree(30));
      return nsVariant(mat);
    }
    case nsVariant::Type::Matrix4:
    {
      nsMat4 mat = nsMat4::MakeIdentity();

      mat = nsMat4::MakeAxisRotation(nsVec3(0.0f, 1.0f, 0.0f), nsAngle::MakeFromDegree(30));
      mat.SetTranslationVector(nsVec3(1.0f, 2.0f, 3.0f));
      return nsVariant(mat);
    }
    case nsVariant::Type::String:
      return nsVariant("Test");
    case nsVariant::Type::StringView:
      return nsVariant("Test");
    case nsVariant::Type::Time:
      return nsVariant(nsTime::MakeFromSeconds(123.0f));
    case nsVariant::Type::Uuid:
    {
      return nsVariant(nsUuid::MakeUuid());
    }
    case nsVariant::Type::Angle:
      return nsVariant(nsAngle::MakeFromDegree(30.0f));
    case nsVariant::Type::DataBuffer:
    {
      nsDataBuffer data;
      data.PushBack(12);
      data.PushBack(55);
      data.PushBack(88);
      return nsVariant(data);
    }
    case nsVariant::Type::VariantArray:
      return nsVariantArray();
    case nsVariant::Type::VariantDictionary:
      return nsVariantDictionary();
    case nsVariant::Type::TypedPointer:
      return nsVariant(nsTypedPointer(nullptr, nullptr));
    case nsVariant::Type::TypedObject:
      NS_ASSERT_NOT_IMPLEMENTED;

    default:
      NS_REPORT_FAILURE("Invalid case statement");
      return nsVariant();
  }
  return nsVariant();
}

void RecursiveModifyProperty(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsObjectAccessorBase* pObjectAccessor)
{
  if (pProp->GetCategory() == nsPropertyCategory::Member)
  {
    if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
    {
      if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
      {
        const nsUuid oldGuid = pObjectAccessor->Get<nsUuid>(pObject, pProp);
        nsUuid newGuid = nsUuid::MakeUuid();
        if (oldGuid.IsValid())
        {
          NS_TEST_BOOL(pObjectAccessor->RemoveObject(pObjectAccessor->GetObject(oldGuid)).m_Result.Succeeded());
        }

        NS_TEST_BOOL(pObjectAccessor->AddObject(pObject, pProp, nsVariant(), pProp->GetSpecificType(), newGuid).m_Result.Succeeded());

        const nsDocumentObject* pChild = pObject->GetChild(newGuid);
        NS_ASSERT_DEV(pChild != nullptr, "References child object does not exist!");
      }
      else
      {
        nsVariant value = GetVariantFromType(pProp->GetSpecificType()->GetVariantType());
        NS_TEST_BOOL(pObjectAccessor->SetValue(pObject, pProp, value).m_Result.Succeeded());
      }
    }
    else
    {
      if (pProp->GetFlags().IsAnySet(nsPropertyFlags::IsEnum | nsPropertyFlags::Bitflags | nsPropertyFlags::StandardType))
      {
        nsVariant value = GetVariantFromType(pProp->GetSpecificType()->GetVariantType());
        NS_TEST_BOOL(pObjectAccessor->SetValue(pObject, pProp, value).m_Result.Succeeded());
      }
      else if (pProp->GetFlags().IsSet(nsPropertyFlags::Class))
      {
        // Noting to do here, value cannot change
      }
    }
  }
  else if (pProp->GetCategory() == nsPropertyCategory::Array || pProp->GetCategory() == nsPropertyCategory::Set)
  {
    if (pProp->GetFlags().IsAnySet(nsPropertyFlags::StandardType | nsPropertyFlags::Pointer) &&
        !pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
    {
      nsInt32 iCurrentCount = pObjectAccessor->GetCount(pObject, pProp);
      for (nsInt32 i = iCurrentCount - 1; i >= 0; --i)
      {
        pObjectAccessor->RemoveValue(pObject, pProp, i).AssertSuccess();
      }

      nsVariant value1 = nsReflectionUtils::GetDefaultValue(pProp, 0);
      nsVariant value2 = GetVariantFromType(pProp->GetSpecificType()->GetVariantType());
      NS_TEST_BOOL(pObjectAccessor->InsertValue(pObject, pProp, value1, 0).m_Result.Succeeded());
      NS_TEST_BOOL(pObjectAccessor->InsertValue(pObject, pProp, value2, 1).m_Result.Succeeded());
    }
    else if (pProp->GetFlags().IsSet(nsPropertyFlags::Class))
    {
      nsInt32 iCurrentCount = pObject->GetTypeAccessor().GetCount(pProp->GetPropertyName());
      nsHybridArray<nsVariant, 16> currentValues;
      pObject->GetTypeAccessor().GetValues(pProp->GetPropertyName(), currentValues);
      for (nsInt32 i = iCurrentCount - 1; i >= 0; --i)
      {
        NS_TEST_BOOL(pObjectAccessor->RemoveObject(pObjectAccessor->GetObject(currentValues[i].Get<nsUuid>())).m_Result.Succeeded());
      }

      if (pProp->GetCategory() == nsPropertyCategory::Array)
      {
        nsUuid newGuid = nsUuid::MakeUuid();
        NS_TEST_BOOL(pObjectAccessor->AddObject(pObject, pProp, 0, pProp->GetSpecificType(), newGuid).m_Result.Succeeded());
      }
    }
  }
  else if (pProp->GetCategory() == nsPropertyCategory::Map)
  {
    if (pProp->GetFlags().IsAnySet(nsPropertyFlags::StandardType | nsPropertyFlags::Pointer) &&
        !pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
    {
      nsInt32 iCurrentCount = pObjectAccessor->GetCount(pObject, pProp);
      nsHybridArray<nsVariant, 16> keys;
      pObjectAccessor->GetKeys(pObject, pProp, keys).AssertSuccess();
      for (const nsVariant& key : keys)
      {
        pObjectAccessor->RemoveValue(pObject, pProp, key).AssertSuccess();
      }

      nsVariant value1 = nsReflectionUtils::GetDefaultValue(pProp, "Dummy");
      nsVariant value2 = GetVariantFromType(pProp->GetSpecificType()->GetVariantType());
      NS_TEST_BOOL(pObjectAccessor->InsertValue(pObject, pProp, value1, "value1").m_Result.Succeeded());
      NS_TEST_BOOL(pObjectAccessor->InsertValue(pObject, pProp, value2, "value2").m_Result.Succeeded());
    }
    else if (pProp->GetFlags().IsSet(nsPropertyFlags::Class))
    {
      nsInt32 iCurrentCount = pObject->GetTypeAccessor().GetCount(pProp->GetPropertyName());
      nsHybridArray<nsVariant, 16> currentValues;
      pObject->GetTypeAccessor().GetValues(pProp->GetPropertyName(), currentValues);
      for (nsInt32 i = iCurrentCount - 1; i >= 0; --i)
      {
        NS_TEST_BOOL(pObjectAccessor->RemoveObject(pObjectAccessor->GetObject(currentValues[i].Get<nsUuid>())).m_Result.Succeeded());
      }

      nsUuid newGuid = nsUuid::MakeUuid();
      NS_TEST_BOOL(pObjectAccessor->AddObject(pObject, pProp, "value1", pProp->GetSpecificType(), newGuid).m_Result.Succeeded());
    }
  }
}

void RecursiveModifyObject(const nsDocumentObject* pObject, nsObjectAccessorBase* pAccessor)
{
  nsHybridArray<const nsAbstractProperty*, 32> properties;
  pObject->GetTypeAccessor().GetType()->GetAllProperties(properties);
  for (const auto* pProp : properties)
  {
    RecursiveModifyProperty(pObject, pProp, pAccessor);
  }

  for (const nsDocumentObject* pSubObject : pObject->GetChildren())
  {
    RecursiveModifyObject(pSubObject, pAccessor);
  }
}

NS_CREATE_SIMPLE_TEST(DocumentObject, ObjectMirror)
{
  nsTestDocument doc("Test", true);
  doc.InitializeAfterLoading(false);
  nsObjectAccessorBase* pAccessor = doc.GetObjectAccessor();
  nsUuid mirrorGuid;

  pAccessor->StartTransaction("Init");
  nsStatus status = pAccessor->AddObject(nullptr, (const nsAbstractProperty*)nullptr, -1, nsGetStaticRTTI<nsMirrorTest>(), mirrorGuid);
  const nsDocumentObject* pObject = pAccessor->GetObject(mirrorGuid);
  NS_TEST_BOOL(status.m_Result.Succeeded());
  pAccessor->FinishTransaction();

  MirrorCheck(&doc, pObject);

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Document Changes")
  {
    pAccessor->StartTransaction("Document Changes");
    RecursiveModifyObject(pObject, pAccessor);
    pAccessor->FinishTransaction();

    MirrorCheck(&doc, pObject);
  }
  {
    pAccessor->StartTransaction("Document Changes");
    RecursiveModifyObject(pObject, pAccessor);
    pAccessor->FinishTransaction();

    MirrorCheck(&doc, pObject);
  }
}
