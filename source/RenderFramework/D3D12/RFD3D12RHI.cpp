// Copyright (C) Mikael Aboagye All Rights Reserved.
#include "RFD3D12RHI.h"

#include <dxgi1_4.h>

RenderFramework::D3D12::RFD3D12RHI::RFD3D12RHI(SwapInfo& Info) {
  VS = (BOOL)Info.VSyncInterval ? 0 : 1;
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

  rwidth = Info.WidgetWidth;
  rheight = Info.WidgetHeight;
  SwapIndex = DXSwap->GetCurrentBackBufferIndex();

  LoadAssets();
}

RenderFramework::D3D12::RFD3D12RHI::~RFD3D12RHI() { }

void RenderFramework::D3D12::RFD3D12RHI::Present() {
  static float clear_color[4] = {0.568f, 0.733f, 1.0f, 1.0f};
  SwapIndex = DXSwap->GetCurrentBackBufferIndex();
  SwapIndex = DXSwap->GetCurrentBackBufferIndex();
}

void RenderFramework::D3D12::RFD3D12RHI::LoadAssets() { }