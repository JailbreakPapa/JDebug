#include <Inspector/InspectorPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <Inspector/MainWidget.moc.h>
#include <Inspector/MainWindow.moc.h>
#include <Inspector/StatVisWidget.moc.h>
#include <QGraphicsPathItem>
#include <QGraphicsView>
#include <QSettings>

nsQtStatVisWidget* nsQtStatVisWidget::s_pWidget = nullptr;
nsInt32 nsQtStatVisWidget::s_iCurColor = 0;

namespace StatVisWidgetDetail
{
  static QColor s_Colors[nsQtStatVisWidget::s_uiMaxColors] = {
    QColor(255, 106, 0), // orange
    QColor(182, 255, 0), // lime green
    QColor(255, 0, 255), // pink
    QColor(0, 148, 255), // light blue
    QColor(255, 0, 0),   // red
    QColor(0, 255, 255), // turquoise
    QColor(178, 0, 255), // purple
    QColor(0, 38, 255),  // dark blue
    QColor(72, 0, 255),  // lilac
  };
}

nsQtStatVisWidget::nsQtStatVisWidget(QWidget* pParent, nsInt32 iWindowNumber)
  : ads::CDockWidget(QString("StatVisWidget") + QString::number(iWindowNumber), pParent)
  , m_ShowWindowAction(pParent)
{
  m_iWindowNumber = iWindowNumber;
  m_DisplayInterval = nsTime::MakeFromSeconds(60.0);

  s_pWidget = this;

  setupUi(this);
  setWidget(StatVisWidgetFrame);

  {
    nsQtScopedUpdatesDisabled _1(ComboTimeframe);

    ComboTimeframe->addItem("Timeframe: 10 seconds");
    ComboTimeframe->addItem("Timeframe: 30 seconds");
    ComboTimeframe->addItem("Timeframe: 1 minute");
    ComboTimeframe->addItem("Timeframe: 2 minutes");
    ComboTimeframe->addItem("Timeframe: 5 minutes");
    ComboTimeframe->addItem("Timeframe: 10 minutes");
    ComboTimeframe->setCurrentIndex(2);
  }

  ButtonRemove->setEnabled(false);

  m_pPathMax = m_Scene.addPath(QPainterPath(), QPen(QBrush(QColor(64, 64, 64)), 0));

  for (nsUInt32 i = 0; i < s_uiMaxColors; ++i)
    m_pPath[i] = m_Scene.addPath(QPainterPath(), QPen(QBrush(StatVisWidgetDetail::s_Colors[i]), 0));

  QTransform t = StatHistoryView->transform();
  t.scale(1, -1);
  StatHistoryView->setTransform(t);

  StatHistoryView->setScene(&m_Scene);

  StatHistoryView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  StatHistoryView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  StatHistoryView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
  // TimeView->setMaximumHeight(100);

  m_ShowWindowAction.setCheckable(true);

  nsStringBuilder sStatHistory;
  sStatHistory.SetFormat("StatHistory{0}", m_iWindowNumber);

  QSettings Settings;
  Settings.beginGroup(sStatHistory.GetData());
  LineName->setText(Settings.value(QLatin1String("Name"), QLatin1String(sStatHistory.GetData())).toString());
  SpinMin->setValue(Settings.value(QLatin1String("Min"), 0.0).toDouble());
  SpinMax->setValue(Settings.value(QLatin1String("Max"), 1.0).toDouble());
  Settings.endGroup();

  NS_VERIFY(nullptr != QWidget::connect(&m_ShowWindowAction, SIGNAL(triggered()), this, SLOT(on_ToggleVisible())), "");
}


nsQtStatVisWidget::~nsQtStatVisWidget() = default;

void nsQtStatVisWidget::on_ComboTimeframe_currentIndexChanged(int index)
{
  const nsUInt32 uiSeconds[] = {
    10,
    30,
    60 * 1,
    60 * 2,
    60 * 5,
    60 * 10,
  };

  m_DisplayInterval = nsTime::MakeFromSeconds(uiSeconds[index]);
}

void nsQtStatVisWidget::on_LineName_textChanged(const QString& text)
{
  nsStringBuilder sStatHistory;
  sStatHistory.SetFormat("StatHistory{0}", m_iWindowNumber);

  QSettings Settings;
  Settings.beginGroup(sStatHistory.GetData());
  Settings.setValue(QLatin1String("Name"), LineName->text());
  Settings.endGroup();

  setWindowTitle(QLatin1String("Stats: ") + LineName->text());
  m_ShowWindowAction.setText(LineName->text());
}

void nsQtStatVisWidget::on_SpinMin_valueChanged(double val)
{
  nsStringBuilder sStatHistory;
  sStatHistory.SetFormat("StatHistory{0}", m_iWindowNumber);

  QSettings Settings;
  Settings.beginGroup(sStatHistory.GetData());
  Settings.setValue(QLatin1String("Min"), val);
  Settings.endGroup();
}

void nsQtStatVisWidget::on_SpinMax_valueChanged(double val)
{
  nsStringBuilder sStatHistory;
  sStatHistory.SetFormat("StatHistory{0}", m_iWindowNumber);

  QSettings Settings;
  Settings.beginGroup(sStatHistory.GetData());
  Settings.setValue(QLatin1String("Max"), val);
  Settings.endGroup();
}

void nsQtStatVisWidget::on_ToggleVisible()
{
  toggleView(isClosed());

  if (!isClosed())
    raise();
}

