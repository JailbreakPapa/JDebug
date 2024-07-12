#include <RenderFramework/RenderHelpers/DXHelper.h>
#include <RenderFramework/RenderHelpers/RenderTypes.h>

void RFCommandQueue::CreateSyncObjects(ID3D12Device* Device) {
  ThrowIfFailed(
      Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));
  m_fenceValue = 1;
  // Create an event handle to use for frame synchronization.
  m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  if (m_fenceEvent == nullptr) {
    ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
  }
}

void RFCommandQueue::WaitForFrame() {
  // Signal and increment the fence value.
  const UINT64 fence = m_fenceValue;
  ThrowIfFailed(Queue->Signal(Fence.Get(), fence));
  m_fenceValue++;
  // Wait until the previous frame is finished.
  if (Fence->GetCompletedValue() < fence) {
    ThrowIfFailed(Fence->SetEventOnCompletion(fence, m_fenceEvent));
    WaitForSingleObject(m_fenceEvent, INFINITE);
  }
}