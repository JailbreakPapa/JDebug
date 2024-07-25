#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Models/LogModel.moc.h>
#include <QColor>
#include <QThread>


nsQtLogModel::nsQtLogModel(QObject* pParent)
  : QAbstractItemModel(pParent)
{
  m_bIsValid = true;
  m_LogLevel = nsLogMsgType::InfoMsg;
}

void nsQtLogModel::Invalidate()
{
  if (!m_bIsValid)
    return;

  beginResetModel();
  m_bIsValid = false;
  endResetModel();
}

void nsQtLogModel::Clear()
{
  if (m_AllMessages.IsEmpty())
    return;

  {
    NS_LOCK(m_NewMessagesMutex);

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

void nsQtLogModel::SetLogLevel(nsLogMsgType::Enum logLevel)
{
  if (m_LogLevel == logLevel)
    return;

  m_LogLevel = logLevel;
  Invalidate();
}

void nsQtLogModel::SetSearchText(const char* szText)
{
  if (m_sSearchText == szText)
    return;

  m_sSearchText = szText;
  Invalidate();
}

void nsQtLogModel::AddLogMsg(const nsLogEntry& msg)
{
  {
    NS_LOCK(m_NewMessagesMutex);
    m_NewMessages.PushBack(msg);
  }

  // always queue the message processing, otherwise it can happen that an error during this
  // triggers recursive logging, which is forbidden
  QMetaObject::invokeMethod(this, "ProcessNewMessages", Qt::ConnectionType::QueuedConnection);

  return;
}

bool nsQtLogModel::IsFiltered(const nsLogEntry& lm) const
{
  if (lm.m_Type < nsLogMsgType::None)
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
// nsQtLogModel QAbstractItemModel functions
////////////////////////////////////////////////////////////////////////

QVariant nsQtLogModel::data(const QModelIndex& index, int iRole) const
{
  if (!index.isValid() || index.column() != 0)
    return QVariant();

  UpdateVisibleEntries();

  const nsInt32 iRow = index.row();
  if (iRow < 0 || iRow >= (nsInt32)m_VisibleMessages.GetCount())
    return QVariant();

  const nsLogEntry& msg = *m_VisibleMessages[iRow];

  switch (iRole)
  {
    case Qt::DisplayRole:
    {
      if (msg.m_sMsg.FindSubString("\n") != nullptr)
      {
        nsStringBuilder sTemp = msg.m_sMsg;
        sTemp.ReplaceAll("\n", " ");
        return nsMakeQString(sTemp);
      }
      return nsMakeQString(msg.m_sMsg);
    }
    case Qt::ToolTipRole:
    {
      return nsMakeQString(msg.m_sMsg);
    }
    case Qt::ForegroundRole:
    {
      switch (msg.m_Type)
      {
        case nsLogMsgType::BeginGroup:
          return nsToQtColor(nsColorScheme::LightUI(nsColorScheme::Gray));
        case nsLogMsgType::EndGroup:
          return nsToQtColor(nsColorScheme::DarkUI(nsColorScheme::Gray));
        case nsLogMsgType::ErrorMsg:
          return nsToQtColor(nsColorScheme::LightUI(nsColorScheme::Red));
        case nsLogMsgType::SeriousWarningMsg:
          return nsToQtColor(nsColorScheme::LightUI(nsColorScheme::Orange));
        case nsLogMsgType::WarningMsg:
          return nsToQtColor(nsColorScheme::LightUI(nsColorScheme::Yellow));
        case nsLogMsgType::SuccessMsg:
          return nsToQtColor(nsColorScheme::LightUI(nsColorScheme::Green));
        case nsLogMsgType::DevMsg:
          return nsToQtColor(nsColorScheme::LightUI(nsColorScheme::Blue));
        case nsLogMsgType::DebugMsg:
          return nsToQtColor(nsColorScheme::LightUI(nsColorScheme::Cyan));
        default:
          return QVariant();
      }
    }

    default:
      return QVariant();
  }
}

Qt::ItemFlags nsQtLogModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
    return Qt::ItemFlags();

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant nsQtLogModel::headerData(int iSection, Qt::Orientation orientation, int iRole) const
{
  return QVariant();
}

QModelIndex nsQtLogModel::index(int iRow, int iColumn, const QModelIndex& parent) const
{
  if (parent.isValid() || iColumn != 0)
    return QModelIndex();

  return createIndex(iRow, iColumn, iRow);
}

QModelIndex nsQtLogModel::parent(const QModelIndex& index) const
{
  return QModelIndex();
}

int nsQtLogModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid())
    return 0;

  UpdateVisibleEntries();

  return (int)m_VisibleMessages.GetCount();
}

int nsQtLogModel::columnCount(const QModelIndex& parent) const
{
  return 1;
}


void nsQtLogModel::ProcessNewMessages()
{
  bool bNewErrors = false;
  nsStringBuilder sLatestWarning;
  nsStringBuilder sLatestError;

  {
    NS_LOCK(m_NewMessagesMutex);
    nsStringBuilder s;
    for (const auto& msg : m_NewMessages)
    {
      m_AllMessages.PushBack(msg);

      if (msg.m_Type == nsLogMsgType::BeginGroup || msg.m_Type == nsLogMsgType::EndGroup)
      {
        s.SetPrintf("%*s<<< %s", msg.m_uiIndentation, "", msg.m_sMsg.GetData());

        if (msg.m_Type == nsLogMsgType::EndGroup)
        {
          s.AppendFormat(" ({0} sec) >>>", nsArgF(msg.m_fSeconds, 3));
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
        s.SetPrintf("%*s%s", 4 * msg.m_uiIndentation, "", msg.m_sMsg.GetData());
        m_AllMessages.PeekBack().m_sMsg = s;

        if (msg.m_Type == nsLogMsgType::ErrorMsg)
        {
          sLatestError = msg.m_sMsg;
          bNewErrors = true;
          ++m_uiNumErrors;
        }
        else if (msg.m_Type == nsLogMsgType::SeriousWarningMsg)
        {
          sLatestWarning = msg.m_sMsg;
          bNewErrors = true;
          ++m_uiNumSeriousWarnings;
        }
        else if (msg.m_Type == nsLogMsgType::WarningMsg)
        {
          sLatestWarning = msg.m_sMsg;
          bNewErrors = true;
          ++m_uiNumWarnings;
        }
      }


      // if the message would not be shown anyway, don't trigger an update
      if (IsFiltered(msg))
        continue;

      if (msg.m_Type == nsLogMsgType::BeginGroup)
      {
        m_BlockQueue.PushBack(&m_AllMessages.PeekBack());
        continue;
      }
      else if (msg.m_Type == nsLogMsgType::EndGroup)
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

void nsQtLogModel::UpdateVisibleEntries() const
{
  if (m_bIsValid)
    return;


  m_bIsValid = true;
  m_VisibleMessages.Clear();
  for (const auto& msg : m_AllMessages)
  {
    if (IsFiltered(msg))
      continue;

    if (msg.m_Type == nsLogMsgType::EndGroup)
    {
      if (!m_VisibleMessages.IsEmpty())
      {
        if (m_VisibleMessages.PeekBack()->m_Type == nsLogMsgType::BeginGroup)
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
