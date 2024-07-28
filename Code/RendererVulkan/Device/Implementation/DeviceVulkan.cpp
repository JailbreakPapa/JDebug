#include <RendererVulkan/RendererVulkanPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#endif

#include <Core/System/Window.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <RendererFoundation/CommandEncoder/RenderCommandEncoder.h>
#include <RendererFoundation/Device/DeviceFactory.h>
#include <RendererFoundation/Device/SwapChain.h>
#include <RendererFoundation/Profiling/Profiling.h>
#include <RendererFoundation/RendererReflection.h>
#include <RendererVulkan/Cache/ResourceCacheVulkan.h>
#include <RendererVulkan/CommandEncoder/CommandEncoderImplVulkan.h>
#include <RendererVulkan/Device/DeviceVulkan.h>
#include <RendererVulkan/Device/InitContext.h>
#include <RendererVulkan/Device/PassVulkan.h>
#include <RendererVulkan/Device/SwapChainVulkan.h>
#include <RendererVulkan/Pools/CommandBufferPoolVulkan.h>
#include <RendererVulkan/Pools/DescriptorSetPoolVulkan.h>
#include <RendererVulkan/Pools/FencePoolVulkan.h>
#include <RendererVulkan/Pools/QueryPoolVulkan.h>
#include <RendererVulkan/Pools/SemaphorePoolVulkan.h>
#include <RendererVulkan/Pools/StagingBufferPoolVulkan.h>
#include <RendererVulkan/Resources/BufferVulkan.h>
#include <RendererVulkan/Resources/FallbackResourcesVulkan.h>
#include <RendererVulkan/Resources/QueryVulkan.h>
#include <RendererVulkan/Resources/RenderTargetViewVulkan.h>
#include <RendererVulkan/Resources/ResourceViewVulkan.h>
#include <RendererVulkan/Resources/SharedTextureVulkan.h>
#include <RendererVulkan/Resources/TextureVulkan.h>
#include <RendererVulkan/Resources/UnorderedAccessViewVulkan.h>
#include <RendererVulkan/Shader/ShaderVulkan.h>
#include <RendererVulkan/Shader/VertexDeclarationVulkan.h>
#include <RendererVulkan/State/StateVulkan.h>
#include <RendererVulkan/Utils/ImageCopyVulkan.h>
#include <RendererVulkan/Utils/PipelineBarrierVulkan.h>

#if NS_ENABLED(NS_SUPPORTS_GLFW)
#  include <GLFW/glfw3.h>
#endif

#if NS_ENABLED(NS_PLATFORM_LINUX) || NS_ENABLED(NS_PLATFORM_ANDROID)
#  include <errno.h>
#  include <unistd.h>
#endif


NS_DEFINE_AS_POD_TYPE(VkLayerProperties);

namespace
{
  VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
  {
    switch (messageSeverity)
    {
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        nsLog::Debug("VK: {}", pCallbackData->pMessage);
        break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        nsLog::Info("VK: {}", pCallbackData->pMessage);
        break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        nsLog::Warning("VK: {}", pCallbackData->pMessage);
        break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        nsLog::Error("VK: {}", pCallbackData->pMessage);
        break;
      default:
        break;
    }
    // Only layers are allowed to return true here.
    return VK_FALSE;
  }

  bool isInstanceLayerPresent(const char* layerName)
  {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    nsDynamicArray<VkLayerProperties> availableLayers;
    availableLayers.SetCountUninitialized(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.GetData());

    for (const auto& layerProperties : availableLayers)
    {
      if (strcmp(layerName, layerProperties.layerName) == 0)
      {
        return true;
      }
    }

    return false;
  }
} // namespace

// Need to implement these extension functions so vulkan hpp can call them.
// They're basically just adapters calling the function pointer retrieved previously.

PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXTFunc;
PFN_vkQueueBeginDebugUtilsLabelEXT vkQueueBeginDebugUtilsLabelEXTFunc;
PFN_vkQueueEndDebugUtilsLabelEXT vkQueueEndDebugUtilsLabelEXTFunc;
PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXTFunc;
PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXTFunc;
PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXTFunc;

VkResult vkSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pObjectName)
{
  return vkSetDebugUtilsObjectNameEXTFunc(device, pObjectName);
}

void vkQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo)
{
  return vkQueueBeginDebugUtilsLabelEXTFunc(queue, pLabelInfo);
}

void vkQueueEndDebugUtilsLabelEXT(VkQueue queue)
{
  return vkQueueEndDebugUtilsLabelEXTFunc(queue);
}
//
// void vkQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo)
//{
//  return vkQueueInsertDebugUtilsLabelEXTFunc(queue, pLabelInfo);
//}

void vkCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo)
{
  return vkCmdBeginDebugUtilsLabelEXTFunc(commandBuffer, pLabelInfo);
}

void vkCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer)
{
  return vkCmdEndDebugUtilsLabelEXTFunc(commandBuffer);
}

void vkCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo)
{
  return vkCmdInsertDebugUtilsLabelEXTFunc(commandBuffer, pLabelInfo);
}

nsInternal::NewInstance<nsGALDevice> CreateVulkanDevice(nsAllocator* pAllocator, const nsGALDeviceCreationDescription& Description)
{
  return NS_NEW(pAllocator, nsGALDeviceVulkan, Description);
}

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(RendererVulkan, DeviceFactory)

ON_CORESYSTEMS_STARTUP
{
  nsGALDeviceFactory::RegisterCreatorFunc("Vulkan", &CreateVulkanDevice, "VULKAN", "nsShaderCompilerDXC");
}

ON_CORESYSTEMS_SHUTDOWN
{
  nsGALDeviceFactory::UnregisterCreatorFunc("Vulkan");
}

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsGALDeviceVulkan::nsGALDeviceVulkan(const nsGALDeviceCreationDescription& Description)
  : nsGALDevice(Description)
{
}

nsGALDeviceVulkan::~nsGALDeviceVulkan() = default;

// Init & shutdown functions


vk::Result nsGALDeviceVulkan::SelectInstanceExtensions(nsHybridArray<const char*, 6>& extensions)
{
  // Fetch the list of extensions supported by the runtime.
  nsUInt32 extensionCount;
  VK_SUCCEED_OR_RETURN_LOG(vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
  nsDynamicArray<vk::ExtensionProperties> extensionProperties;
  extensionProperties.SetCount(extensionCount);
  VK_SUCCEED_OR_RETURN_LOG(vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.GetData()));

  NS_LOG_BLOCK("InstanceExtensions");
  for (auto& ext : extensionProperties)
  {
    nsLog::Info("{}", ext.extensionName.data());
  }

  // Add a specific extension to the list of extensions to be enabled, if it is supported.
  auto AddExtIfSupported = [&](const char* extensionName, bool& enableFlag) -> vk::Result
  {
    auto it = std::find_if(begin(extensionProperties), end(extensionProperties), [&](const vk::ExtensionProperties& prop)
      { return nsStringUtils::IsEqual(prop.extensionName.data(), extensionName); });
    if (it != end(extensionProperties))
    {
      extensions.PushBack(extensionName);
      enableFlag = true;
      return vk::Result::eSuccess;
    }
    enableFlag = false;
    return vk::Result::eErrorExtensionNotPresent;
  };

  VK_SUCCEED_OR_RETURN_LOG(AddExtIfSupported(VK_KHR_SURFACE_EXTENSION_NAME, m_extensions.m_bSurface));
#if NS_ENABLED(NS_SUPPORTS_GLFW)
  uint32_t iNumGlfwExtensions = 0;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&iNumGlfwExtensions);
  bool dummy = false;
  for (uint32_t i = 0; i < iNumGlfwExtensions; ++i)
  {
    VK_SUCCEED_OR_RETURN_LOG(AddExtIfSupported(glfwExtensions[i], dummy));
  }
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
  VK_SUCCEED_OR_RETURN_LOG(AddExtIfSupported(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, m_extensions.m_bWin32Surface));
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
  VK_SUCCEED_OR_RETURN_LOG(AddExtIfSupported(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, m_extensions.m_bAndroidSurface));
#else
#  error "Vulkan platform not supported"
#endif
  AddExtIfSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, m_extensions.m_bDebugUtils);
  m_extensions.m_bDebugUtilsMarkers = m_extensions.m_bDebugUtils;

  AddExtIfSupported(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME, m_extensions.m_bExternalMemoryCapabilities);
  AddExtIfSupported(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME, m_extensions.m_bExternalSemaphoreCapabilities);

  return vk::Result::eSuccess;
}


vk::Result nsGALDeviceVulkan::SelectDeviceExtensions(vk::DeviceCreateInfo& deviceCreateInfo, nsHybridArray<const char*, 6>& extensions)
{
  // Fetch the list of extensions supported by the runtime.
  nsUInt32 extensionCount;
  VK_SUCCEED_OR_RETURN_LOG(m_physicalDevice.enumerateDeviceExtensionProperties(nullptr, &extensionCount, nullptr));
  nsDynamicArray<vk::ExtensionProperties> extensionProperties;
  extensionProperties.SetCount(extensionCount);
  VK_SUCCEED_OR_RETURN_LOG(m_physicalDevice.enumerateDeviceExtensionProperties(nullptr, &extensionCount, extensionProperties.GetData()));

  NS_LOG_BLOCK("DeviceExtensions");
  for (auto& ext : extensionProperties)
  {
    nsLog::Info("{}", ext.extensionName.data());
  }

  // Add a specific extension to the list of extensions to be enabled, if it is supported.
  auto AddExtIfSupported = [&](const char* extensionName, bool& enableFlag) -> vk::Result
  {
    auto it = std::find_if(begin(extensionProperties), end(extensionProperties), [&](const vk::ExtensionProperties& prop)
      { return nsStringUtils::IsEqual(prop.extensionName.data(), extensionName); });
    if (it != end(extensionProperties))
    {
      extensions.PushBack(extensionName);
      enableFlag = true;
      return vk::Result::eSuccess;
    }
    enableFlag = false;
    nsLog::Warning("Extension '{}' not supported", extensionName);
    return vk::Result::eErrorExtensionNotPresent;
  };

  VK_SUCCEED_OR_RETURN_LOG(AddExtIfSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME, m_extensions.m_bDeviceSwapChain));
  AddExtIfSupported(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME, m_extensions.m_bShaderViewportIndexLayer);

  vk::PhysicalDeviceFeatures2 features;
  features.pNext = &m_extensions.m_borderColorEXT;
  m_physicalDevice.getFeatures2(&features);

  m_supportedStages = vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader;
  if (features.features.geometryShader)
  {
    m_supportedStages |= vk::PipelineStageFlagBits::eGeometryShader;
  }
  else
  {
    nsLog::Warning("Geometry shaders are not supported.");
  }

  if (features.features.tessellationShader)
  {
    m_supportedStages |= vk::PipelineStageFlagBits::eTessellationControlShader | vk::PipelineStageFlagBits::eTessellationEvaluationShader;
  }
  else
  {
    nsLog::Warning("Tessellation shaders are not supported.");
  }

  // Only use the extension if it allows us to not specify a format or we would need to create different samplers for every texture.
  if (m_extensions.m_borderColorEXT.customBorderColors && m_extensions.m_borderColorEXT.customBorderColorWithoutFormat)
  {
    AddExtIfSupported(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME, m_extensions.m_bBorderColorFloat);
    if (m_extensions.m_bBorderColorFloat)
    {
      m_extensions.m_borderColorEXT.pNext = const_cast<void*>(deviceCreateInfo.pNext);
      deviceCreateInfo.pNext = &m_extensions.m_borderColorEXT;
    }
  }

  AddExtIfSupported(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME, m_extensions.m_bImageFormatList);

  AddExtIfSupported(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME, m_extensions.m_bTimelineSemaphore);

  if (m_extensions.m_bTimelineSemaphore)
  {
    m_extensions.m_timelineSemaphoresEXT.pNext = const_cast<void*>(deviceCreateInfo.pNext);
    deviceCreateInfo.pNext = &m_extensions.m_timelineSemaphoresEXT;
    m_extensions.m_timelineSemaphoresEXT.timelineSemaphore = true;
  }

  AddExtIfSupported(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME, m_extensions.m_bExternalMemory);
  AddExtIfSupported(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME, m_extensions.m_bExternalSemaphore);
