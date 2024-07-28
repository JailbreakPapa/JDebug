#include <RendererFoundation/RendererFoundationPCH.h>

#include <RendererFoundation/Device/DeviceFactory.h>

struct CreatorFuncInfo
{
  nsGALDeviceFactory::CreatorFunc m_Func;
  nsString m_sShaderModel;
  nsString m_sShaderCompiler;
};

static nsHashTable<nsString, CreatorFuncInfo> s_CreatorFuncs;

CreatorFuncInfo* GetCreatorFuncInfo(nsStringView sRendererName)
{
  auto pFuncInfo = s_CreatorFuncs.GetValue(sRendererName);
  if (pFuncInfo == nullptr)
  {
    nsStringBuilder sPluginName = "nsRenderer";
    sPluginName.Append(sRendererName);

    NS_VERIFY(nsPlugin::LoadPlugin(sPluginName).Succeeded(), "Renderer plugin '{}' not found", sPluginName);

    pFuncInfo = s_CreatorFuncs.GetValue(sRendererName);
    NS_ASSERT_DEV(pFuncInfo != nullptr, "Renderer '{}' is not registered", sRendererName);
  }

  return pFuncInfo;
}

nsInternal::NewInstance<nsGALDevice> nsGALDeviceFactory::CreateDevice(nsStringView sRendererName, nsAllocator* pAllocator, const nsGALDeviceCreationDescription& desc)
{
  if (auto pFuncInfo = GetCreatorFuncInfo(sRendererName))
  {
    return pFuncInfo->m_Func(pAllocator, desc);
  }

  return nsInternal::NewInstance<nsGALDevice>(nullptr, pAllocator);
}

void nsGALDeviceFactory::GetShaderModelAndCompiler(nsStringView sRendererName, const char*& ref_szShaderModel, const char*& ref_szShaderCompiler)
{
  if (auto pFuncInfo = GetCreatorFuncInfo(sRendererName))
  {
    ref_szShaderModel = pFuncInfo->m_sShaderModel;
    ref_szShaderCompiler = pFuncInfo->m_sShaderCompiler;
  }
}

void nsGALDeviceFactory::RegisterCreatorFunc(const char* szRendererName, const CreatorFunc& func, const char* szShaderModel, const char* szShaderCompiler)
{
  CreatorFuncInfo funcInfo;
  funcInfo.m_Func = func;
  funcInfo.m_sShaderModel = szShaderModel;
  funcInfo.m_sShaderCompiler = szShaderCompiler;

  NS_VERIFY(s_CreatorFuncs.Insert(szRendererName, funcInfo) == false, "Creator func already registered");
}

void nsGALDeviceFactory::UnregisterCreatorFunc(const char* szRendererName)
{
  NS_VERIFY(s_CreatorFuncs.Remove(szRendererName), "Creator func not registered");
}
