#pragma once

#include <Foundation/Tracks/ColorGradient.h>
#include <GuiFoundation/GuiFoundationDLL.h>

#include <QWidget>

class QMouseEvent;

class WD_GUIFOUNDATION_DLL wdQtColorGradientWidget : public QWidget
{
  Q_OBJECT

public:
  explicit wdQtColorGradientWidget(QWidget* pParent);
  ~wdQtColorGradientWidget();

  void SetScrubberPosition(double fPosition);

  void setColorGradientData(const wdColorGradient* pGradient);

  void setEditMode(bool bEdit);
  void setShowColorCPs(bool bShow);
  void setShowAlphaCPs(bool bShow);
  void setShowIntensityCPs(bool bShow);
  void setShowCoords(bool bTop, bool bBottom);

  void FrameExtents();
  void ClearSelectedCP();
  void SelectCP(wdInt32 iColorCP, wdInt32 iAlphaCP, wdInt32 iIntensityCP);

Q_SIGNALS:
  void GradientClicked();
  void addColorCp(double fPosX, const wdColorGammaUB& color);
  void addAlphaCp(double fPosX, wdUInt8 value);
  void addIntensityCp(double fPosX, float fIntensity);
  void moveColorCpToPos(wdInt32 iIndex, double fNewPosX);
  void moveAlphaCpToPos(wdInt32 iIndex, double fNewPosX);
  void moveIntensityCpToPos(wdInt32 iIndex, double fNewPosX);
  void deleteColorCp(wdInt32 iIndex);
  void deleteAlphaCp(wdInt32 iIndex);
  void deleteIntensityCp(wdInt32 iIndex);
  void selectionChanged(wdInt32 iColorCP, wdInt32 iAlphaCP, wdInt32 iIntensityCP);
  void beginOperation();
  void endOperation(bool bCommit);
  void triggerPickColor();

private:
  enum class Area
  {
    None = 0,
    Gradient = 1,
    ColorCPs = 2,
    AlphaCPs = 3,
    IntensityCPs = 4,
  };


  virtual void paintEvent(QPaintEvent* event) override;
  virtual void mousePressEvent(QMouseEvent* event) override;
  virtual void mouseReleaseEvent(QMouseEvent* event) override;
  virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
  virtual void mouseMoveEvent(QMouseEvent* event) override;

  void UpdateMouseCursor(QMouseEvent* event);

  virtual void wheelEvent(QWheelEvent* event) override;

  void ClampDisplayExtents(double zoomCenter = 0.5);

  virtual void keyPressEvent(QKeyEvent* event) override;

  void PaintColorGradient(QPainter& p) const;
  void PaintCpBackground(QPainter& p, const QRect& area) const;
  void PaintColorCpArea(QPainter& p);
  void PaintAlphaCpArea(QPainter& p);
  void PaintIntensityCpArea(QPainter& p);
  void PaintCoordinateStrips(QPainter& p) const;
  void PaintCoordinateStrip(QPainter& p, const QRect& area) const;
  void PaintCoordinateLines(QPainter& p);

  void PaintControlPoint(
    QPainter& p, const QRect& area, double posX, const wdColorGammaUB& outlineColor, const wdColorGammaUB& fillColor, bool selected) const;
  void PaintColorCPs(QPainter& p) const;
  void PaintAlphaCPs(QPainter& p) const;
  void PaintIntensityCPs(QPainter& p) const;
  void PaintScrubber(QPainter& p) const;

  QRect GetColorCpArea() const;
  QRect GetAlphaCpArea() const;
  QRect GetIntensityCpArea() const;
  QRect GetGradientArea() const;
  QRect GetCoordAreaTop() const;
  QRect GetCoordAreaBottom() const;

  double WindowToGradientCoord(wdInt32 mouseWindowPosX) const;
  wdInt32 GradientToWindowCoord(double gradientPosX) const;

  wdInt32 FindClosestColorCp(wdInt32 iWindowPosX) const;
  wdInt32 FindClosestAlphaCp(wdInt32 iWindowPosX) const;
  wdInt32 FindClosestIntensityCp(wdInt32 iWindowPosX) const;

  bool HoversControlPoint(const QPoint& windowPos) const;
  bool HoversControlPoint(const QPoint& windowPos, wdInt32& iHoverColorCp, wdInt32& iHoverAlphaCp, wdInt32& iHoverIntensityCp) const;
  Area HoversInteractiveArea(const QPoint& windowPos) const;

  void EvaluateAt(wdInt32 windowPos, wdColorGammaUB& rgba, float& intensity) const;

  double ComputeCoordinateDisplayStep() const;

  const wdColorGradient* m_pColorGradientData;

  bool m_bEditMode;
  bool m_bShowColorCPs;
  bool m_bShowAlphaCPs;
  bool m_bShowIntensityCPs;
  bool m_bDraggingCP;
  bool m_bTempMode;
  bool m_bShowCoordsTop;
  bool m_bShowCoordsBottom;

  double m_fDisplayExtentMinX;
  double m_fDisplayExtentMaxX;

  wdInt32 m_iSelectedColorCP;
  wdInt32 m_iSelectedAlphaCP;
  wdInt32 m_iSelectedIntensityCP;

  QPoint m_LastMousePosition;
  QPixmap m_AlphaPattern;

  bool m_bShowScrubber = false;
  double m_fScrubberPosition = 0;
};