#if NS_ENABLED(NS_PLATFORM_LINUX) || NS_ENABLED(NS_PLATFORM_ANDROID)
  AddExtIfSupported(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME, m_extensions.m_bExternalMemoryFd);
  AddExtIfSupported(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME, m_extensions.m_bExternalSemaphoreFd);
#elif NS_ENABLED(NS_PLATFORM_WINDOWS)
  AddExtIfSupported(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME, m_extensions.m_bExternalMemoryWin32);
  AddExtIfSupported(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME, m_extensions.m_bExternalSemaphoreWin32);
#endif

  return vk::Result::eSuccess;
}

#define NS_GET_INSTANCE_PROC_ADDR(name) m_extensions.pfn_##name = reinterpret_cast<PFN_##name>(vkGetInstanceProcAddr(m_instance, #name));

nsStringView nsGALDeviceVulkan::GetRendererPlatform()
{
  return "Vulkan";
}

nsResult nsGALDeviceVulkan::InitPlatform()
{
  NS_LOG_BLOCK("nsGALDeviceVulkan::InitPlatform");

  const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
  {
    // Create instance
    // We require Vulkan 1.1 because of three features:
    // 1. Descriptor set pools return vk::Result::eErrorOutOfPoolMemory if exhausted. Removing the requirement to count usage yourself.
    // 2. Viewport height can be negative which performs y-inversion of the clip-space to framebuffer-space transform.
    // 3. Vulkan 1.0 is a pain to work with.
    vk::ApplicationInfo applicationInfo = {};
    applicationInfo.apiVersion = VK_API_VERSION_1_1;
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // TODO put nsEngine version here
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);      // TODO put nsEngine version here
    applicationInfo.pApplicationName = "nsEngine";
    applicationInfo.pEngineName = "nsEngine";

    nsHybridArray<const char*, 6> instanceExtensions;
    VK_SUCCEED_OR_RETURN_NS_FAILURE(SelectInstanceExtensions(instanceExtensions));

    vk::InstanceCreateInfo instanceCreateInfo;
    // enabling support for win32 surfaces
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    instanceCreateInfo.enabledExtensionCount = instanceExtensions.GetCount();
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.GetData();

    instanceCreateInfo.enabledLayerCount = 0;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (m_Description.m_bDebugDevice)
    {
      debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      debugCreateInfo.messageSeverity = /*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |*/ VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      debugCreateInfo.pfnUserCallback = debugCallback;
      debugCreateInfo.pUserData = nullptr;

      if (isInstanceLayerPresent(layers[0]))
      {
        instanceCreateInfo.enabledLayerCount = NS_ARRAY_SIZE(layers);
        instanceCreateInfo.ppEnabledLayerNames = layers;
      }
      else
      {
        nsLog::Warning("The khronos validation layer is not supported on this device. Will run without validation layer.");
      }

      if (m_extensions.m_bDebugUtils)
      {
        debugCreateInfo.pNext = instanceCreateInfo.pNext;
        instanceCreateInfo.pNext = &debugCreateInfo;
      }

      // Comment out if to force enable synchronization validation on any platform.
      if (false)
      {
        const char* layer_name = "VK_LAYER_KHRONOS_validation";

        const VkBool32 setting_validate_core = VK_TRUE;
        const VkBool32 setting_validate_sync = VK_TRUE;
        const VkBool32 setting_thread_safety = VK_TRUE;
        const char* setting_debug_action[] = {"VK_DBG_LAYER_ACTION_LOG_MSG"};
        const char* setting_report_flags[] = {"info", "warn", "perf", "error", "debug"};
        const VkBool32 setting_enable_message_limit = VK_TRUE;
        const int32_t setting_duplicate_message_limit = 3;

        const VkLayerSettingEXT settings[] = {
          {layer_name, "sync_queue_submit", VkLayerSettingTypeEXT::VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_validate_sync},
          {layer_name, "validate_core", VkLayerSettingTypeEXT::VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_validate_core},
          {layer_name, "validate_sync", VkLayerSettingTypeEXT::VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_validate_sync},
          {layer_name, "thread_safety", VkLayerSettingTypeEXT::VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_thread_safety},
          {layer_name, "debug_action", VkLayerSettingTypeEXT::VK_LAYER_SETTING_TYPE_STRING_EXT, 1, setting_debug_action},
          {layer_name, "report_flags", VkLayerSettingTypeEXT::VK_LAYER_SETTING_TYPE_STRING_EXT, NS_ARRAY_SIZE(setting_report_flags), setting_report_flags},
          {layer_name, "enable_message_limit", VkLayerSettingTypeEXT::VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_enable_message_limit},
          {layer_name, "duplicate_message_limit", VkLayerSettingTypeEXT::VK_LAYER_SETTING_TYPE_INT32_EXT, 1, &setting_duplicate_message_limit}};

        VkLayerSettingsCreateInfoEXT layer_settings_create_info = {
          VkStructureType::VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, NS_ARRAY_SIZE(settings), settings};

        {
          layer_settings_create_info.pNext = instanceCreateInfo.pNext;
          instanceCreateInfo.pNext = &layer_settings_create_info;
        }
      }
    }

    m_instance = vk::createInstance(instanceCreateInfo);

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
    if (m_extensions.m_bDebugUtils)
    {
      NS_GET_INSTANCE_PROC_ADDR(vkCreateDebugUtilsMessengerEXT);
      NS_GET_INSTANCE_PROC_ADDR(vkDestroyDebugUtilsMessengerEXT);
      NS_GET_INSTANCE_PROC_ADDR(vkSetDebugUtilsObjectNameEXT);
      VK_SUCCEED_OR_RETURN_NS_FAILURE(m_extensions.pfn_vkCreateDebugUtilsMessengerEXT(m_instance, &debugCreateInfo, nullptr, &m_debugMessenger));
    }
#endif

    if (!m_instance)
    {
      nsLog::Error("Failed to create Vulkan instance!");
      return NS_FAILURE;
    }
  }

  {
    // physical device
    nsUInt32 physicalDeviceCount = 0;
    nsHybridArray<vk::PhysicalDevice, 2> physicalDevices;
    VK_SUCCEED_OR_RETURN_NS_FAILURE(m_instance.enumeratePhysicalDevices(&physicalDeviceCount, nullptr));
    if (physicalDeviceCount == 0)
    {
      nsLog::Error("No available physical device to create a Vulkan device on!");
      return NS_FAILURE;
    }

    physicalDevices.SetCount(physicalDeviceCount);
    VK_SUCCEED_OR_RETURN_NS_FAILURE(m_instance.enumeratePhysicalDevices(&physicalDeviceCount, physicalDevices.GetData()));

    // TODO choosable physical device?
    // TODO making sure we have a hardware device?
    m_physicalDevice = physicalDevices[0];
    m_properties = m_physicalDevice.getProperties();
    nsLog::Warning("Selected physical device \"{}\" for device creation.", m_properties.deviceName);

    // This is a workaround for broken lavapipe drivers which cannot handle label scopes that span across multiple command buffers.
    nsStringBuilder sDeviceName = nsStringUtf8(m_properties.deviceName).GetView();
    if (sDeviceName.FindSubString_NoCase("LLVMPIPE") != nullptr)
    {
      m_extensions.m_bDebugUtilsMarkers = false;
    }
    // TODO call vkGetPhysicalDeviceFeatures2 with VkPhysicalDeviceTimelineSemaphoreFeatures and figure out if time
  }

  nsHybridArray<vk::QueueFamilyProperties, 4> queueFamilyProperties;
  {
    // Device
    nsUInt32 queueFamilyPropertyCount = 0;
    m_physicalDevice.getQueueFamilyProperties(&queueFamilyPropertyCount, nullptr);
    if (queueFamilyPropertyCount == 0)
    {
      nsLog::Error("No available device queues on physical device!");
      return NS_FAILURE;
    }
    queueFamilyProperties.SetCount(queueFamilyPropertyCount);
    m_physicalDevice.getQueueFamilyProperties(&queueFamilyPropertyCount, queueFamilyProperties.GetData());

    {
      NS_LOG_BLOCK("Queue Families");
      for (nsUInt32 i = 0; i < queueFamilyProperties.GetCount(); ++i)
      {
        const vk::QueueFamilyProperties& queueFamilyProperty = queueFamilyProperties[i];
        nsLog::Info("Queue count: {}, flags: {}", queueFamilyProperty.queueCount, vk::to_string(queueFamilyProperty.queueFlags).data());
      }
    }

    // Select best queue family for graphics and transfers.
    for (nsUInt32 i = 0; i < queueFamilyProperties.GetCount(); ++i)
    {
      const vk::QueueFamilyProperties& queueFamilyProperty = queueFamilyProperties[i];
      if (queueFamilyProperty.queueCount == 0)
        continue;
      constexpr auto graphicsFlags = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute;
      if ((queueFamilyProperty.queueFlags & graphicsFlags) == graphicsFlags)
      {
        m_graphicsQueue.m_uiQueueFamily = i;
      }
      if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eTransfer)
      {
        if (m_transferQueue.m_uiQueueFamily == -1)
        {
          m_transferQueue.m_uiQueueFamily = i;
        }
        else if ((queueFamilyProperty.queueFlags & graphicsFlags) == vk::QueueFlagBits())
        {
          // Prefer a queue that can't be used for graphics.
          m_transferQueue.m_uiQueueFamily = i;
        }
      }
    }
    if (m_graphicsQueue.m_uiQueueFamily == -1)
    {
      nsLog::Error("No graphics queue found.");
      return NS_FAILURE;
    }
    if (m_transferQueue.m_uiQueueFamily == -1)
    {
      nsLog::Warning("No transfer queue found.");
    }

    constexpr float queuePriority = 0.f;

    nsHybridArray<vk::DeviceQueueCreateInfo, 2> queues;

    vk::DeviceQueueCreateInfo& graphicsQueueCreateInfo = queues.ExpandAndGetRef();
    graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;
    graphicsQueueCreateInfo.queueCount = 1;
    graphicsQueueCreateInfo.queueFamilyIndex = m_graphicsQueue.m_uiQueueFamily;

    if (m_graphicsQueue.m_uiQueueFamily != m_transferQueue.m_uiQueueFamily && m_transferQueue.m_uiQueueFamily != -1)
    {
      vk::DeviceQueueCreateInfo& transferQueueCreateInfo = queues.ExpandAndGetRef();
      transferQueueCreateInfo.pQueuePriorities = &queuePriority;
      transferQueueCreateInfo.queueCount = 1;
      transferQueueCreateInfo.queueFamilyIndex = m_transferQueue.m_uiQueueFamily;
    }

    // #TODO_VULKAN test that this returns the same as 'layers' passed into the instance.
    nsUInt32 uiLayers;
    VK_SUCCEED_OR_RETURN_NS_FAILURE(m_physicalDevice.enumerateDeviceLayerProperties(&uiLayers, nullptr));
    nsDynamicArray<vk::LayerProperties> deviceLayers;
    deviceLayers.SetCount(uiLayers);
    VK_SUCCEED_OR_RETURN_NS_FAILURE(m_physicalDevice.enumerateDeviceLayerProperties(&uiLayers, deviceLayers.GetData()));

    vk::DeviceCreateInfo deviceCreateInfo = {};
    nsHybridArray<const char*, 6> deviceExtensions;
    VK_SUCCEED_OR_RETURN_NS_FAILURE(SelectDeviceExtensions(deviceCreateInfo, deviceExtensions));

    deviceCreateInfo.enabledExtensionCount = deviceExtensions.GetCount();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.GetData();
    // Device layers are deprecated but provided (same as in instance) for backwards compatibility.
    deviceCreateInfo.enabledLayerCount = NS_ARRAY_SIZE(layers);
    deviceCreateInfo.ppEnabledLayerNames = layers;

    vk::PhysicalDeviceFeatures physicalDeviceFeatures = m_physicalDevice.getFeatures();
    deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures; // Enabling all available features for now
    deviceCreateInfo.queueCreateInfoCount = queues.GetCount();
    deviceCreateInfo.pQueueCreateInfos = queues.GetData();

    VK_SUCCEED_OR_RETURN_NS_FAILURE(m_physicalDevice.createDevice(&deviceCreateInfo, nullptr, &m_device));
    m_device.getQueue(m_graphicsQueue.m_uiQueueFamily, m_graphicsQueue.m_uiQueueIndex, &m_graphicsQueue.m_queue);

    if (m_graphicsQueue.m_uiQueueFamily != m_transferQueue.m_uiQueueFamily && m_transferQueue.m_uiQueueFamily != -1)
    {
      m_device.getQueue(m_transferQueue.m_uiQueueFamily, m_transferQueue.m_uiQueueIndex, &m_transferQueue.m_queue);
    }

    m_dispatchContext.Init(*this);
  }

  vkSetDebugUtilsObjectNameEXTFunc = (PFN_vkSetDebugUtilsObjectNameEXT)m_device.getProcAddr("vkSetDebugUtilsObjectNameEXT");
  vkQueueBeginDebugUtilsLabelEXTFunc = (PFN_vkQueueBeginDebugUtilsLabelEXT)m_device.getProcAddr("vkQueueBeginDebugUtilsLabelEXT");
  vkQueueEndDebugUtilsLabelEXTFunc = (PFN_vkQueueEndDebugUtilsLabelEXT)m_device.getProcAddr("vkQueueEndDebugUtilsLabelEXT");
  vkCmdBeginDebugUtilsLabelEXTFunc = (PFN_vkCmdBeginDebugUtilsLabelEXT)m_device.getProcAddr("vkCmdBeginDebugUtilsLabelEXT");
  vkCmdEndDebugUtilsLabelEXTFunc = (PFN_vkCmdEndDebugUtilsLabelEXT)m_device.getProcAddr("vkCmdEndDebugUtilsLabelEXT");
  vkCmdInsertDebugUtilsLabelEXTFunc = (PFN_vkCmdInsertDebugUtilsLabelEXT)m_device.getProcAddr("vkCmdInsertDebugUtilsLabelEXT");

  VK_SUCCEED_OR_RETURN_NS_FAILURE(nsMemoryAllocatorVulkan::Initialize(m_physicalDevice, m_device, m_instance));

  m_memoryProperties = m_physicalDevice.getMemoryProperties();

  // Fill lookup table
  FillFormatLookupTable();

  nsClipSpaceDepthRange::Default = nsClipSpaceDepthRange::ZeroToOne;
  // We use nsClipSpaceYMode::Regular and rely in the Vulkan 1.1 feature that a negative height performs y-inversion of the clip-space to framebuffer-space transform.
  // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_KHR_maintenance1.html
  nsClipSpaceYMode::RenderToTextureDefault = nsClipSpaceYMode::Regular;

  m_pPipelineBarrier = NS_NEW(&m_Allocator, nsPipelineBarrierVulkan);
  m_pCommandBufferPool = NS_NEW(&m_Allocator, nsCommandBufferPoolVulkan);
  m_pCommandBufferPool->Initialize(m_device, m_graphicsQueue.m_uiQueueFamily);
  m_pStagingBufferPool = NS_NEW(&m_Allocator, nsStagingBufferPoolVulkan);
  m_pStagingBufferPool->Initialize(this);
  m_pQueryPool = NS_NEW(&m_Allocator, nsQueryPoolVulkan);
  m_pQueryPool->Initialize(this, queueFamilyProperties[m_graphicsQueue.m_uiQueueFamily].timestampValidBits);
  m_pInitContext = NS_NEW(&m_Allocator, nsInitContextVulkan, this);

  nsSemaphorePoolVulkan::Initialize(m_device);
  nsFencePoolVulkan::Initialize(m_device);
  nsResourceCacheVulkan::Initialize(this, m_device);
  nsDescriptorSetPoolVulkan::Initialize(m_device);
  nsImageCopyVulkan::Initialize(*this);

  m_pDefaultPass = NS_NEW(&m_Allocator, nsGALPassVulkan, *this);

  nsGALWindowSwapChain::SetFactoryMethod([this](const nsGALWindowSwapChainCreationDescription& desc) -> nsGALSwapChainHandle
    { return CreateSwapChain([this, &desc](nsAllocator* pAllocator) -> nsGALSwapChain*
        { return NS_NEW(pAllocator, nsGALSwapChainVulkan, desc); }); });

  return NS_SUCCESS;
}

