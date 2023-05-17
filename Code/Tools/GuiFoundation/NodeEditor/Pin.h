#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/NodeEditor/NodeScene.moc.h>
#include <QGraphicsPathItem>

class wdPin;
class wdQtConnection;

enum class wdQtPinHighlightState
{
  None,
  CannotConnect,
  CannotConnectSameDirection,
  CanAddConnection,
  CanReplaceConnection,
};

class WD_GUIFOUNDATION_DLL wdQtPin : public QGraphicsPathItem
{
public:
  wdQtPin();
  ~wdQtPin();
  virtual int type() const override { return wdQtNodeScene::Pin; }

  void AddConnection(wdQtConnection* pConnection);
  void RemoveConnection(wdQtConnection* pConnection);
  wdArrayPtr<wdQtConnection*> GetConnections() { return m_Connections; }
  bool HasAnyConnections() const { return !m_Connections.IsEmpty(); }

  const wdPin* GetPin() const { return m_pPin; }
  virtual void SetPin(const wdPin& pin);
  virtual void ConnectedStateChanged(bool bConnected);

  virtual QPointF GetPinPos() const;
  virtual QPointF GetPinDir() const;
  virtual QRectF GetPinRect() const;
  virtual void UpdateConnections();
  void SetHighlightState(wdQtPinHighlightState state);

  void SetActive(bool bActive);

  virtual void ExtendContextMenu(QMenu& ref_menu) {}
  virtual void keyPressEvent(QKeyEvent* pEvent) override {}

protected:
  virtual bool AdjustRenderingForHighlight(wdQtPinHighlightState state);
  virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

  wdQtPinHighlightState m_HighlightState = wdQtPinHighlightState::None;
  QGraphicsTextItem* m_pLabel;
  QPointF m_PinCenter;

private:
  bool m_bIsActive = true;

  const wdPin* m_pPin = nullptr;
  wdHybridArray<wdQtConnection*, 6> m_Connections;
};
