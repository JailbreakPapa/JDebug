#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>

#include <cstdint>
#pragma once
#ifndef RFRENDERSTATE
#ifdef _DEBUG
#define RFRENDERDEBUG
#elif NDEBUG
#define RFRENDERRELEASE
#endif
#endif
inline constexpr static int FrameC = 3;
struct SwapInfo {
  uint16_t MaxFrames = 60;
  uint8_t VSyncInterval = 1;
  HWND hwnd_;
  uint32_t WidgetWidth = 800;
  uint32_t WidgetHeight = 600;
};

struct RFCommandQueue {
 public:
  //************************************
  // Method:    CreateSyncObjects
  // FullName:  RFCommandQueue::CreateSyncObjects
  // Access:    public
  // Returns:   void
  // Qualifier:
  // Summary: Create synchronization objects.
  //************************************
  void CreateSyncObjects();
  //************************************
  // Method:    WaitForFrame
  // FullName:  RFCommandQueue::WaitForFrame
  // Access:    public
  // Returns:   void
  // Qualifier:
  // Source: Wait For Everything To Execute.
  //************************************
  void WaitForFrame();

 public:
  Microsoft::WRL::ComPtr<ID3D12CommandQueue> Queue;
  Microsoft::WRL::ComPtr<ID3D12Fence> Fence[FrameC];
  // Synchronization objects.
  UINT m_frameIndex;
  HANDLE m_fenceEvent;
  UINT64 m_fenceValue;
  // Is The Queue Ready To be cleaned/reused?
  bool Ready = false;
};