void nsGALDeviceVulkan::SetDebugName(const vk::DebugUtilsObjectNameInfoEXT& info, nsVulkanAllocation allocation)
{
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  if (m_extensions.m_bDebugUtils)
  {
    m_device.setDebugUtilsObjectNameEXT(info);
  }
  if (allocation)
    nsMemoryAllocatorVulkan::SetAllocationUserData(allocation, info.pObjectName);
#endif
}

void nsGALDeviceVulkan::ReportLiveGpuObjects()
{
  // This is automatically done in the validation layer and can't be easily done manually.
}

void nsGALDeviceVulkan::UploadBufferStaging(nsStagingBufferPoolVulkan* pStagingBufferPool, nsPipelineBarrierVulkan* pPipelineBarrier, vk::CommandBuffer commandBuffer, const nsGALBufferVulkan* pBuffer, nsArrayPtr<const nsUInt8> pInitialData, vk::DeviceSize dstOffset)
{
  void* pData = nullptr;

  // #TODO_VULKAN Use transfer queue
  nsStagingBufferVulkan stagingBuffer = pStagingBufferPool->AllocateBuffer(0, pInitialData.GetCount());
  // nsMemoryUtils::Copy(reinterpret_cast<nsUInt8*>(stagingBuffer.m_allocInfo.m_pMappedData), pInitialData.GetPtr(), pInitialData.GetCount());
  nsMemoryAllocatorVulkan::MapMemory(stagingBuffer.m_alloc, &pData);
  nsMemoryUtils::Copy(reinterpret_cast<nsUInt8*>(pData), pInitialData.GetPtr(), pInitialData.GetCount());
  nsMemoryAllocatorVulkan::UnmapMemory(stagingBuffer.m_alloc);

  vk::BufferCopy region;
  region.srcOffset = 0;
  region.dstOffset = dstOffset;
  region.size = pInitialData.GetCount();

  // #TODO_VULKAN atomic min size violation?
  commandBuffer.copyBuffer(stagingBuffer.m_buffer, pBuffer->GetVkBuffer(), 1, &region);

  pPipelineBarrier->AccessBuffer(pBuffer, region.dstOffset, region.size, vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferWrite, pBuffer->GetUsedByPipelineStage(), pBuffer->GetAccessMask());

  // #TODO_VULKAN Custom delete later / return to nsStagingBufferPoolVulkan once this is on the transfer queue and runs async to graphics queue.
  pStagingBufferPool->ReclaimBuffer(stagingBuffer);
}

void nsGALDeviceVulkan::UploadTextureStaging(nsStagingBufferPoolVulkan* pStagingBufferPool, nsPipelineBarrierVulkan* pPipelineBarrier, vk::CommandBuffer commandBuffer, const nsGALTextureVulkan* pTexture, const vk::ImageSubresourceLayers& subResource, const nsGALSystemMemoryDescription& data)
{
  const vk::Offset3D imageOffset = {0, 0, 0};
  const vk::Extent3D imageExtent = pTexture->GetMipLevelSize(subResource.mipLevel);

  auto getRange = [](const vk::ImageSubresourceLayers& layers) -> vk::ImageSubresourceRange
  {
    vk::ImageSubresourceRange range;
    range.aspectMask = layers.aspectMask;
    range.baseMipLevel = layers.mipLevel;
    range.levelCount = 1;
    range.baseArrayLayer = layers.baseArrayLayer;
    range.layerCount = layers.layerCount;
    return range;
  };

  pPipelineBarrier->EnsureImageLayout(pTexture, getRange(subResource), vk::ImageLayout::eTransferDstOptimal, vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferWrite);
  pPipelineBarrier->Flush();

  for (nsUInt32 i = 0; i < subResource.layerCount; i++)
  {
    auto pLayerData = reinterpret_cast<const nsUInt8*>(data.m_pData) + i * data.m_uiSlicePitch;
    const vk::Format format = pTexture->GetImageFormat();
    const nsUInt8 uiBlockSize = vk::blockSize(format);
    const auto blockExtent = vk::blockExtent(format);
    const VkExtent3D blockCount = {
      (imageExtent.width + blockExtent[0] - 1) / blockExtent[0],
      (imageExtent.height + blockExtent[1] - 1) / blockExtent[1],
      (imageExtent.depth + blockExtent[2] - 1) / blockExtent[2]};

    const vk::DeviceSize uiTotalSize = uiBlockSize * blockCount.width * blockCount.height * blockCount.depth;
    nsStagingBufferVulkan stagingBuffer = pStagingBufferPool->AllocateBuffer(0, uiTotalSize);

    const nsUInt32 uiBufferRowPitch = uiBlockSize * blockCount.width;
    const nsUInt32 uiBufferSlicePitch = uiBufferRowPitch * blockCount.height;
    NS_ASSERT_DEV(uiBufferRowPitch == data.m_uiRowPitch, "Row pitch with padding is not implemented yet.");
    NS_ASSERT_DEV(uiBufferSlicePitch == data.m_uiSlicePitch, "Row pitch with padding is not implemented yet.");

    void* pData = nullptr;
    nsMemoryAllocatorVulkan::MapMemory(stagingBuffer.m_alloc, &pData);
    nsMemoryUtils::Copy(reinterpret_cast<nsUInt8*>(pData), pLayerData, uiTotalSize);
    nsMemoryAllocatorVulkan::UnmapMemory(stagingBuffer.m_alloc);

    vk::BufferImageCopy region = {};
    region.imageSubresource = subResource;
    region.imageOffset = imageOffset;
    region.imageExtent = imageExtent;

    region.bufferOffset = 0;
    region.bufferRowLength = blockExtent[0] * uiBufferRowPitch / uiBlockSize;
    region.bufferImageHeight = blockExtent[1] * uiBufferSlicePitch / uiBufferRowPitch;

    // #TODO_VULKAN atomic min size violation?
    commandBuffer.copyBufferToImage(stagingBuffer.m_buffer, pTexture->GetImage(), pTexture->GetPreferredLayout(vk::ImageLayout::eTransferDstOptimal), 1, &region);
    pStagingBufferPool->ReclaimBuffer(stagingBuffer);
  }

  pPipelineBarrier->EnsureImageLayout(pTexture, getRange(subResource), pTexture->GetPreferredLayout(), pTexture->GetUsedByPipelineStage(), pTexture->GetAccessMask());
}

nsResult nsGALDeviceVulkan::ShutdownPlatform()
{
  nsImageCopyVulkan::DeInitialize(*this);
  DestroyDeadObjects(); // nsImageCopyVulkan might add dead objects, so make sure the list is cleared again

  nsGALWindowSwapChain::SetFactoryMethod({});
  if (m_lastCommandBufferFinished)
    ReclaimLater(m_lastCommandBufferFinished, m_pCommandBufferPool.Borrow());
  auto& pCommandEncoder = m_pDefaultPass->m_pCommandEncoderImpl;

  // We couldn't create a device in the first place, so early out of shutdown
  if (!m_device)
  {
    return NS_SUCCESS;
  }

  WaitIdlePlatform();

  m_pDefaultPass = nullptr;
  m_pPipelineBarrier = nullptr;
  m_pCommandBufferPool->DeInitialize();
  m_pCommandBufferPool = nullptr;
  m_pStagingBufferPool->DeInitialize();
  m_pStagingBufferPool = nullptr;
  m_pQueryPool->DeInitialize();
  m_pQueryPool = nullptr;
  m_pInitContext = nullptr;

  nsSemaphorePoolVulkan::DeInitialize();
  nsFencePoolVulkan::DeInitialize();
  nsResourceCacheVulkan::DeInitialize();
  nsDescriptorSetPoolVulkan::DeInitialize();

  nsMemoryAllocatorVulkan::DeInitialize();

  m_device.waitIdle();
  m_device.destroy();

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  if (m_extensions.m_bDebugUtils && m_extensions.pfn_vkDestroyDebugUtilsMessengerEXT != nullptr)
  {
    m_extensions.pfn_vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
  }
#endif

  m_instance.destroy();
  ReportLiveGpuObjects();

  return NS_SUCCESS;
}

// Pipeline & Pass functions

