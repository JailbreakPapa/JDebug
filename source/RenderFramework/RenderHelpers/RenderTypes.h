#include <Windows.h>
#include <wrl.h>

#include <cstdint>
#pragma once
inline constexpr static int FrameC = 3;
struct SwapInfo {
  uint16_t MaxFrames = 60;
  uint8_t VSyncInterval = 1;
  HWND hwnd_;
  uint32_t WidgetWidth = 800;
  uint32_t WidgetHeight = 600;
};

struct RFCommandQueue {
  Microsoft::WRL::ComPtr<ID3D12CommandQueue> Queue;
  Microsoft::WRL::ComPtr<ID3D12Fence> Fence[FrameC];
  // Is The Queue Ready To Present?
  bool Ready = false;
};