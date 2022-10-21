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

  //************************************
  // Method:    AllQueuesReady
  // FullName:  RenderFramework::D3D12::RFD3D12RHI::AllQueuesReady
  // Access:    public
  // Returns:   bool
  // Qualifier:
  // Summary: Checks if ALL Queues on all threads are ready to present.
  //************************************
  bool AllQueuesReady();
  static uint16_t SwapIndex;

 protected:
  //************************************
  // Method:    BindResources
  // FullName:  RenderFramework::D3D12::RFD3D12RHI::BindResources
  // Access:    protected
  // Returns:   void
  // Qualifier:
  // Summary: Binds All resources needed to present.
  //************************************
  void BindResources();

  //************************************
  // Method:    SetupRTV
  // FullName:  RenderFramework::D3D12::RFD3D12RHI::SetupRTV
  // Access:    protected
  // Returns:   void
  // Qualifier:
  //************************************
  void SetupRTV();
  //************************************
  // Method:    SetupDSV
  // FullName:  RenderFramework::D3D12::RFD3D12RHI::SetupDSV
  // Access:    protected
  // Returns:   void
  // Qualifier:
  //************************************
  void SetupDSV();

 private:
  ComPtr<ID3D12Device> DXDevice;
  ComPtr<ID3D12Debug> DXDebug;
  ComPtr<ID3D12DebugDevice1> DXDebugDevice;
  ComPtr<IDXGIFactory> DXFactory;
  ComPtr<IDXGISwapChain3> DXSwap;
  ComPtr<ID3D12CommandAllocator> DrawAlloc;
  ComPtr<ID3D12Resource> RTVResource;
  ComPtr<ID3D12Resource> DSVResource;
  ComPtr<ID3D12DescriptorHeap> RTVHeap;
  ComPtr<ID3D12DescriptorHeap> DSVHeap;
  ComPtr<ID3D12GraphicsCommandList> DrawList;
  uint32_t RTVSize;
  uint32_t DSVSize;
  RFCommandQueue DrawQueue;

 private:
};
}  // namespace RenderFramework::D3D12