vk::CommandBuffer& nsGALDeviceVulkan::GetCurrentCommandBuffer()
{
  vk::CommandBuffer& commandBuffer = m_PerFrameData[m_uiCurrentPerFrameData].m_currentCommandBuffer;
  if (!commandBuffer)
  {
    // Restart new command buffer if none is active already.
    commandBuffer = m_pCommandBufferPool->RequestCommandBuffer();
    vk::CommandBufferBeginInfo beginInfo;
    VK_ASSERT_DEBUG(commandBuffer.begin(&beginInfo));
    GetCurrentPipelineBarrier().SetCommandBuffer(&commandBuffer);

    m_pDefaultPass->SetCurrentCommandBuffer(&commandBuffer, m_pPipelineBarrier.Borrow());
    // We can't carry state across individual command buffers.
    m_pDefaultPass->MarkDirty();
  }
  return commandBuffer;
}

nsPipelineBarrierVulkan& nsGALDeviceVulkan::GetCurrentPipelineBarrier()
{
  vk::CommandBuffer& commandBuffer = m_PerFrameData[m_uiCurrentPerFrameData].m_currentCommandBuffer;
  if (!commandBuffer)
  {
    GetCurrentCommandBuffer();
  }
  return *m_pPipelineBarrier.Borrow();
}

nsQueryPoolVulkan& nsGALDeviceVulkan::GetQueryPool() const
{
  return *m_pQueryPool.Borrow();
}

nsStagingBufferPoolVulkan& nsGALDeviceVulkan::GetStagingBufferPool() const
{
  return *m_pStagingBufferPool.Borrow();
}

nsInitContextVulkan& nsGALDeviceVulkan::GetInitContext() const
{
  return *m_pInitContext.Borrow();
}

nsProxyAllocator& nsGALDeviceVulkan::GetAllocator()
{
  return m_Allocator;
}

nsGALTextureHandle nsGALDeviceVulkan::CreateTextureInternal(const nsGALTextureCreationDescription& Description, nsArrayPtr<nsGALSystemMemoryDescription> pInitialData, bool bLinearCPU, bool bStaging)
{
  nsGALTextureVulkan* pTexture = NS_NEW(&m_Allocator, nsGALTextureVulkan, Description, bLinearCPU, bStaging);

  if (!pTexture->InitPlatform(this, pInitialData).Succeeded())
  {
    NS_DELETE(&m_Allocator, pTexture);
    return nsGALTextureHandle();
  }

  return FinalizeTextureInternal(Description, pTexture);
}

nsGALBufferHandle nsGALDeviceVulkan::CreateBufferInternal(const nsGALBufferCreationDescription& Description, nsArrayPtr<const nsUInt8> pInitialData, bool bCPU)
{
  nsGALBufferVulkan* pBuffer = NS_NEW(&m_Allocator, nsGALBufferVulkan, Description, bCPU);

  if (!pBuffer->InitPlatform(this, pInitialData).Succeeded())
  {
    NS_DELETE(&m_Allocator, pBuffer);
    return nsGALBufferHandle();
  }

  return FinalizeBufferInternal(Description, pBuffer);
}

void nsGALDeviceVulkan::BeginPipelinePlatform(const char* szName, nsGALSwapChain* pSwapChain)
{
  NS_PROFILE_SCOPE("BeginPipelinePlatform");

  GetCurrentCommandBuffer();
#if NS_ENABLED(NS_USE_PROFILING)
  m_pPipelineTimingScope = nsProfilingScopeAndMarker::Start(m_pDefaultPass->m_pRenderCommandEncoder.Borrow(), szName);
#endif

  if (pSwapChain)
  {
    pSwapChain->AcquireNextRenderTarget(this);
  }
}

void nsGALDeviceVulkan::EndPipelinePlatform(nsGALSwapChain* pSwapChain)
{
  NS_PROFILE_SCOPE("EndPipelinePlatform");

#if NS_ENABLED(NS_USE_PROFILING)
  nsProfilingScopeAndMarker::Stop(m_pDefaultPass->m_pRenderCommandEncoder.Borrow(), m_pPipelineTimingScope);
#endif
  if (pSwapChain)
  {
    pSwapChain->PresentRenderTarget(this);
  }

  // Render context is reset on every end pipeline so it will re-submit all state change for the next render pass. Thus it is safe at this point to do a full reset.
  // Technically don't have to reset here, MarkDirty would also be fine but we do need to do a Reset at the end of the frame as pointers held by the nsGALCommandEncoderImplVulkan may not be valid in the next frame.
  m_pDefaultPass->Reset();
}

vk::Fence nsGALDeviceVulkan::Submit(bool bAddSignalSemaphore)
{
  m_pDefaultPass->SetCurrentCommandBuffer(nullptr, nullptr);

  vk::CommandBuffer initCommandBuffer = m_pInitContext->GetFinishedCommandBuffer();
  bool bHasCmdBuffer = initCommandBuffer || m_PerFrameData[m_uiCurrentPerFrameData].m_currentCommandBuffer;

  nsHybridArray<vk::CommandBuffer, 2> buffers;
  vk::SubmitInfo submitInfo = {};
  if (bHasCmdBuffer)
  {
    vk::CommandBuffer mainCommandBuffer = m_PerFrameData[m_uiCurrentPerFrameData].m_currentCommandBuffer;
    if (initCommandBuffer)
    {
      // Any background loading that happened up to this point needs to be submitted first.
      // The main render command buffer assumes that all new resources are in their default state which is made sure by submitting this command buffer.
      buffers.PushBack(initCommandBuffer);
    }
    if (mainCommandBuffer)
    {
      GetCurrentPipelineBarrier().Submit();
      mainCommandBuffer.end();
      buffers.PushBack(mainCommandBuffer);
    }
    submitInfo.commandBufferCount = buffers.GetCount();
    submitInfo.pCommandBuffers = buffers.GetData();
  }

  if (m_lastCommandBufferFinished)
  {
    AddWaitSemaphore(nsGALDeviceVulkan::SemaphoreInfo::MakeWaitSemaphore(m_lastCommandBufferFinished, vk::PipelineStageFlagBits::eAllCommands));
    ReclaimLater(m_lastCommandBufferFinished);
  }

  if (bAddSignalSemaphore)
  {
    m_lastCommandBufferFinished = nsSemaphorePoolVulkan::RequestSemaphore();
    AddSignalSemaphore(nsGALDeviceVulkan::SemaphoreInfo::MakeSignalSemaphore(m_lastCommandBufferFinished));
  }
  vk::Fence renderFence = nsFencePoolVulkan::RequestFence();

  nsHybridArray<vk::Semaphore, 3> waitSemaphores;
  nsHybridArray<vk::PipelineStageFlags, 3> waitStages;
  nsHybridArray<vk::Semaphore, 3> signalSemaphores;

  nsHybridArray<nsUInt64, 3> waitSemaphoreValues;
  nsHybridArray<nsUInt64, 3> signalSemaphoreValues;
  for (const SemaphoreInfo sem : m_waitSemaphores)
  {
    waitSemaphores.PushBack(sem.m_semaphore);
    if (sem.m_type == vk::SemaphoreType::eTimeline)
    {
      waitSemaphoreValues.PushBack(sem.m_uiValue);
    }
    waitStages.PushBack(vk::PipelineStageFlagBits::eAllCommands);
  }
  m_waitSemaphores.Clear();

  for (const SemaphoreInfo sem : m_signalSemaphores)
  {
    signalSemaphores.PushBack(sem.m_semaphore);
    if (sem.m_type == vk::SemaphoreType::eTimeline)
    {
      signalSemaphoreValues.PushBack(sem.m_uiValue);
    }
  }
  m_signalSemaphores.Clear();


  // If a timeline semaphore is present, all semaphores need a value, even binary ones because validation says so.
  if (waitSemaphoreValues.GetCount() > 0)
  {
    waitSemaphoreValues.SetCount(waitSemaphores.GetCount());
  }
  if (signalSemaphoreValues.GetCount() > 0)
  {
    signalSemaphoreValues.SetCount(signalSemaphores.GetCount());
  }

  vk::TimelineSemaphoreSubmitInfo timelineInfo;
  timelineInfo.waitSemaphoreValueCount = waitSemaphoreValues.GetCount();
  NS_CHECK_AT_COMPILETIME(sizeof(nsUInt64) == sizeof(uint64_t));
  timelineInfo.pWaitSemaphoreValues = reinterpret_cast<const uint64_t*>(waitSemaphoreValues.GetData());
  timelineInfo.signalSemaphoreValueCount = signalSemaphoreValues.GetCount();
  timelineInfo.pSignalSemaphoreValues = reinterpret_cast<const uint64_t*>(signalSemaphoreValues.GetData());

  if (timelineInfo.waitSemaphoreValueCount > 0 || timelineInfo.signalSemaphoreValueCount > 0)
  {
    // Only add timeline info if we have a timeline semaphore or validation layer complains.
    submitInfo.pNext = &timelineInfo;

    NS_ASSERT_DEBUG(timelineInfo.waitSemaphoreValueCount == 0 || waitSemaphores.GetCount() == waitSemaphoreValues.GetCount(), "If a timeline semaphore is present, all semaphores need a wait value.");
    NS_ASSERT_DEBUG(timelineInfo.signalSemaphoreValueCount == 0 || signalSemaphores.GetCount() == signalSemaphoreValues.GetCount(), "If a timeline semaphore is present, all semaphores need a signal value.");
  }
  NS_ASSERT_DEBUG(waitSemaphores.GetCount() == waitStages.GetCount(), "Each wait semaphore needs a wait stage");

  submitInfo.waitSemaphoreCount = waitSemaphores.GetCount();
  submitInfo.pWaitSemaphores = waitSemaphores.GetData();
  submitInfo.pWaitDstStageMask = waitStages.GetData();
  submitInfo.signalSemaphoreCount = signalSemaphores.GetCount();
  submitInfo.pSignalSemaphores = signalSemaphores.GetData();

  {
    m_PerFrameData[m_uiCurrentPerFrameData].m_CommandBufferFences.PushBack(renderFence);
    m_graphicsQueue.m_queue.submit(1, &submitInfo, renderFence);
  }

  auto res = renderFence;
  ReclaimLater(renderFence);
  if (m_PerFrameData[m_uiCurrentPerFrameData].m_currentCommandBuffer)
  {
    ReclaimLater(m_PerFrameData[m_uiCurrentPerFrameData].m_currentCommandBuffer, m_pCommandBufferPool.Borrow());
  }
  return res;
}

nsGALPass* nsGALDeviceVulkan::BeginPassPlatform(const char* szName)
{
  GetCurrentCommandBuffer();
#if NS_ENABLED(NS_USE_PROFILING)
  m_pPassTimingScope = nsProfilingScopeAndMarker::Start(m_pDefaultPass->m_pRenderCommandEncoder.Borrow(), szName);
#endif
  return m_pDefaultPass.Borrow();
}

void nsGALDeviceVulkan::EndPassPlatform(nsGALPass* pPass)
{
#if NS_ENABLED(NS_USE_PROFILING)
  nsProfilingScopeAndMarker::Stop(m_pDefaultPass->m_pRenderCommandEncoder.Borrow(), m_pPassTimingScope);
#endif
}


// State creation functions

nsGALBlendState* nsGALDeviceVulkan::CreateBlendStatePlatform(const nsGALBlendStateCreationDescription& Description)
{
  nsGALBlendStateVulkan* pState = NS_NEW(&m_Allocator, nsGALBlendStateVulkan, Description);

  if (pState->InitPlatform(this).Succeeded())
  {
    return pState;
  }
  else
  {
    NS_DELETE(&m_Allocator, pState);
    return nullptr;
  }
}

void nsGALDeviceVulkan::DestroyBlendStatePlatform(nsGALBlendState* pBlendState)
{
  nsGALBlendStateVulkan* pState = static_cast<nsGALBlendStateVulkan*>(pBlendState);
  nsResourceCacheVulkan::ResourceDeleted(pState);
  pState->DeInitPlatform(this).IgnoreResult();
  NS_DELETE(&m_Allocator, pState);
}

nsGALDepthStencilState* nsGALDeviceVulkan::CreateDepthStencilStatePlatform(const nsGALDepthStencilStateCreationDescription& Description)
{
  nsGALDepthStencilStateVulkan* pVulkanDepthStencilState = NS_NEW(&m_Allocator, nsGALDepthStencilStateVulkan, Description);

  if (pVulkanDepthStencilState->InitPlatform(this).Succeeded())
  {
    return pVulkanDepthStencilState;
  }
  else
  {
    NS_DELETE(&m_Allocator, pVulkanDepthStencilState);
    return nullptr;
  }
}

