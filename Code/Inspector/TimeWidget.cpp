#include <Inspector/InspectorPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <Inspector/TimeWidget.moc.h>
#include <QGraphicsPathItem>
#include <QGraphicsView>

nsQtTimeWidget* nsQtTimeWidget::s_pWidget = nullptr;

static QColor s_Colors[nsQtTimeWidget::s_uiMaxColors] = {
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

nsQtTimeWidget::nsQtTimeWidget(QWidget* pParent)
  : ads::CDockWidget("Time Widget", pParent)
{
  s_pWidget = this;

  setupUi(this);
  setWidget(TimeWidgetFrame);

  setIcon(QIcon(":/Icons/Icons/Time.svg"));

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

  m_pPathMax = m_Scene.addPath(QPainterPath(), QPen(QBrush(QColor(64, 64, 64)), 0));

  for (nsUInt32 i = 0; i < s_uiMaxColors; ++i)
    m_pPath[i] = m_Scene.addPath(QPainterPath(), QPen(QBrush(s_Colors[i]), 0));

  QTransform t = TimeView->transform();
  t.scale(1, -1);
  TimeView->setTransform(t);

  TimeView->setScene(&m_Scene);

  TimeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  TimeView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  TimeView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
  // TimeView->setMaximumHeight(100);

  ResetStats();
}

void nsQtTimeWidget::ResetStats()
{
  m_ClockData.Clear();

  m_uiMaxSamples = 40000;
  m_DisplayInterval = nsTime::MakeFromSeconds(60.0);
  m_uiColorsUsed = 1;
  m_bClocksChanged = true;

  ListClocks->clear();
}

void nsQtTimeWidget::UpdateStats()
{
  if (!isVisible())
    return;

  if (!nsTelemetry::IsConnectedToServer())
  {
    ListClocks->setEnabled(false);
    return;
  }

  ListClocks->setEnabled(true);

  if (m_bClocksChanged)
  {
    m_bClocksChanged = false;

    nsQtScopedUpdatesDisabled _1(ListClocks);

    ListClocks->clear();

    for (nsMap<nsString, nsQtTimeWidget::ClockData>::Iterator it = m_ClockData.GetIterator(); it.IsValid(); ++it)
    {
      ListClocks->addItem(it.Key().GetData());

      QListWidgetItem* pItem = ListClocks->item(ListClocks->count() - 1);
      pItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
      pItem->setCheckState(it.Value().m_bDisplay ? Qt::Checked : Qt::Unchecked);
      pItem->setData(Qt::UserRole, QString(it.Key().GetData()));

      pItem->setForeground(s_Colors[it.Value().m_uiColor % s_uiMaxColors]);

      it.Value().m_pListItem = pItem;
    }
  }

  QPainterPath pp[s_uiMaxColors];

  nsTime tMin = nsTime::MakeFromSeconds(100.0);
  nsTime tMax = nsTime::MakeFromSeconds(0.0);

  for (nsMap<nsString, ClockData>::Iterator it = s_pWidget->m_ClockData.GetIterator(); it.IsValid(); ++it)
  {
    if (it.Value().m_TimeSamples.IsEmpty() || !it.Value().m_bDisplay)
      continue;

    const nsUInt32 uiColorPath = it.Value().m_uiColor % s_uiMaxColors;
    ClockData& Clock = it.Value();
    const nsDeque<TimeSample>& Samples = Clock.m_TimeSamples;

    nsUInt32 uiFirstSample = 0;

    while ((uiFirstSample < Samples.GetCount()) && (m_MaxGlobalTime - Samples[uiFirstSample].m_AtGlobalTime > m_DisplayInterval))
      ++uiFirstSample;

    if (uiFirstSample < Samples.GetCount())
    {
      pp[uiColorPath].moveTo(
        QPointF((Samples[uiFirstSample].m_AtGlobalTime - m_MaxGlobalTime).GetSeconds(), Samples[uiFirstSample].m_Timestep.GetSeconds()));

      for (nsUInt32 i = uiFirstSample + 1; i < Samples.GetCount(); ++i)
      {
        pp[uiColorPath].lineTo(QPointF((Samples[i].m_AtGlobalTime - m_MaxGlobalTime).GetSeconds(), Samples[i].m_Timestep.GetSeconds()));

        tMin = nsMath::Min(tMin, Samples[i].m_Timestep);
        tMax = nsMath::Max(tMax, Samples[i].m_Timestep);
      }
    }
  }

  for (nsUInt32 i = 0; i < s_uiMaxColors; ++i)
    m_pPath[i]->setPath(pp[i]);

  // render the helper lines for time values
  {
    QPainterPath pMax;

    for (nsUInt32 i = 1; i < 10; ++i)
    {
      pMax.moveTo(QPointF(-m_DisplayInterval.GetSeconds(), nsTime::MakeFromMilliseconds(10.0 * i).GetSeconds()));
      pMax.lineTo(QPointF(0, nsTime::MakeFromMilliseconds(10.0 * i).GetSeconds()));
    }

    m_pPathMax->setPath(pMax);
  }

  nsTime tShowMax = nsTime::MakeFromSeconds(1.0 / 10.0);

  for (nsUInt32 t = 25; t < 100; t += 25)
  {
    tShowMax = nsTime::MakeFromMilliseconds(1) * t;

    if (tMax < tShowMax)
      break;
  }

  {
    TimeView->setSceneRect(QRectF(-m_DisplayInterval.GetSeconds(), 0, m_DisplayInterval.GetSeconds(), tShowMax.GetSeconds()));
    TimeView->fitInView(QRectF(-m_DisplayInterval.GetSeconds(), 0, m_DisplayInterval.GetSeconds(), tShowMax.GetSeconds()));
  }

  // once a second update the display of the clocks in the list
  if (nsTime::Now() - m_LastUpdatedClockList > nsTime::MakeFromSeconds(1))
  {
    m_LastUpdatedClockList = nsTime::Now();

    nsStringBuilder s;
    s.SetFormat("Max: {0}ms", nsArgF(tShowMax.GetMilliseconds(), 0));
    LabelMaxTime->setText(s.GetData());

    for (nsMap<nsString, nsQtTimeWidget::ClockData>::Iterator it = m_ClockData.GetIterator(); it.IsValid(); ++it)
    {
      const nsQtTimeWidget::ClockData& Clock = it.Value();

      if (!Clock.m_pListItem || Clock.m_TimeSamples.IsEmpty())
        continue;

      nsStringBuilder sTooltip;
      sTooltip.SetFormat("<p>Clock: {0}<br>Max Time Step: <b>{1}ms</b><br>Min Time Step: <b>{2}ms</b><br></p>", it.Key().GetData(),
        nsArgF(Clock.m_MaxTimestep.GetMilliseconds(), 2), nsArgF(Clock.m_MinTimestep.GetMilliseconds(), 2));

      Clock.m_pListItem->setToolTip(sTooltip.GetData());
    }
  }
}

void nsQtTimeWidget::ProcessTelemetry(void* pUnuseed)
{
  if (s_pWidget == nullptr)
    return;

  nsTelemetryMessage Msg;

  nsStringBuilder sTemp;

  while (nsTelemetry::RetrieveMessage('TIME', Msg) == NS_SUCCESS)
  {
    nsString sClockName;
    Msg.GetReader() >> sClockName;

    sTemp.SetFormat("{0} [smoothed]", sClockName);

    ClockData& ad = s_pWidget->m_ClockData[sClockName];
    ClockData& ads = s_pWidget->m_ClockData[sTemp.GetData()];

    TimeSample Sample;
    TimeSample SampleSmooth;

    Msg.GetReader() >> Sample.m_AtGlobalTime;
    Msg.GetReader() >> Sample.m_Timestep;

    SampleSmooth.m_AtGlobalTime = Sample.m_AtGlobalTime;
    Msg.GetReader() >> SampleSmooth.m_Timestep;

    s_pWidget->m_MaxGlobalTime = nsMath::Max(s_pWidget->m_MaxGlobalTime, Sample.m_AtGlobalTime);

    if (ad.m_TimeSamples.GetCount() > 1 && (nsMath::IsEqual(ad.m_TimeSamples.PeekBack().m_Timestep, Sample.m_Timestep, nsTime::MakeFromMicroseconds(100))))
      ad.m_TimeSamples.PeekBack() = Sample;
    else
      ad.m_TimeSamples.PushBack(Sample);

    if (ads.m_TimeSamples.GetCount() > 1 &&
        (nsMath::IsEqual(ads.m_TimeSamples.PeekBack().m_Timestep, SampleSmooth.m_Timestep, nsTime::MakeFromMicroseconds(100))))
      ads.m_TimeSamples.PeekBack() = SampleSmooth;
    else
      ads.m_TimeSamples.PushBack(SampleSmooth);

    ad.m_MinTimestep = nsMath::Min(ad.m_MinTimestep, Sample.m_Timestep);
    ad.m_MaxTimestep = nsMath::Max(ad.m_MaxTimestep, Sample.m_Timestep);

    ads.m_MinTimestep = nsMath::Min(ads.m_MinTimestep, SampleSmooth.m_Timestep);
    ads.m_MaxTimestep = nsMath::Max(ads.m_MaxTimestep, SampleSmooth.m_Timestep);

    if (ad.m_uiColor == 0xFF)
    {
      ad.m_uiColor = s_pWidget->m_uiColorsUsed;
      ++s_pWidget->m_uiColorsUsed;
      s_pWidget->m_bClocksChanged = true;
    }

    if (ads.m_uiColor == 0xFF)
    {
      ads.m_uiColor = s_pWidget->m_uiColorsUsed;
      ++s_pWidget->m_uiColorsUsed;
      s_pWidget->m_bClocksChanged = true;
    }

    if (ad.m_TimeSamples.GetCount() > s_pWidget->m_uiMaxSamples)
      ad.m_TimeSamples.PopFront(ad.m_TimeSamples.GetCount() - s_pWidget->m_uiMaxSamples);

    if (ads.m_TimeSamples.GetCount() > s_pWidget->m_uiMaxSamples)
      ads.m_TimeSamples.PopFront(ads.m_TimeSamples.GetCount() - s_pWidget->m_uiMaxSamples);
  }
}

void nsQtTimeWidget::on_ListClocks_itemChanged(QListWidgetItem* item)
{
  m_ClockData[item->data(Qt::UserRole).toString().toUtf8().data()].m_bDisplay = (item->checkState() == Qt::Checked);
}

void nsQtTimeWidget::on_ComboTimeframe_currentIndexChanged(int index)
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
