#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Math/Math.h>
#include <GuiFoundation/Widgets/ImageWidget.moc.h>
#include <QGraphicsPixmapItem>
#include <QScrollArea>
#include <QScrollBar>

wdQtImageScene::wdQtImageScene(QObject* pParent)
  : QGraphicsScene(pParent)
{
  m_pImageItem = nullptr;
  setItemIndexMethod(QGraphicsScene::NoIndex);
}

void wdQtImageScene::SetImage(QPixmap pixmap)
{
  if (m_pImageItem)
    delete m_pImageItem;

  m_Pixmap = pixmap;
  m_pImageItem = addPixmap(m_Pixmap);
  setSceneRect(0, 0, m_Pixmap.width(), m_Pixmap.height());
}



wdQtImageWidget::wdQtImageWidget(QWidget* pParent, bool bShowButtons)
  : QWidget(pParent)
{
  setupUi(this);
  m_pScene = new wdQtImageScene(GraphicsView);
  GraphicsView->setScene(m_pScene);

  m_fCurrentScale = 1.0f;

  if (!bShowButtons)
    ButtonBar->setVisible(false);
}

wdQtImageWidget::~wdQtImageWidget() {}

void wdQtImageWidget::SetImageSize(float fScale)
{
  if (m_fCurrentScale == fScale)
    return;

  m_fCurrentScale = fScale;
  ImageApplyScale();
}

void wdQtImageWidget::ScaleImage(float fFactor)
{
  float fPrevScale = m_fCurrentScale;
  m_fCurrentScale = wdMath::Clamp(m_fCurrentScale * fFactor, 0.2f, 5.0f);

  fFactor = m_fCurrentScale / fPrevScale;
  ImageApplyScale();
}

void wdQtImageWidget::ImageApplyScale()
{
  QTransform scale = QTransform::fromScale(m_fCurrentScale, m_fCurrentScale);
  GraphicsView->setTransform(scale);
}

void wdQtImageWidget::SetImage(QPixmap pixmap)
{
  m_pScene->SetImage(pixmap);
  ImageApplyScale();
}

void wdQtImageWidget::on_ButtonZoomIn_clicked()
{
  ScaleImage(1.25f);
}

void wdQtImageWidget::on_ButtonZoomOut_clicked()
{
  ScaleImage(0.75f);
}

void wdQtImageWidget::on_ButtonResetZoom_clicked()
{
  SetImageSize(1.0f);
}
