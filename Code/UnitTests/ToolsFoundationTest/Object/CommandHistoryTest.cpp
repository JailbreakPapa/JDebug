#include <ToolsFoundationTest/ToolsFoundationTestPCH.h>

#include <ToolsFoundationTest/Object/TestObjectManager.h>
#include <ToolsFoundationTest/Reflection/ReflectionTestClasses.h>

NS_CREATE_SIMPLE_TEST(DocumentObject, CommandHistory)
{
  nsTestDocument doc("Test", true);
  doc.InitializeAfterLoading(false);
  nsObjectAccessorBase* pAccessor = doc.GetObjectAccessor();

  auto CreateObject = [&doc, &pAccessor](const nsRTTI* pType) -> const nsDocumentObject*
  {
    nsUuid objGuid;
    pAccessor->StartTransaction("Add Object");
    NS_TEST_STATUS(pAccessor->AddObject(nullptr, (const nsAbstractProperty*)nullptr, -1, pType, objGuid));
    pAccessor->FinishTransaction();
    return pAccessor->GetObject(objGuid);
  };

  auto StoreOriginalState = [&doc](nsAbstractObjectGraph& ref_graph, const nsDocumentObject* pRoot)
  {
    nsDocumentObjectConverterWriter writer(&ref_graph, doc.GetObjectManager(), [](const nsDocumentObject*, const nsAbstractProperty* p)
      { return p->GetAttributeByType<nsHiddenAttribute>() == nullptr; });
    nsAbstractObjectNode* pAbstractObj = writer.AddObjectToGraph(pRoot);
  };

  auto CompareAgainstOriginalState = [&doc](nsAbstractObjectGraph& ref_original, const nsDocumentObject* pRoot)
  {
    nsAbstractObjectGraph graph;
    nsDocumentObjectConverterWriter writer2(&graph, doc.GetObjectManager(), [](const nsDocumentObject*, const nsAbstractProperty* p)
      { return p->GetAttributeByType<nsHiddenAttribute>() == nullptr; });
    nsAbstractObjectNode* pAbstractObj2 = writer2.AddObjectToGraph(pRoot);

    nsDeque<nsAbstractGraphDiffOperation> diff;
    graph.CreateDiffWithBaseGraph(ref_original, diff);
    NS_TEST_BOOL(diff.GetCount() == 0);
  };

  const nsDocumentObject* pRoot = CreateObject(nsGetStaticRTTI<nsMirrorTest>());

  nsUuid mathGuid = pAccessor->Get<nsUuid>(pRoot, "Math");
  nsUuid objectGuid = pAccessor->Get<nsUuid>(pRoot, "Object");

  const nsDocumentObject* pMath = pAccessor->GetObject(mathGuid);
  const nsDocumentObject* pObjectTest = pAccessor->GetObject(objectGuid);

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetValue")
  {

    NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), 1);

    auto TestSetValue = [&](const nsDocumentObject* pObject, const char* szProperty, nsVariant value)
    {
      nsAbstractObjectGraph graph;
      StoreOriginalState(graph, pObject);

      nsUInt32 uiUndoHistorySize = doc.GetCommandHistory()->GetUndoStackSize();

      pAccessor->StartTransaction("SetValue");
      NS_TEST_STATUS(pAccessor->SetValue(pObject, szProperty, value));
      pAccessor->FinishTransaction();
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize + 1);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 0);
      nsVariant newValue;
      NS_TEST_STATUS(pAccessor->GetValue(pObject, szProperty, newValue));
      NS_TEST_BOOL(newValue == value);

      NS_TEST_STATUS(doc.GetCommandHistory()->Undo());
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 1);
      CompareAgainstOriginalState(graph, pObject);

      NS_TEST_STATUS(doc.GetCommandHistory()->Redo());
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize + 1);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 0);
      NS_TEST_STATUS(pAccessor->GetValue(pObject, szProperty, newValue));
      NS_TEST_BOOL(newValue == value);
    };

    // Math
    TestSetValue(pMath, "Vec2", nsVec2(1, 2));
    TestSetValue(pMath, "Vec3", nsVec3(1, 2, 3));
    TestSetValue(pMath, "Vec4", nsVec4(1, 2, 3, 4));
    TestSetValue(pMath, "Vec2I", nsVec2I32(1, 2));
    TestSetValue(pMath, "Vec3I", nsVec3I32(1, 2, 3));
    TestSetValue(pMath, "Vec4I", nsVec4I32(1, 2, 3, 4));
    nsQuat qValue;
    qValue = nsQuat::MakeFromEulerAngles(nsAngle::MakeFromDegree(30), nsAngle::MakeFromDegree(30), nsAngle::MakeFromDegree(30));
    TestSetValue(pMath, "Quat", qValue);
    nsMat3 mValue;
    mValue = nsMat3::MakeRotationX(nsAngle::MakeFromDegree(30));
    TestSetValue(pMath, "Mat3", mValue);
    nsMat4 mValue2;
    mValue2.SetIdentity();
    mValue2 = nsMat4::MakeRotationX(nsAngle::MakeFromDegree(30));
    TestSetValue(pMath, "Mat4", mValue2);

    // Integer
    const nsDocumentObject* pInteger = CreateObject(nsGetStaticRTTI<nsIntegerStruct>());
    TestSetValue(pInteger, "Int8", nsInt8(-5));
    TestSetValue(pInteger, "UInt8", nsUInt8(5));
    TestSetValue(pInteger, "Int16", nsInt16(-5));
    TestSetValue(pInteger, "UInt16", nsUInt16(5));
    TestSetValue(pInteger, "Int32", nsInt32(-5));
    TestSetValue(pInteger, "UInt32", nsUInt32(5));
    TestSetValue(pInteger, "Int64", nsInt64(-5));
    TestSetValue(pInteger, "UInt64", nsUInt64(5));

    // Test automatic type conversions
    TestSetValue(pInteger, "Int8", nsInt16(-5));
    TestSetValue(pInteger, "Int8", nsInt32(-5));
    TestSetValue(pInteger, "Int8", nsInt64(-5));
    TestSetValue(pInteger, "Int8", float(-5));
    TestSetValue(pInteger, "Int8", nsUInt8(5));

    TestSetValue(pInteger, "Int64", nsInt32(-5));
    TestSetValue(pInteger, "Int64", nsInt16(-5));
    TestSetValue(pInteger, "Int64", nsInt8(-5));
    TestSetValue(pInteger, "Int64", float(-5));
    TestSetValue(pInteger, "Int64", nsUInt8(5));

    TestSetValue(pInteger, "UInt64", nsUInt32(5));
    TestSetValue(pInteger, "UInt64", nsUInt16(5));
    TestSetValue(pInteger, "UInt64", nsUInt8(5));
    TestSetValue(pInteger, "UInt64", float(5));
    TestSetValue(pInteger, "UInt64", nsInt8(5));

    // Float
    const nsDocumentObject* pFloat = CreateObject(nsGetStaticRTTI<nsFloatStruct>());
    TestSetValue(pFloat, "Float", -5.0f);
    TestSetValue(pFloat, "Double", -5.0);
    TestSetValue(pFloat, "Time", nsTime::MakeFromMinutes(3.0f));
    TestSetValue(pFloat, "Angle", nsAngle::MakeFromDegree(45.0f));

    TestSetValue(pFloat, "Float", 5.0);
    TestSetValue(pFloat, "Float", nsInt8(-5));
    TestSetValue(pFloat, "Float", nsUInt8(5));

    // Misc PODs
    const nsDocumentObject* pPOD = CreateObject(nsGetStaticRTTI<nsPODClass>());
    TestSetValue(pPOD, "Bool", true);
    TestSetValue(pPOD, "Bool", false);
    TestSetValue(pPOD, "Color", nsColor(1.0f, 2.0f, 3.0f, 4.0f));
    TestSetValue(pPOD, "ColorUB", nsColorGammaUB(200, 100, 255));
    TestSetValue(pPOD, "String", "Test");
    nsVarianceTypeAngle customFloat;
    customFloat.m_Value = nsAngle::MakeFromDegree(45.0f);
    customFloat.m_fVariance = 1.0f;
    TestSetValue(pPOD, "VarianceAngle", customFloat);

    // Enumerations
    const nsDocumentObject* pEnum = CreateObject(nsGetStaticRTTI<nsEnumerationsClass>());
    TestSetValue(pEnum, "Enum", (nsInt8)nsExampleEnum::Value2);
    TestSetValue(pEnum, "Enum", (nsInt64)nsExampleEnum::Value2);
    TestSetValue(pEnum, "Bitflags", (nsUInt8)nsExampleBitflags::Value2);
    TestSetValue(pEnum, "Bitflags", (nsInt64)nsExampleBitflags::Value2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "InsertValue")
  {
    auto TestInsertValue = [&](const nsDocumentObject* pObject, const char* szProperty, nsVariant value, nsVariant index)
    {
      nsAbstractObjectGraph graph;
      StoreOriginalState(graph, pObject);

      const nsUInt32 uiUndoHistorySize = doc.GetCommandHistory()->GetUndoStackSize();
      const nsInt32 iArraySize = pAccessor->GetCount(pObject, szProperty);

      pAccessor->StartTransaction("InsertValue");
      NS_TEST_STATUS(pAccessor->InsertValue(pObject, szProperty, value, index));
      pAccessor->FinishTransaction();
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize + 1);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 0);
      NS_TEST_INT(pAccessor->GetCount(pObject, szProperty), iArraySize + 1);
      nsVariant newValue;
      NS_TEST_STATUS(pAccessor->GetValue(pObject, szProperty, newValue, index));
      NS_TEST_BOOL(newValue == value);

      NS_TEST_STATUS(doc.GetCommandHistory()->Undo());
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 1);
      NS_TEST_INT(pAccessor->GetCount(pObject, szProperty), iArraySize);
      CompareAgainstOriginalState(graph, pObject);

      NS_TEST_STATUS(doc.GetCommandHistory()->Redo());
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize + 1);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 0);
      NS_TEST_INT(pAccessor->GetCount(pObject, szProperty), iArraySize + 1);
      NS_TEST_STATUS(pAccessor->GetValue(pObject, szProperty, newValue, index));
      NS_TEST_BOOL(newValue == value);
    };

    TestInsertValue(pObjectTest, "StandardTypeArray", double(0), 0);
    TestInsertValue(pObjectTest, "StandardTypeArray", double(2), 1);
    TestInsertValue(pObjectTest, "StandardTypeArray", double(1), 1);

    TestInsertValue(pObjectTest, "StandardTypeSet", "A", 0);
    TestInsertValue(pObjectTest, "StandardTypeSet", "C", 1);
    TestInsertValue(pObjectTest, "StandardTypeSet", "B", 1);

    TestInsertValue(pObjectTest, "StandardTypeMap", double(0), "A");
    TestInsertValue(pObjectTest, "StandardTypeMap", double(2), "C");
    TestInsertValue(pObjectTest, "StandardTypeMap", double(1), "B");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MoveValue")
  {
    auto TestMoveValue = [&](const nsDocumentObject* pObject, const char* szProperty, nsVariant oldIndex, nsVariant newIndex, nsArrayPtr<nsVariant> expectedOutcome)
    {
      nsAbstractObjectGraph graph;
      StoreOriginalState(graph, pObject);

      const nsUInt32 uiUndoHistorySize = doc.GetCommandHistory()->GetUndoStackSize();
      const nsInt32 iArraySize = pAccessor->GetCount(pObject, szProperty);
      NS_TEST_INT(iArraySize, expectedOutcome.GetCount());

      nsDynamicArray<nsVariant> values;
      NS_TEST_STATUS(pAccessor->GetValues(pObject, szProperty, values));
      NS_TEST_INT(iArraySize, values.GetCount());

      pAccessor->StartTransaction("MoveValue");
      NS_TEST_STATUS(pAccessor->MoveValue(pObject, szProperty, oldIndex, newIndex));
      pAccessor->FinishTransaction();
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize + 1);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 0);
      NS_TEST_INT(pAccessor->GetCount(pObject, szProperty), iArraySize);

      for (nsInt32 i = 0; i < iArraySize; i++)
      {
        nsVariant newValue;
        NS_TEST_STATUS(pAccessor->GetValue(pObject, szProperty, newValue, i));
        NS_TEST_BOOL(newValue == expectedOutcome[i]);
      }

      NS_TEST_STATUS(doc.GetCommandHistory()->Undo());
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 1);
      NS_TEST_INT(pAccessor->GetCount(pObject, szProperty), iArraySize);
      CompareAgainstOriginalState(graph, pObject);

      NS_TEST_STATUS(doc.GetCommandHistory()->Redo());
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize + 1);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 0);
      NS_TEST_INT(pAccessor->GetCount(pObject, szProperty), iArraySize);

      for (nsInt32 i = 0; i < iArraySize; i++)
      {
        nsVariant newValue;
        NS_TEST_STATUS(pAccessor->GetValue(pObject, szProperty, newValue, i));
        NS_TEST_BOOL(newValue == expectedOutcome[i]);
      }
    };

    {
      nsVariant expectedValues[3] = {0, 1, 2};
      // Move first element before or after itself (no-op)
      TestMoveValue(pObjectTest, "StandardTypeArray", 0, 0, nsArrayPtr<nsVariant>(expectedValues));
      TestMoveValue(pObjectTest, "StandardTypeArray", 0, 1, nsArrayPtr<nsVariant>(expectedValues));
      // Move last element before or after itself (no-op)
      TestMoveValue(pObjectTest, "StandardTypeArray", 2, 2, nsArrayPtr<nsVariant>(expectedValues));
      TestMoveValue(pObjectTest, "StandardTypeArray", 2, 3, nsArrayPtr<nsVariant>(expectedValues));
    }
    {
      // Move first element to the end.
      nsVariant expectedValues[3] = {1, 2, 0};
      TestMoveValue(pObjectTest, "StandardTypeArray", 0, 3, nsArrayPtr<nsVariant>(expectedValues));
    }
    {
      // Move last element to the front.
      nsVariant expectedValues[3] = {0, 1, 2};
      TestMoveValue(pObjectTest, "StandardTypeArray", 2, 0, nsArrayPtr<nsVariant>(expectedValues));
    }
    {
      // Move first element to the middle
      nsVariant expectedValues[3] = {1, 0, 2};
      TestMoveValue(pObjectTest, "StandardTypeArray", 0, 2, nsArrayPtr<nsVariant>(expectedValues));
    }
    {
      // Move last element to the middle
      nsVariant expectedValues[3] = {1, 2, 0};
      TestMoveValue(pObjectTest, "StandardTypeArray", 2, 1, nsArrayPtr<nsVariant>(expectedValues));
    }

    {
      nsVariant expectedValues[3] = {"A", "B", "C"};
      // Move first element before or after itself (no-op)
      TestMoveValue(pObjectTest, "StandardTypeSet", 0, 0, nsArrayPtr<nsVariant>(expectedValues));
      TestMoveValue(pObjectTest, "StandardTypeSet", 0, 1, nsArrayPtr<nsVariant>(expectedValues));
      // Move last element before or after itself (no-op)
      TestMoveValue(pObjectTest, "StandardTypeSet", 2, 2, nsArrayPtr<nsVariant>(expectedValues));
      TestMoveValue(pObjectTest, "StandardTypeSet", 2, 3, nsArrayPtr<nsVariant>(expectedValues));
    }
    {
      // Move first element to the end.
      nsVariant expectedValues[3] = {"B", "C", "A"};
      TestMoveValue(pObjectTest, "StandardTypeSet", 0, 3, nsArrayPtr<nsVariant>(expectedValues));
    }
    {
      // Move last element to the front.
      nsVariant expectedValues[3] = {"A", "B", "C"};
      TestMoveValue(pObjectTest, "StandardTypeSet", 2, 0, nsArrayPtr<nsVariant>(expectedValues));
    }
    {
      // Move first element to the middle
      nsVariant expectedValues[3] = {"B", "A", "C"};
      TestMoveValue(pObjectTest, "StandardTypeSet", 0, 2, nsArrayPtr<nsVariant>(expectedValues));
    }
    {
      // Move last element to the middle
      nsVariant expectedValues[3] = {"B", "C", "A"};
      TestMoveValue(pObjectTest, "StandardTypeSet", 2, 1, nsArrayPtr<nsVariant>(expectedValues));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RemoveValue")
  {
    auto TestRemoveValue = [&](const nsDocumentObject* pObject, const char* szProperty, nsVariant index, nsArrayPtr<nsVariant> expectedOutcome)
    {
      nsAbstractObjectGraph graph;
      StoreOriginalState(graph, pObject);

      const nsUInt32 uiUndoHistorySize = doc.GetCommandHistory()->GetUndoStackSize();
      const nsInt32 iArraySize = pAccessor->GetCount(pObject, szProperty);
      NS_TEST_INT(iArraySize - 1, expectedOutcome.GetCount());

      nsDynamicArray<nsVariant> values;
      nsDynamicArray<nsVariant> keys;
      {
        NS_TEST_STATUS(pAccessor->GetValues(pObject, szProperty, values));
        NS_TEST_INT(iArraySize, values.GetCount());

        NS_TEST_STATUS(pAccessor->GetKeys(pObject, szProperty, keys));
        NS_TEST_INT(iArraySize, keys.GetCount());
        nsUInt32 uiIndex = keys.IndexOf(index);
        keys.RemoveAtAndSwap(uiIndex);
        values.RemoveAtAndSwap(uiIndex);
        NS_TEST_INT(iArraySize - 1, keys.GetCount());
      }

      pAccessor->StartTransaction("RemoveValue");
      NS_TEST_STATUS(pAccessor->RemoveValue(pObject, szProperty, index));
      pAccessor->FinishTransaction();
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize + 1);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 0);
      NS_TEST_INT(pAccessor->GetCount(pObject, szProperty), iArraySize - 1);

      if (pObject->GetType()->FindPropertyByName(szProperty)->GetCategory() == nsPropertyCategory::Map)
      {
        for (nsInt32 i = 0; i < iArraySize - 1; i++)
        {
          const nsVariant& key = keys[i];
          const nsVariant& value = values[i];
          nsVariant newValue;
          NS_TEST_STATUS(pAccessor->GetValue(pObject, szProperty, newValue, key));
          NS_TEST_BOOL(newValue == value);
          NS_TEST_BOOL(expectedOutcome.Contains(newValue));
        }
      }
      else
      {
        for (nsInt32 i = 0; i < iArraySize - 1; i++)
        {
          nsVariant newValue;
          NS_TEST_STATUS(pAccessor->GetValue(pObject, szProperty, newValue, i));
          NS_TEST_BOOL(newValue == expectedOutcome[i]);
        }
      }

      NS_TEST_STATUS(doc.GetCommandHistory()->Undo());
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 1);
      NS_TEST_INT(pAccessor->GetCount(pObject, szProperty), iArraySize);
      CompareAgainstOriginalState(graph, pObject);

      NS_TEST_STATUS(doc.GetCommandHistory()->Redo());
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize + 1);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 0);
      NS_TEST_INT(pAccessor->GetCount(pObject, szProperty), iArraySize - 1);

      if (pObject->GetType()->FindPropertyByName(szProperty)->GetCategory() == nsPropertyCategory::Map)
      {
        for (nsInt32 i = 0; i < iArraySize - 1; i++)
        {
          const nsVariant& key = keys[i];
          const nsVariant& value = values[i];
          nsVariant newValue;
          NS_TEST_STATUS(pAccessor->GetValue(pObject, szProperty, newValue, key));
          NS_TEST_BOOL(newValue == value);
          NS_TEST_BOOL(expectedOutcome.Contains(newValue));
        }
      }
      else
      {
        for (nsInt32 i = 0; i < iArraySize - 1; i++)
        {
          nsVariant newValue;
          NS_TEST_STATUS(pAccessor->GetValue(pObject, szProperty, newValue, i));
          NS_TEST_BOOL(newValue == expectedOutcome[i]);
        }
      }
    };

    // StandardTypeArray
    {
      nsVariant expectedValues[2] = {2, 0};
      TestRemoveValue(pObjectTest, "StandardTypeArray", 0, nsArrayPtr<nsVariant>(expectedValues));
    }
    {
      nsVariant expectedValues[1] = {2};
      TestRemoveValue(pObjectTest, "StandardTypeArray", 1, nsArrayPtr<nsVariant>(expectedValues));
    }
    {
      TestRemoveValue(pObjectTest, "StandardTypeArray", 0, nsArrayPtr<nsVariant>());
    }
    // StandardTypeSet
    {
      nsVariant expectedValues[2] = {"B", "C"};
      TestRemoveValue(pObjectTest, "StandardTypeSet", 2, nsArrayPtr<nsVariant>(expectedValues));
    }
    {
      nsVariant expectedValues[1] = {"C"};
      TestRemoveValue(pObjectTest, "StandardTypeSet", 0, nsArrayPtr<nsVariant>(expectedValues));
    }
    {
      TestRemoveValue(pObjectTest, "StandardTypeSet", 0, nsArrayPtr<nsVariant>());
    }
    // StandardTypeMap
    {
      nsVariant expectedValues[2] = {1, 2};
      TestRemoveValue(pObjectTest, "StandardTypeMap", "A", nsArrayPtr<nsVariant>(expectedValues));
    }
    {
      nsVariant expectedValues[1] = {1};
      TestRemoveValue(pObjectTest, "StandardTypeMap", "C", nsArrayPtr<nsVariant>(expectedValues));
    }
    {
      TestRemoveValue(pObjectTest, "StandardTypeMap", "B", nsArrayPtr<nsVariant>());
    }
  }

  auto CreateGuid = [](const char* szType, nsInt32 iIndex) -> nsUuid
  {
    nsUuid A = nsUuid::MakeStableUuidFromString(szType);
    nsUuid B = nsUuid::MakeStableUuidFromInt(iIndex);
    A.CombineWithSeed(B);
    return A;
  };

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddObject")
  {
    auto TestAddObject = [&](const nsDocumentObject* pObject, const char* szProperty, nsVariant index, const nsRTTI* pType, nsUuid& inout_object)
    {
      nsAbstractObjectGraph graph;
      StoreOriginalState(graph, pObject);

      const nsUInt32 uiUndoHistorySize = doc.GetCommandHistory()->GetUndoStackSize();
      const nsInt32 iArraySize = pAccessor->GetCount(pObject, szProperty);

      pAccessor->StartTransaction("TestAddObject");
      NS_TEST_STATUS(pAccessor->AddObject(pObject, szProperty, index, pType, inout_object));
      pAccessor->FinishTransaction();
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize + 1);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 0);
      NS_TEST_INT(pAccessor->GetCount(pObject, szProperty), iArraySize + 1);
      nsVariant newValue;
      NS_TEST_STATUS(pAccessor->GetValue(pObject, szProperty, newValue, index));
      NS_TEST_BOOL(newValue == inout_object);

      NS_TEST_STATUS(doc.GetCommandHistory()->Undo());
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 1);
      NS_TEST_INT(pAccessor->GetCount(pObject, szProperty), iArraySize);
      CompareAgainstOriginalState(graph, pObject);

      NS_TEST_STATUS(doc.GetCommandHistory()->Redo());
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize + 1);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 0);
      NS_TEST_INT(pAccessor->GetCount(pObject, szProperty), iArraySize + 1);
      NS_TEST_STATUS(pAccessor->GetValue(pObject, szProperty, newValue, index));
      NS_TEST_BOOL(newValue == inout_object);
    };

    nsUuid A = CreateGuid("ClassArray", 0);
    nsUuid B = CreateGuid("ClassArray", 1);
    nsUuid C = CreateGuid("ClassArray", 2);

    TestAddObject(pObjectTest, "ClassArray", 0, nsGetStaticRTTI<OuterClass>(), A);
    TestAddObject(pObjectTest, "ClassArray", 1, nsGetStaticRTTI<OuterClass>(), C);
    TestAddObject(pObjectTest, "ClassArray", 1, nsGetStaticRTTI<OuterClass>(), B);

    A = CreateGuid("ClassPtrArray", 0);
    B = CreateGuid("ClassPtrArray", 1);
    C = CreateGuid("ClassPtrArray", 2);

    TestAddObject(pObjectTest, "ClassPtrArray", 0, nsGetStaticRTTI<OuterClass>(), A);
    TestAddObject(pObjectTest, "ClassPtrArray", 1, nsGetStaticRTTI<OuterClass>(), C);
    TestAddObject(pObjectTest, "ClassPtrArray", 1, nsGetStaticRTTI<OuterClass>(), B);

    A = CreateGuid("SubObjectSet", 0);
    B = CreateGuid("SubObjectSet", 1);
    C = CreateGuid("SubObjectSet", 2);

    TestAddObject(pObjectTest, "SubObjectSet", 0, nsGetStaticRTTI<nsObjectTest>(), A);
    TestAddObject(pObjectTest, "SubObjectSet", 1, nsGetStaticRTTI<nsObjectTest>(), C);
    TestAddObject(pObjectTest, "SubObjectSet", 1, nsGetStaticRTTI<nsObjectTest>(), B);

    A = CreateGuid("ClassMap", 0);
    B = CreateGuid("ClassMap", 1);
    C = CreateGuid("ClassMap", 2);

    TestAddObject(pObjectTest, "ClassMap", "A", nsGetStaticRTTI<OuterClass>(), A);
    TestAddObject(pObjectTest, "ClassMap", "C", nsGetStaticRTTI<OuterClass>(), C);
    TestAddObject(pObjectTest, "ClassMap", "B", nsGetStaticRTTI<OuterClass>(), B);

    A = CreateGuid("ClassPtrMap", 0);
    B = CreateGuid("ClassPtrMap", 1);
    C = CreateGuid("ClassPtrMap", 2);

    TestAddObject(pObjectTest, "ClassPtrMap", "A", nsGetStaticRTTI<OuterClass>(), A);
    TestAddObject(pObjectTest, "ClassPtrMap", "C", nsGetStaticRTTI<OuterClass>(), C);
    TestAddObject(pObjectTest, "ClassPtrMap", "B", nsGetStaticRTTI<OuterClass>(), B);
  }


  NS_TEST_BLOCK(nsTestBlock::Enabled, "MoveObject")
  {
    auto TestMoveObjectFailure = [&](const nsDocumentObject* pObject, const char* szProperty, nsVariant newIndex)
    {
      pAccessor->StartTransaction("MoveObject");
      NS_TEST_BOOL(pAccessor->MoveObject(pObject, pObject->GetParent(), szProperty, newIndex).Failed());
      pAccessor->CancelTransaction();
    };

    auto TestMoveObject = [&](const nsDocumentObject* pObject, const char* szProperty, nsVariant newIndex, nsArrayPtr<nsUuid> expectedOutcome)
    {
      nsAbstractObjectGraph graph;
      StoreOriginalState(graph, pObject->GetParent());

      const nsUInt32 uiUndoHistorySize = doc.GetCommandHistory()->GetUndoStackSize();
      const nsInt32 iArraySize = pAccessor->GetCount(pObject->GetParent(), szProperty);
      NS_TEST_INT(iArraySize, expectedOutcome.GetCount());

      nsDynamicArray<nsVariant> values;
      NS_TEST_STATUS(pAccessor->GetValues(pObject->GetParent(), szProperty, values));
      NS_TEST_INT(iArraySize, values.GetCount());

      pAccessor->StartTransaction("MoveObject");
      NS_TEST_STATUS(pAccessor->MoveObject(pObject, pObject->GetParent(), szProperty, newIndex));
      pAccessor->FinishTransaction();
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize + 1);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 0);
      NS_TEST_INT(pAccessor->GetCount(pObject->GetParent(), szProperty), iArraySize);

      for (nsInt32 i = 0; i < iArraySize; i++)
      {
        nsVariant newValue;
        NS_TEST_STATUS(pAccessor->GetValue(pObject->GetParent(), szProperty, newValue, i));
        NS_TEST_BOOL(newValue == expectedOutcome[i]);
      }

      NS_TEST_STATUS(doc.GetCommandHistory()->Undo());
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 1);
      NS_TEST_INT(pAccessor->GetCount(pObject->GetParent(), szProperty), iArraySize);
      CompareAgainstOriginalState(graph, pObject->GetParent());

      NS_TEST_STATUS(doc.GetCommandHistory()->Redo());
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize + 1);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 0);
      NS_TEST_INT(pAccessor->GetCount(pObject->GetParent(), szProperty), iArraySize);

      for (nsInt32 i = 0; i < iArraySize; i++)
      {
        nsVariant newValue;
        NS_TEST_STATUS(pAccessor->GetValue(pObject->GetParent(), szProperty, newValue, i));
        NS_TEST_BOOL(newValue == expectedOutcome[i]);
      }
    };

    nsUuid A = CreateGuid("ClassArray", 0);
    nsUuid B = CreateGuid("ClassArray", 1);
    nsUuid C = CreateGuid("ClassArray", 2);
    const nsDocumentObject* pA = pAccessor->GetObject(A);
    const nsDocumentObject* pB = pAccessor->GetObject(B);
    const nsDocumentObject* pC = pAccessor->GetObject(C);

    {
      // Move first element before or after itself (no-op)
      TestMoveObjectFailure(pA, "ClassArray", 0);
      TestMoveObjectFailure(pA, "ClassArray", 1);
      // Move last element before or after itself (no-op)
      TestMoveObjectFailure(pC, "ClassArray", 2);
      TestMoveObjectFailure(pC, "ClassArray", 3);
    }
    {
      // Move first element to the end.
      nsUuid expectedValues[3] = {B, C, A};
      TestMoveObject(pA, "ClassArray", 3, nsArrayPtr<nsUuid>(expectedValues));
    }
    {
      // Move last element to the front.
      nsUuid expectedValues[3] = {A, B, C};
      TestMoveObject(pA, "ClassArray", 0, nsArrayPtr<nsUuid>(expectedValues));
    }
    {
      // Move first element to the middle
      nsUuid expectedValues[3] = {B, A, C};
      TestMoveObject(pA, "ClassArray", 2, nsArrayPtr<nsUuid>(expectedValues));
    }
    {
      // Move last element to the middle
      nsUuid expectedValues[3] = {B, C, A};
      TestMoveObject(pC, "ClassArray", 1, nsArrayPtr<nsUuid>(expectedValues));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RemoveObject")
  {
    auto TestRemoveObject = [&](const nsDocumentObject* pObject, nsArrayPtr<nsUuid> expectedOutcome)
    {
      auto pParent = pObject->GetParent();
      nsString sProperty = pObject->GetParentProperty();

      nsAbstractObjectGraph graph;
      StoreOriginalState(graph, pParent);
      const nsUInt32 uiUndoHistorySize = doc.GetCommandHistory()->GetUndoStackSize();
      const nsInt32 iArraySize = pAccessor->GetCount(pParent, sProperty);
      NS_TEST_INT(iArraySize - 1, expectedOutcome.GetCount());


      nsDynamicArray<nsVariant> values;
      nsDynamicArray<nsVariant> keys;
      {
        NS_TEST_STATUS(pAccessor->GetValues(pParent, sProperty, values));
        NS_TEST_INT(iArraySize, values.GetCount());

        NS_TEST_STATUS(pAccessor->GetKeys(pParent, sProperty, keys));
        NS_TEST_INT(iArraySize, keys.GetCount());
        nsUInt32 uiIndex = keys.IndexOf(pObject->GetPropertyIndex());
        keys.RemoveAtAndSwap(uiIndex);
        values.RemoveAtAndSwap(uiIndex);
        NS_TEST_INT(iArraySize - 1, keys.GetCount());
      }

      pAccessor->StartTransaction("RemoveValue");
      NS_TEST_STATUS(pAccessor->RemoveObject(pObject));
      pAccessor->FinishTransaction();
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize + 1);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 0);
      NS_TEST_INT(pAccessor->GetCount(pParent, sProperty), iArraySize - 1);

      if (pParent->GetType()->FindPropertyByName(sProperty)->GetCategory() == nsPropertyCategory::Map)
      {
        for (nsInt32 i = 0; i < iArraySize - 1; i++)
        {
          const nsVariant& key = keys[i];
          const nsVariant& value = values[i];
          nsVariant newValue;
          NS_TEST_STATUS(pAccessor->GetValue(pParent, sProperty, newValue, key));
          NS_TEST_BOOL(newValue == value);
          NS_TEST_BOOL(expectedOutcome.Contains(newValue.Get<nsUuid>()));
        }
      }
      else
      {
        for (nsInt32 i = 0; i < iArraySize - 1; i++)
        {
          nsVariant newValue;
          NS_TEST_STATUS(pAccessor->GetValue(pParent, sProperty, newValue, i));
          NS_TEST_BOOL(newValue == expectedOutcome[i]);
        }
      }

      NS_TEST_STATUS(doc.GetCommandHistory()->Undo());
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 1);
      NS_TEST_INT(pAccessor->GetCount(pParent, sProperty), iArraySize);
      CompareAgainstOriginalState(graph, pParent);

      NS_TEST_STATUS(doc.GetCommandHistory()->Redo());
      NS_TEST_INT(doc.GetCommandHistory()->GetUndoStackSize(), uiUndoHistorySize + 1);
      NS_TEST_INT(doc.GetCommandHistory()->GetRedoStackSize(), 0);
      NS_TEST_INT(pAccessor->GetCount(pParent, sProperty), iArraySize - 1);

      if (pParent->GetType()->FindPropertyByName(sProperty)->GetCategory() == nsPropertyCategory::Map)
      {
        for (nsInt32 i = 0; i < iArraySize - 1; i++)
        {
          const nsVariant& key = keys[i];
          const nsVariant& value = values[i];
          nsVariant newValue;
          NS_TEST_STATUS(pAccessor->GetValue(pParent, sProperty, newValue, key));
          NS_TEST_BOOL(newValue == value);
          NS_TEST_BOOL(expectedOutcome.Contains(newValue.Get<nsUuid>()));
        }
      }
      else
      {
        for (nsInt32 i = 0; i < iArraySize - 1; i++)
        {
          nsVariant newValue;
          NS_TEST_STATUS(pAccessor->GetValue(pParent, sProperty, newValue, i));
          NS_TEST_BOOL(newValue == expectedOutcome[i]);
        }
      }
    };

    auto ClearContainer = [&](const char* szContainer)
    {
      nsUuid A = CreateGuid(szContainer, 0);
      nsUuid B = CreateGuid(szContainer, 1);
      nsUuid C = CreateGuid(szContainer, 2);
      const nsDocumentObject* pA = pAccessor->GetObject(A);
      const nsDocumentObject* pB = pAccessor->GetObject(B);
      const nsDocumentObject* pC = pAccessor->GetObject(C);
      {
        nsUuid expectedValues[2] = {B, C};
        TestRemoveObject(pA, nsArrayPtr<nsUuid>(expectedValues));
      }
      {
        nsUuid expectedValues[1] = {C};
        TestRemoveObject(pB, nsArrayPtr<nsUuid>(expectedValues));
      }
      {
        TestRemoveObject(pC, nsArrayPtr<nsUuid>());
      }
    };

    ClearContainer("ClassArray");
    ClearContainer("ClassPtrArray");
    ClearContainer("SubObjectSet");
    ClearContainer("ClassMap");
    ClearContainer("ClassPtrMap");
  }
}