void nsGALDeviceVulkan::DestroyDepthStencilStatePlatform(nsGALDepthStencilState* pDepthStencilState)
{
  nsGALDepthStencilStateVulkan* pVulkanDepthStencilState = static_cast<nsGALDepthStencilStateVulkan*>(pDepthStencilState);
  nsResourceCacheVulkan::ResourceDeleted(pVulkanDepthStencilState);
  pVulkanDepthStencilState->DeInitPlatform(this).IgnoreResult();
  NS_DELETE(&m_Allocator, pVulkanDepthStencilState);
}

nsGALRasterizerState* nsGALDeviceVulkan::CreateRasterizerStatePlatform(const nsGALRasterizerStateCreationDescription& Description)
{
  nsGALRasterizerStateVulkan* pVulkanRasterizerState = NS_NEW(&m_Allocator, nsGALRasterizerStateVulkan, Description);

  if (pVulkanRasterizerState->InitPlatform(this).Succeeded())
  {
    return pVulkanRasterizerState;
  }
  else
  {
    NS_DELETE(&m_Allocator, pVulkanRasterizerState);
    return nullptr;
  }
}

void nsGALDeviceVulkan::DestroyRasterizerStatePlatform(nsGALRasterizerState* pRasterizerState)
{
  nsGALRasterizerStateVulkan* pVulkanRasterizerState = static_cast<nsGALRasterizerStateVulkan*>(pRasterizerState);
  nsResourceCacheVulkan::ResourceDeleted(pVulkanRasterizerState);
  pVulkanRasterizerState->DeInitPlatform(this).IgnoreResult();
  NS_DELETE(&m_Allocator, pVulkanRasterizerState);
}

nsGALSamplerState* nsGALDeviceVulkan::CreateSamplerStatePlatform(const nsGALSamplerStateCreationDescription& Description)
{
  nsGALSamplerStateVulkan* pVulkanSamplerState = NS_NEW(&m_Allocator, nsGALSamplerStateVulkan, Description);

  if (pVulkanSamplerState->InitPlatform(this).Succeeded())
  {
    return pVulkanSamplerState;
  }
  else
  {
    NS_DELETE(&m_Allocator, pVulkanSamplerState);
    return nullptr;
  }
}

void nsGALDeviceVulkan::DestroySamplerStatePlatform(nsGALSamplerState* pSamplerState)
{
  nsGALSamplerStateVulkan* pVulkanSamplerState = static_cast<nsGALSamplerStateVulkan*>(pSamplerState);
  pVulkanSamplerState->DeInitPlatform(this).IgnoreResult();
  NS_DELETE(&m_Allocator, pVulkanSamplerState);
}


// Resource creation functions

nsGALShader* nsGALDeviceVulkan::CreateShaderPlatform(const nsGALShaderCreationDescription& Description)
{
  nsGALShaderVulkan* pShader = NS_NEW(&m_Allocator, nsGALShaderVulkan, Description);

  if (!pShader->InitPlatform(this).Succeeded())
  {
    NS_DELETE(&m_Allocator, pShader);
    return nullptr;
  }

  return pShader;
}

void nsGALDeviceVulkan::DestroyShaderPlatform(nsGALShader* pShader)
{
  nsGALShaderVulkan* pVulkanShader = static_cast<nsGALShaderVulkan*>(pShader);
  nsResourceCacheVulkan::ShaderDeleted(pVulkanShader);
  pVulkanShader->DeInitPlatform(this).IgnoreResult();
  NS_DELETE(&m_Allocator, pVulkanShader);
}

nsGALBuffer* nsGALDeviceVulkan::CreateBufferPlatform(
  const nsGALBufferCreationDescription& Description, nsArrayPtr<const nsUInt8> pInitialData)
{
  nsGALBufferVulkan* pBuffer = NS_NEW(&m_Allocator, nsGALBufferVulkan, Description);

  if (!pBuffer->InitPlatform(this, pInitialData).Succeeded())
  {
    NS_DELETE(&m_Allocator, pBuffer);
    return nullptr;
  }

  return pBuffer;
}

void nsGALDeviceVulkan::DestroyBufferPlatform(nsGALBuffer* pBuffer)
{
  nsGALBufferVulkan* pVulkanBuffer = static_cast<nsGALBufferVulkan*>(pBuffer);
  GetCurrentPipelineBarrier().BufferDestroyed(pVulkanBuffer);
  pVulkanBuffer->DeInitPlatform(this).IgnoreResult();
  NS_DELETE(&m_Allocator, pVulkanBuffer);
}

nsGALTexture* nsGALDeviceVulkan::CreateTexturePlatform(const nsGALTextureCreationDescription& Description, nsArrayPtr<nsGALSystemMemoryDescription> pInitialData)
{
  nsGALTextureVulkan* pTexture = NS_NEW(&m_Allocator, nsGALTextureVulkan, Description, false, false);

  if (!pTexture->InitPlatform(this, pInitialData).Succeeded())
  {
    NS_DELETE(&m_Allocator, pTexture);
    return nullptr;
  }

  return pTexture;
}

void nsGALDeviceVulkan::DestroyTexturePlatform(nsGALTexture* pTexture)
{
  nsGALTextureVulkan* pVulkanTexture = static_cast<nsGALTextureVulkan*>(pTexture);
  GetCurrentPipelineBarrier().TextureDestroyed(pVulkanTexture);
  m_pInitContext->TextureDestroyed(pVulkanTexture);

  pVulkanTexture->DeInitPlatform(this).IgnoreResult();
  NS_DELETE(&m_Allocator, pVulkanTexture);
}

nsGALTexture* nsGALDeviceVulkan::CreateSharedTexturePlatform(const nsGALTextureCreationDescription& Description, nsArrayPtr<nsGALSystemMemoryDescription> pInitialData, nsEnum<nsGALSharedTextureType> sharedType, nsGALPlatformSharedHandle handle)
{
  nsGALSharedTextureVulkan* pTexture = NS_NEW(&m_Allocator, nsGALSharedTextureVulkan, Description, sharedType, handle);

  if (!pTexture->InitPlatform(this, pInitialData).Succeeded())
  {
    NS_DELETE(&m_Allocator, pTexture);
    return nullptr;
  }

  return pTexture;
}

void nsGALDeviceVulkan::DestroySharedTexturePlatform(nsGALTexture* pTexture)
{
  nsGALSharedTextureVulkan* pVulkanTexture = static_cast<nsGALSharedTextureVulkan*>(pTexture);
  GetCurrentPipelineBarrier().TextureDestroyed(pVulkanTexture);
  m_pInitContext->TextureDestroyed(pVulkanTexture);

  pVulkanTexture->DeInitPlatform(this).IgnoreResult();
  NS_DELETE(&m_Allocator, pVulkanTexture);
}

nsGALTextureResourceView* nsGALDeviceVulkan::CreateResourceViewPlatform(nsGALTexture* pResource, const nsGALTextureResourceViewCreationDescription& Description)
{
  nsGALTextureResourceViewVulkan* pResourceView = NS_NEW(&m_Allocator, nsGALTextureResourceViewVulkan, pResource, Description);

  if (!pResourceView->InitPlatform(this).Succeeded())
  {
    NS_DELETE(&m_Allocator, pResourceView);
    return nullptr;
  }

  return pResourceView;
}

void nsGALDeviceVulkan::DestroyResourceViewPlatform(nsGALTextureResourceView* pResourceView)
{
  nsGALTextureResourceViewVulkan* pVulkanResourceView = static_cast<nsGALTextureResourceViewVulkan*>(pResourceView);
  pVulkanResourceView->DeInitPlatform(this).IgnoreResult();
  NS_DELETE(&m_Allocator, pVulkanResourceView);
}

nsGALBufferResourceView* nsGALDeviceVulkan::CreateResourceViewPlatform(nsGALBuffer* pResource, const nsGALBufferResourceViewCreationDescription& Description)
{
  nsGALBufferResourceViewVulkan* pResourceView = NS_NEW(&m_Allocator, nsGALBufferResourceViewVulkan, pResource, Description);

  if (!pResourceView->InitPlatform(this).Succeeded())
  {
    NS_DELETE(&m_Allocator, pResourceView);
    return nullptr;
  }

  return pResourceView;
}

void nsGALDeviceVulkan::DestroyResourceViewPlatform(nsGALBufferResourceView* pResourceView)
{
  nsGALBufferResourceViewVulkan* pVulkanResourceView = static_cast<nsGALBufferResourceViewVulkan*>(pResourceView);
  pVulkanResourceView->DeInitPlatform(this).IgnoreResult();
  NS_DELETE(&m_Allocator, pVulkanResourceView);
}

nsGALRenderTargetView* nsGALDeviceVulkan::CreateRenderTargetViewPlatform(
  nsGALTexture* pTexture, const nsGALRenderTargetViewCreationDescription& Description)
{
  nsGALRenderTargetViewVulkan* pRTView = NS_NEW(&m_Allocator, nsGALRenderTargetViewVulkan, pTexture, Description);

  if (!pRTView->InitPlatform(this).Succeeded())
  {
    NS_DELETE(&m_Allocator, pRTView);
    return nullptr;
  }

  return pRTView;
}

void nsGALDeviceVulkan::DestroyRenderTargetViewPlatform(nsGALRenderTargetView* pRenderTargetView)
{
  nsGALRenderTargetViewVulkan* pVulkanRenderTargetView = static_cast<nsGALRenderTargetViewVulkan*>(pRenderTargetView);
  pVulkanRenderTargetView->DeInitPlatform(this).IgnoreResult();
  NS_DELETE(&m_Allocator, pVulkanRenderTargetView);
}

nsGALTextureUnorderedAccessView* nsGALDeviceVulkan::CreateUnorderedAccessViewPlatform(
  nsGALTexture* pTextureOfBuffer, const nsGALTextureUnorderedAccessViewCreationDescription& Description)
{
  nsGALTextureUnorderedAccessViewVulkan* pUnorderedAccessView = NS_NEW(&m_Allocator, nsGALTextureUnorderedAccessViewVulkan, pTextureOfBuffer, Description);

  if (!pUnorderedAccessView->InitPlatform(this).Succeeded())
  {
    NS_DELETE(&m_Allocator, pUnorderedAccessView);
    return nullptr;
  }

  return pUnorderedAccessView;
}

void nsGALDeviceVulkan::DestroyUnorderedAccessViewPlatform(nsGALTextureUnorderedAccessView* pUnorderedAccessView)
{
  nsGALTextureUnorderedAccessViewVulkan* pUnorderedAccessViewVulkan = static_cast<nsGALTextureUnorderedAccessViewVulkan*>(pUnorderedAccessView);
  pUnorderedAccessViewVulkan->DeInitPlatform(this).IgnoreResult();
  NS_DELETE(&m_Allocator, pUnorderedAccessViewVulkan);
}

nsGALBufferUnorderedAccessView* nsGALDeviceVulkan::CreateUnorderedAccessViewPlatform(
  nsGALBuffer* pBufferOfBuffer, const nsGALBufferUnorderedAccessViewCreationDescription& Description)
{
  nsGALBufferUnorderedAccessViewVulkan* pUnorderedAccessView = NS_NEW(&m_Allocator, nsGALBufferUnorderedAccessViewVulkan, pBufferOfBuffer, Description);

  if (!pUnorderedAccessView->InitPlatform(this).Succeeded())
  {
    NS_DELETE(&m_Allocator, pUnorderedAccessView);
    return nullptr;
  }

  return pUnorderedAccessView;
}

void nsGALDeviceVulkan::DestroyUnorderedAccessViewPlatform(nsGALBufferUnorderedAccessView* pUnorderedAccessView)
{
  nsGALBufferUnorderedAccessViewVulkan* pUnorderedAccessViewVulkan = static_cast<nsGALBufferUnorderedAccessViewVulkan*>(pUnorderedAccessView);
  pUnorderedAccessViewVulkan->DeInitPlatform(this).IgnoreResult();
  NS_DELETE(&m_Allocator, pUnorderedAccessViewVulkan);
}

// Other rendering creation functions
nsGALQuery* nsGALDeviceVulkan::CreateQueryPlatform(const nsGALQueryCreationDescription& Description)
{
  nsGALQueryVulkan* pQuery = NS_NEW(&m_Allocator, nsGALQueryVulkan, Description);

  if (!pQuery->InitPlatform(this).Succeeded())
  {
    NS_DELETE(&m_Allocator, pQuery);
    return nullptr;
  }

  return pQuery;
}

