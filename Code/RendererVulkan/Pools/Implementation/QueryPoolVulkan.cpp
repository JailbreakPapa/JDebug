#include <RendererVulkan/RendererVulkanPCH.h>

#include <RendererVulkan/Pools/QueryPoolVulkan.h>

#include <RendererVulkan/Device/DeviceVulkan.h>

void nsQueryPoolVulkan::Initialize(nsGALDeviceVulkan* pDevice, nsUInt32 uiValidBits)
{
  m_pDevice = pDevice;
  m_device = pDevice->GetVulkanDevice();
  NS_ASSERT_DEV(pDevice->GetPhysicalDeviceProperties().limits.timestampComputeAndGraphics, "Timestamps not supported by hardware.");
  m_fNanoSecondsPerTick = pDevice->GetPhysicalDeviceProperties().limits.timestampPeriod;

  m_uiValidBitsMask = uiValidBits == 64 ? nsMath::MaxValue<nsUInt64>() : ((1ull << uiValidBits) - 1);
}

void nsQueryPoolVulkan::DeInitialize()
{
  for (FramePool& framePool : m_pendingFrames)
  {
    for (nsUInt32 i = 0; i < framePool.m_pools.GetCount(); i++)
    {
      m_freePools.PushBack(framePool.m_pools[i]);
    }
  }
  m_pendingFrames.Clear();
  for (TimestampPool* pPool : m_freePools)
  {
    m_device.destroyQueryPool(pPool->m_pool);
    NS_DEFAULT_DELETE(pPool);
  }
  m_freePools.Clear();
}

void nsQueryPoolVulkan::Calibrate()
{
  // To correlate CPU to GPU time, we create an event, wait a bit for the GPU to get stuck on it and then signal it.
  // We then observe the time right after on the CPU and on the GPU via a timestamp query.
  vk::EventCreateInfo eventCreateInfo;
  vk::Event event;
  VK_ASSERT_DEV(m_device.createEvent(&eventCreateInfo, nullptr, &event));

  m_device.waitIdle();
  vk::CommandBuffer cb = m_pDevice->GetCurrentCommandBuffer();
  cb.waitEvents(1, &event, vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eHost | vk::PipelineStageFlagBits::eTopOfPipe, 0, nullptr, 0, nullptr, 0, nullptr);
  auto hTimestamp = GetTimestamp();
  InsertTimestamp(cb, hTimestamp, vk::PipelineStageFlagBits::eTopOfPipe);
  vk::Fence fence = m_pDevice->Submit();

  nsTime systemTS;
  nsThreadUtils::Sleep(nsTime::Milliseconds(100)); // Waiting for 100ms should be enough for the GPU to have gotten stuck on the event right?
  m_device.setEvent(event);
  systemTS = nsTime::Now();

  VK_ASSERT_DEV(m_device.waitForFences(1, &fence, true, nsMath::MaxValue<nsUInt64>()));

  nsTime gpuTS;
  GetTimestampResult(hTimestamp, gpuTS, true).AssertSuccess();
  m_gpuToCpuDelta = systemTS - gpuTS;

  m_device.destroyEvent(event);
}

void nsQueryPoolVulkan::BeginFrame(vk::CommandBuffer commandBuffer)
{
  nsUInt64 uiCurrentFrame = m_pDevice->GetCurrentFrame();
  nsUInt64 uiSafeFrame = m_pDevice->GetSafeFrame();

  m_pCurrentFrame = &m_pendingFrames.ExpandAndGetRef();
  m_pCurrentFrame->m_uiFrameCounter = uiCurrentFrame;
  m_pCurrentFrame->m_uiNextIndex = 0;
  m_pCurrentFrame->m_pools.PushBack(GetFreePool());

  // Get results
  for (FramePool& framePool : m_pendingFrames)
  {
    if (framePool.m_uiFrameCounter > uiSafeFrame)
      break;
    for (nsUInt32 i = 0; i < framePool.m_pools.GetCount(); i++)
    {
      TimestampPool* pPool = framePool.m_pools[i];

      if (!pPool->m_bReady)
      {
        nsUInt32 uiQueryCount = s_uiPoolSize;
        if (i + 1 == m_pendingFrames[0].m_pools.GetCount())
          uiQueryCount = framePool.m_uiNextIndex % s_uiPoolSize;
        if (uiQueryCount > 0)
        {
          vk::Result res = m_device.getQueryPoolResults(pPool->m_pool, 0, uiQueryCount, s_uiPoolSize * sizeof(nsUInt64), pPool->m_queryResults.GetData(), sizeof(nsUInt64), vk::QueryResultFlagBits::e64);
          if (res == vk::Result::eSuccess)
          {
            pPool->m_bReady = true;
          }
        }
      }
    }
  }

  // Clear out old frames
  while (m_pendingFrames[0].m_uiFrameCounter + s_uiRetainFrames < uiSafeFrame)
  {
    for (TimestampPool* pPool : m_pendingFrames[0].m_pools)
    {
      pPool->m_bReady = false;
      m_freePools.PushBack(pPool);
      m_resetPools.PushBack(pPool->m_pool);
    }
    m_pendingFrames.PopFront();
  }

  m_uiFirstFrameIndex = m_pendingFrames[0].m_uiFrameCounter;

  if (m_gpuToCpuDelta.IsZero())
  {
    Calibrate();
  }

  for (vk::QueryPool pool : m_resetPools)
  {
    commandBuffer.resetQueryPool(pool, 0, s_uiPoolSize);
  }
  m_resetPools.Clear();
}

