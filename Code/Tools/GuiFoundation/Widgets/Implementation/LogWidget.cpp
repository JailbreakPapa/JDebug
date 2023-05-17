#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Models/LogModel.moc.h>
#include <GuiFoundation/Widgets/LogWidget.moc.h>
#include <QClipboard>
#include <QKeyEvent>

wdQtLogWidget::wdQtLogWidget(QWidget* pParent)
  : QWidget(pParent)
{
  setupUi(this);

  m_pLog = new wdQtLogModel(this);
  ListViewLog->setModel(m_pLog);
  ListViewLog->setUniformItemSizes(true);
  ListViewLog->installEventFilter(this);
  connect(m_pLog, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex& parent, int iFirst, int iLast) { ScrollToBottomIfAtEnd(iFirst); });

  const int logIndex = ((int)wdLogMsgType::All - (int)wdLogMsgType::InfoMsg);
  ComboFilter->setCurrentIndex(logIndex);
}

wdQtLogWidget::~wdQtLogWidget() = default;

void wdQtLogWidget::ShowControls(bool bShow)
{
  ButtonClearLog->setVisible(bShow);
  ComboFilter->setVisible(bShow);
  Search->setVisible(bShow);
}

wdQtLogModel* wdQtLogWidget::GetLog()
{
  return m_pLog;
}

wdQtSearchWidget* wdQtLogWidget::GetSearchWidget()
{
  return Search;
}

void wdQtLogWidget::SetLogLevel(wdLogMsgType::Enum logLevel)
{
  WD_ASSERT_DEBUG(logLevel >= (int)wdLogMsgType::ErrorMsg && logLevel <= wdLogMsgType::All, "Invalid log level set.");
  ComboFilter->setCurrentIndex((int)wdLogMsgType::All - (int)logLevel);
}

wdLogMsgType::Enum wdQtLogWidget::GetLogLevel() const
{
  int index = ComboFilter->currentIndex();
  return (wdLogMsgType::Enum)((int)wdLogMsgType::All - index);
}

bool wdQtLogWidget::eventFilter(QObject* pObject, QEvent* pEvent)
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

void wdQtLogWidget::ScrollToBottomIfAtEnd(int iNumElements)
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

void wdQtLogWidget::on_ButtonClearLog_clicked()
{
  m_pLog->Clear();
}

void wdQtLogWidget::on_Search_textChanged(const QString& text)
{
  m_pLog->SetSearchText(text.toUtf8().data());
}

void wdQtLogWidget::on_ComboFilter_currentIndexChanged(int index)
{
  const wdLogMsgType::Enum LogLevel = (wdLogMsgType::Enum)((int)wdLogMsgType::All - index);
  m_pLog->SetLogLevel(LogLevel);
}
