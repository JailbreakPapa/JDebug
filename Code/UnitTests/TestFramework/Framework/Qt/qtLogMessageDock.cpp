#include <TestFramework/TestFrameworkPCH.h>

#ifdef NS_USE_QT

#  include <QStringBuilder>
#  include <TestFramework/Framework/Qt/qtLogMessageDock.h>
#  include <TestFramework/Framework/TestFramework.h>

////////////////////////////////////////////////////////////////////////
// nsQtLogMessageDock public functions
////////////////////////////////////////////////////////////////////////

nsQtLogMessageDock::nsQtLogMessageDock(QObject* pParent, const nsTestFrameworkResult* pResult)
{
  setupUi(this);
  m_pModel = new nsQtLogMessageModel(this, pResult);
  ListView->setModel(m_pModel);
}

nsQtLogMessageDock::~nsQtLogMessageDock()
{
  ListView->setModel(nullptr);
  delete m_pModel;
  m_pModel = nullptr;
}

void nsQtLogMessageDock::resetModel()
{
  m_pModel->resetModel();
}

void nsQtLogMessageDock::currentTestResultChanged(const nsTestResultData* pTestResult)
{
  m_pModel->currentTestResultChanged(pTestResult);
  ListView->scrollToBottom();
}

void nsQtLogMessageDock::currentTestSelectionChanged(const nsTestResultData* pTestResult)
{
  m_pModel->currentTestSelectionChanged(pTestResult);
  ListView->scrollTo(m_pModel->GetLastIndexOfTestSelection(), QAbstractItemView::EnsureVisible);
  ListView->scrollTo(m_pModel->GetFirstIndexOfTestSelection(), QAbstractItemView::EnsureVisible);
}

////////////////////////////////////////////////////////////////////////
// nsQtLogMessageModel public functions
////////////////////////////////////////////////////////////////////////

nsQtLogMessageModel::nsQtLogMessageModel(QObject* pParent, const nsTestFrameworkResult* pResult)
  : QAbstractItemModel(pParent)
  , m_pTestResult(pResult)
{
}

nsQtLogMessageModel::~nsQtLogMessageModel() = default;

void nsQtLogMessageModel::resetModel()
{
  beginResetModel();
  currentTestResultChanged(nullptr);
  endResetModel();
}

QModelIndex nsQtLogMessageModel::GetFirstIndexOfTestSelection()
{
  if (m_pCurrentTestSelection == nullptr || m_pCurrentTestSelection->m_iFirstOutput == -1)
    return QModelIndex();

  nsInt32 iEntries = (nsInt32)m_VisibleEntries.size();
  for (int i = 0; i < iEntries; ++i)
  {
    if ((nsInt32)m_VisibleEntries[i] >= m_pCurrentTestSelection->m_iFirstOutput)
      return index(i, 0);
  }
  return index(rowCount() - 1, 0);
}

QModelIndex nsQtLogMessageModel::GetLastIndexOfTestSelection()
{
  if (m_pCurrentTestSelection == nullptr || m_pCurrentTestSelection->m_iLastOutput == -1)
    return QModelIndex();

  nsInt32 iEntries = (nsInt32)m_VisibleEntries.size();
  for (int i = 0; i < iEntries; ++i)
  {
    if ((nsInt32)m_VisibleEntries[i] >= m_pCurrentTestSelection->m_iLastOutput)
      return index(i, 0);
  }
  return index(rowCount() - 1, 0);
}

void nsQtLogMessageModel::currentTestResultChanged(const nsTestResultData* pTestResult)
{
  UpdateVisibleEntries();
  currentTestSelectionChanged(pTestResult);
}

void nsQtLogMessageModel::currentTestSelectionChanged(const nsTestResultData* pTestResult)
{
  m_pCurrentTestSelection = pTestResult;
  if (m_pCurrentTestSelection != nullptr)
  {
    dataChanged(index(m_pCurrentTestSelection->m_iFirstOutput, 0), index(m_pCurrentTestSelection->m_iLastOutput, 0));
  }
}


////////////////////////////////////////////////////////////////////////
// nsQtLogMessageModel QAbstractItemModel functions
////////////////////////////////////////////////////////////////////////

