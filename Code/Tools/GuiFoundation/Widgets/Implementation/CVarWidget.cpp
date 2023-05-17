#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Configuration/CVar.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <GuiFoundation/Widgets/CVarWidget.moc.h>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

wdQtCVarWidget::wdQtCVarWidget(QWidget* pParent)
  : QWidget(pParent)
{
  setupUi(this);

  m_pItemModel = new wdQtCVarModel(this);

  m_pFilterModel = new QSortFilterProxyModel(this);
  m_pFilterModel->setSourceModel(m_pItemModel);

  m_pItemDelegate = new wdQtCVarItemDelegate(this);
  m_pItemDelegate->m_pModel = m_pItemModel;

  CVarsView->setModel(m_pFilterModel);
  CVarsView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  CVarsView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
  CVarsView->setHeaderHidden(false);
  CVarsView->setEditTriggers(QAbstractItemView::EditTrigger::CurrentChanged | QAbstractItemView::EditTrigger::SelectedClicked);
  CVarsView->setItemDelegateForColumn(1, m_pItemDelegate);

  connect(SearchWidget, &wdQtSearchWidget::textChanged, this, &wdQtCVarWidget::SearchTextChanged);
  connect(ConsoleInput, &wdQtSearchWidget::enterPressed, this, &wdQtCVarWidget::ConsoleEnterPressed);
  connect(ConsoleInput, &wdQtSearchWidget::specialKeyPressed, this, &wdQtCVarWidget::ConsoleSpecialKeyPressed);

  m_Console.Events().AddEventHandler(wdMakeDelegate(&wdQtCVarWidget::OnConsoleEvent, this));

  ConsoleInput->setPlaceholderText("> TAB to auto-complete");
}

wdQtCVarWidget::~wdQtCVarWidget() {}

void wdQtCVarWidget::Clear()
{
  QPointer<QWidget> pFocusWidget = QApplication::focusWidget();
  clearFocus();
  m_pItemModel->BeginResetModel();
  m_pItemModel->EndResetModel();

  if (pFocusWidget)
    pFocusWidget->setFocus();
}

void wdQtCVarWidget::RebuildCVarUI(const wdMap<wdString, wdCVarWidgetData>& cvars)
{
  // for now update and rebuild are the same
  UpdateCVarUI(cvars);
}

void wdQtCVarWidget::UpdateCVarUI(const wdMap<wdString, wdCVarWidgetData>& cvars)
{
  int row = 0;

  m_pItemModel->BeginResetModel();

  for (auto it = cvars.GetIterator(); it.IsValid(); ++it, ++row)
  {
    it.Value().m_bNewEntry = false;

    auto item = m_pItemModel->CreateEntry(it.Key().GetData());
    item->m_sDescription = it.Value().m_sDescription;
    item->m_sPlugin = it.Value().m_sPlugin;

    switch (it.Value().m_uiType)
    {
      case wdCVarType::Bool:
        item->m_Value = it.Value().m_bValue;
        break;
      case wdCVarType::Float:
        item->m_Value = it.Value().m_fValue;
        break;
      case wdCVarType::Int:
        item->m_Value = it.Value().m_iValue;
        break;
      case wdCVarType::String:
        item->m_Value = it.Value().m_sValue;
        break;
    }
  }

  m_pItemModel->EndResetModel();

  CVarsView->expandAll();
  CVarsView->resizeColumnToContents(0);
  CVarsView->resizeColumnToContents(1);
}

void wdQtCVarWidget::AddConsoleStrings(const wdStringBuilder& sEncoded)
{
  wdHybridArray<wdStringView, 64> lines;
  sEncoded.Split(false, lines, ";;");

  wdStringBuilder tmp;

  for (auto l : lines)
  {
    l.Shrink(4, 0); // skip the line type number at the front (for now)

    if (l.StartsWith("<"))
    {
      l.Shrink(1, 0);
      ConsoleInput->setText(l.GetData(tmp));
    }
    else
    {
      GetConsole().AddConsoleString(l);
    }
  }
}

