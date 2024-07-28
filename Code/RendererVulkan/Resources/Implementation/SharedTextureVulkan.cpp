#include <RendererVulkan/RendererVulkanPCH.h>

#include <Foundation/System/Process.h>

#include <RendererVulkan/Device/DeviceVulkan.h>
#include <RendererVulkan/Device/InitContext.h>
#include <RendererVulkan/Resources/SharedTextureVulkan.h>
#include <RendererVulkan/Utils/PipelineBarrierVulkan.h>

#if NS_ENABLED(NS_PLATFORM_LINUX)
#  include <errno.h>
#  include <sys/syscall.h>
#  include <unistd.h>
#endif

nsGALSharedTextureVulkan::nsGALSharedTextureVulkan(const nsGALTextureCreationDescription& Description, nsEnum<nsGALSharedTextureType> sharedType, nsGALPlatformSharedHandle hSharedHandle)
  : nsGALTextureVulkan(Description, false, false)
  , m_SharedType(sharedType)
  , m_hSharedHandle(hSharedHandle)
{
}

nsGALSharedTextureVulkan::~nsGALSharedTextureVulkan()
{
}

nsResult nsGALSharedTextureVulkan::InitPlatform(nsGALDevice* pDevice, nsArrayPtr<nsGALSystemMemoryDescription> pInitialData)
{
  m_pDevice = static_cast<nsGALDeviceVulkan*>(pDevice);

  vk::ImageFormatListCreateInfo imageFormats;
  vk::ImageCreateInfo createInfo = {};

  m_imageFormat = ComputeImageFormat(m_pDevice, m_Description.m_Format, createInfo, imageFormats, m_bStaging);

  ComputeCreateInfo(m_pDevice, m_Description, createInfo, m_stages, m_access, m_preferredLayout);
  if (m_bLinearCPU)
  {
    ComputeCreateInfoLinear(createInfo, m_stages, m_access);
  }

  if (m_Description.m_pExisitingNativeObject == nullptr)
  {
    vk::ExternalMemoryImageCreateInfo extMemoryCreateInfo;
    if (m_SharedType == nsGALSharedTextureType::Exported || m_SharedType == nsGALSharedTextureType::Imported)
    {
#if NS_ENABLED(NS_PLATFORM_LINUX)
      extMemoryCreateInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueFd;
#elif NS_ENABLED(NS_PLATFORM_WINDOWS)
      extMemoryCreateInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
#endif
      extMemoryCreateInfo.pNext = createInfo.pNext;
      createInfo.pNext = &extMemoryCreateInfo;

      m_preferredLayout = vk::ImageLayout::eGeneral;
    }

    if (m_SharedType == nsGALSharedTextureType::None || m_SharedType == nsGALSharedTextureType::Exported)
    {

      nsVulkanAllocationCreateInfo allocInfo;
      ComputeAllocInfo(m_bLinearCPU, allocInfo);

      if (m_SharedType == nsGALSharedTextureType::Exported)
      {
        allocInfo.m_bExportSharedAllocation = true;
      }

      vk::ImageFormatProperties props2;
      VK_ASSERT_DEBUG(m_pDevice->GetVulkanPhysicalDevice().getImageFormatProperties(createInfo.format, createInfo.imageType, createInfo.tiling, createInfo.usage, createInfo.flags, &props2));
      VK_SUCCEED_OR_RETURN_NS_FAILURE(nsMemoryAllocatorVulkan::CreateImage(createInfo, allocInfo, m_image, m_alloc, &m_allocInfo));

      if (m_SharedType == nsGALSharedTextureType::Exported)
      {
        if (!m_pDevice->GetExtensions().m_bTimelineSemaphore)
        {
          nsLog::Error("Can not create shared textures because timeline semaphores are not supported");
          return NS_FAILURE;
        }

#if NS_ENABLED(NS_PLATFORM_LINUX)
        if (!m_pDevice->GetExtensions().m_bExternalMemoryFd)
        {
          nsLog::Error("Can not create shared textures because external memory fd is not supported");
          return NS_FAILURE;
        }

        if (!m_pDevice->GetExtensions().m_bExternalSemaphoreFd)
        {
          nsLog::Error("Can not create shared textures because external semaphore fd is not supported");
          return NS_FAILURE;
        }

        vk::MemoryGetFdInfoKHR getWin32HandleInfo{m_allocInfo.m_deviceMemory, vk::ExternalMemoryHandleTypeFlagBits::eOpaqueFd};
        int fd = -1;
        vk::Device device = m_pDevice->GetVulkanDevice();
        VK_SUCCEED_OR_RETURN_NS_FAILURE(device.getMemoryFdKHR(&getWin32HandleInfo, &fd, m_pDevice->GetDispatchContext()));
        m_hSharedHandle.m_uiProcessId = nsProcess::GetCurrentProcessID();
        m_hSharedHandle.m_hSharedTexture = (size_t)fd;
        m_hSharedHandle.m_uiMemoryTypeIndex = m_allocInfo.m_memoryType;
        m_hSharedHandle.m_uiSize = m_allocInfo.m_size;

        vk::ExportSemaphoreCreateInfoKHR exportInfo{vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueFd};
        vk::SemaphoreTypeCreateInfoKHR semTypeCreateInfo{vk::SemaphoreType::eTimeline, 0, &exportInfo};
        vk::SemaphoreCreateInfo semCreateInfo{{}, &semTypeCreateInfo};
        m_SharedSemaphore = device.createSemaphore(semCreateInfo);

        int semaphoreFd = -1;
        vk::SemaphoreGetFdInfoKHR getSemaphoreWin32Info{m_SharedSemaphore, vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueFd};
        VK_SUCCEED_OR_RETURN_NS_FAILURE(device.getSemaphoreFdKHR(&getSemaphoreWin32Info, &semaphoreFd, m_pDevice->GetDispatchContext()));
        m_hSharedHandle.m_hSemaphore = (size_t)semaphoreFd;
#elif NS_ENABLED(NS_PLATFORM_WINDOWS)
        if (!m_pDevice->GetExtensions().m_bExternalMemoryWin32)
        {
          nsLog::Error("Can not create shared textures because external memory win32 is not supported");
          return NS_FAILURE;
        }

        if (!m_pDevice->GetExtensions().m_bExternalSemaphoreWin32)
        {
          nsLog::Error("Can not create shared textures because external semaphore win32 is not supported");
          return NS_FAILURE;
        }

        vk::Device device = m_pDevice->GetVulkanDevice();
        vk::MemoryGetWin32HandleInfoKHR getWin32HandleInfo{m_allocInfo.m_deviceMemory, vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32};
        HANDLE handle = 0;
        VK_SUCCEED_OR_RETURN_NS_FAILURE(device.getMemoryWin32HandleKHR(&getWin32HandleInfo, &handle, m_pDevice->GetDispatchContext()));
        m_hSharedHandle.m_uiProcessId = nsProcess::GetCurrentProcessID();
        m_hSharedHandle.m_hSharedTexture = (size_t)handle;
        m_hSharedHandle.m_uiMemoryTypeIndex = m_allocInfo.m_memoryType;
        m_hSharedHandle.m_uiSize = m_allocInfo.m_size;

        vk::ExportSemaphoreWin32HandleInfoKHR exportInfoWin32;
        exportInfoWin32.dwAccess = GENERIC_ALL;
        vk::ExportSemaphoreCreateInfoKHR exportInfo{vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueWin32, &exportInfoWin32};
        vk::SemaphoreTypeCreateInfoKHR semTypeCreateInfo{vk::SemaphoreType::eTimeline, 0, &exportInfo};
        vk::SemaphoreCreateInfo semCreateInfo{{}, &semTypeCreateInfo};
        VK_SUCCEED_OR_RETURN_NS_FAILURE(device.createSemaphore(&semCreateInfo, nullptr, &m_SharedSemaphore));

        HANDLE semaphoreHandle = 0;
        vk::SemaphoreGetWin32HandleInfoKHR getSemaphoreWin32Info{m_SharedSemaphore, vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueWin32};
        VK_SUCCEED_OR_RETURN_NS_FAILURE(device.getSemaphoreWin32HandleKHR(&getSemaphoreWin32Info, &semaphoreHandle, m_pDevice->GetDispatchContext()));
        m_hSharedHandle.m_hSemaphore = (size_t)semaphoreHandle;
#else
        NS_ASSERT_NOT_IMPLEMENTED
#endif
      }
    }
    else if (m_SharedType == nsGALSharedTextureType::Imported)
    {
      if (!m_pDevice->GetExtensions().m_bTimelineSemaphore)
      {
        nsLog::Error("Can not open shared texture: timeline semaphores not supported");
        return NS_FAILURE;
      }

#if NS_ENABLED(NS_PLATFORM_LINUX)
      if (m_hSharedHandle.m_hSharedTexture == 0 || m_hSharedHandle.m_hSemaphore == 0)
      {
        nsLog::Error("Can not open shared texture: invalid handle given");
        return NS_FAILURE;
      }


      if (!m_pDevice->GetExtensions().m_bExternalMemoryFd)
      {
        nsLog::Error("Can not open shared texture: external memory fd not supported");
        return NS_FAILURE;
      }

      if (!m_pDevice->GetExtensions().m_bExternalSemaphoreFd)
      {
        nsLog::Error("Can not open shared texture: external semaphore fd not supported");
        return NS_FAILURE;
      }

      bool bNeedToImportForeignProcessFileDescriptors = m_hSharedHandle.m_uiProcessId != nsProcess::GetCurrentProcessID();
      if (bNeedToImportForeignProcessFileDescriptors)
      {
        int processFd = syscall(SYS_pidfd_open, m_hSharedHandle.m_uiProcessId, 0);
        if (processFd == -1)
        {
          nsLog::Error("SYS_pidfd_open failed with errno: {}", nsArgErrno(errno));
          m_hSharedHandle.m_hSharedTexture = 0;
          m_hSharedHandle.m_hSemaphore = 0;
          return NS_FAILURE;
        }
        NS_SCOPE_EXIT(close(processFd));

        m_hSharedHandle.m_hSharedTexture = syscall(SYS_pidfd_getfd, processFd, m_hSharedHandle.m_hSharedTexture, 0);
        if (m_hSharedHandle.m_hSharedTexture == -1)
        {
          nsLog::Error("SYS_pidfd_getfd for texture failed with errno: {}", nsArgErrno(errno));
          m_hSharedHandle.m_hSharedTexture = 0;
          m_hSharedHandle.m_hSemaphore = 0;
          return NS_FAILURE;
        }

        m_hSharedHandle.m_hSemaphore = syscall(SYS_pidfd_getfd, processFd, m_hSharedHandle.m_hSemaphore, 0);
        if (m_hSharedHandle.m_hSemaphore == -1)
        {
          nsLog::Error("SYS_pidfd_getfd for semaphore failed with errno: {}", nsArgErrno(errno));
          m_hSharedHandle.m_hSemaphore = 0;
          return NS_FAILURE;
        }
      }

      vk::Device device = m_pDevice->GetVulkanDevice();

      // Import semaphore
      vk::SemaphoreTypeCreateInfoKHR semTypeCreateInfo{vk::SemaphoreType::eTimeline, 0};
      vk::SemaphoreCreateInfo semCreateInfo{{}, &semTypeCreateInfo};
      m_SharedSemaphore = device.createSemaphore(semCreateInfo);

      vk::ImportSemaphoreFdInfoKHR importSemaphoreInfo{m_SharedSemaphore, {}, vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueFd, static_cast<int>(m_hSharedHandle.m_hSemaphore)};
      VK_SUCCEED_OR_RETURN_NS_FAILURE(device.importSemaphoreFdKHR(&importSemaphoreInfo, m_pDevice->GetDispatchContext()));

      // Create Image
      VK_SUCCEED_OR_RETURN_NS_FAILURE(device.createImage(&createInfo, nullptr, &m_image));

      vk::ImageMemoryRequirementsInfo2 imageRequirementsInfo{m_image};
      vk::MemoryRequirements2 imageMemoryRequirements;
      device.getImageMemoryRequirements2(&imageRequirementsInfo, &imageMemoryRequirements);

      // Import memory
      NS_ASSERT_DEBUG(imageMemoryRequirements.memoryRequirements.size == m_hSharedHandle.m_uiSize, "");

      vk::ImportMemoryFdInfoKHR fdInfo{vk::ExternalMemoryHandleTypeFlagBits::eOpaqueFd, static_cast<int>(m_hSharedHandle.m_hSharedTexture)};
      vk::MemoryAllocateInfo allocateInfo{imageMemoryRequirements.memoryRequirements.size, m_hSharedHandle.m_uiMemoryTypeIndex, &fdInfo};

      m_allocInfo = {};
      VK_SUCCEED_OR_RETURN_NS_FAILURE(device.allocateMemory(&allocateInfo, nullptr, &m_allocInfo.m_deviceMemory));
      m_allocInfo.m_offset = 0;
      m_allocInfo.m_size = imageMemoryRequirements.memoryRequirements.size;
      m_allocInfo.m_memoryType = m_hSharedHandle.m_uiMemoryTypeIndex;

      device.bindImageMemory(m_image, m_allocInfo.m_deviceMemory, 0);
#elif NS_ENABLED(NS_PLATFORM_WINDOWS)
      if (m_hSharedHandle.m_hSharedTexture == 0 || m_hSharedHandle.m_hSemaphore == 0)
      {
        nsLog::Error("Can not open shared texture: invalid handle given");
        return NS_FAILURE;
      }


      if (!m_pDevice->GetExtensions().m_bExternalMemoryWin32)
      {
        nsLog::Error("Can not open shared texture: external memory win32 not supported");
        return NS_FAILURE;
      }

      if (!m_pDevice->GetExtensions().m_bExternalSemaphoreWin32)
      {
        nsLog::Error("Can not open shared texture: external semaphore win32 not supported");
        return NS_FAILURE;
      }

      bool bNeedToImportForeignProcessFileDescriptors = m_hSharedHandle.m_uiProcessId != nsProcess::GetCurrentProcessID();
      if (bNeedToImportForeignProcessFileDescriptors)
      {
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, m_hSharedHandle.m_uiProcessId);
        if (hProcess == 0)
        {
          nsLog::Error("OpenProcess failed with error: {}", nsArgErrorCode(GetLastError()));
          m_hSharedHandle.m_hSharedTexture = 0;
          m_hSharedHandle.m_hSemaphore = 0;
          return NS_FAILURE;
        }

        HANDLE duplicateA = 0;
        BOOL res = DuplicateHandle(hProcess, reinterpret_cast<HANDLE>(m_hSharedHandle.m_hSharedTexture), GetCurrentProcess(), &duplicateA, 0, FALSE, DUPLICATE_SAME_ACCESS);
        m_hSharedHandle.m_hSharedTexture = reinterpret_cast<nsUInt64>(duplicateA);
        if (res == FALSE)
        {
          nsLog::Error("DuplicateHandle failed with error: {}", nsArgErrorCode(GetLastError()));
          m_hSharedHandle.m_hSharedTexture = 0;
          m_hSharedHandle.m_hSemaphore = 0;
          return NS_FAILURE;
        }

        HANDLE duplicateB = 0;
        res = DuplicateHandle(hProcess, reinterpret_cast<HANDLE>(m_hSharedHandle.m_hSemaphore), GetCurrentProcess(), &duplicateB, 0, FALSE, DUPLICATE_SAME_ACCESS);
        m_hSharedHandle.m_hSemaphore = reinterpret_cast<nsUInt64>(duplicateB);
        if (res == FALSE)
        {
          nsLog::Error("DuplicateHandle failed with error: {}", nsArgErrorCode(GetLastError()));
          m_hSharedHandle.m_hSemaphore = 0;
          return NS_FAILURE;
        }
      }

      vk::Device device = m_pDevice->GetVulkanDevice();

      // Import semaphore
      vk::SemaphoreTypeCreateInfoKHR semTypeCreateInfo{vk::SemaphoreType::eTimeline, 0};
      vk::SemaphoreCreateInfo semCreateInfo{{}, &semTypeCreateInfo};
      m_SharedSemaphore = device.createSemaphore(semCreateInfo);

      vk::ImportSemaphoreWin32HandleInfoKHR importSemaphoreInfo{m_SharedSemaphore, {}, vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueWin32, reinterpret_cast<HANDLE>(m_hSharedHandle.m_hSemaphore)};
      vk::Result res = device.importSemaphoreWin32HandleKHR(&importSemaphoreInfo, m_pDevice->GetDispatchContext());
      VK_SUCCEED_OR_RETURN_NS_FAILURE(res);

      // Create Image
      VK_SUCCEED_OR_RETURN_NS_FAILURE(device.createImage(&createInfo, nullptr, &m_image));

      vk::ImageMemoryRequirementsInfo2 imageRequirementsInfo{m_image};
      vk::MemoryRequirements2 imageMemoryRequirements;
      device.getImageMemoryRequirements2(&imageRequirementsInfo, &imageMemoryRequirements);

      // Import memory
      NS_ASSERT_DEBUG(imageMemoryRequirements.memoryRequirements.size == m_hSharedHandle.m_uiSize, "");

      vk::ImportMemoryWin32HandleInfoKHR fdInfo{vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32, reinterpret_cast<HANDLE>(m_hSharedHandle.m_hSharedTexture)};
      vk::MemoryAllocateInfo allocateInfo{imageMemoryRequirements.memoryRequirements.size, m_hSharedHandle.m_uiMemoryTypeIndex, &fdInfo};

      m_allocInfo = {};
      VK_SUCCEED_OR_RETURN_NS_FAILURE(device.allocateMemory(&allocateInfo, nullptr, &m_allocInfo.m_deviceMemory));
      m_allocInfo.m_offset = 0;
      m_allocInfo.m_size = imageMemoryRequirements.memoryRequirements.size;
      m_allocInfo.m_memoryType = m_hSharedHandle.m_uiMemoryTypeIndex;

      device.bindImageMemory(m_image, m_allocInfo.m_deviceMemory, 0);
#else
      NS_ASSERT_NOT_IMPLEMENTED
#endif
    }
  }
  else
  {
    m_image = static_cast<VkImage>(m_Description.m_pExisitingNativeObject);
  }
  m_pDevice->GetInitContext().InitTexture(this, createInfo, pInitialData);

  if (m_Description.m_ResourceAccess.m_bReadBack)
  {
    return CreateStagingBuffer(createInfo);
  }

  return NS_SUCCESS;
}