QVariant nsQtLogMessageModel::data(const QModelIndex& index, int iRole) const
{
  if (!index.isValid() || m_pTestResult == nullptr || index.column() != 0)
    return QVariant();

  const nsInt32 iRow = index.row();
  if (iRow < 0 || iRow >= (nsInt32)m_VisibleEntries.size())
    return QVariant();

  const nsUInt32 uiLogIdx = m_VisibleEntries[iRow];
  const nsUInt8 uiIndention = m_VisibleEntriesIndention[iRow];
  const nsTestOutputMessage& Message = *m_pTestResult->GetOutputMessage(uiLogIdx);
  const nsTestErrorMessage* pError = (Message.m_iErrorIndex != -1) ? m_pTestResult->GetErrorMessage(Message.m_iErrorIndex) : nullptr;
  switch (iRole)
  {
    case Qt::DisplayRole:
    {
      if (pError != nullptr)
      {
        QString sBlockStart = QLatin1String("\n") % QString((uiIndention + 1) * 3, ' ');
        QString sBlockName =
          pError->m_sBlock.empty() ? QLatin1String("") : (sBlockStart % QLatin1String("Block: ") + QLatin1String(pError->m_sBlock.c_str()));
        QString sMessage =
          pError->m_sMessage.empty() ? QLatin1String("") : (sBlockStart % QLatin1String("Message: ") + QLatin1String(pError->m_sMessage.c_str()));
        QString sErrorMessage = QString(uiIndention * 3, ' ') % QString(Message.m_sMessage.c_str()) % sBlockName % sBlockStart %
                                QLatin1String("File: ") % QLatin1String(pError->m_sFile.c_str()) % sBlockStart % QLatin1String("Line: ") %
                                QString::number(pError->m_iLine) % sBlockStart % QLatin1String("Function: ") %
                                QLatin1String(pError->m_sFunction.c_str()) % sMessage;

        return sErrorMessage;
      }
      return QString(uiIndention * 3, ' ') + QString(Message.m_sMessage.c_str());
    }
    case Qt::ForegroundRole:
    {
      switch (Message.m_Type)
      {
        case nsTestOutput::BeginBlock:
        case nsTestOutput::Message:
          return QColor(Qt::yellow);
        case nsTestOutput::Error:
          return QColor(Qt::red);
        case nsTestOutput::Success:
          return QColor(Qt::green);
        case nsTestOutput::Warning:
          return QColor(qRgb(255, 100, 0));
        case nsTestOutput::StartOutput:
        case nsTestOutput::EndBlock:
        case nsTestOutput::ImportantInfo:
        case nsTestOutput::Details:
        case nsTestOutput::Duration:
        case nsTestOutput::FinalResult:
          return QVariant();
        default:
          return QVariant();
      }
    }
    case Qt::BackgroundRole:
    {
      QPalette palette = QApplication::palette();
      if (m_pCurrentTestSelection != nullptr && m_pCurrentTestSelection->m_iFirstOutput != -1)
      {
        if (m_pCurrentTestSelection->m_iFirstOutput <= (nsInt32)uiLogIdx && (nsInt32)uiLogIdx <= m_pCurrentTestSelection->m_iLastOutput)
        {
          return palette.midlight().color();
        }
      }
      return palette.base().color();
    }

    default:
      return QVariant();
  }
}

Qt::ItemFlags nsQtLogMessageModel::flags(const QModelIndex& index) const
{
  if (!index.isValid() || m_pTestResult == nullptr)
    return Qt::ItemFlags();

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant nsQtLogMessageModel::headerData(int iSection, Qt::Orientation orientation, int iRole) const
{
  if (orientation == Qt::Horizontal && iRole == Qt::DisplayRole)
  {
    switch (iSection)
    {
      case 0:
        return QString("Log Entry");
    }
  }
  return QVariant();
}

QModelIndex nsQtLogMessageModel::index(int iRow, int iColumn, const QModelIndex& parent) const
{
  if (parent.isValid() || m_pTestResult == nullptr || iColumn != 0)
    return QModelIndex();

  return createIndex(iRow, iColumn, iRow);
}

QModelIndex nsQtLogMessageModel::parent(const QModelIndex& index) const
{
  return QModelIndex();
}

int nsQtLogMessageModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid() || m_pTestResult == nullptr)
    return 0;

  return (int)m_VisibleEntries.size();
}

int nsQtLogMessageModel::columnCount(const QModelIndex& parent) const
{
  return 1;
}


////////////////////////////////////////////////////////////////////////
// nsQtLogMessageModel private functions
////////////////////////////////////////////////////////////////////////

void nsQtLogMessageModel::UpdateVisibleEntries()
{
  m_VisibleEntries.clear();
  m_VisibleEntriesIndention.clear();
  if (m_pTestResult == nullptr)
    return;

  nsUInt8 uiIndention = 0;
  nsUInt32 uiEntries = m_pTestResult->GetOutputMessageCount();
  /// \todo filter out uninteresting messages
  for (nsUInt32 i = 0; i < uiEntries; ++i)
  {
    nsTestOutput::Enum Type = m_pTestResult->GetOutputMessage(i)->m_Type;
    if (Type == nsTestOutput::BeginBlock)
      uiIndention++;
    if (Type == nsTestOutput::EndBlock)
      uiIndention--;

    m_VisibleEntries.push_back(i);
    m_VisibleEntriesIndention.push_back(uiIndention);
  }
  beginResetModel();
  endResetModel();
}

#endif
