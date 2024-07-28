#pragma once

#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/RendererFoundationDLL.h>

class NS_RENDERERFOUNDATION_DLL nsGALShader : public nsGALObject<nsGALShaderCreationDescription>
{
public:
  virtual void SetDebugName(const char* szName) const = 0;

  /// Returns the list of shader resources and their binding information. These must be bound before the shader can be used.
  nsArrayPtr<const nsShaderResourceBinding> GetBindingMapping() const;
  /// Convenience function that finds 'sName' in GetBindingMapping and returns it if present.
  const nsShaderResourceBinding* GetShaderResourceBinding(const nsTempHashedString& sName) const;

  /// Returns the list of vertex input attributes. Compute shaders return an empty array.
  nsArrayPtr<const nsShaderVertexInputAttribute> GetVertexInputAttributes() const;

protected:
  friend class nsGALDevice;

  virtual nsResult InitPlatform(nsGALDevice* pDevice) = 0;
  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) = 0;

  nsResult CreateBindingMapping(bool bAllowMultipleBindingPerName);
  void DestroyBindingMapping();

  nsGALShader(const nsGALShaderCreationDescription& Description);
  virtual ~nsGALShader();

protected:
  nsDynamicArray<nsShaderResourceBinding> m_BindingMapping;
};
