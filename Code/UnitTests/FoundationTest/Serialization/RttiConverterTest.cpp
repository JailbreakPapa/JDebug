#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/BinarySerializer.h>
#include <Foundation/Serialization/DdlSerializer.h>
#include <Foundation/Serialization/ReflectionSerializer.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <FoundationTest/Reflection/ReflectionTestClasses.h>

NS_CREATE_SIMPLE_TEST_GROUP(Serialization);

class TestContext : public nsRttiConverterContext
{
public:
  virtual nsInternal::NewInstance<void> CreateObject(const nsUuid& guid, const nsRTTI* pRtti) override
  {
    auto pObj = pRtti->GetAllocator()->Allocate<void>();
    RegisterObject(guid, pRtti, pObj);
    return pObj;
  }

  virtual void DeleteObject(const nsUuid& guid) override
  {
    auto object = GetObjectByGUID(guid);
    object.m_pType->GetAllocator()->Deallocate(object.m_pObject);

    UnregisterObject(guid);
  }
};

template <typename T>
void TestSerialize(T* pObject)
{
  nsAbstractObjectGraph graph;
  TestContext context;
  nsRttiConverterWriter conv(&graph, &context, true, true);

  const nsRTTI* pRtti = nsGetStaticRTTI<T>();
  const nsUuid guid = nsUuid::MakeUuid();

  context.RegisterObject(guid, pRtti, pObject);
  nsAbstractObjectNode* pNode = conv.AddObjectToGraph(pRtti, pObject, "root");

  NS_TEST_BOOL(pNode->GetGuid() == guid);
  NS_TEST_STRING(pNode->GetType(), pRtti->GetTypeName());
  NS_TEST_INT(pNode->GetProperties().GetCount(), pNode->GetProperties().GetCount());

  {
    nsContiguousMemoryStreamStorage storage;
    nsMemoryStreamWriter writer(&storage);
    nsMemoryStreamReader reader(&storage);

    nsAbstractGraphDdlSerializer::Write(writer, &graph);

    nsStringBuilder sData, sData2;
    sData.SetSubString_ElementCount((const char*)storage.GetData(), storage.GetStorageSize32());


    nsRttiConverterReader convRead(&graph, &context);
    auto* pRootNode = graph.GetNodeByName("root");
    NS_TEST_BOOL(pRootNode != nullptr);

    T target;
    convRead.ApplyPropertiesToObject(pRootNode, pRtti, &target);
    NS_TEST_BOOL(target == *pObject);

    // Overwrite again to test for leaks as existing values have to be removed first by nsRttiConverterReader.
    convRead.ApplyPropertiesToObject(pRootNode, pRtti, &target);
    NS_TEST_BOOL(target == *pObject);

    {
      T clone;
      nsReflectionSerializer::Clone(pObject, &clone, pRtti);
      NS_TEST_BOOL(clone == *pObject);
      NS_TEST_BOOL(nsReflectionUtils::IsEqual(&clone, pObject, pRtti));
    }

    {
      T* pClone = nsReflectionSerializer::Clone(pObject);
      NS_TEST_BOOL(*pClone == *pObject);
      NS_TEST_BOOL(nsReflectionUtils::IsEqual(pClone, pObject));
      // Overwrite again to test for leaks as existing values have to be removed first by clone.
      nsReflectionSerializer::Clone(pObject, pClone, pRtti);
      NS_TEST_BOOL(*pClone == *pObject);
      NS_TEST_BOOL(nsReflectionUtils::IsEqual(pClone, pObject, pRtti));
      pRtti->GetAllocator()->Deallocate(pClone);
    }

    nsAbstractObjectGraph graph2;
    nsAbstractGraphDdlSerializer::Read(reader, &graph2).IgnoreResult();

    nsContiguousMemoryStreamStorage storage2;
    nsMemoryStreamWriter writer2(&storage2);

    nsAbstractGraphDdlSerializer::Write(writer2, &graph2);
    sData2.SetSubString_ElementCount((const char*)storage2.GetData(), storage2.GetStorageSize32());

    NS_TEST_BOOL(sData == sData2);
  }

  {
    nsContiguousMemoryStreamStorage storage;
    nsMemoryStreamWriter writer(&storage);
    nsMemoryStreamReader reader(&storage);

    nsAbstractGraphBinarySerializer::Write(writer, &graph);

    nsRttiConverterReader convRead(&graph, &context);
    auto* pRootNode = graph.GetNodeByName("root");
    NS_TEST_BOOL(pRootNode != nullptr);

    T target;
    convRead.ApplyPropertiesToObject(pRootNode, pRtti, &target);
    NS_TEST_BOOL(target == *pObject);

    nsAbstractObjectGraph graph2;
    nsAbstractGraphBinarySerializer::Read(reader, &graph2);

    nsContiguousMemoryStreamStorage storage2;
    nsMemoryStreamWriter writer2(&storage2);

    nsAbstractGraphBinarySerializer::Write(writer2, &graph2);

    NS_TEST_INT(storage.GetStorageSize32(), storage2.GetStorageSize32());

    if (storage.GetStorageSize32() == storage2.GetStorageSize32())
    {
      NS_TEST_BOOL(nsMemoryUtils::RawByteCompare(storage.GetData(), storage2.GetData(), storage.GetStorageSize32()) == 0);
    }
  }
}

