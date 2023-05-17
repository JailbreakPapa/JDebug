#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <ToolsFoundation/Object/DocumentObjectVisitor.h>

wdDocumentObjectVisitor::wdDocumentObjectVisitor(
  const wdDocumentObjectManager* pManager, const char* szChildrenProperty /*= "Children"*/, const char* szRootProperty /*= "Children"*/)
  : m_pManager(pManager)
  , m_sChildrenProperty(szChildrenProperty)
  , m_sRootProperty(szRootProperty)
{
  const wdAbstractProperty* pRootProp = m_pManager->GetRootObject()->GetType()->FindPropertyByName(szRootProperty);
  WD_ASSERT_DEV(pRootProp, "Given root property '{0}' does not exist on root object", szRootProperty);
  WD_ASSERT_DEV(pRootProp->GetCategory() == wdPropertyCategory::Set || pRootProp->GetCategory() == wdPropertyCategory::Array,
    "Traverser only works on arrays and sets.");

  // const wdAbstractProperty* pChildProp = pRootProp->GetSpecificType()->FindPropertyByName(szChildrenProperty);
  // WD_ASSERT_DEV(pChildProp, "Given child property '{0}' does not exist", szChildrenProperty);
  // WD_ASSERT_DEV(pChildProp->GetCategory() == wdPropertyCategory::Set || pRootProp->GetCategory() == wdPropertyCategory::Array, "Traverser
  // only works on arrays and sets.");
}

void wdDocumentObjectVisitor::Visit(const wdDocumentObject* pObject, bool bVisitStart, VisitorFunction function)
{
  const char* szProperty = m_sChildrenProperty;
  if (pObject == nullptr || pObject == m_pManager->GetRootObject())
  {
    pObject = m_pManager->GetRootObject();
    szProperty = m_sRootProperty;
  }

  if (!bVisitStart || function(pObject))
  {
    TraverseChildren(pObject, szProperty, function);
  }
}

void wdDocumentObjectVisitor::TraverseChildren(const wdDocumentObject* pObject, const char* szProperty, VisitorFunction& function)
{
  const wdInt32 iChildren = pObject->GetTypeAccessor().GetCount(szProperty);
  for (wdInt32 i = 0; i < iChildren; i++)
  {
    wdVariant obj = pObject->GetTypeAccessor().GetValue(szProperty, i);
    WD_ASSERT_DEBUG(obj.IsValid() && obj.IsA<wdUuid>(), "null obj found during traversal.");
    const wdDocumentObject* pChild = m_pManager->GetObject(obj.Get<wdUuid>());
    if (function(pChild))
    {
      TraverseChildren(pChild, m_sChildrenProperty, function);
    }
  }
}
