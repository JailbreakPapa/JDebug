#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Models/LogModel.moc.h>
#include <QColor>
#include <QThread>


wdQtLogModel::wdQtLogModel(QObject* pParent)
  : QAbstractItemModel(pParent)
{
  m_bIsValid = true;
  m_LogLevel = wdLogMsgType::InfoMsg;
}

void wdQtLogModel::Invalidate()
{
  if (!m_bIsValid)
    return;

  beginResetModel();
  m_bIsValid = false;
  endResetModel();
}

void wdQtLogModel::Clear()
{
  if (m_AllMessages.IsEmpty())
    return;

  {
    WD_LOCK(m_NewMessagesMutex);

    m_uiNumErrors = 0;
    m_uiNumSeriousWarnings = 0;
    m_uiNumWarnings = 0;

    m_AllMessages.Clear();
    m_VisibleMessages.Clear();
    m_BlockQueue.Clear();
    Invalidate();
    m_bIsValid = true;
  }

  Q_EMIT NewErrorsOrWarnings(nullptr, false);
}

void wdQtLogModel::SetLogLevel(wdLogMsgType::Enum logLevel)
{
  if (m_LogLevel == logLevel)
    return;

  m_LogLevel = logLevel;
  Invalidate();
}

void wdQtLogModel::SetSearchText(const char* szText)
{
  if (m_sSearchText == szText)
    return;

  m_sSearchText = szText;
  Invalidate();
}

void wdQtLogModel::AddLogMsg(const wdLogEntry& msg)
{
  {
    WD_LOCK(m_NewMessagesMutex);
    m_NewMessages.PushBack(msg);
  }

  // always queue the message processing, otherwise it can happen that an error during this
  // triggers recursive logging, which is forbidden
  QMetaObject::invokeMethod(this, "ProcessNewMessages", Qt::ConnectionType::QueuedConnection);

  return;
}

bool wdQtLogModel::IsFiltered(const wdLogEntry& lm) const
{
  if (lm.m_Type < wdLogMsgType::None)
    return false;

  if (lm.m_Type > m_LogLevel)
    return true;

  if (m_sSearchText.IsEmpty())
    return false;

  if (lm.m_sMsg.FindSubString_NoCase(m_sSearchText.GetData()))
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////
// wdQtLogModel QAbstractItemModel functions
////////////////////////////////////////////////////////////////////////

QVariant wdQtLogModel::data(const QModelIndex& index, int iRole) const
{
  if (!index.isValid() || index.column() != 0)
    return QVariant();

  UpdateVisibleEntries();

  const wdInt32 iRow = index.row();
  if (iRow < 0 || iRow >= (wdInt32)m_VisibleMessages.GetCount())
    return QVariant();

  const wdLogEntry& msg = *m_VisibleMessages[iRow];

  switch (iRole)
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    {
      return QString::fromUtf8(msg.m_sMsg.GetData());
    }
    case Qt::ForegroundRole:
    {
      switch (msg.m_Type)
      {
        case wdLogMsgType::BeginGroup:
          return wdToQtColor(wdColorScheme::LightUI(wdColorScheme::Gray));
        case wdLogMsgType::EndGroup:
          return wdToQtColor(wdColorScheme::DarkUI(wdColorScheme::Gray));
        case wdLogMsgType::ErrorMsg:
          return wdToQtColor(wdColorScheme::LightUI(wdColorScheme::Red));
        case wdLogMsgType::SeriousWarningMsg:
          return wdToQtColor(wdColorScheme::LightUI(wdColorScheme::Orange));
        case wdLogMsgType::WarningMsg:
          return wdToQtColor(wdColorScheme::LightUI(wdColorScheme::Yellow));
        case wdLogMsgType::SuccessMsg:
          return wdToQtColor(wdColorScheme::LightUI(wdColorScheme::Green));
        case wdLogMsgType::DevMsg:
          return wdToQtColor(wdColorScheme::LightUI(wdColorScheme::Blue));
        case wdLogMsgType::DebugMsg:
          return wdToQtColor(wdColorScheme::LightUI(wdColorScheme::Cyan));
        default:
          return QVariant();
      }
    }

    default:
      return QVariant();
  }
}

Qt::ItemFlags wdQtLogModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
    return Qt::ItemFlags();

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant wdQtLogModel::headerData(int iSection, Qt::Orientation orientation, int iRole) const
{
  return QVariant();
}