void wdQtCVarWidget::SearchTextChanged(const QString& text)
{
  m_pFilterModel->setRecursiveFilteringEnabled(true);
  m_pFilterModel->setFilterRole(Qt::UserRole);
  m_pFilterModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

  QRegularExpression e;

  QString st = "(?=.*" + text + ".*)";
  st.replace(" ", ".*)(?=.*");

  e.setPattern(st);
  e.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
  m_pFilterModel->setFilterRegularExpression(e);
  CVarsView->expandAll();
}

void wdQtCVarWidget::ConsoleEnterPressed()
{
  m_Console.AddToInputHistory(ConsoleInput->text().toUtf8().data());
  m_Console.ExecuteCommand(ConsoleInput->text().toUtf8().data());
  ConsoleInput->setText("");
}

void wdQtCVarWidget::ConsoleSpecialKeyPressed(Qt::Key key)
{
  if (key == Qt::Key_Tab)
  {
    wdStringBuilder input = ConsoleInput->text().toUtf8().data();

    if (m_Console.AutoComplete(input))
    {
      ConsoleInput->setText(input.GetData());
    }
  }
  if (key == Qt::Key_Up)
  {
    wdStringBuilder input = ConsoleInput->text().toUtf8().data();
    m_Console.RetrieveInputHistory(1, input);
    ConsoleInput->setText(input.GetData());
  }
  if (key == Qt::Key_Down)
  {
    wdStringBuilder input = ConsoleInput->text().toUtf8().data();
    m_Console.RetrieveInputHistory(-1, input);
    ConsoleInput->setText(input.GetData());
  }
  if (key == Qt::Key_F2)
  {
    if (m_Console.GetInputHistory().GetCount() >= 1)
    {
      m_Console.ExecuteCommand(m_Console.GetInputHistory()[0]);
    }
  }
  if (key == Qt::Key_F3)
  {
    if (m_Console.GetInputHistory().GetCount() >= 2)
    {
      m_Console.ExecuteCommand(m_Console.GetInputHistory()[1]);
    }
  }
}

void wdQtCVarWidget::OnConsoleEvent(const wdConsoleEvent& e)
{
  if (e.m_Type == wdConsoleEvent::Type::OutputLineAdded)
  {
    QString t = ConsoleOutput->toPlainText();
    t += e.m_AddedpConsoleString->m_sText;
    t += "\n";
    ConsoleOutput->setPlainText(t);
    ConsoleOutput->verticalScrollBar()->setValue(ConsoleOutput->verticalScrollBar()->maximum());
  }
}

wdQtCVarModel::wdQtCVarModel(wdQtCVarWidget* pOwner)
  : QAbstractItemModel(pOwner)
{
  m_pOwner = pOwner;
}

wdQtCVarModel::~wdQtCVarModel() = default;

void wdQtCVarModel::BeginResetModel()
{
  beginResetModel();
  m_RootEntries.Clear();
  m_AllEntries.Clear();
}

void wdQtCVarModel::EndResetModel()
{
  endResetModel();
}

QVariant wdQtCVarModel::headerData(int iSection, Qt::Orientation orientation, int iRole /*= Qt::DisplayRole*/) const
{
  if (iRole == Qt::DisplayRole)
  {
    switch (iSection)
    {
      case 0:
        return "Name";

      case 1:
        return "Value";

      case 2:
        return "Description";

      case 3:
        return "Plugin";
    }
  }

  return QAbstractItemModel::headerData(iSection, orientation, iRole);
}

bool wdQtCVarModel::setData(const QModelIndex& index, const QVariant& value, int iRole /*= Qt::EditRole*/)
{
  if (index.column() == 1 && iRole == Qt::EditRole)
  {
    wdQtCVarModel::Entry* e = reinterpret_cast<wdQtCVarModel::Entry*>(index.internalId());

    switch (e->m_Value.GetType())
    {
      case wdVariantType::Bool:
        e->m_Value = value.toBool();
        m_pOwner->onBoolChanged(e->m_sFullName, value.toBool());
        break;
      case wdVariantType::Int32:
        e->m_Value = value.toInt();
        m_pOwner->onIntChanged(e->m_sFullName, value.toInt());
        break;
      case wdVariantType::Float:
        e->m_Value = value.toFloat();
        m_pOwner->onFloatChanged(e->m_sFullName, value.toFloat());
        break;
      case wdVariantType::String:
        e->m_Value = value.toString().toUtf8().data();
        m_pOwner->onStringChanged(e->m_sFullName, value.toString().toUtf8().data());
        break;
      default:
        break;
    }
  }

  return QAbstractItemModel::setData(index, value, iRole);
}

