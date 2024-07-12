#include <ToolsFoundationTest/ToolsFoundationTestPCH.h>

#include <ToolsFoundationTest/Object/TestObjectManager.h>
#include <ToolsFoundationTest/Reflection/ReflectionTestClasses.h>

NS_CREATE_SIMPLE_TEST_GROUP(DocumentObject);

NS_CREATE_SIMPLE_TEST(DocumentObject, DocumentObjectManager)
{
  nsTestDocumentObjectManager manager;
  nsDocumentObject* pObject = nullptr;
  nsDocumentObject* pChildObject = nullptr;
  nsDocumentObject* pChildren[4] = {nullptr};
  nsDocumentObject* pSubElementObject[4] = {nullptr};

  NS_TEST_BLOCK(nsTestBlock::Enabled, "DocumentObject")
  {
    NS_TEST_BOOL(manager.CanAdd(nsObjectTest::GetStaticRTTI(), nullptr, "", 0).m_Result.Succeeded());
    pObject = manager.CreateObject(nsObjectTest::GetStaticRTTI());
    manager.AddObject(pObject, nullptr, "", 0);

    const char* szProperty = "SubObjectSet";
    NS_TEST_BOOL(manager.CanAdd(OuterClass::GetStaticRTTI(), pObject, szProperty, 0).m_Result.Failed());
    NS_TEST_BOOL(manager.CanAdd(nsObjectTest::GetStaticRTTI(), pObject, szProperty, 0).m_Result.Succeeded());
    pChildObject = manager.CreateObject(nsObjectTest::GetStaticRTTI());
    manager.AddObject(pChildObject, pObject, "SubObjectSet", 0);
    NS_TEST_INT(pObject->GetTypeAccessor().GetCount(szProperty), 1);

    NS_TEST_BOOL(manager.CanAdd(OuterClass::GetStaticRTTI(), pObject, "ClassPtrArray", 0).m_Result.Succeeded());
    NS_TEST_BOOL(manager.CanAdd(ExtendedOuterClass::GetStaticRTTI(), pObject, "ClassPtrArray", 0).m_Result.Succeeded());
    NS_TEST_BOOL(!manager.CanAdd(nsReflectedClass::GetStaticRTTI(), pObject, "ClassPtrArray", 0).m_Result.Succeeded());

    for (nsInt32 i = 0; i < NS_ARRAY_SIZE(pChildren); i++)
    {
      NS_TEST_BOOL(manager.CanAdd(nsObjectTest::GetStaticRTTI(), pChildObject, szProperty, i).m_Result.Succeeded());
      pChildren[i] = manager.CreateObject(nsObjectTest::GetStaticRTTI());
      manager.AddObject(pChildren[i], pChildObject, szProperty, i);
      NS_TEST_INT(pChildObject->GetTypeAccessor().GetCount(szProperty), i + 1);
    }
    NS_TEST_INT(pChildObject->GetTypeAccessor().GetCount(szProperty), 4);

    NS_TEST_BOOL_MSG(manager.CanMove(pObject, pChildObject, szProperty, 0).m_Result.Failed(), "Can't move to own child");
    NS_TEST_BOOL_MSG(manager.CanMove(pChildren[1], pChildObject, szProperty, 1).m_Result.Failed(), "Can't move before onself");
    NS_TEST_BOOL_MSG(manager.CanMove(pChildren[1], pChildObject, szProperty, 2).m_Result.Failed(), "Can't move after oneself");
    NS_TEST_BOOL_MSG(manager.CanMove(pChildren[1], pChildren[1], szProperty, 0).m_Result.Failed(), "Can't move into yourself");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "DocumentSubElementObject")
  {
    const char* szProperty = "ClassArray";
    for (nsInt32 i = 0; i < NS_ARRAY_SIZE(pSubElementObject); i++)
    {
      NS_TEST_BOOL(manager.CanAdd(OuterClass::GetStaticRTTI(), pObject, szProperty, i).m_Result.Succeeded());
      pSubElementObject[i] = manager.CreateObject(OuterClass::GetStaticRTTI());
      manager.AddObject(pSubElementObject[i], pObject, szProperty, i);
      NS_TEST_INT(pObject->GetTypeAccessor().GetCount(szProperty), i + 1);
    }

    NS_TEST_BOOL(manager.CanRemove(pSubElementObject[0]).m_Result.Succeeded());
    manager.RemoveObject(pSubElementObject[0]);
    manager.DestroyObject(pSubElementObject[0]);
    pSubElementObject[0] = nullptr;
    NS_TEST_INT(pObject->GetTypeAccessor().GetCount(szProperty), 3);

    nsVariant value = pObject->GetTypeAccessor().GetValue(szProperty, 0);
    NS_TEST_BOOL(value.IsA<nsUuid>() && value.Get<nsUuid>() == pSubElementObject[1]->GetGuid());
    value = pObject->GetTypeAccessor().GetValue(szProperty, 1);
    NS_TEST_BOOL(value.IsA<nsUuid>() && value.Get<nsUuid>() == pSubElementObject[2]->GetGuid());
    value = pObject->GetTypeAccessor().GetValue(szProperty, 2);
    NS_TEST_BOOL(value.IsA<nsUuid>() && value.Get<nsUuid>() == pSubElementObject[3]->GetGuid());

    NS_TEST_BOOL(manager.CanMove(pSubElementObject[1], pObject, szProperty, 2).m_Result.Succeeded());
    manager.MoveObject(pSubElementObject[1], pObject, szProperty, 2);
    NS_TEST_BOOL(manager.CanMove(pSubElementObject[3], pObject, szProperty, 0).m_Result.Succeeded());
    manager.MoveObject(pSubElementObject[3], pObject, szProperty, 0);

    value = pObject->GetTypeAccessor().GetValue(szProperty, 0);
    NS_TEST_BOOL(value.IsA<nsUuid>() && value.Get<nsUuid>() == pSubElementObject[3]->GetGuid());
    value = pObject->GetTypeAccessor().GetValue(szProperty, 1);
    NS_TEST_BOOL(value.IsA<nsUuid>() && value.Get<nsUuid>() == pSubElementObject[2]->GetGuid());
    value = pObject->GetTypeAccessor().GetValue(szProperty, 2);
    NS_TEST_BOOL(value.IsA<nsUuid>() && value.Get<nsUuid>() == pSubElementObject[1]->GetGuid());

    NS_TEST_BOOL(manager.CanRemove(pSubElementObject[3]).m_Result.Succeeded());
    manager.RemoveObject(pSubElementObject[3]);
    manager.DestroyObject(pSubElementObject[3]);
    pSubElementObject[3] = nullptr;
    NS_TEST_INT(pObject->GetTypeAccessor().GetCount(szProperty), 2);

    value = pObject->GetTypeAccessor().GetValue(szProperty, 0);
    NS_TEST_BOOL(value.IsA<nsUuid>() && value.Get<nsUuid>() == pSubElementObject[2]->GetGuid());
    value = pObject->GetTypeAccessor().GetValue(szProperty, 1);
    NS_TEST_BOOL(value.IsA<nsUuid>() && value.Get<nsUuid>() == pSubElementObject[1]->GetGuid());

    NS_TEST_BOOL(manager.CanMove(pSubElementObject[1], pChildObject, szProperty, 0).m_Result.Succeeded());
    manager.MoveObject(pSubElementObject[1], pChildObject, szProperty, 0);
    NS_TEST_BOOL(manager.CanMove(pSubElementObject[2], pChildObject, szProperty, 0).m_Result.Succeeded());
    manager.MoveObject(pSubElementObject[2], pChildObject, szProperty, 0);

    NS_TEST_INT(pObject->GetTypeAccessor().GetCount(szProperty), 0);
    NS_TEST_INT(pChildObject->GetTypeAccessor().GetCount(szProperty), 2);

    value = pChildObject->GetTypeAccessor().GetValue(szProperty, 0);
    NS_TEST_BOOL(value.IsA<nsUuid>() && value.Get<nsUuid>() == pSubElementObject[2]->GetGuid());
    value = pChildObject->GetTypeAccessor().GetValue(szProperty, 1);
    NS_TEST_BOOL(value.IsA<nsUuid>() && value.Get<nsUuid>() == pSubElementObject[1]->GetGuid());
  }

  manager.DestroyAllObjects();
}