QModelIndex wdQtLogModel::index(int iRow, int iColumn, const QModelIndex& parent) const
{
  if (parent.isValid() || iColumn != 0)
    return QModelIndex();

  return createIndex(iRow, iColumn, iRow);
}

QModelIndex wdQtLogModel::parent(const QModelIndex& index) const
{
  return QModelIndex();
}

int wdQtLogModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid())
    return 0;

  UpdateVisibleEntries();

  return (int)m_VisibleMessages.GetCount();
}

int wdQtLogModel::columnCount(const QModelIndex& parent) const
{
  return 1;
}


void wdQtLogModel::ProcessNewMessages()
{
  bool bNewErrors = false;
  wdStringBuilder sLatestWarning;
  wdStringBuilder sLatestError;

  {
    WD_LOCK(m_NewMessagesMutex);
    wdStringBuilder s;
    for (const auto& msg : m_NewMessages)
    {
      m_AllMessages.PushBack(msg);

      if (msg.m_Type == wdLogMsgType::BeginGroup || msg.m_Type == wdLogMsgType::EndGroup)
      {
        s.Printf("%*s<<< %s", msg.m_uiIndentation, "", msg.m_sMsg.GetData());

        if (msg.m_Type == wdLogMsgType::EndGroup)
        {
          s.AppendFormat(" ({0} sec) >>>", wdArgF(msg.m_fSeconds, 3));
        }
        else if (!msg.m_sTag.IsEmpty())
        {
          s.Append(" (", msg.m_sTag, ") >>>");
        }
        else
        {
          s.Append(" >>>");
        }

        m_AllMessages.PeekBack().m_sMsg = s;
      }
      else
      {
        s.Printf("%*s%s", 4 * msg.m_uiIndentation, "", msg.m_sMsg.GetData());
        m_AllMessages.PeekBack().m_sMsg = s;

        if (msg.m_Type == wdLogMsgType::ErrorMsg)
        {
          sLatestError = msg.m_sMsg;
          bNewErrors = true;
          ++m_uiNumErrors;
        }
        else if (msg.m_Type == wdLogMsgType::SeriousWarningMsg)
        {
          sLatestWarning = msg.m_sMsg;
          bNewErrors = true;
          ++m_uiNumSeriousWarnings;
        }
        else if (msg.m_Type == wdLogMsgType::WarningMsg)
        {
          sLatestWarning = msg.m_sMsg;
          bNewErrors = true;
          ++m_uiNumWarnings;
        }
      }


      // if the message would not be shown anyway, don't trigger an update
      if (IsFiltered(msg))
        continue;

      if (msg.m_Type == wdLogMsgType::BeginGroup)
      {
        m_BlockQueue.PushBack(&m_AllMessages.PeekBack());
        continue;
      }
      else if (msg.m_Type == wdLogMsgType::EndGroup)
      {
        if (!m_BlockQueue.IsEmpty())
        {
          m_BlockQueue.PopBack();
          continue;
        }
      }

      for (auto pMsg : m_BlockQueue)
      {
        beginInsertRows(QModelIndex(), m_VisibleMessages.GetCount(), m_VisibleMessages.GetCount());
        m_VisibleMessages.PushBack(pMsg);
        endInsertRows();
      }

      m_BlockQueue.Clear();

      beginInsertRows(QModelIndex(), m_VisibleMessages.GetCount(), m_VisibleMessages.GetCount());
      m_VisibleMessages.PushBack(&m_AllMessages.PeekBack());
      endInsertRows();
    }

    m_NewMessages.Clear();
  }

  if (bNewErrors)
  {
    if (!sLatestError.IsEmpty())
    {
      Q_EMIT NewErrorsOrWarnings(sLatestError, true);
    }
    else
    {
      Q_EMIT NewErrorsOrWarnings(sLatestWarning, false);
    }
  }
}

void wdQtLogModel::UpdateVisibleEntries() const
{
  if (m_bIsValid)
    return;


  m_bIsValid = true;
  m_VisibleMessages.Clear();
  for (const auto& msg : m_AllMessages)
  {
    if (IsFiltered(msg))
      continue;

    if (msg.m_Type == wdLogMsgType::EndGroup)
    {
      if (!m_VisibleMessages.IsEmpty())
      {
        if (m_VisibleMessages.PeekBack()->m_Type == wdLogMsgType::BeginGroup)
          m_VisibleMessages.PopBack();
        else
          m_VisibleMessages.PushBack(&msg);
      }
    }
    else
    {
      m_VisibleMessages.PushBack(&msg);
    }
  }
}
