#pragma once

#include <RendererFoundation/RendererFoundationDLL.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
// Needed for vulkan.hpp which includes headers that include windows.h which then define min, breaking std::min used in vulkan.hpp :-/
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#endif

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
#  define VK_USE_PLATFORM_WIN32_KHR
#elif NS_ENABLED(NS_PLATFORM_LINUX)
#  define VK_USE_PLATFORM_XCB_KHR
#elif NS_ENABLED(NS_PLATFORM_ANDROID)
#  define VK_USE_PLATFORM_ANDROID_KHR
#endif

#define VULKAN_HPP_NO_NODISCARD_WARNINGS // TODO: temporarily disable warnings to make it compile. Need to fix all the warnings later.
#include <vulkan/vulkan.hpp>

#if NS_ENABLED(NS_PLATFORM_ANDROID)
#  include <vulkan/vulkan_android.h>
#elif NS_ENABLED(NS_PLATFORM_WINDOWS)
#  include <vulkan/vulkan_win32.h>
#endif

// Configure the DLL Import/Export Define
#if NS_ENABLED(NS_COMPILE_ENGINE_AS_DLL)
#  ifdef BUILDSYSTEM_BUILDING_RENDERERVULKAN_LIB
#    define NS_RENDERERVULKAN_DLL NS_DECL_EXPORT
#  else
#    define NS_RENDERERVULKAN_DLL NS_DECL_IMPORT
#  endif
#else
#  define NS_RENDERERVULKAN_DLL
#endif

// Uncomment to log all layout transitions.
// #define VK_LOG_LAYOUT_CHANGES

#define NS_GAL_VULKAN_RELEASE(vulkanObj) \
  do                                     \
  {                                      \
    if ((vulkanObj) != nullptr)          \
    {                                    \
      (vulkanObj)->Release();            \
      (vulkanObj) = nullptr;             \
    }                                    \
  } while (0)

#define VK_ASSERT_DEBUG(code)                                                                                           \
  do                                                                                                                    \
  {                                                                                                                     \
    auto s = (code);                                                                                                    \
    NS_ASSERT_DEBUG(static_cast<vk::Result>(s) == vk::Result::eSuccess, "Vukan call '{0}' failed with: {1} in {2}:{3}", \
      NS_STRINGIZE(code), vk::to_string(static_cast<vk::Result>(s)).data(), NS_SOURCE_FILE, NS_SOURCE_LINE);            \
  } while (false)

#define VK_ASSERT_DEV(code)                                                                                           \
  do                                                                                                                  \
  {                                                                                                                   \
    auto s = (code);                                                                                                  \
    NS_ASSERT_DEV(static_cast<vk::Result>(s) == vk::Result::eSuccess, "Vukan call '{0}' failed with: {1} in {2}:{3}", \
      NS_STRINGIZE(code), vk::to_string(static_cast<vk::Result>(s)).data(), NS_SOURCE_FILE, NS_SOURCE_LINE);          \
  } while (false)

#define VK_LOG_ERROR(code)                                                                                                                                                \
  do                                                                                                                                                                      \
  {                                                                                                                                                                       \
    auto s = (code);                                                                                                                                                      \
    if (static_cast<vk::Result>(s) != vk::Result::eSuccess)                                                                                                               \
    {                                                                                                                                                                     \
      nsLog::Error("Vukan call '{0}' failed with: {1} in {2}:{3}", NS_STRINGIZE(code), vk::to_string(static_cast<vk::Result>(s)).data(), NS_SOURCE_FILE, NS_SOURCE_LINE); \
    }                                                                                                                                                                     \
  } while (false)

#define VK_SUCCEED_OR_RETURN_LOG(code)                                                                                                                                    \
  do                                                                                                                                                                      \
  {                                                                                                                                                                       \
    auto s = (code);                                                                                                                                                      \
    if (static_cast<vk::Result>(s) != vk::Result::eSuccess)                                                                                                               \
    {                                                                                                                                                                     \
      nsLog::Error("Vukan call '{0}' failed with: {1} in {2}:{3}", NS_STRINGIZE(code), vk::to_string(static_cast<vk::Result>(s)).data(), NS_SOURCE_FILE, NS_SOURCE_LINE); \
      return s;                                                                                                                                                           \
    }                                                                                                                                                                     \
  } while (false)

#define VK_SUCCEED_OR_RETURN_NS_FAILURE(code)                                                                                                                             \
  do                                                                                                                                                                      \
  {                                                                                                                                                                       \
    auto s = (code);                                                                                                                                                      \
    if (static_cast<vk::Result>(s) != vk::Result::eSuccess)                                                                                                               \
    {                                                                                                                                                                     \
      nsLog::Error("Vukan call '{0}' failed with: {1} in {2}:{3}", NS_STRINGIZE(code), vk::to_string(static_cast<vk::Result>(s)).data(), NS_SOURCE_FILE, NS_SOURCE_LINE); \
      return NS_FAILURE;                                                                                                                                                  \
    }                                                                                                                                                                     \
  } while (false)
