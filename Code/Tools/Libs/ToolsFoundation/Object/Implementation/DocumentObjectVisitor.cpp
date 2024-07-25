#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <ToolsFoundation/Object/DocumentObjectVisitor.h>

nsDocumentObjectVisitor::nsDocumentObjectVisitor(
  const nsDocumentObjectManager* pManager, nsStringView sChildrenProperty /*= "Children"*/, nsStringView sRootProperty /*= "Children"*/)
  : m_pManager(pManager)
  , m_sChildrenProperty(sChildrenProperty)
  , m_sRootProperty(sRootProperty)
{
  const nsAbstractProperty* pRootProp = m_pManager->GetRootObject()->GetType()->FindPropertyByName(sRootProperty);
  NS_ASSERT_DEV(pRootProp, "Given root property '{0}' does not exist on root object", sRootProperty);
  NS_ASSERT_DEV(pRootProp->GetCategory() == nsPropertyCategory::Set || pRootProp->GetCategory() == nsPropertyCategory::Array,
    "Traverser only works on arrays and sets.");

  // const nsAbstractProperty* pChildProp = pRootProp->GetSpecificType()->FindPropertyByName(szChildrenProperty);
  // NS_ASSERT_DEV(pChildProp, "Given child property '{0}' does not exist", szChildrenProperty);
  // NS_ASSERT_DEV(pChildProp->GetCategory() == nsPropertyCategory::Set || pRootProp->GetCategory() == nsPropertyCategory::Array, "Traverser
  // only works on arrays and sets.");
}

void nsDocumentObjectVisitor::Visit(const nsDocumentObject* pObject, bool bVisitStart, VisitorFunction function)
{
  nsStringView sProperty = m_sChildrenProperty;
  if (pObject == nullptr || pObject == m_pManager->GetRootObject())
  {
    pObject = m_pManager->GetRootObject();
    sProperty = m_sRootProperty;
  }

  if (!bVisitStart || function(pObject))
  {
    TraverseChildren(pObject, sProperty, function);
  }
}

void nsDocumentObjectVisitor::TraverseChildren(const nsDocumentObject* pObject, nsStringView sProperty, VisitorFunction& function)
{
  const nsInt32 iChildren = pObject->GetTypeAccessor().GetCount(sProperty);
  for (nsInt32 i = 0; i < iChildren; i++)
  {
    nsVariant obj = pObject->GetTypeAccessor().GetValue(sProperty, i);
    NS_ASSERT_DEBUG(obj.IsValid() && obj.IsA<nsUuid>(), "null obj found during traversal.");
    const nsDocumentObject* pChild = m_pManager->GetObject(obj.Get<nsUuid>());
    if (function(pChild))
    {
      TraverseChildren(pChild, m_sChildrenProperty, function);
    }
  }
}