NS_CREATE_SIMPLE_TEST(Serialization, RttiConverter)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "PODs")
  {
    nsTestStruct t1;
    t1.m_fFloat1 = 5.0f;
    t1.m_UInt8 = 222;
    t1.m_variant = "A";
    t1.m_Angle = nsAngle::MakeFromDegree(5);
    t1.m_DataBuffer.PushBack(1);
    t1.m_DataBuffer.PushBack(5);
    t1.m_vVec3I = nsVec3I32(0, 1, 333);
    TestSerialize(&t1);

    {
      nsTestStruct clone;
      nsReflectionSerializer::Clone(&t1, &clone, nsGetStaticRTTI<nsTestStruct>());
      NS_TEST_BOOL(t1 == clone);
      NS_TEST_BOOL(nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestStruct>()));
      clone.m_variant = "Test";
      NS_TEST_BOOL(!nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestStruct>()));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "EmbededStruct")
  {
    nsTestClass1 t1;
    t1.m_Color = nsColor::Yellow;
    t1.m_Struct.m_fFloat1 = 5.0f;
    t1.m_Struct.m_UInt8 = 222;
    t1.m_Struct.m_variant = "A";
    t1.m_Struct.m_Angle = nsAngle::MakeFromDegree(5);
    t1.m_Struct.m_DataBuffer.PushBack(1);
    t1.m_Struct.m_DataBuffer.PushBack(5);
    t1.m_Struct.m_vVec3I = nsVec3I32(0, 1, 333);
    TestSerialize(&t1);

    {
      nsTestClass1 clone;
      nsReflectionSerializer::Clone(&t1, &clone, nsGetStaticRTTI<nsTestClass1>());
      NS_TEST_BOOL(t1 == clone);
      NS_TEST_BOOL(nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestClass1>()));
      clone.m_Struct.m_DataBuffer[1] = 6;
      NS_TEST_BOOL(!nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestClass1>()));
      clone.m_Struct.m_DataBuffer[1] = 5;
      clone.m_Struct.m_variant = nsVec3(1, 2, 3);
      NS_TEST_BOOL(!nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestClass1>()));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Enum")
  {
    nsTestEnumStruct t1;
    t1.m_enum = nsExampleEnum::Value2;
    t1.m_enumClass = nsExampleEnum::Value3;
    t1.SetEnum(nsExampleEnum::Value2);
    t1.SetEnumClass(nsExampleEnum::Value3);
    TestSerialize(&t1);

    {
      nsTestEnumStruct clone;
      nsReflectionSerializer::Clone(&t1, &clone, nsGetStaticRTTI<nsTestEnumStruct>());
      NS_TEST_BOOL(t1 == clone);
      NS_TEST_BOOL(nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestEnumStruct>()));
      clone.m_enum = nsExampleEnum::Value3;
      NS_TEST_BOOL(!nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestEnumStruct>()));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Bitflags")
  {
    nsTestBitflagsStruct t1;
    t1.m_bitflagsClass.SetValue(0);
    t1.SetBitflagsClass(nsExampleBitflags::Value1 | nsExampleBitflags::Value2);
    TestSerialize(&t1);

    {
      nsTestBitflagsStruct clone;
      nsReflectionSerializer::Clone(&t1, &clone, nsGetStaticRTTI<nsTestBitflagsStruct>());
      NS_TEST_BOOL(t1 == clone);
      NS_TEST_BOOL(nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestBitflagsStruct>()));
      clone.m_bitflagsClass = nsExampleBitflags::Value1;
      NS_TEST_BOOL(!nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestBitflagsStruct>()));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Derived Class")
  {
    nsTestClass2 t1;
    t1.m_Color = nsColor::Yellow;
    t1.m_Struct.m_fFloat1 = 5.0f;
    t1.m_Struct.m_UInt8 = 222;
    t1.m_Struct.m_variant = "A";
    t1.m_Struct.m_Angle = nsAngle::MakeFromDegree(5);
    t1.m_Struct.m_DataBuffer.PushBack(1);
    t1.m_Struct.m_DataBuffer.PushBack(5);
    t1.m_Struct.m_vVec3I = nsVec3I32(0, 1, 333);
    t1.m_Time = nsTime::MakeFromSeconds(22.2f);
    t1.m_enumClass = nsExampleEnum::Value3;
    t1.m_bitflagsClass = nsExampleBitflags::Value1 | nsExampleBitflags::Value2;
    t1.m_array.PushBack(40.0f);
    t1.m_array.PushBack(-1.5f);
    t1.m_Variant = nsVec4(1, 2, 3, 4);
    t1.SetCharPtr("Hello");
    t1.SetString("World");
    t1.SetStringView("!!!");
    TestSerialize(&t1);

    {
      nsTestClass2 clone;
      nsReflectionSerializer::Clone(&t1, &clone, nsGetStaticRTTI<nsTestClass2>());
      NS_TEST_BOOL(t1 == clone);
      NS_TEST_BOOL(nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestClass2>()));
      clone.m_Struct.m_DataBuffer[1] = 6;
      NS_TEST_BOOL(!nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestClass2>()));
      clone.m_Struct.m_DataBuffer[1] = 5;
      t1.m_array.PushBack(-1.33f);
      NS_TEST_BOOL(!nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestClass2>()));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Arrays")
  {
    nsTestArrays t1;
    t1.m_Hybrid.PushBack(4.5f);
    t1.m_Hybrid.PushBack(2.3f);
    t1.m_HybridChar.PushBack("Test");

    nsTestStruct3 ts;
    ts.m_fFloat1 = 5.0f;
    ts.m_UInt8 = 22;
    t1.m_Dynamic.PushBack(ts);
    t1.m_Dynamic.PushBack(ts);
    t1.m_Deque.PushBack(nsTestArrays());
    TestSerialize(&t1);

    {
      nsTestArrays clone;
      nsReflectionSerializer::Clone(&t1, &clone, nsGetStaticRTTI<nsTestArrays>());
      NS_TEST_BOOL(t1 == clone);
      NS_TEST_BOOL(nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestArrays>()));
      clone.m_Dynamic.PushBack(nsTestStruct3());
      NS_TEST_BOOL(!nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestArrays>()));
      clone.m_Dynamic.PopBack();
      clone.m_Hybrid.PushBack(444.0f);
      NS_TEST_BOOL(!nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestArrays>()));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Sets")
  {
    nsTestSets t1;
    t1.m_SetMember.Insert(0);
    t1.m_SetMember.Insert(5);
    t1.m_SetMember.Insert(-33);
    t1.m_SetAccessor.Insert(-0.0f);
    t1.m_SetAccessor.Insert(5.4f);
    t1.m_SetAccessor.Insert(-33.0f);
    t1.m_Deque.PushBack(3);
    t1.m_Deque.PushBack(33);
    t1.m_Array.PushBack("Test");
    t1.m_Array.PushBack("Bla");
    TestSerialize(&t1);

    {
      nsTestSets clone;
      nsReflectionSerializer::Clone(&t1, &clone, nsGetStaticRTTI<nsTestSets>());
      NS_TEST_BOOL(t1 == clone);
      NS_TEST_BOOL(nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestSets>()));
      clone.m_SetMember.Insert(12);
      NS_TEST_BOOL(!nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestSets>()));
      clone.m_SetMember.Remove(12);
      clone.m_Array.PushBack("Bla2");
      NS_TEST_BOOL(!nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestSets>()));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Pointer")
  {
    nsTestPtr t1;
    t1.m_sString = "Ttttest";
    t1.m_pArrays = NS_DEFAULT_NEW(nsTestArrays);
    t1.m_pArraysDirect = NS_DEFAULT_NEW(nsTestArrays);
    t1.m_ArrayPtr.PushBack(NS_DEFAULT_NEW(nsTestArrays));
    t1.m_SetPtr.Insert(NS_DEFAULT_NEW(nsTestSets));
    TestSerialize(&t1);

    {
      nsTestPtr clone;
      nsReflectionSerializer::Clone(&t1, &clone, nsGetStaticRTTI<nsTestPtr>());
      NS_TEST_BOOL(t1 == clone);
      NS_TEST_BOOL(nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestPtr>()));
      clone.m_SetPtr.GetIterator().Key()->m_Deque.PushBack(42);
      NS_TEST_BOOL(!nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestPtr>()));
      clone.m_SetPtr.GetIterator().Key()->m_Deque.PopBack();
      clone.m_ArrayPtr[0]->m_Hybrid.PushBack(123.0f);
      NS_TEST_BOOL(!nsReflectionUtils::IsEqual(&t1, &clone, nsGetStaticRTTI<nsTestPtr>()));
    }
  }
}