void nsGALDeviceVulkan::DestroyQueryPlatform(nsGALQuery* pQuery)
{
  nsGALQueryVulkan* pQueryVulkan = static_cast<nsGALQueryVulkan*>(pQuery);
  pQueryVulkan->DeInitPlatform(this).IgnoreResult();
  NS_DELETE(&m_Allocator, pQueryVulkan);
}

nsGALVertexDeclaration* nsGALDeviceVulkan::CreateVertexDeclarationPlatform(const nsGALVertexDeclarationCreationDescription& Description)
{
  nsGALVertexDeclarationVulkan* pVertexDeclaration = NS_NEW(&m_Allocator, nsGALVertexDeclarationVulkan, Description);

  if (pVertexDeclaration->InitPlatform(this).Succeeded())
  {
    return pVertexDeclaration;
  }
  else
  {
    NS_DELETE(&m_Allocator, pVertexDeclaration);
    return nullptr;
  }
}

void nsGALDeviceVulkan::DestroyVertexDeclarationPlatform(nsGALVertexDeclaration* pVertexDeclaration)
{
  nsGALVertexDeclarationVulkan* pVertexDeclarationVulkan = static_cast<nsGALVertexDeclarationVulkan*>(pVertexDeclaration);
  nsResourceCacheVulkan::ResourceDeleted(pVertexDeclarationVulkan);
  pVertexDeclarationVulkan->DeInitPlatform(this).IgnoreResult();
  NS_DELETE(&m_Allocator, pVertexDeclarationVulkan);
}

nsGALTimestampHandle nsGALDeviceVulkan::GetTimestampPlatform()
{
  return m_pQueryPool->GetTimestamp();
}

nsResult nsGALDeviceVulkan::GetTimestampResultPlatform(nsGALTimestampHandle hTimestamp, nsTime& result)
{
  return m_pQueryPool->GetTimestampResult(hTimestamp, result);
}

// Misc functions

void nsGALDeviceVulkan::BeginFramePlatform(const nsUInt64 uiRenderFrame)
{
  auto& pCommandEncoder = m_pDefaultPass->m_pCommandEncoderImpl;

  // check if fence is reached
  if (m_PerFrameData[m_uiCurrentPerFrameData].m_uiFrame != ((nsUInt64)-1))
  {
    auto& perFrameData = m_PerFrameData[m_uiCurrentPerFrameData];
    for (vk::Fence fence : perFrameData.m_CommandBufferFences)
    {
      vk::Result fenceStatus = m_device.getFenceStatus(fence);
      if (fenceStatus == vk::Result::eNotReady)
      {
        m_device.waitForFences(1, &fence, true, 1000000000);
      }
    }
    perFrameData.m_CommandBufferFences.Clear();

    {
      NS_LOCK(m_PerFrameData[m_uiCurrentPerFrameData].m_pendingDeletionsMutex);
      DeletePendingResources(m_PerFrameData[m_uiCurrentPerFrameData].m_pendingDeletionsPrevious);
    }
    {
      NS_LOCK(m_PerFrameData[m_uiCurrentPerFrameData].m_reclaimResourcesMutex);
      ReclaimResources(m_PerFrameData[m_uiCurrentPerFrameData].m_reclaimResourcesPrevious);
    }
    m_uiSafeFrame = m_PerFrameData[m_uiCurrentPerFrameData].m_uiFrame;
  }
  {
    auto& perFrameData = m_PerFrameData[m_uiNextPerFrameData];
    perFrameData.m_fInvTicksPerSecond = -1.0f;
  }

  m_PerFrameData[m_uiCurrentPerFrameData].m_uiFrame = m_uiFrameCounter;

  m_pQueryPool->BeginFrame(GetCurrentCommandBuffer());
  GetCurrentCommandBuffer();

#if NS_ENABLED(NS_USE_PROFILING)
  nsStringBuilder sb;
  sb.SetFormat("Frame {}", uiRenderFrame);
  m_pFrameTimingScope = nsProfilingScopeAndMarker::Start(m_pDefaultPass->m_pRenderCommandEncoder.Borrow(), sb);
#endif
}

void nsGALDeviceVulkan::EndFramePlatform()
{
#if NS_ENABLED(NS_USE_PROFILING)
  {
    // #TODO_VULKAN This is very wasteful, in normal cases the last endPipeline will have submitted the command buffer via the swapchain. Thus, we start and submit a command buffer here with only the timestamp in it.
    GetCurrentCommandBuffer();
    nsProfilingScopeAndMarker::Stop(m_pDefaultPass->m_pRenderCommandEncoder.Borrow(), m_pFrameTimingScope);
  }
#endif

  if (m_PerFrameData[m_uiCurrentPerFrameData].m_currentCommandBuffer)
  {
    Submit();
  }

  {
    // Resources can be added to deletion / reclaim outside of the render frame. These will not be covered by the fences. To handle this, we swap the resources arrays so for any newly added resources we know they are not part of the batch that is deleted / reclaimed with the frame.
    auto& currentFrameData = m_PerFrameData[m_uiCurrentPerFrameData];
    {
      NS_LOCK(currentFrameData.m_pendingDeletionsMutex);
      currentFrameData.m_pendingDeletionsPrevious.Swap(currentFrameData.m_pendingDeletions);
    }
    {
      NS_LOCK(currentFrameData.m_reclaimResourcesMutex);
      currentFrameData.m_reclaimResourcesPrevious.Swap(currentFrameData.m_reclaimResources);
    }
  }
  m_uiCurrentPerFrameData = (m_uiCurrentPerFrameData + 1) % NS_ARRAY_SIZE(m_PerFrameData);
  m_uiNextPerFrameData = (m_uiCurrentPerFrameData + 1) % NS_ARRAY_SIZE(m_PerFrameData);
  ++m_uiFrameCounter;
}

void nsGALDeviceVulkan::FillCapabilitiesPlatform()
{
  vk::PhysicalDeviceMemoryProperties memProperties = m_physicalDevice.getMemoryProperties();
  vk::PhysicalDeviceFeatures features = m_physicalDevice.getFeatures();

  nsUInt64 dedicatedMemory = 0;
  nsUInt64 systemMemory = 0;
  for (uint32_t i = 0; i < memProperties.memoryHeapCount; ++i)
  {
    if (memProperties.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal)
    {
      dedicatedMemory += memProperties.memoryHeaps[i].size;
    }
    else
    {
      systemMemory += memProperties.memoryHeaps[i].size;
    }
  }

  {
    m_Capabilities.m_sAdapterName = nsStringUtf8(m_properties.deviceName).GetData();
    m_Capabilities.m_uiDedicatedVRAM = static_cast<nsUInt64>(dedicatedMemory);
    m_Capabilities.m_uiDedicatedSystemRAM = static_cast<nsUInt64>(systemMemory);
    m_Capabilities.m_uiSharedSystemRAM = static_cast<nsUInt64>(0); // TODO
    m_Capabilities.m_bHardwareAccelerated = m_properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
  }

  m_Capabilities.m_bMultithreadedResourceCreation = true;

  m_Capabilities.m_bNoOverwriteBufferUpdate = true; // TODO how to check

  m_Capabilities.m_bShaderStageSupported[nsGALShaderStage::VertexShader] = true;
  m_Capabilities.m_bShaderStageSupported[nsGALShaderStage::HullShader] = features.tessellationShader;
  m_Capabilities.m_bShaderStageSupported[nsGALShaderStage::DomainShader] = features.tessellationShader;
  m_Capabilities.m_bShaderStageSupported[nsGALShaderStage::GeometryShader] = features.geometryShader;
  m_Capabilities.m_bShaderStageSupported[nsGALShaderStage::PixelShader] = true;
  m_Capabilities.m_bShaderStageSupported[nsGALShaderStage::ComputeShader] = true; // we check this when creating the queue, always has to be supported
  m_Capabilities.m_bInstancing = true;
  m_Capabilities.m_b32BitIndices = true;
  m_Capabilities.m_bIndirectDraw = true;
  m_Capabilities.m_uiMaxConstantBuffers = nsMath::Min(m_properties.limits.maxDescriptorSetUniformBuffers, (nsUInt32)nsMath::MaxValue<nsUInt16>());
  m_Capabilities.m_uiMaxPushConstantsSize = nsMath::Min(m_properties.limits.maxPushConstantsSize, (nsUInt32)nsMath::MaxValue<nsUInt16>());
  ;
  m_Capabilities.m_bTextureArrays = true;
  m_Capabilities.m_bCubemapArrays = true;
#if NS_ENABLED(NS_PLATFORM_LINUX) || NS_ENABLED(NS_PLATFORM_ANDROID)
  m_Capabilities.m_bSharedTextures = m_extensions.m_bTimelineSemaphore && m_extensions.m_bExternalMemoryFd && m_extensions.m_bExternalSemaphoreFd;
#elif NS_ENABLED(NS_PLATFORM_WINDOWS)
  m_Capabilities.m_bSharedTextures = m_extensions.m_bTimelineSemaphore && m_extensions.m_bExternalMemoryWin32 && m_extensions.m_bExternalSemaphoreWin32;
#else
  NS_ASSERT_NOT_IMPLEMENTED;
#endif
  m_Capabilities.m_uiMaxTextureDimension = m_properties.limits.maxImageDimension1D;
  m_Capabilities.m_uiMaxCubemapDimension = m_properties.limits.maxImageDimensionCube;
  m_Capabilities.m_uiMax3DTextureDimension = m_properties.limits.maxImageDimension3D;
  m_Capabilities.m_uiMaxAnisotropy = static_cast<nsUInt16>(m_properties.limits.maxSamplerAnisotropy);
  m_Capabilities.m_uiMaxRendertargets = m_properties.limits.maxColorAttachments;
  m_Capabilities.m_uiUAVCount = nsMath::Min(nsMath::Min(m_properties.limits.maxDescriptorSetStorageBuffers, m_properties.limits.maxDescriptorSetStorageImages), (nsUInt32)nsMath::MaxValue<nsUInt16>());
  m_Capabilities.m_bAlphaToCoverage = true;
  m_Capabilities.m_bVertexShaderRenderTargetArrayIndex = m_extensions.m_bShaderViewportIndexLayer;

  m_Capabilities.m_bConservativeRasterization = false; // need to query for VK_EXT_CONSERVATIVE_RASTERIZATION

  m_Capabilities.m_FormatSupport.SetCount(nsGALResourceFormat::ENUM_COUNT);
  for (nsUInt32 i = 0; i < nsGALResourceFormat::ENUM_COUNT; i++)
  {
    nsGALResourceFormat::Enum format = (nsGALResourceFormat::Enum)i;
    const nsGALFormatLookupEntryVulkan& entry = m_FormatLookupTable.GetFormatInfo(format);
    const vk::FormatProperties formatProps = GetVulkanPhysicalDevice().getFormatProperties(entry.m_format);

    if (formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage)
    {
      m_Capabilities.m_FormatSupport[i].Add(nsGALResourceFormatSupport::Texture);
      vk::ImageFormatProperties props;
      vk::Result res = GetVulkanPhysicalDevice().getImageFormatProperties(entry.m_format, vk::ImageType::e2D, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled, {}, &props);
      if (res == vk::Result::eSuccess)
      {
        if (props.sampleCounts & vk::SampleCountFlagBits::e2)
          m_Capabilities.m_FormatSupport[i].Add(nsGALResourceFormatSupport::MSAA2x);
        if (props.sampleCounts & vk::SampleCountFlagBits::e4)
          m_Capabilities.m_FormatSupport[i].Add(nsGALResourceFormatSupport::MSAA4x);
        if (props.sampleCounts & vk::SampleCountFlagBits::e8)
          m_Capabilities.m_FormatSupport[i].Add(nsGALResourceFormatSupport::MSAA8x);
      }
    }
    if (formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eStorageImage)
      m_Capabilities.m_FormatSupport[i].Add(nsGALResourceFormatSupport::TextureRW);
    if (formatProps.bufferFeatures & vk::FormatFeatureFlagBits::eVertexBuffer)
      m_Capabilities.m_FormatSupport[i].Add(nsGALResourceFormatSupport::VertexAttribute);
    if (nsGALResourceFormat::IsDepthFormat(format))
    {
      if (formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
        m_Capabilities.m_FormatSupport[i].Add(nsGALResourceFormatSupport::RenderTarget);
    }
    else
    {
      if (formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eColorAttachment)
        m_Capabilities.m_FormatSupport[i].Add(nsGALResourceFormatSupport::RenderTarget);
    }
  }
}

void nsGALDeviceVulkan::FlushPlatform()
{
  Submit();
}

void nsGALDeviceVulkan::WaitIdlePlatform()
{
  // Make sure command buffers get flushed. Also, no need to add a wait semaphore if we flush anyway, all commands will be done.
  Submit(false);
  m_device.waitIdle();
  DestroyDeadObjects();
  for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(m_PerFrameData); ++i)
  {
    // First, we wait for all fences for all submit calls. This is necessary to make sure no resources of the frame are still in use by the GPU.
    auto& perFrameData = m_PerFrameData[i];
    for (vk::Fence fence : perFrameData.m_CommandBufferFences)
    {
      vk::Result fenceStatus = m_device.getFenceStatus(fence);
      if (fenceStatus == vk::Result::eNotReady)
      {
        m_device.waitForFences(1, &fence, true, 1000000000);
      }
    }
    perFrameData.m_CommandBufferFences.Clear();
  }

  for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(m_PerFrameData); ++i)
  {
    {
      NS_LOCK(m_PerFrameData[i].m_pendingDeletionsMutex);
      DeletePendingResources(m_PerFrameData[i].m_pendingDeletionsPrevious);
      DeletePendingResources(m_PerFrameData[i].m_pendingDeletions);
    }
    {
      NS_LOCK(m_PerFrameData[i].m_reclaimResourcesMutex);
      ReclaimResources(m_PerFrameData[i].m_reclaimResourcesPrevious);
      ReclaimResources(m_PerFrameData[i].m_reclaimResources);
    }
  }
}

