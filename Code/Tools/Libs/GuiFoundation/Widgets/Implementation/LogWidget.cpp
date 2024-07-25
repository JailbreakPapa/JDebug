#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Models/LogModel.moc.h>
#include <GuiFoundation/Widgets/LogWidget.moc.h>
#include <QClipboard>
#include <QKeyEvent>

nsMap<nsString, nsQtLogWidget::LogItemContextActionCallback> nsQtLogWidget::s_LogCallbacks;

nsQtLogWidget::nsQtLogWidget(QWidget* pParent)
  : QWidget(pParent)
{
  setupUi(this);

  m_pLog = new nsQtLogModel(this);
  ListViewLog->setModel(m_pLog);
  ListViewLog->setUniformItemSizes(true);
  ListViewLog->installEventFilter(this);
  connect(m_pLog, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex& parent, int iFirst, int iLast)
    { ScrollToBottomIfAtEnd(iFirst); });
  connect(ListViewLog, &QAbstractItemView::doubleClicked, this, &nsQtLogWidget::OnItemDoubleClicked);

  const int logIndex = ((int)nsLogMsgType::All - (int)nsLogMsgType::InfoMsg);
  ComboFilter->setCurrentIndex(logIndex);
}

nsQtLogWidget::~nsQtLogWidget() = default;

void nsQtLogWidget::ShowControls(bool bShow)
{
  ButtonClearLog->setVisible(bShow);
  ComboFilter->setVisible(bShow);
  Search->setVisible(bShow);
}

nsQtLogModel* nsQtLogWidget::GetLog()
{
  return m_pLog;
}

nsQtSearchWidget* nsQtLogWidget::GetSearchWidget()
{
  return Search;
}

void nsQtLogWidget::SetLogLevel(nsLogMsgType::Enum logLevel)
{
  NS_ASSERT_DEBUG(logLevel >= (int)nsLogMsgType::ErrorMsg && logLevel <= nsLogMsgType::All, "Invalid log level set.");
  ComboFilter->setCurrentIndex((int)nsLogMsgType::All - (int)logLevel);
}

nsLogMsgType::Enum nsQtLogWidget::GetLogLevel() const
{
  int index = ComboFilter->currentIndex();
  return (nsLogMsgType::Enum)((int)nsLogMsgType::All - index);
}

bool nsQtLogWidget::eventFilter(QObject* pObject, QEvent* pEvent)
{
  if (pObject == ListViewLog)
  {
    if (pEvent->type() == QEvent::ShortcutOverride)
    {
      // Intercept copy
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(pEvent);
      if (keyEvent->matches(QKeySequence::StandardKey::Copy))
      {
        keyEvent->accept();
        return true;
      }
    }
    else if (pEvent->type() == QEvent::KeyPress)
    {
      // Copy entire selection
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(pEvent);
      if (keyEvent->matches(QKeySequence::StandardKey::Copy))
      {
        QModelIndexList selection = ListViewLog->selectionModel()->selectedRows(0);
        QStringList sTemp;
        sTemp.reserve(selection.count());
        for (const QModelIndex& index : selection)
        {
          QString sLine = m_pLog->data(index, Qt::DisplayRole).toString();
          sTemp.push_back(sLine);
        }

        QString sFullText = sTemp.join(QStringLiteral("\n"));
        QApplication::clipboard()->setText(sFullText);
        keyEvent->accept();
        return true;
      }
    }
  }

  return false;
}

bool nsQtLogWidget::AddLogItemContextActionCallback(const nsStringView& sName, const LogItemContextActionCallback& logCallback)
{
  if (sName.IsEmpty())
    return false;

  if (s_LogCallbacks.Contains(sName))
    return false;

  s_LogCallbacks[sName] = logCallback;
  return true;
}

bool nsQtLogWidget::RemoveLogItemContextActionCallback(const nsStringView& sName)
{
  if (sName.IsEmpty())
    return false;

  return s_LogCallbacks.Remove(sName);
}

void nsQtLogWidget::ScrollToBottomIfAtEnd(int iNumElements)
{
  if (ListViewLog->selectionModel()->hasSelection())
  {
    if (ListViewLog->selectionModel()->selectedIndexes()[0].row() + 1 >= iNumElements)
    {
      ListViewLog->selectionModel()->clearSelection();
      ListViewLog->scrollToBottom();
    }
  }
  else
    ListViewLog->scrollToBottom();
}

void nsQtLogWidget::on_ButtonClearLog_clicked()
{
  m_pLog->Clear();
}

void nsQtLogWidget::on_Search_textChanged(const QString& text)
{
  m_pLog->SetSearchText(text.toUtf8().data());
}

void nsQtLogWidget::on_ComboFilter_currentIndexChanged(int index)
{
  const nsLogMsgType::Enum LogLevel = (nsLogMsgType::Enum)((int)nsLogMsgType::All - index);
  m_pLog->SetLogLevel(LogLevel);
}

void nsQtLogWidget::OnItemDoubleClicked(QModelIndex idx)
{
  const nsString sLine(m_pLog->data(idx, Qt::DisplayRole).toString().toUtf8().data());

  for (auto const& callback : s_LogCallbacks)
  {
    callback.Value()(sLine);
  }
}