QVariant wdQtCVarModel::data(const QModelIndex& index, int iRole) const
{
  if (!index.isValid())
    return QVariant();

  wdQtCVarModel::Entry* e = reinterpret_cast<wdQtCVarModel::Entry*>(index.internalId());

  if (iRole == Qt::UserRole)
  {
    return e->m_sFullName.GetData();
  }

  if (iRole == Qt::DisplayRole)
  {
    switch (index.column())
    {
      case 0:
        return e->m_sDisplayString;

      case 1:
        return e->m_Value.ConvertTo<wdString>().GetData();

      case 2:
        return e->m_sDescription;
    }
  }

  if (iRole == Qt::DecorationRole && index.column() == 0)
  {
    if (e->m_Value.IsValid())
    {
      return wdQtUiServices::GetCachedIconResource(":/GuiFoundation/Icons/CVar.png");
    }
  }

  if (iRole == Qt::ToolTipRole)
  {
    if (e->m_Value.IsValid())
    {
      if (index.column() == 0)
      {
        return QString(e->m_sFullName) + " | " + e->m_sPlugin;
      }

      if (index.column() == 2)
      {
        return e->m_sDescription;
      }
    }
  }

  if (iRole == Qt::EditRole && index.column() == 1)
  {
    switch (e->m_Value.GetType())
    {
      case wdVariantType::Bool:
        return e->m_Value.Get<bool>();
      case wdVariantType::Int32:
        return e->m_Value.Get<wdInt32>();
      case wdVariantType::Float:
        return e->m_Value.ConvertTo<double>();
      case wdVariantType::String:
        return e->m_Value.Get<wdString>().GetData();
      default:
        break;
    }
  }
  return QVariant();
}

Qt::ItemFlags wdQtCVarModel::flags(const QModelIndex& index) const
{
  if (index.column() == 1)
  {
    wdQtCVarModel::Entry* e = reinterpret_cast<wdQtCVarModel::Entry*>(index.internalId());

    if (e->m_Value.IsValid())
    {
      return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsEditable;
    }
  }

  return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
}

QModelIndex wdQtCVarModel::index(int iRow, int iColumn, const QModelIndex& parent /*= QModelIndex()*/) const
{
  if (parent.isValid())
  {
    wdQtCVarModel::Entry* e = reinterpret_cast<wdQtCVarModel::Entry*>(parent.internalId());
    return createIndex(iRow, iColumn, const_cast<wdQtCVarModel::Entry*>(e->m_ChildEntries[iRow]));
  }
  else
  {
    return createIndex(iRow, iColumn, const_cast<wdQtCVarModel::Entry*>(m_RootEntries[iRow]));
  }
}

QModelIndex wdQtCVarModel::parent(const QModelIndex& index) const
{
  if (!index.isValid())
    return QModelIndex();

  wdQtCVarModel::Entry* e = reinterpret_cast<wdQtCVarModel::Entry*>(index.internalId());

  if (e->m_pParentEntry == nullptr)
    return QModelIndex();

  wdQtCVarModel::Entry* p = e->m_pParentEntry;

  // find the parent entry's row index
  if (p->m_pParentEntry == nullptr)
  {
    // if the parent has no parent itself, it is a root entry and we need to search that array
    for (wdUInt32 row = 0; row < m_RootEntries.GetCount(); ++row)
    {
      if (m_RootEntries[row] == p)
      {
        return createIndex(row, index.column(), p);
      }
    }
  }
  else
  {
    // if the parent has a parent itself, search that array for the row index
    for (wdUInt32 row = 0; row < p->m_pParentEntry->m_ChildEntries.GetCount(); ++row)
    {
      if (p->m_pParentEntry->m_ChildEntries[row] == e)
      {
        return createIndex(row, index.column(), p);
      }
    }
  }

  return QModelIndex();
}

int wdQtCVarModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
  if (parent.isValid())
  {
    wdQtCVarModel::Entry* e = reinterpret_cast<wdQtCVarModel::Entry*>(parent.internalId());

    return (int)e->m_ChildEntries.GetCount();
  }
  else
  {
    return (int)m_RootEntries.GetCount();
  }
}

