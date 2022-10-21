// Copyright (C) Mikael Aboagye All Rights Reserved.
#include "RFD3D12RHI.h"

RenderFramework::D3D12::RFD3D12RHI::RFD3D12RHI(SwapInfo& Info) {
  ThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(&DXFactory)));
#ifdef RFRENDERDEBUG
  ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(DXDebug.GetAddressOf())));
  DXDebug->EnableDebugLayer();
#endif
  ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0,
                                  IID_PPV_ARGS(DXDevice.GetAddressOf())));
#ifdef RFRENDERDEBUG
  ThrowIfFailed(DXDevice->QueryInterface(DXDebugDevice.GetAddressOf()));
  DXDebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
#endif
  D3D12_COMMAND_QUEUE_DESC d{D3D12_COMMAND_LIST_TYPE_DIRECT,
                             D3D12_COMMAND_QUEUE_PRIORITY_HIGH,
                             D3D12_COMMAND_QUEUE_FLAG_NONE, 0};
  ThrowIfFailed(
      DXDevice->CreateCommandQueue(&d, IID_PPV_ARGS(&DrawQueue.Queue)));
  // SwapChain Creation.
  ComPtr<IDXGISwapChain> sa;
  DXGI_SWAP_CHAIN_DESC sd{
      DXGI_MODE_DESC{
          Info.WidgetWidth, Info.WidgetHeight, DXGI_RATIONAL{Info.MaxFrames, 1},
          DXGI_FORMAT_B8G8R8A8_UNORM,
          DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
          DXGI_MODE_SCALING_UNSPECIFIED},
      DXGI_SAMPLE_DESC{1, 0},
      DXGI_USAGE_RENDER_TARGET_OUTPUT,
      FrameC,
      Info.hwnd_,
      true,
      DXGI_SWAP_EFFECT_FLIP_DISCARD,
      DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH};
  ThrowIfFailed(DXFactory->CreateSwapChain(DrawQueue.Queue.Get(), &sd,
                                           sa.GetAddressOf()));
  ThrowIfFailed(sa.As(&DXSwap));
  SetupRTV();
  SetupDSV();
  LoadAssets();
  BindResources();
}

RenderFramework::D3D12::RFD3D12RHI::~RFD3D12RHI() { }

void RenderFramework::D3D12::RFD3D12RHI::Present() {
  // Call At The End.
  DrawQueue.WaitForFrame();
}

bool RenderFramework::D3D12::RFD3D12RHI::AllQueuesReady() { return false; }

void RenderFramework::D3D12::RFD3D12RHI::BindResources() { }

void RenderFramework::D3D12::RFD3D12RHI::SetupRTV() {
  for (auto a = 0; a < FrameC; a++) {
  }
}

void RenderFramework::D3D12::RFD3D12RHI::SetupDSV() { }

void RenderFramework::D3D12::RFD3D12RHI::LoadAssets() {
  // CALL AT THE END.
  DrawQueue.CreateSyncObjects(DXDevice.Get());
}