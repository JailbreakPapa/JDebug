// Copyright (C) Mikael Aboagye All Rights Reserved.
#pragma once
#include <RenderFramework/RenderHelpers/DXHelper.h>
#include <RenderFramework/RenderHelpers/RenderTypes.h>

namespace RenderFramework::D3D12 {
class RFD3D12RHI {
 public:
  //************************************
  // Method:    RFD3D12RHI
  // FullName:  RenderFramework::D3D12::RFD3D12RHI::RFD3D12RHI
  // Access:    public
  // Returns:   nothing
  // Qualifier:
  // Parameter: SwapInfo & Info
  // Summary: Creates ID3D12 Context,And DXGI Context.
  //************************************
  explicit RFD3D12RHI(SwapInfo& Info);
  //************************************
  // Method:    ~RFD3D12RHI
  // FullName:  RenderFramework::D3D12::RFD3D12RHI::~RFD3D12RHI
  // Access:    virtual public
  // Returns:   nothing/destructor
  // Qualifier:
  // Summary: D3D12 RHI Destructor: Cleans up after the object is destroyed.
  //************************************
  virtual ~RFD3D12RHI();

  //************************************
  // Method:    Present
  // FullName:  RenderFramework::D3D12::RFD3D12RHI::Present
  // Access:    public
  // Returns:   void
  // Qualifier:
  // Summary: Presents To The Swap chain. All Queues Must Be Ready to present.
  //************************************
  void Present();



 protected:
  //************************************
  // Method:    LoadAssets
  // FullName:  RenderFramework::D3D12::RFD3D12RHI::LoadAssets
  // Access:    protected
  // Returns:   void
  // Qualifier:
  // Source: Loads ALL Assets That are being drawn.
  //************************************
  void LoadAssets();

 private:
  ComPtr<ID3D12Device> DXDevice;
  ComPtr<ID3D12Debug> DXDebug;
  ComPtr<ID3D12DebugDevice1> DXDebugDevice;
  ComPtr<IDXGIFactory> DXFactory;
  ComPtr<IDXGISwapChain3> DXSwap;


  uint32_t RTVSize;
  uint32_t DSVSize;
  RFCommandQueue DrawQueue;
  // VSync.
  BOOL VS = FALSE;
  int rwidth;
  int rheight;

 private:
};
}  // namespace RenderFramework::D3D12