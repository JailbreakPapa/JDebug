#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Dialogs/PickDocumentObjectDlg.moc.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

nsQtPickDocumentObjectDlg::nsQtPickDocumentObjectDlg(QWidget* pParent, const nsArrayPtr<Element>& objects, const nsUuid& currentObject)
  : QDialog(pParent)
  , m_Objects(objects)
  , m_CurrentObject(currentObject)
{
  setupUi(this);

  UpdateTable();
}


void nsQtPickDocumentObjectDlg::UpdateTable()
{
  QTreeWidget* pTree = ObjectTree;
  pTree->clear();
  pTree->setExpandsOnDoubleClick(false);

  nsMap<const nsDocumentObjectManager*, QTreeWidgetItem*> roots;

  nsStringBuilder name, temp;
  QTreeWidgetItem* pSelected = nullptr;

  for (auto& e : m_Objects)
  {
    const nsDocumentObjectManager* pManager = e.m_pObject->GetDocumentObjectManager();

    bool existed = false;
    auto itRootItem = roots.FindOrAdd(pManager, &existed);

    QTreeWidgetItem* pRoot = itRootItem.Value();

    if (!existed)
    {
      pRoot = new QTreeWidgetItem();
      itRootItem.Value() = pRoot;

      temp = pManager->GetDocument()->GetDocumentPath();
      name = temp.GetFileNameAndExtension();

      pTree->addTopLevelItem(pRoot);

      pRoot->setText(0, name.GetData());
      pRoot->setExpanded(true);
    }

    QTreeWidgetItem* pItem = new QTreeWidgetItem();
    pItem->setText(0, e.m_sDisplayName.GetData());
    pItem->setData(0, Qt::UserRole, QVariant::fromValue<void*>((void*)e.m_pObject));

    pRoot->addChild(pItem);

    if (e.m_pObject->GetGuid() == m_CurrentObject)
    {
      pSelected = pItem;
    }
  }

  if (pSelected != nullptr)
  {
    pSelected->setSelected(true);
  }
}

void nsQtPickDocumentObjectDlg::on_ObjectTree_itemDoubleClicked(QTreeWidgetItem* pItem, int column)
{
  if (pItem->parent() == nullptr)
    return;

  QVariant var = pItem->data(0, Qt::UserRole);
  if (!var.isValid())
    return;

  m_pPickedObject = reinterpret_cast<const nsDocumentObject*>(var.value<void*>());

  if (m_pPickedObject != nullptr)
  {
    accept();
  }
}
