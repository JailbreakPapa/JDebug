#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>

#include <Foundation/Time/Time.h>

#include <QToolBar>
#include <QWidget>

class QMouseEvent;
class QPushButton;
class QLineEdit;

class NS_GUIFOUNDATION_DLL nsQtTimeScrubberWidget : public QWidget
{
  Q_OBJECT

public:
  explicit nsQtTimeScrubberWidget(QWidget* pParent);
  ~nsQtTimeScrubberWidget();

  /// \brief Sets the duration in 'ticks'. There are 4800 ticks per second.
  void SetDuration(nsUInt64 uiNumTicks);

  /// \brief Sets the duration.
  void SetDuration(nsTime time);

  /// \brief Sets the current position in 'ticks'. There are 4800 ticks per second.
  void SetScrubberPosition(nsUInt64 uiTick);

  /// \brief Sets the current position.
  void SetScrubberPosition(nsTime time);

Q_SIGNALS:
  void ScrubberPosChangedEvent(nsUInt64 uiNewScrubberTickPos);

private:
  virtual void paintEvent(QPaintEvent* event) override;
  virtual void mousePressEvent(QMouseEvent* event) override;
  virtual void mouseReleaseEvent(QMouseEvent* event) override;
  virtual void mouseMoveEvent(QMouseEvent* event) override;
  void SetScrubberPosFromPixelCoord(nsInt32 x);

  nsUInt64 m_uiDurationTicks = 0;
  nsTime m_Duration;
  nsUInt64 m_uiScrubberTickPos = 0;
  double m_fNormScrubberPosition = 0.0;
  bool m_bDragging = false;
};

class NS_GUIFOUNDATION_DLL nsQtTimeScrubberToolbar : public QToolBar
{
  Q_OBJECT

public:
  explicit nsQtTimeScrubberToolbar(QWidget* pParent);

  /// \brief Sets the duration in 'ticks'. There are 4800 ticks per second.
  void SetDuration(nsUInt64 uiNumTicks);

  /// \brief Sets the current position in 'ticks'. There are 4800 ticks per second.
  void SetScrubberPosition(nsUInt64 uiTick);

  void SetButtonState(bool bPlaying, bool bRepeatEnabled);

Q_SIGNALS:
  void ScrubberPosChangedEvent(nsUInt64 uiNewScrubberTickPos);
  void PlayPauseEvent();
  void RepeatEvent();
  void DurationChangedEvent(double fDuration);
  void AdjustDurationEvent();

private:
  nsQtTimeScrubberWidget* m_pScrubber = nullptr;
  QPushButton* m_pPlayButton = nullptr;
  QPushButton* m_pRepeatButton = nullptr;
  QLineEdit* m_pDuration = nullptr;
  QPushButton* m_pAdjustDurationButton = nullptr;
};
