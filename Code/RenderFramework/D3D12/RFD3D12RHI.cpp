// Copyright (C) Mikael Aboagye All Rights Reserved.
#include "RFD3D12RHI.h"

#include <dxgi1_4.h>

RenderFramework::D3D12::RFD3D12RHI::RFD3D12RHI(SwapInfo& Info) {
  rwidth = Info.WidgetWidth;
  rheight = Info.WidgetHeight;
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
  SwapIndex = DXSwap->GetCurrentBackBufferIndex();
  ThrowIfFailed(DXDevice->CreateCommandAllocator(
      D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(DrawAlloc.GetAddressOf())));
  ThrowIfFailed(DXDevice->CreateCommandList(
      NULL, D3D12_COMMAND_LIST_TYPE_DIRECT, DrawAlloc.Get(), nullptr,
      IID_PPV_ARGS(DrawList.GetAddressOf())));
  DrawList->Close();
  LoadAssets();
}

RenderFramework::D3D12::RFD3D12RHI::~RFD3D12RHI() { }

void RenderFramework::D3D12::RFD3D12RHI::Present() {
  static float clear_color[4] = {0.568f, 1.0f, 1.0f, 1.0f};
  SwapIndex = DXSwap->GetCurrentBackBufferIndex();
  DrawAlloc->Reset();
  DrawList->Reset(DrawAlloc.Get(), nullptr);
  SwapIndex = DXSwap->GetCurrentBackBufferIndex();
  CD3DX12_CPU_DESCRIPTOR_HANDLE m_rtvhandle(
      RTVHeap->GetCPUDescriptorHandleForHeapStart(), SwapIndex, RTVSize);
  CD3DX12_CPU_DESCRIPTOR_HANDLE m_dsvhandle(
      DSVHeap->GetCPUDescriptorHandleForHeapStart(), SwapIndex, DSVSize);
  DrawList->OMSetRenderTargets(1, &m_rtvhandle, TRUE, &m_dsvhandle);
  /// <summary>
  /// The DebugLayer will scream at us currently due to not transitioning the RTV & DSV Resources accordingly.
  /// We Should Do that....
  /// </summary>
  DrawList->ClearRenderTargetView(m_rtvhandle, clear_color, NULL, nullptr);
  DrawList->ClearDepthStencilView(m_dsvhandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0,
                                  NULL, nullptr);
  DrawList->Close();

  DrawQueue.Queue->ExecuteCommandLists(
      1, (ID3D12CommandList**)DrawList.GetAddressOf());
  if (VS)
    ThrowIfFailed(DXSwap->Present(1, 0));
  else {
    ThrowIfFailed(DXSwap->Present(0, 0));
  }

  // Call At The End.
  DrawQueue.WaitForFrame();
  SwapIndex = DXSwap->GetCurrentBackBufferIndex();
}

bool RenderFramework::D3D12::RFD3D12RHI::AllQueuesReady() { return false; }

void RenderFramework::D3D12::RFD3D12RHI::LoadAssets() {
  D3D12_DESCRIPTOR_HEAP_DESC rtvheapdesc{D3D12_DESCRIPTOR_HEAP_TYPE_RTV, FrameC,
                                         D3D12_DESCRIPTOR_HEAP_FLAG_NONE, NULL};
  ThrowIfFailed(DXDevice->CreateDescriptorHeap(
      &rtvheapdesc, IID_PPV_ARGS(RTVHeap.GetAddressOf())));
  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvhandle(
      RTVHeap->GetCPUDescriptorHandleForHeapStart());
  RTVSize = DXDevice->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  for (auto a = 0; a < FrameC; a++) {
    ThrowIfFailed(
        DXSwap->GetBuffer(a, IID_PPV_ARGS(RTVResource[a].GetAddressOf())));
    DXDevice->CreateRenderTargetView(RTVResource[a].Get(), nullptr, rtvhandle);
    rtvhandle.Offset(RTVSize);
  }
  D3D12_DESCRIPTOR_HEAP_DESC dsvheapdesc{D3D12_DESCRIPTOR_HEAP_TYPE_DSV, FrameC,
                                         D3D12_DESCRIPTOR_HEAP_FLAG_NONE, NULL};
  ThrowIfFailed(DXDevice->CreateDescriptorHeap(
      &dsvheapdesc, IID_PPV_ARGS(DSVHeap.GetAddressOf())));
  CD3DX12_CPU_DESCRIPTOR_HANDLE dsvhandle(
      DSVHeap->GetCPUDescriptorHandleForHeapStart());
  DSVSize = DXDevice->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

  D3D12_DEPTH_STENCIL_VIEW_DESC vd{
      DXGI_FORMAT_D24_UNORM_S8_UINT,
      D3D12_DSV_DIMENSION_TEXTURE2D,
      D3D12_DSV_FLAG_NONE,
      1,
  };
  CD3DX12_RESOURCE_DESC dsvt{
      D3D12_RESOURCE_DIMENSION_TEXTURE2D,
      NULL,
      (UINT64)rwidth,
      (UINT64)rheight,
      1,
      0,
      vd.Format,
      1,
      0,
      D3D12_TEXTURE_LAYOUT_UNKNOWN,
      D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL};
  CD3DX12_HEAP_PROPERTIES dsvheap(D3D12_HEAP_TYPE_DEFAULT);
  for (auto a = 0; a < FrameC; a++) {
    // NOTE: DSVResource Will Be Null Since We DIDNT Create the resources by
    // them selfs! CREATE WITH A COMMITTED RESOURCE(S)!!
    ThrowIfFailed(DXDevice->CreateCommittedResource(
        &dsvheap, D3D12_HEAP_FLAG_NONE, &dsvt,
        D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON, nullptr,
        IID_PPV_ARGS(DSVResource[a].GetAddressOf())));
    DXDevice->CreateDepthStencilView(DSVResource[a].Get(), &vd, dsvhandle);
    dsvhandle.Offset(DSVSize);
  }
  ThrowIfFailed(DXDevice->CreateCommandAllocator(
      D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(DrawAlloc.GetAddressOf())));
  ThrowIfFailed(DXDevice->CreateCommandList(
      NULL, D3D12_COMMAND_LIST_TYPE_DIRECT, DrawAlloc.Get(), nullptr,
      IID_PPV_ARGS(DrawList.GetAddressOf())));
  DrawList->Reset(DrawAlloc.Get(), nullptr);

  // Do PSO(S), Draw Commands Here.(On Different Threads,Etc...)

  // CALL AT THE END.
  DrawQueue.CreateSyncObjects(DXDevice.Get());
}