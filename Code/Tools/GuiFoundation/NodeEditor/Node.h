#pragma once

#include <Foundation/Containers/HybridArray.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/NodeEditor/NodeScene.moc.h>
#include <QGraphicsWidget>

// Avoid conflicts with windows.
#ifdef GetObject
#  undef GetObject
#endif

class wdQtPin;
class wdDocumentNodeManager;
class QLabel;
class wdDocumentObject;
class QGraphicsGridLayout;
class QGraphicsTextItem;
class QGraphicsDropShadowEffect;

struct wdNodeFlags
{
  typedef wdUInt8 StorageType;

  enum Enum
  {
    None = 0,
    Moved = WD_BIT(0),
    UpdateTitle = WD_BIT(1),
    Default = None
  };

  struct Bits
  {
    StorageType Moved : 1;
    StorageType UpdateTitle : 1;
  };
};

class WD_GUIFOUNDATION_DLL wdQtNode : public QGraphicsPathItem
{
public:
  wdQtNode();
  ~wdQtNode();
  virtual int type() const override { return wdQtNodeScene::Node; }

  const wdDocumentObject* GetObject() const { return m_pObject; }
  virtual void InitNode(const wdDocumentNodeManager* pManager, const wdDocumentObject* pObject);

  virtual void UpdateGeometry();

  void CreatePins();

  wdQtPin* GetInputPin(const wdPin& pin);
  wdQtPin* GetOutputPin(const wdPin& pin);

  wdBitflags<wdNodeFlags> GetFlags() const;
  void ResetFlags();

  void EnableDropShadow(bool bEnable);
  virtual void UpdateState();

  const wdHybridArray<wdQtPin*, 6>& GetInputPins() const { return m_Inputs; }
  const wdHybridArray<wdQtPin*, 6>& GetOutputPins() const { return m_Outputs; }

  void SetActive(bool bActive);

  virtual void ExtendContextMenu(QMenu& ref_menu) {}

protected:
  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
  virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

  QColor m_HeaderColor;
  QRectF m_HeaderRect;
  QGraphicsTextItem* m_pLabel = nullptr;

private:
  const wdDocumentNodeManager* m_pManager = nullptr;
  const wdDocumentObject* m_pObject = nullptr;
  wdBitflags<wdNodeFlags> m_DirtyFlags;

  bool m_bIsActive = true;

  QGraphicsDropShadowEffect* m_pShadow = nullptr;

  // Pins
  wdHybridArray<wdQtPin*, 6> m_Inputs;
  wdHybridArray<wdQtPin*, 6> m_Outputs;
};
