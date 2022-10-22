// Copyright (C) Mikael Aboagye All Rights Reserved.
#pragma once

#include <RenderFramework/D3D12/RFD3D12RHI.h>
#include <RenderFramework/RenderHelpers/RenderTypes.h>
#include <qpointer.h>

#include <QWidget>

#include "ui_jdrenderview.h"
class JDRenderView : public QWidget {
  Q_OBJECT

 public:
  JDRenderView(QWidget *parent = nullptr);
  ~JDRenderView();

 private:
  SwapInfo P{60, 1, (HWND)this->winId(), (uint32_t)this->width(),
             (uint32_t)this->height()};
  Ui::JDRenderViewClass ui;
};
