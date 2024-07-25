#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/NodeEditor/NodeScene.moc.h>
#include <QGraphicsPathItem>

class nsPin;
class nsQtConnection;

enum class nsQtPinHighlightState
{
  None,
  CannotConnect,
  CannotConnectSameDirection,
  CanAddConnection,
  CanReplaceConnection,
};

class NS_GUIFOUNDATION_DLL nsQtPin : public QGraphicsPathItem
{
public:
  nsQtPin();
  ~nsQtPin();
  virtual int type() const override { return nsQtNodeScene::Pin; }

  void AddConnection(nsQtConnection* pConnection);
  void RemoveConnection(nsQtConnection* pConnection);
  nsArrayPtr<nsQtConnection*> GetConnections() { return m_Connections; }
  bool HasAnyConnections() const { return !m_Connections.IsEmpty(); }

  const nsPin* GetPin() const { return m_pPin; }
  virtual void SetPin(const nsPin& pin);
  virtual void ConnectedStateChanged(bool bConnected);

  virtual QPointF GetPinPos() const;
  virtual QPointF GetPinDir() const;
  virtual QRectF GetPinRect() const;
  virtual void UpdateConnections();
  void SetHighlightState(nsQtPinHighlightState state);

  void SetActive(bool bActive);

  virtual void ExtendContextMenu(QMenu& ref_menu) {}
  virtual void keyPressEvent(QKeyEvent* pEvent) override {}

protected:
  virtual bool UpdatePinColors(const nsColorGammaUB* pOverwriteColor = nullptr);
  virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

  nsQtPinHighlightState m_HighlightState = nsQtPinHighlightState::None;
  QGraphicsTextItem* m_pLabel;
  QPointF m_PinCenter;

  bool m_bTranslatePinName = true;

private:
  bool m_bIsActive = true;

  const nsPin* m_pPin = nullptr;
  nsHybridArray<nsQtConnection*, 6> m_Connections;
};
