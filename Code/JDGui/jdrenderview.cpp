#include "jdrenderview.h"

JDRenderView::JDRenderView(QWidget *parent) : QWidget(parent) {
  ui.setupUi(this);
  std::unique_ptr<RenderFramework::D3D12::RFD3D12RHI> DX12 =
      std::make_unique<RenderFramework::D3D12::RFD3D12RHI>(P);
  DX12->Present();
}

JDRenderView::~JDRenderView() { }