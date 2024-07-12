/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/Widgets/GroupBoxBase.moc.h>

class NS_GUIFOUNDATION_DLL nsQtInlinedGroupBox : public nsQtGroupBoxBase
{
  Q_OBJECT
public:
  explicit nsQtInlinedGroupBox(QWidget* pParent);

  virtual void SetTitle(const char* szTitle) override;
  virtual void SetIcon(const QIcon& icon) override;
  virtual void SetFillColor(const QColor& color) override;

  virtual void SetCollapseState(bool bCollapsed) override;
  virtual bool GetCollapseState() const override;

  virtual QWidget* GetContent() override;
  virtual QWidget* GetHeader() override;

protected:
  virtual void paintEvent(QPaintEvent* event) override;
  virtual bool eventFilter(QObject* object, QEvent* event) override;

protected:
  QWidget* m_pContent;
  QWidget* m_pHeader;
};

