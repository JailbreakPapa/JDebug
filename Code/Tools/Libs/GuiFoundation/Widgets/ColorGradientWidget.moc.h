#pragma once

#include <Foundation/Tracks/ColorGradient.h>
#include <GuiFoundation/GuiFoundationDLL.h>

#include <QWidget>

class QMouseEvent;

class NS_GUIFOUNDATION_DLL nsQtColorGradientWidget : public QWidget
{
  Q_OBJECT

public:
  explicit nsQtColorGradientWidget(QWidget* pParent);
  ~nsQtColorGradientWidget();

  void SetScrubberPosition(double fPosition);

  void setColorGradientData(const nsColorGradient* pGradient);

  void setEditMode(bool bEdit);
  void setShowColorCPs(bool bShow);
  void setShowAlphaCPs(bool bShow);
  void setShowIntensityCPs(bool bShow);
  void setShowCoords(bool bTop, bool bBottom);

  void FrameExtents();
  void ClearSelectedCP();
  void SelectCP(nsInt32 iColorCP, nsInt32 iAlphaCP, nsInt32 iIntensityCP);

Q_SIGNALS:
  void GradientClicked();
  void addColorCp(double fPosX, const nsColorGammaUB& color);
  void addAlphaCp(double fPosX, nsUInt8 value);
  void addIntensityCp(double fPosX, float fIntensity);
  void moveColorCpToPos(nsInt32 iIndex, double fNewPosX);
  void moveAlphaCpToPos(nsInt32 iIndex, double fNewPosX);
  void moveIntensityCpToPos(nsInt32 iIndex, double fNewPosX);
  void deleteColorCp(nsInt32 iIndex);
  void deleteAlphaCp(nsInt32 iIndex);
  void deleteIntensityCp(nsInt32 iIndex);
  void selectionChanged(nsInt32 iColorCP, nsInt32 iAlphaCP, nsInt32 iIntensityCP);
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
    QPainter& p, const QRect& area, double posX, const nsColorGammaUB& outlineColor, const nsColorGammaUB& fillColor, bool selected) const;
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

  double WindowToGradientCoord(nsInt32 mouseWindowPosX) const;
  nsInt32 GradientToWindowCoord(double gradientPosX) const;

  nsInt32 FindClosestColorCp(nsInt32 iWindowPosX) const;
  nsInt32 FindClosestAlphaCp(nsInt32 iWindowPosX) const;
  nsInt32 FindClosestIntensityCp(nsInt32 iWindowPosX) const;

  bool HoversControlPoint(const QPoint& windowPos) const;
  bool HoversControlPoint(const QPoint& windowPos, nsInt32& iHoverColorCp, nsInt32& iHoverAlphaCp, nsInt32& iHoverIntensityCp) const;
  Area HoversInteractiveArea(const QPoint& windowPos) const;

  void EvaluateAt(nsInt32 windowPos, nsColorGammaUB& rgba, float& intensity) const;

  double ComputeCoordinateDisplayStep() const;

  const nsColorGradient* m_pColorGradientData;

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

  nsInt32 m_iSelectedColorCP;
  nsInt32 m_iSelectedAlphaCP;
  nsInt32 m_iSelectedIntensityCP;

  QPointF m_LastMousePosition;
  QPixmap m_AlphaPattern;

  bool m_bShowScrubber = false;
  double m_fScrubberPosition = 0;
};