int wdQtCVarModel::columnCount(const QModelIndex& index /*= QModelIndex()*/) const
{
  return 3;
}

wdQtCVarModel::Entry* wdQtCVarModel::CreateEntry(const char* szName)
{
  wdStringBuilder tmp = szName;
  wdStringBuilder tmp2;

  wdHybridArray<wdStringView, 8> pieces;
  tmp.Split(false, pieces, ".", "_");

  wdDynamicArray<Entry*>* vals = &m_RootEntries;
  Entry* parentEntry = nullptr;

  for (wdUInt32 p = 0; p < pieces.GetCount(); ++p)
  {
    QString piece = pieces[p].GetData(tmp2);
    for (wdUInt32 v = 0; v < vals->GetCount(); ++v)
    {
      if ((*vals)[v]->m_sDisplayString == piece)
      {
        parentEntry = (*vals)[v];
        vals = &((*vals)[v]->m_ChildEntries);
        goto found;
      }
    }

    {
      auto& newItem = m_AllEntries.ExpandAndGetRef();
      newItem.m_sFullName = szName;
      newItem.m_sDisplayString = piece;
      newItem.m_pParentEntry = parentEntry;

      vals->PushBack(&newItem);

      parentEntry = &newItem;
      vals = &newItem.m_ChildEntries;
    }
  found:;
  }

  return parentEntry;
}

QWidget* wdQtCVarItemDelegate::createEditor(QWidget* pParent, const QStyleOptionViewItem& option, const QModelIndex& idx) const
{
  m_Index = static_cast<const QSortFilterProxyModel*>(idx.model())->mapToSource(idx);
  wdQtCVarModel::Entry* e = reinterpret_cast<wdQtCVarModel::Entry*>(m_Index.internalPointer());

  if (!e->m_Value.IsValid())
    return nullptr;

  if (e->m_Value.IsA<bool>())
  {
    QComboBox* ret = new QComboBox(pParent);
    ret->addItem("true");
    ret->addItem("false");

    connect(ret, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboChanged(int)));
    return ret;
  }

  if (e->m_Value.IsA<wdInt32>())
  {
    QLineEdit* ret = new QLineEdit(pParent);
    ret->setValidator(new QIntValidator(ret));
    return ret;
  }

  if (e->m_Value.IsA<float>())
  {
    QLineEdit* ret = new QLineEdit(pParent);
    auto val = new QDoubleValidator(ret);
    val->setDecimals(3);
    ret->setValidator(val);
    return ret;
  }

  if (e->m_Value.IsA<wdString>())
  {
    QLineEdit* ret = new QLineEdit(pParent);
    return ret;
  }

  return nullptr;
}

void wdQtCVarItemDelegate::setEditorData(QWidget* pEditor, const QModelIndex& index) const
{
  QVariant value = index.model()->data(index, Qt::EditRole);

  if (QLineEdit* pLine = qobject_cast<QLineEdit*>(pEditor))
  {
    if (value.type() == QVariant::Type::Double)
    {
      double f = value.toDouble();

      pLine->setText(QString("%1").arg(f, 0, (char)103, 3));
    }
    else
    {
      pLine->setText(value.toString());
    }

    pLine->selectAll();
  }

  if (QComboBox* pLine = qobject_cast<QComboBox*>(pEditor))
  {
    pLine->setCurrentIndex(value.toBool() ? 0 : 1);
    pLine->showPopup();
  }
}

void wdQtCVarItemDelegate::setModelData(QWidget* pEditor, QAbstractItemModel* pModel, const QModelIndex& index) const
{
  if (QLineEdit* pLine = qobject_cast<QLineEdit*>(pEditor))
  {
    pModel->setData(index, pLine->text(), Qt::EditRole);
  }

  if (QComboBox* pLine = qobject_cast<QComboBox*>(pEditor))
  {
    pModel->setData(index, pLine->currentText(), Qt::EditRole);
  }
}

void wdQtCVarItemDelegate::onComboChanged(int)
{
  setModelData(qobject_cast<QWidget*>(sender()), m_pModel, m_Index);
}
