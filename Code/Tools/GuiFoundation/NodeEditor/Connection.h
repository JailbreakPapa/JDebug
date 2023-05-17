#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/NodeEditor/NodeScene.moc.h>
#include <QGraphicsPathItem>

class wdPin;

class WD_GUIFOUNDATION_DLL wdQtConnection : public QGraphicsPathItem
{
public:
  explicit wdQtConnection(QGraphicsItem* pParent = 0);
  ~wdQtConnection();
  virtual int type() const override { return wdQtNodeScene::Connection; }

  const wdDocumentObject* GetObject() const { return m_pObject; }
  const wdConnection* GetConnection() const { return m_pConnection; }
  void InitConnection(const wdDocumentObject* pObject, const wdConnection* pConnection);

  void SetPosIn(const QPointF& point);
  void SetPosOut(const QPointF& point);
  void SetDirIn(const QPointF& dir);
  void SetDirOut(const QPointF& dir);

  virtual void UpdateGeometry();
  virtual QPen DeterminePen() const;

  const QPointF& GetInPos() const { return m_InPoint; }
  const QPointF& GetOutPos() const { return m_OutPoint; }

  bool m_bAdjacentNodeSelected = false;

  virtual void ExtendContextMenu(QMenu& ref_menu) {}

protected:
  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

  const wdDocumentObject* m_pObject = nullptr;
  const wdConnection* m_pConnection = nullptr;

  QPointF m_InPoint;
  QPointF m_OutPoint;
  QPointF m_InDir;
  QPointF m_OutDir;
};
