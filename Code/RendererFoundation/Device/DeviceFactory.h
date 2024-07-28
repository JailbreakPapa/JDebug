#pragma once

#include <Foundation/Types/Delegate.h>
#include <RendererFoundation/RendererFoundationDLL.h>

struct NS_RENDERERFOUNDATION_DLL nsGALDeviceFactory
{
  using CreatorFunc = nsDelegate<nsInternal::NewInstance<nsGALDevice>(nsAllocator*, const nsGALDeviceCreationDescription&)>;

  static nsInternal::NewInstance<nsGALDevice> CreateDevice(nsStringView sRendererName, nsAllocator* pAllocator, const nsGALDeviceCreationDescription& desc);

  static void GetShaderModelAndCompiler(nsStringView sRendererName, const char*& ref_szShaderModel, const char*& ref_szShaderCompiler);

  static void RegisterCreatorFunc(const char* szRendererName, const CreatorFunc& func, const char* szShaderModel, const char* szShaderCompiler);
  static void UnregisterCreatorFunc(const char* szRendererName);
};
