#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>

#include <Foundation/Time/Time.h>

#include <QToolBar>
#include <QWidget>

class QMouseEvent;
class QPushButton;
class QLineEdit;

class WD_GUIFOUNDATION_DLL wdQtTimeScrubberWidget : public QWidget
{
  Q_OBJECT

public:
  explicit wdQtTimeScrubberWidget(QWidget* pParent);
  ~wdQtTimeScrubberWidget();

  /// \brief Sets the duration in 'ticks'. There are 4800 ticks per second.
  void SetDuration(wdUInt64 uiNumTicks);

  /// \brief Sets the duration.
  void SetDuration(wdTime time);

  /// \brief Sets the current position in 'ticks'. There are 4800 ticks per second.
  void SetScrubberPosition(wdUInt64 uiTick);

  /// \brief Sets the current position.
  void SetScrubberPosition(wdTime time);

Q_SIGNALS:
  void ScrubberPosChangedEvent(wdUInt64 uiNewScrubberTickPos);

private:
  virtual void paintEvent(QPaintEvent* event) override;
  virtual void mousePressEvent(QMouseEvent* event) override;
  virtual void mouseReleaseEvent(QMouseEvent* event) override;
  virtual void mouseMoveEvent(QMouseEvent* event) override;
  void SetScrubberPosFromPixelCoord(wdInt32 x);

  wdUInt64 m_uiDurationTicks = 0;
  wdTime m_Duration;
  wdUInt64 m_uiScrubberTickPos = 0;
  double m_fNormScrubberPosition = 0.0;
  bool m_bDragging = false;
};

class WD_GUIFOUNDATION_DLL wdQtTimeScrubberToolbar : public QToolBar
{
  Q_OBJECT

public:
  explicit wdQtTimeScrubberToolbar(QWidget* pParent);

  /// \brief Sets the duration in 'ticks'. There are 4800 ticks per second.
  void SetDuration(wdUInt64 uiNumTicks);

  /// \brief Sets the current position in 'ticks'. There are 4800 ticks per second.
  void SetScrubberPosition(wdUInt64 uiTick);

  void SetButtonState(bool bPlaying, bool bRepeatEnabled);

Q_SIGNALS:
  void ScrubberPosChangedEvent(wdUInt64 uiNewScrubberTickPos);
  void PlayPauseEvent();
  void RepeatEvent();
  void DurationChangedEvent(double fDuration);
  void AdjustDurationEvent();

private:
  wdQtTimeScrubberWidget* m_pScrubber = nullptr;
  QPushButton* m_pPlayButton = nullptr;
  QPushButton* m_pRepeatButton = nullptr;
  QLineEdit* m_pDuration = nullptr;
  QPushButton* m_pAdjustDurationButton = nullptr;
};