nsGALTimestampHandle nsQueryPoolVulkan::GetTimestamp()
{
  const nsUInt64 uiPoolIndex = m_pCurrentFrame->m_uiNextIndex / s_uiPoolSize;
  if (uiPoolIndex == m_pCurrentFrame->m_pools.GetCount())
  {
    m_pCurrentFrame->m_pools.PushBack(GetFreePool());
  }

  nsGALTimestampHandle res = {m_pCurrentFrame->m_uiNextIndex, m_pCurrentFrame->m_uiFrameCounter};
  m_pCurrentFrame->m_uiNextIndex++;
  return res;
}

void nsQueryPoolVulkan::InsertTimestamp(vk::CommandBuffer commandBuffer, nsGALTimestampHandle hTimestamp, vk::PipelineStageFlagBits pipelineStage)
{
  for (vk::QueryPool pool : m_resetPools)
  {
    commandBuffer.resetQueryPool(pool, 0, s_uiPoolSize);
  }
  m_resetPools.Clear();
  const nsUInt32 uiPoolIndex = (nsUInt32)hTimestamp.m_uiIndex / s_uiPoolSize;
  const nsUInt32 uiQueryIndex = (nsUInt32)hTimestamp.m_uiIndex % s_uiPoolSize;
  NS_ASSERT_DEBUG(hTimestamp.m_uiFrameCounter == m_pCurrentFrame->m_uiFrameCounter, "Timestamps must be created and used in the same frame!");

  commandBuffer.writeTimestamp(pipelineStage, m_pCurrentFrame->m_pools[uiPoolIndex]->m_pool, uiQueryIndex);
}

nsResult nsQueryPoolVulkan::GetTimestampResult(nsGALTimestampHandle hTimestamp, nsTime& result, bool bForce)
{
  if (hTimestamp.m_uiFrameCounter >= m_uiFirstFrameIndex)
  {
    const nsUInt32 uiFrameIndex = static_cast<nsUInt32>(hTimestamp.m_uiFrameCounter - m_uiFirstFrameIndex);
    const nsUInt32 uiPoolIndex = (nsUInt32)hTimestamp.m_uiIndex / s_uiPoolSize;
    const nsUInt32 uiQueryIndex = (nsUInt32)hTimestamp.m_uiIndex % s_uiPoolSize;
    FramePool& framePools = m_pendingFrames[uiFrameIndex];
    TimestampPool* pPool = framePools.m_pools[uiPoolIndex];
    if (pPool->m_bReady)
    {
      result = nsTime::Nanoseconds(m_fNanoSecondsPerTick * pPool->m_queryResults[uiQueryIndex]) + m_gpuToCpuDelta;
      return NS_SUCCESS;
    }
    else if (bForce)
    {
      vk::Result res = m_device.getQueryPoolResults(pPool->m_pool, uiQueryIndex, 1, sizeof(nsUInt64), &pPool->m_queryResults[uiQueryIndex], sizeof(nsUInt64), vk::QueryResultFlagBits::e64);
      result = nsTime::Nanoseconds(m_fNanoSecondsPerTick * pPool->m_queryResults[uiQueryIndex]) + m_gpuToCpuDelta;
      return res == vk::Result::eSuccess ? NS_SUCCESS : NS_FAILURE;
    }
    return NS_FAILURE;
  }
  else
  {
    // expired
    result = nsTime();
    return NS_SUCCESS;
  }
}

nsQueryPoolVulkan::TimestampPool* nsQueryPoolVulkan::GetFreePool()
{
  if (!m_freePools.IsEmpty())
  {
    TimestampPool* pPool = m_freePools.PeekBack();
    m_freePools.PopBack();
    pPool->m_queryResults.Clear();
    pPool->m_queryResults.SetCount(s_uiPoolSize, 0);
    return pPool;
  }

  vk::QueryPoolCreateInfo info;
  info.queryType = vk::QueryType::eTimestamp;
  info.queryCount = s_uiPoolSize;

  TimestampPool* pPool = NS_DEFAULT_NEW(TimestampPool);
  pPool->m_queryResults.SetCount(s_uiPoolSize, 0);
  VK_ASSERT_DEV(m_device.createQueryPool(&info, nullptr, &pPool->m_pool));

  m_resetPools.PushBack(pPool->m_pool);
  return pPool;
}