vk::PipelineStageFlags nsGALDeviceVulkan::GetSupportedStages() const
{
  return m_supportedStages;
}

nsInt32 nsGALDeviceVulkan::GetMemoryIndex(vk::MemoryPropertyFlags properties, const vk::MemoryRequirements& requirements) const
{

  for (nsUInt32 i = 0; i < m_memoryProperties.memoryTypeCount; ++i)
  {
    const vk::MemoryType& type = m_memoryProperties.memoryTypes[i];
    if (requirements.memoryTypeBits & (1 << i) && (type.propertyFlags & properties))
    {
      return i;
    }
  }

  return -1;
}

void nsGALDeviceVulkan::DeleteLaterImpl(const PendingDeletion& deletion)
{
  NS_LOCK(m_PerFrameData[m_uiCurrentPerFrameData].m_pendingDeletionsMutex);
  m_PerFrameData[m_uiCurrentPerFrameData].m_pendingDeletions.PushBack(deletion);
}

void nsGALDeviceVulkan::ReclaimLater(const ReclaimResource& reclaim)
{
  NS_LOCK(m_PerFrameData[m_uiCurrentPerFrameData].m_reclaimResourcesMutex);
  m_PerFrameData[m_uiCurrentPerFrameData].m_reclaimResources.PushBack(reclaim);
}

void nsGALDeviceVulkan::DeletePendingResources(nsDeque<PendingDeletion>& pendingDeletions)
{
  for (PendingDeletion& deletion : pendingDeletions)
  {
    switch (deletion.m_type)
    {
      case vk::ObjectType::eUnknown:
        if (deletion.m_flags.IsSet(PendingDeletionFlags::IsFileDescriptor))
        {
#if NS_ENABLED(NS_PLATFORM_LINUX) || NS_ENABLED(NS_PLATFORM_ANDROID)
          int fileDescriptor = static_cast<int>(reinterpret_cast<size_t>(deletion.m_pObject));
          int res = close(fileDescriptor);
          if (res == -1)
          {
            nsLog::Error("close() failed on file descriptor with errno: {}", nsArgErrno(errno));
          }
#else
          NS_ASSERT_NOT_IMPLEMENTED;
#endif
        }
        else
        {
          NS_REPORT_FAILURE("Unknown pending deletion");
        }
        break;
      case vk::ObjectType::eImageView:
        m_device.destroyImageView(reinterpret_cast<vk::ImageView&>(deletion.m_pObject));
        break;
      case vk::ObjectType::eImage:
      {
        auto& image = reinterpret_cast<vk::Image&>(deletion.m_pObject);
        OnBeforeImageDestroyed.Broadcast(OnBeforeImageDestroyedData{image, *this});
        if (deletion.m_flags.IsSet(PendingDeletionFlags::UsesExternalMemory))
        {
          m_device.destroyImage(image);
          auto& deviceMemory = reinterpret_cast<vk::DeviceMemory&>(deletion.m_pContext);
          m_device.freeMemory(deviceMemory);
        }
        else
        {
          nsMemoryAllocatorVulkan::DestroyImage(image, deletion.m_allocation);
        }
      }
      break;
      case vk::ObjectType::eBuffer:
        nsMemoryAllocatorVulkan::DestroyBuffer(reinterpret_cast<vk::Buffer&>(deletion.m_pObject), deletion.m_allocation);
        break;
      case vk::ObjectType::eBufferView:
        m_device.destroyBufferView(reinterpret_cast<vk::BufferView&>(deletion.m_pObject));
        break;
      case vk::ObjectType::eFramebuffer:
        m_device.destroyFramebuffer(reinterpret_cast<vk::Framebuffer&>(deletion.m_pObject));
        break;
      case vk::ObjectType::eRenderPass:
        m_device.destroyRenderPass(reinterpret_cast<vk::RenderPass&>(deletion.m_pObject));
        break;
      case vk::ObjectType::eSampler:
        m_device.destroySampler(reinterpret_cast<vk::Sampler&>(deletion.m_pObject));
        break;
      case vk::ObjectType::eSemaphore:
        m_device.destroySemaphore(reinterpret_cast<vk::Semaphore&>(deletion.m_pObject));
        break;
      case vk::ObjectType::eSwapchainKHR:
        m_device.destroySwapchainKHR(reinterpret_cast<vk::SwapchainKHR&>(deletion.m_pObject));
        break;
      case vk::ObjectType::eSurfaceKHR:
        m_instance.destroySurfaceKHR(reinterpret_cast<vk::SurfaceKHR&>(deletion.m_pObject));
        if (nsWindowBase* pWindow = reinterpret_cast<nsWindowBase*>(deletion.m_pContext))
        {
          pWindow->RemoveReference();
        }
        break;
      case vk::ObjectType::eShaderModule:
        m_device.destroyShaderModule(reinterpret_cast<vk::ShaderModule&>(deletion.m_pObject));
        break;
      case vk::ObjectType::ePipeline:
        m_device.destroyPipeline(reinterpret_cast<vk::Pipeline&>(deletion.m_pObject));
        break;
      default:
        NS_REPORT_FAILURE("This object type is not implemented");
        break;
    }
  }
  pendingDeletions.Clear();
}

void nsGALDeviceVulkan::ReclaimResources(nsDeque<ReclaimResource>& resources)
{
  for (ReclaimResource& resource : resources)
  {
    switch (resource.m_type)
    {
      case vk::ObjectType::eSemaphore:
        nsSemaphorePoolVulkan::ReclaimSemaphore(reinterpret_cast<vk::Semaphore&>(resource.m_pObject));
        break;
      case vk::ObjectType::eFence:
        nsFencePoolVulkan::ReclaimFence(reinterpret_cast<vk::Fence&>(resource.m_pObject));
        break;
      case vk::ObjectType::eCommandBuffer:
        static_cast<nsCommandBufferPoolVulkan*>(resource.m_pContext)->ReclaimCommandBuffer(reinterpret_cast<vk::CommandBuffer&>(resource.m_pObject));
        break;
      case vk::ObjectType::eDescriptorPool:
        nsDescriptorSetPoolVulkan::ReclaimPool(reinterpret_cast<vk::DescriptorPool&>(resource.m_pObject));
        break;
      default:
        NS_REPORT_FAILURE("This object type is not implemented");
        break;
    }
  }
  resources.Clear();
}

