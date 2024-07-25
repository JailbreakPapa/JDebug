#pragma once

#include <GuiFoundation/NodeEditor/Node.h>

/**
 * @class nsQtCommentNode
 * 
 * @brief Comment node for the node editor.
 * 
 * @JailbreakPapa (Mikael A.) - The comment node should just derive off of QGraphicsPathItem, as it wont need to be represented as a normal node, as it only contains a text label.
 */
class NS_GUIFOUNDATION_DLL nsQtCommentNode : public QGraphicsPathItem
{
public:
    /**
     * @brief Default constructor.
     */
    nsQtCommentNode();

    /**
     * @brief Destructor.
     */
    ~nsQtCommentNode();

    /**
     * @brief Returns the type of the comment node.
     * 
     * @return The type of the comment node.
     */
    virtual int type() const override { return nsQtNodeScene::Comment; }

    /**
     * @brief Gets the associated nsDocumentObject.
     * 
     * @return The associated nsDocumentObject.
     */
    const nsDocumentObject* GetObject() const { return m_pObject; }

    /**
     * @brief Initializes the comment node with the specified manager and object.
     * 
     * @param pManager The nsDocumentNodeManager.
     * @param pObject The nsDocumentObject.
     */
    virtual void InitComment(const nsDocumentNodeManager* pManager, const nsDocumentObject* pObject);
    void CreateComment();
    /**
     * @brief Updates the comment node's rect (Containing Space, relative to the NodeScene.) 
     * This also updates the geometry if needed.
     */
    virtual void UpdateCommentRect();

    /**
     * @brief Sets the main text of the comment node.
     * 
     * @param text The main text.
     */
    void SetCommentMainText(const nsString& text);

    /**
     * @brief Sets the subtext of the comment node.
     * 
     * @param text The subtext.
     */
    void SetCommentSubText(const nsString& text);

    /**
     * @brief Enables or disables the drop shadow effect for the comment node.
     * 
     * @param bEnable True to enable the drop shadow effect, false to disable it.
     */
    void EnableDropShadow(bool bEnable);

protected:
    /**
     * @brief Paints the comment node.
     * 
     * @param painter The QPainter object.
     * @param option The QStyleOptionGraphicsItem object.
     * @param widget The QWidget object.
     */
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    nsBitflags<nsNodeFlags> GetFlags() const;
    /**
     * @brief Handles item change events.
     * 
     * @param change The type of change.
     * @param value The new value.
     * @return The new value.
     */
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    nsBitflags<nsNodeFlags> m_DirtyFlags; ///< The dirty flags.
    QColor m_HeaderColor; ///< The color of the header.
    QRectF m_HeaderRect; ///< The rectangle of the header.
    QGraphicsTextItem* m_pTitleLabel = nullptr; ///< The title label.
    QGraphicsTextItem* m_pSubtitleLabel = nullptr; ///< The subtitle label.
    QGraphicsPixmapItem* m_pIcon = nullptr; ///< The icon displayed on the comment node.
    const nsDocumentNodeManager* m_pManager = nullptr; ///< The nsDocumentNodeManager.
    const nsDocumentObject* m_pObject = nullptr; ///< The nsDocumentObject.
    QGraphicsDropShadowEffect* m_pShadow = nullptr; ///< The drop shadow effect.
};