nsResult nsGALSharedTextureVulkan::DeInitPlatform(nsGALDevice* pDevice)
{
  nsGALDeviceVulkan* pVulkanDevice = static_cast<nsGALDeviceVulkan*>(pDevice);

  if (m_SharedType == nsGALSharedTextureType::Imported)
  {
    pVulkanDevice->DeleteLater(m_image, m_allocInfo.m_deviceMemory);
    pVulkanDevice->DeleteLater(m_SharedSemaphore);
  }
  else if (m_SharedType == nsGALSharedTextureType::Exported)
  {
    pVulkanDevice->DeleteLater(m_image, m_alloc);
    pVulkanDevice->DeleteLater(m_SharedSemaphore);
  }

  auto res = SUPER::DeInitPlatform(pDevice);

#if NS_ENABLED(NS_PLATFORM_LINUX)
  if (m_hSharedHandle.m_hSharedTexture != 0)
  {
    pVulkanDevice->DeleteLaterImpl({vk::ObjectType::eUnknown, {nsGALDeviceVulkan::PendingDeletionFlags::IsFileDescriptor}, (void*)static_cast<size_t>(m_hSharedHandle.m_hSharedTexture), nullptr});
    m_hSharedHandle.m_hSharedTexture = 0;
  }
  if (m_hSharedHandle.m_hSemaphore != 0)
  {
    pVulkanDevice->DeleteLaterImpl({vk::ObjectType::eUnknown, {nsGALDeviceVulkan::PendingDeletionFlags::IsFileDescriptor}, (void*)static_cast<size_t>(m_hSharedHandle.m_hSemaphore), nullptr});
    m_hSharedHandle.m_hSemaphore = 0;
  }
#endif
  return res;
}

nsGALPlatformSharedHandle nsGALSharedTextureVulkan::GetSharedHandle() const
{
  return m_hSharedHandle;
}

void nsGALSharedTextureVulkan::WaitSemaphoreGPU(nsUInt64 uiValue) const
{
  m_pDevice->AddWaitSemaphore(nsGALDeviceVulkan::SemaphoreInfo::MakeWaitSemaphore(m_SharedSemaphore, vk::PipelineStageFlagBits::eAllCommands, vk::SemaphoreType::eTimeline, uiValue));
}

void nsGALSharedTextureVulkan::SignalSemaphoreGPU(nsUInt64 uiValue) const
{
  m_pDevice->AddSignalSemaphore(nsGALDeviceVulkan::SemaphoreInfo::MakeSignalSemaphore(m_SharedSemaphore, vk::SemaphoreType::eTimeline, uiValue));
  // TODO, transition texture into GENERAL layout
  m_pDevice->GetCurrentPipelineBarrier().EnsureImageLayout(this, GetPreferredLayout(), GetUsedByPipelineStage(), GetAccessMask());
}