void nsGALDeviceVulkan::FillFormatLookupTable()
{
  /// The list below is in the same order as the nsGALResourceFormat enum. No format should be missing except the ones that are just different names for the same enum value.
  vk::Format R32G32B32A32_Formats[] = {vk::Format::eR32G32B32A32Sfloat, vk::Format::eR32G32B32A32Uint, vk::Format::eR32G32B32A32Sint};
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGBAFloat, nsGALFormatLookupEntryVulkan(vk::Format::eR32G32B32A32Sfloat, R32G32B32A32_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGBAUInt, nsGALFormatLookupEntryVulkan(vk::Format::eR32G32B32A32Uint, R32G32B32A32_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGBAInt, nsGALFormatLookupEntryVulkan(vk::Format::eR32G32B32A32Sint, R32G32B32A32_Formats));

  vk::Format R32G32B32_Formats[] = {vk::Format::eR32G32B32Sfloat, vk::Format::eR32G32B32Uint, vk::Format::eR32G32B32Sint};
  // TODO 3-channel formats are not really supported under vulkan judging by experience
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGBFloat, nsGALFormatLookupEntryVulkan(vk::Format::eR32G32B32Sfloat, R32G32B32_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGBUInt, nsGALFormatLookupEntryVulkan(vk::Format::eR32G32B32Uint, R32G32B32_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGBInt, nsGALFormatLookupEntryVulkan(vk::Format::eR32G32B32Sint, R32G32B32_Formats));

  // TODO dunno if these are actually supported for the respective Vulkan device
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::B5G6R5UNormalized, nsGALFormatLookupEntryVulkan(vk::Format::eR5G6B5UnormPack16));

  vk::Format B8G8R8A8_Formats[] = {vk::Format::eB8G8R8A8Unorm, vk::Format::eB8G8R8A8Srgb};
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::BGRAUByteNormalized, nsGALFormatLookupEntryVulkan(vk::Format::eB8G8R8A8Unorm, B8G8R8A8_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::BGRAUByteNormalizedsRGB, nsGALFormatLookupEntryVulkan(vk::Format::eB8G8R8A8Srgb, B8G8R8A8_Formats));

  vk::Format R16G16B16A16_Formats[] = {vk::Format::eR16G16B16A16Sfloat, vk::Format::eR16G16B16A16Uint, vk::Format::eR16G16B16A16Unorm, vk::Format::eR16G16B16A16Sint, vk::Format::eR16G16B16A16Snorm};
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGBAHalf, nsGALFormatLookupEntryVulkan(vk::Format::eR16G16B16A16Sfloat, R16G16B16A16_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGBAUShort, nsGALFormatLookupEntryVulkan(vk::Format::eR16G16B16A16Uint, R16G16B16A16_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGBAUShortNormalized, nsGALFormatLookupEntryVulkan(vk::Format::eR16G16B16A16Unorm, R16G16B16A16_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGBAShort, nsGALFormatLookupEntryVulkan(vk::Format::eR16G16B16A16Sint, R16G16B16A16_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGBAShortNormalized, nsGALFormatLookupEntryVulkan(vk::Format::eR16G16B16A16Snorm, R16G16B16A16_Formats));

  vk::Format R32G32_Formats[] = {vk::Format::eR32G32Sfloat, vk::Format::eR32G32Uint, vk::Format::eR32G32Sint};
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGFloat, nsGALFormatLookupEntryVulkan(vk::Format::eR32G32Sfloat, R32G32_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGUInt, nsGALFormatLookupEntryVulkan(vk::Format::eR32G32Uint, R32G32_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGInt, nsGALFormatLookupEntryVulkan(vk::Format::eR32G32Sint, R32G32_Formats));

  vk::Format R10G10B10A2_Formats[] = {vk::Format::eA2B10G10R10UintPack32, vk::Format::eA2B10G10R10UnormPack32};
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGB10A2UInt, nsGALFormatLookupEntryVulkan(vk::Format::eA2B10G10R10UintPack32, R10G10B10A2_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGB10A2UIntNormalized, nsGALFormatLookupEntryVulkan(vk::Format::eA2B10G10R10UnormPack32, R10G10B10A2_Formats));

  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RG11B10Float, nsGALFormatLookupEntryVulkan(vk::Format::eB10G11R11UfloatPack32));

  vk::Format R8G8B8A8_Formats[] = {vk::Format::eR8G8B8A8Unorm, vk::Format::eR8G8B8A8Srgb, vk::Format::eR8G8B8A8Uint, vk::Format::eR8G8B8A8Snorm, vk::Format::eR8G8B8A8Sint};
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGBAUByteNormalized, nsGALFormatLookupEntryVulkan(vk::Format::eR8G8B8A8Unorm, R8G8B8A8_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGBAUByteNormalizedsRGB, nsGALFormatLookupEntryVulkan(vk::Format::eR8G8B8A8Srgb, R8G8B8A8_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGBAUByte, nsGALFormatLookupEntryVulkan(vk::Format::eR8G8B8A8Uint, R8G8B8A8_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGBAByteNormalized, nsGALFormatLookupEntryVulkan(vk::Format::eR8G8B8A8Snorm, R8G8B8A8_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGBAByte, nsGALFormatLookupEntryVulkan(vk::Format::eR8G8B8A8Sint, R8G8B8A8_Formats));

  vk::Format R16G16_Formats[] = {vk::Format::eR16G16Sfloat, vk::Format::eR16G16Uint, vk::Format::eR16G16Unorm, vk::Format::eR16G16Sint, vk::Format::eR16G16Snorm};
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGHalf, nsGALFormatLookupEntryVulkan(vk::Format::eR16G16Sfloat, R16G16_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGUShort, nsGALFormatLookupEntryVulkan(vk::Format::eR16G16Uint, R16G16_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGUShortNormalized, nsGALFormatLookupEntryVulkan(vk::Format::eR16G16Unorm, R16G16_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGShort, nsGALFormatLookupEntryVulkan(vk::Format::eR16G16Sint, R16G16_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGShortNormalized, nsGALFormatLookupEntryVulkan(vk::Format::eR16G16Snorm, R16G16_Formats));

  vk::Format R8G8_Formats[] = {vk::Format::eR8G8Uint, vk::Format::eR8G8Unorm, vk::Format::eR8G8Sint, vk::Format::eR8G8Snorm};
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGUByte, nsGALFormatLookupEntryVulkan(vk::Format::eR8G8Uint, R8G8_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGUByteNormalized, nsGALFormatLookupEntryVulkan(vk::Format::eR8G8Unorm, R8G8_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGByte, nsGALFormatLookupEntryVulkan(vk::Format::eR8G8Sint, R8G8_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RGByteNormalized, nsGALFormatLookupEntryVulkan(vk::Format::eR8G8Snorm, R8G8_Formats));

  vk::Format R32_Formats[] = {vk::Format::eR32Sfloat, vk::Format::eR32Uint, vk::Format::eR32Sint};
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RFloat, nsGALFormatLookupEntryVulkan(vk::Format::eR32Sfloat, R32_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RUInt, nsGALFormatLookupEntryVulkan(vk::Format::eR32Uint, R32_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RInt, nsGALFormatLookupEntryVulkan(vk::Format::eR32Sint, R32_Formats));

  vk::Format R16_Formats[] = {vk::Format::eR16Sfloat, vk::Format::eR16Uint, vk::Format::eR16Unorm, vk::Format::eR16Sint, vk::Format::eR16Snorm};
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RHalf, nsGALFormatLookupEntryVulkan(vk::Format::eR16Sfloat, R16_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RUShort, nsGALFormatLookupEntryVulkan(vk::Format::eR16Uint, R16_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RUShortNormalized, nsGALFormatLookupEntryVulkan(vk::Format::eR16Unorm, R16_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RShort, nsGALFormatLookupEntryVulkan(vk::Format::eR16Sint, R16_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RShortNormalized, nsGALFormatLookupEntryVulkan(vk::Format::eR16Snorm, R16_Formats));

  vk::Format R8_Formats[] = {vk::Format::eR8Uint, vk::Format::eR8Unorm, vk::Format::eR8Sint, vk::Format::eR8Snorm};
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RUByte, nsGALFormatLookupEntryVulkan(vk::Format::eR8Uint, R8_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RUByteNormalized, nsGALFormatLookupEntryVulkan(vk::Format::eR8Unorm, R8_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RByte, nsGALFormatLookupEntryVulkan(vk::Format::eR8Sint, R8_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::RByteNormalized, nsGALFormatLookupEntryVulkan(vk::Format::eR8Snorm, R8_Formats));

  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::AUByteNormalized, nsGALFormatLookupEntryVulkan(vk::Format::eR8Unorm));

  auto SelectDepthFormat = [&](const std::vector<vk::Format>& list) -> vk::Format
  {
    for (auto& format : list)
    {
      vk::FormatProperties formatProperties;
      m_physicalDevice.getFormatProperties(format, &formatProperties);
      if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
        return format;
    }
    return vk::Format::eUndefined;
  };

  auto SelectStorageFormat = [](vk::Format depthFormat) -> vk::Format
  {
    switch (depthFormat)
    {
      case vk::Format::eD16Unorm:
        return vk::Format::eR16Unorm;
      case vk::Format::eD16UnormS8Uint:
        return vk::Format::eUndefined;
      case vk::Format::eD24UnormS8Uint:
        return vk::Format::eUndefined;
      case vk::Format::eD32Sfloat:
        return vk::Format::eR32Sfloat;
      case vk::Format::eD32SfloatS8Uint:
        return vk::Format::eR32Sfloat;
      default:
        return vk::Format::eUndefined;
    }
  };

  // Select smallest available depth format.  #TODO_VULKAN support packed eX8D24UnormPack32?
  vk::Format depthFormat = SelectDepthFormat({vk::Format::eD16Unorm, vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint});
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::D16, nsGALFormatLookupEntryVulkan(depthFormat).R(SelectStorageFormat(depthFormat)));

  // Select closest depth stencil format.
  depthFormat = SelectDepthFormat({vk::Format::eD24UnormS8Uint, vk::Format::eD32SfloatS8Uint, vk::Format::eD16UnormS8Uint});
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::D24S8, nsGALFormatLookupEntryVulkan(depthFormat).R(SelectStorageFormat(depthFormat)));

  // Select biggest depth format.
  depthFormat = SelectDepthFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD16Unorm});
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::DFloat, nsGALFormatLookupEntryVulkan(depthFormat).R(SelectStorageFormat(depthFormat)));

  vk::Format BC1_Formats[] = {vk::Format::eBc1RgbaUnormBlock, vk::Format::eBc1RgbaSrgbBlock};
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::BC1, nsGALFormatLookupEntryVulkan(vk::Format::eBc1RgbaUnormBlock, BC1_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::BC1sRGB, nsGALFormatLookupEntryVulkan(vk::Format::eBc1RgbaSrgbBlock, BC1_Formats));

  vk::Format BC2_Formats[] = {vk::Format::eBc2UnormBlock, vk::Format::eBc2SrgbBlock};
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::BC2, nsGALFormatLookupEntryVulkan(vk::Format::eBc2UnormBlock, BC2_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::BC2sRGB, nsGALFormatLookupEntryVulkan(vk::Format::eBc2SrgbBlock, BC2_Formats));

  vk::Format BC3_Formats[] = {vk::Format::eBc3UnormBlock, vk::Format::eBc3SrgbBlock};
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::BC3, nsGALFormatLookupEntryVulkan(vk::Format::eBc3UnormBlock, BC3_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::BC3sRGB, nsGALFormatLookupEntryVulkan(vk::Format::eBc3SrgbBlock, BC3_Formats));

  vk::Format BC4_Formats[] = {vk::Format::eBc4UnormBlock, vk::Format::eBc4SnormBlock};
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::BC4UNormalized, nsGALFormatLookupEntryVulkan(vk::Format::eBc4UnormBlock, BC4_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::BC4Normalized, nsGALFormatLookupEntryVulkan(vk::Format::eBc4SnormBlock, BC4_Formats));

  vk::Format BC5_Formats[] = {vk::Format::eBc5UnormBlock, vk::Format::eBc5SnormBlock};
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::BC5UNormalized, nsGALFormatLookupEntryVulkan(vk::Format::eBc5UnormBlock, BC5_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::BC5Normalized, nsGALFormatLookupEntryVulkan(vk::Format::eBc5SnormBlock, BC5_Formats));

  vk::Format BC6_Formats[] = {vk::Format::eBc6HUfloatBlock, vk::Format::eBc6HSfloatBlock};
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::BC6UFloat, nsGALFormatLookupEntryVulkan(vk::Format::eBc6HUfloatBlock, BC6_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::BC6Float, nsGALFormatLookupEntryVulkan(vk::Format::eBc6HSfloatBlock, BC6_Formats));

  vk::Format BC7_Formats[] = {vk::Format::eBc7UnormBlock, vk::Format::eBc7SrgbBlock};
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::BC7UNormalized, nsGALFormatLookupEntryVulkan(vk::Format::eBc7UnormBlock, BC7_Formats));
  m_FormatLookupTable.SetFormatInfo(nsGALResourceFormat::BC7UNormalizedsRGB, nsGALFormatLookupEntryVulkan(vk::Format::eBc7SrgbBlock, BC7_Formats));

  if (false)
  {
    NS_LOG_BLOCK("GAL Resource Formats");
    for (nsUInt32 i = 1; i < nsGALResourceFormat::ENUM_COUNT; i++)
    {
      const nsGALFormatLookupEntryVulkan& entry = m_FormatLookupTable.GetFormatInfo((nsGALResourceFormat::Enum)i);

      vk::FormatProperties formatProperties;
      m_physicalDevice.getFormatProperties(entry.m_format, &formatProperties);

      const bool bSampled = static_cast<bool>(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage);
      const bool bColorAttachment = static_cast<bool>(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eColorAttachment);
      const bool bDepthAttachment = static_cast<bool>(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment);
      const bool bStorageImage = static_cast<bool>(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eStorageImage);

      const bool bTexel = static_cast<bool>(formatProperties.bufferFeatures & vk::FormatFeatureFlagBits::eUniformTexelBuffer);
      const bool bStorageTexel = static_cast<bool>(formatProperties.bufferFeatures & vk::FormatFeatureFlagBits::eStorageTexelBuffer);
      const bool bVertex = static_cast<bool>(formatProperties.bufferFeatures & vk::FormatFeatureFlagBits::eVertexBuffer);

      nsStringBuilder sTemp;
      nsReflectionUtils::EnumerationToString(nsGetStaticRTTI<nsGALResourceFormat>(), i, sTemp, nsReflectionUtils::EnumConversionMode::ValueNameOnly);

      nsLog::Info("OptTiling S: {}, UAV: {}, CA: {}, DA: {}. Buffer: T: {}, ST: {}, V: {}, Format {} -> {}", bSampled ? 1 : 0, bStorageImage ? 1 : 0, bColorAttachment ? 1 : 0, bDepthAttachment ? 1 : 0, bTexel ? 1 : 0, bStorageTexel ? 1 : 0, bVertex ? 1 : 0, sTemp, vk::to_string(entry.m_format).c_str());
    }
  }
}

const nsGALSharedTexture* nsGALDeviceVulkan::GetSharedTexture(nsGALTextureHandle hTexture) const
{
  auto pTexture = GetTexture(hTexture);
  if (pTexture == nullptr)
  {
    return nullptr;
  }

  // Resolve proxy texture if any
  return static_cast<const nsGALSharedTextureVulkan*>(pTexture->GetParentResource());
}

void nsGALDeviceVulkan::AddWaitSemaphore(const SemaphoreInfo& waitSemaphore)
{
  // #TODO_VULKAN Assert is in render pipeline, thread safety
  if (waitSemaphore.m_type == vk::SemaphoreType::eTimeline)
    m_waitSemaphores.InsertAt(0, waitSemaphore);
  else
    m_waitSemaphores.PushBack(waitSemaphore);
}

void nsGALDeviceVulkan::AddSignalSemaphore(const SemaphoreInfo& signalSemaphore)
{
  // #TODO_VULKAN Assert is in render pipeline, thread safety
  if (signalSemaphore.m_type == vk::SemaphoreType::eTimeline)
    m_signalSemaphores.InsertAt(0, signalSemaphore);
  else
    m_signalSemaphores.PushBack(signalSemaphore);
}


NS_STATICLINK_FILE(RendererVulkan, RendererVulkan_Device_Implementation_DeviceVulkan);
