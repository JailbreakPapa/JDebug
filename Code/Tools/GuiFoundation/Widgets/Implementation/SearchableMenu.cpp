#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Models/TreeSearchFilterModel.moc.h>
#include <GuiFoundation/Widgets/SearchWidget.moc.h>
#include <GuiFoundation/Widgets/SearchableMenu.moc.h>
#include <QBoxLayout>
#include <QMouseEvent>
#include <QStandardItemModel>
#include <QTreeWidget>


class QNullWidget : public QWidget
{
public:
  QNullWidget()
    : QWidget(nullptr)
  {
  }

protected:
  virtual void mousePressEvent(QMouseEvent* e) override { e->accept(); }
  virtual void mouseReleaseEvent(QMouseEvent* e) override { e->accept(); }
  virtual void mouseDoubleClickEvent(QMouseEvent* e) override { e->accept(); }
};


wdQtSearchableMenu::wdQtSearchableMenu(QObject* pParent)
  : QWidgetAction(pParent)
{
  m_pGroup = new QNullWidget();
  m_pGroup->setLayout(new QVBoxLayout(m_pGroup));
  m_pGroup->setContentsMargins(0, 0, 0, 0);

  m_pSearch = new wdQtSearchWidget(m_pGroup);
  connect(m_pSearch, &wdQtSearchWidget::enterPressed, this, &wdQtSearchableMenu::OnEnterPressed);
  connect(m_pSearch, &wdQtSearchWidget::specialKeyPressed, this, &wdQtSearchableMenu::OnSpecialKeyPressed);
  connect(m_pSearch, &wdQtSearchWidget::textChanged, this, &wdQtSearchableMenu::OnSearchChanged);
  connect(m_pSearch, &wdQtSearchWidget::visibleEvent, this, &wdQtSearchableMenu::OnShow);

  m_pGroup->layout()->addWidget(m_pSearch);
  m_pGroup->layout()->setContentsMargins(1, 1, 1, 1);

  m_pItemModel = new QStandardItemModel(m_pGroup);

  m_pFilterModel = new wdQtTreeSearchFilterModel(m_pGroup);
  m_pFilterModel->SetIncludeChildren(true);
  m_pFilterModel->setSourceModel(m_pItemModel);

  m_pTreeView = new QTreeView(m_pGroup);
  m_pTreeView->installEventFilter(this);
  m_pTreeView->setModel(m_pFilterModel);
  m_pTreeView->setHeaderHidden(true);
  m_pTreeView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  m_pTreeView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
  connect(m_pTreeView, &QTreeView::activated, this, &wdQtSearchableMenu::OnItemActivated);


  m_pGroup->layout()->addWidget(m_pTreeView);

  setDefaultWidget(m_pGroup);
}

QStandardItem* wdQtSearchableMenu::CreateCategoryMenu(const char* szCategory)
{
  if (wdStringUtils::IsNullOrEmpty(szCategory))
    return m_pItemModel->invisibleRootItem();

  auto it = m_Hierarchy.Find(szCategory);
  if (it.IsValid())
    return it.Value();

  wdStringBuilder sPath = szCategory;
  sPath.PathParentDirectory();
  sPath.Trim("/");

  QStandardItem* pParentMenu = m_pItemModel->invisibleRootItem();

  if (!sPath.IsEmpty())
  {
    pParentMenu = CreateCategoryMenu(sPath);
  }

  sPath = szCategory;
  sPath = sPath.GetFileName();

  QStandardItem* pThisItem = new QStandardItem(sPath.GetData());
  pThisItem->setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);

  pParentMenu->appendRow(pThisItem);

  m_Hierarchy[szCategory] = pThisItem;

  return pThisItem;
}

bool wdQtSearchableMenu::SelectFirstLeaf(QModelIndex parent)
{
  const int iRows = m_pFilterModel->rowCount(parent);

  for (int i = 0; i < iRows; ++i)
  {
    QModelIndex child = m_pFilterModel->index(i, 0, parent);

    if (!m_pFilterModel->hasChildren(child))
    {
      if (m_pFilterModel->data(child, Qt::UserRole + 1).isValid())
      {
        // set this one item as the new selection
        m_pTreeView->selectionModel()->setCurrentIndex(
          child, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::SelectionFlag::SelectCurrent);
        m_pTreeView->scrollTo(child, QAbstractItemView::EnsureVisible);
        return true;
      }
    }
    else
    {
      if (SelectFirstLeaf(child))
        return true;
    }
  }

  return false;
}

bool wdQtSearchableMenu::eventFilter(QObject* pObject, QEvent* event)
{
  if (pObject == m_pTreeView)
  {
    if (event->type() == QEvent::KeyPress)
    {
      QKeyEvent* pKeyEvent = (QKeyEvent*)(event);

      if (pKeyEvent->key() == Qt::Key_Tab || pKeyEvent->key() == Qt::Key_Backtab)
      {
        m_pSearch->setFocus();
        return true;
      }
    }
  }

  return false;
}

void wdQtSearchableMenu::AddItem(const char* szName, const QVariant& variant, QIcon icon)
{
  QStandardItem* pParent = m_pItemModel->invisibleRootItem();

  const char* szLastCat = wdStringUtils::FindLastSubString(szName, "/");
  if (szLastCat != nullptr)
  {
    wdStringBuilder sCategory;
    sCategory.SetSubString_FromTo(szName, szLastCat);

    pParent = CreateCategoryMenu(sCategory);

    szName = szLastCat + 1;
  }

  QStandardItem* pThisItem = new QStandardItem(szName);
  pThisItem->setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
  pThisItem->setData(variant, Qt::UserRole + 1);
  pThisItem->setIcon(icon);

  pParent->appendRow(pThisItem);
}

QString wdQtSearchableMenu::GetSearchText() const
{
  return m_pSearch->text();
}

void wdQtSearchableMenu::Finalize(const QString& sSearchText)
{
  m_pItemModel->sort(0);

  m_pSearch->setText(sSearchText);
  m_pSearch->setFocus();
}

void wdQtSearchableMenu::OnItemActivated(const QModelIndex& index)
{
  if (!index.isValid())
    return;

  QModelIndex realIndex = m_pFilterModel->mapToSource(index);

  QString sName = m_pItemModel->data(realIndex, Qt::DisplayRole).toString();
  QVariant variant = m_pItemModel->data(realIndex, Qt::UserRole + 1);

  // potentially only a folder item
  if (!variant.isValid())
    return;

  Q_EMIT MenuItemTriggered(sName, variant);
}

void wdQtSearchableMenu::OnEnterPressed()
{
  auto selection = m_pTreeView->selectionModel()->selection();

  if (selection.size() != 1)
    return;

  OnItemActivated(selection.indexes()[0]);
}

void wdQtSearchableMenu::OnSpecialKeyPressed(Qt::Key key)
{
  if (key == Qt::Key_Down || key == Qt::Key_Up || key == Qt::Key_Tab || key == Qt::Key_Backtab)
  {
    m_pTreeView->setFocus();
  }
}

void wdQtSearchableMenu::OnSearchChanged(const QString& text)
{
  m_pFilterModel->SetFilterText(text);

  if (!text.isEmpty())
  {
    SelectFirstLeaf(QModelIndex());
    m_pTreeView->expandAll();
  }

  Q_EMIT SearchTextChanged(text);
}

void wdQtSearchableMenu::OnShow()
{
  m_pSearch->setFocus();
}
