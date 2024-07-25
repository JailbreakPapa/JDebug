#pragma once

#include <Foundation/Containers/HybridArray.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/NodeEditor/NodeScene.moc.h>
#include <QGraphicsWidget>

// Avoid conflicts with windows.
#ifdef GetObject
#  undef GetObject
#endif

class nsQtPin;
class nsDocumentNodeManager;
class QLabel;
class nsDocumentObject;
class QGraphicsTextItem;
class QGraphicsPixmapItem;
class QGraphicsDropShadowEffect;

struct nsNodeFlags
{
  using StorageType = nsUInt8;

  enum Enum
  {
    None = 0,
    Moved = NS_BIT(0),
    UpdateTitle = NS_BIT(1),
    Default = None
  };

  struct Bits
  {
    StorageType Moved : 1;
    StorageType UpdateTitle : 1;
  };
};

class NS_GUIFOUNDATION_DLL nsQtNode : public QGraphicsPathItem
{
public:
  nsQtNode();
  ~nsQtNode();
  virtual int type() const override { return nsQtNodeScene::Node; }

  const nsDocumentObject* GetObject() const { return m_pObject; }
  virtual void InitNode(const nsDocumentNodeManager* pManager, const nsDocumentObject* pObject);

  virtual void UpdateGeometry();

  void CreatePins();

  nsQtPin* GetInputPin(const nsPin& pin);
  nsQtPin* GetOutputPin(const nsPin& pin);

  nsBitflags<nsNodeFlags> GetFlags() const;
  void ResetFlags();

  void EnableDropShadow(bool bEnable);
  virtual void UpdateState();

  const nsHybridArray<nsQtPin*, 6>& GetInputPins() const { return m_Inputs; }
  const nsHybridArray<nsQtPin*, 6>& GetOutputPins() const { return m_Outputs; }

  void SetActive(bool bActive);

  virtual void ExtendContextMenu(QMenu& ref_menu) {}

protected:
  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
  virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

  QColor m_HeaderColor;
  QRectF m_HeaderRect;
  QGraphicsTextItem* m_pTitleLabel = nullptr;
  QGraphicsTextItem* m_pSubtitleLabel = nullptr;
  QGraphicsPixmapItem* m_pIcon = nullptr;

private:
  const nsDocumentNodeManager* m_pManager = nullptr;
  const nsDocumentObject* m_pObject = nullptr;
  nsBitflags<nsNodeFlags> m_DirtyFlags;

  bool m_bIsActive = true;

  QGraphicsDropShadowEffect* m_pShadow = nullptr;

  // Pins
  nsHybridArray<nsQtPin*, 6> m_Inputs;
  nsHybridArray<nsQtPin*, 6> m_Outputs;
};