void nsQtStatVisWidget::on_ListStats_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
  ButtonRemove->setEnabled(current != nullptr);
}

void nsQtStatVisWidget::on_ButtonRemove_clicked()
{
  if (ListStats->currentItem() == nullptr)
    return;

  for (auto it = m_Stats.GetIterator(); it.IsValid(); ++it)
  {
    if (it.Value().m_pListItem == ListStats->currentItem())
    {
      delete it.Value().m_pListItem;
      m_Stats.Remove(it);
      break;
    }
  }
}

void nsQtStatVisWidget::Save()
{
  nsStringBuilder sStatHistory;
  sStatHistory.SetFormat("/StatWindow{0}.stats", m_iWindowNumber);

  QString sFile = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
  QDir dir;
  dir.mkpath(sFile);

  sFile.append(sStatHistory.GetData());

  QFile f(sFile);
  if (!f.open(QIODevice::WriteOnly))
    return;

  QDataStream stream(&f);

  const nsUInt32 uiNumFavorites = m_Stats.GetCount();
  stream << uiNumFavorites;

  for (auto it = m_Stats.GetIterator(); it.IsValid(); ++it)
  {
    const QString s = it.Key().GetData();
    stream << s;
    stream << (it.Value().m_pListItem->checkState() == Qt::Checked);
  }

  f.close();
}

void nsQtStatVisWidget::Load()
{
  nsStringBuilder sStatHistory;
  sStatHistory.SetFormat("/StatWindow{0}.stats", m_iWindowNumber);

  QString sFile = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
  QDir dir;
  dir.mkpath(sFile);

  sFile.append(sStatHistory.GetData());

  QFile f(sFile);
  if (!f.open(QIODevice::ReadOnly))
    return;

  m_Stats.Clear();

  QDataStream stream(&f);

  nsUInt32 uiNumFavorites = 0;
  stream >> uiNumFavorites;

  for (nsUInt32 i = 0; i < uiNumFavorites; ++i)
  {
    QString s;
    stream >> s;

    bool bChecked = true;
    stream >> bChecked;

    nsString nss = s.toUtf8().data();

    AddStat(nss, bChecked, false);
  }

  f.close();
}

void nsQtStatVisWidget::AddStat(const nsString& sStatPath, bool bEnabled, bool bRaiseWindow)
{
  if (bRaiseWindow)
  {
    toggleView(true);
    raise();
  }

  StatsData& Stat = m_Stats[sStatPath];

  if (Stat.m_pListItem == nullptr)
  {
    Stat.m_uiColor = s_iCurColor % nsQtStatVisWidget::s_uiMaxColors;
    ++s_iCurColor;

    Stat.m_pListItem = new QListWidgetItem();
    Stat.m_pListItem->setText(sStatPath.GetData());
    Stat.m_pListItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
    Stat.m_pListItem->setCheckState(bEnabled ? Qt::Checked : Qt::Unchecked);
    Stat.m_pListItem->setForeground(StatVisWidgetDetail::s_Colors[Stat.m_uiColor]);

    ListStats->addItem(Stat.m_pListItem);
  }
}

void nsQtStatVisWidget::UpdateStats()
{
  if (isClosed())
    return;

  QPainterPath pp[s_uiMaxColors];

  for (nsMap<nsString, StatsData>::Iterator it = m_Stats.GetIterator(); it.IsValid(); ++it)
  {
    if (it.Value().m_pListItem->checkState() != Qt::Checked)
      continue;

    const nsDeque<nsQtMainWidget::StatSample>& Samples = nsQtMainWidget::s_pWidget->m_Stats[it.Key()].m_History;

    if (Samples.IsEmpty())
      continue;

    const nsUInt32 uiColorPath = it.Value().m_uiColor;

    nsUInt32 uiFirstSample = 0;

    const nsTime MaxGlobalTime = nsQtMainWidget::s_pWidget->m_MaxGlobalTime;

    while ((uiFirstSample + 1 < Samples.GetCount()) && (MaxGlobalTime - Samples[uiFirstSample + 1].m_AtGlobalTime > m_DisplayInterval))
      ++uiFirstSample;

    if (uiFirstSample < Samples.GetCount())
    {
      pp[uiColorPath].moveTo(QPointF((Samples[uiFirstSample].m_AtGlobalTime - MaxGlobalTime).GetSeconds(), Samples[uiFirstSample].m_Value));

      for (nsUInt32 i = uiFirstSample + 1; i < Samples.GetCount(); ++i)
      {
        pp[uiColorPath].lineTo(QPointF((Samples[i].m_AtGlobalTime - MaxGlobalTime).GetSeconds(), Samples[i].m_Value));
      }

      const double dLatestValue = Samples.PeekBack().m_Value;
      pp[uiColorPath].lineTo(QPointF(0.0, dLatestValue));
    }
  }

  for (nsUInt32 i = 0; i < s_uiMaxColors; ++i)
    m_pPath[i]->setPath(pp[i]);

  {
    double dMin = SpinMin->value();
    double dHeight = SpinMax->value() - SpinMin->value();

    StatHistoryView->setSceneRect(QRectF(-m_DisplayInterval.GetSeconds(), dMin, m_DisplayInterval.GetSeconds(), dHeight));
    StatHistoryView->fitInView(QRectF(-m_DisplayInterval.GetSeconds(), dMin, m_DisplayInterval.GetSeconds(), dHeight));
  }
}
