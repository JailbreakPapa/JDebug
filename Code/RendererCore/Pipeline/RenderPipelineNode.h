#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>
#include <RendererCore/Declarations.h>

class nsRenderPipelineNode;

struct nsRenderPipelineNodePin
{
  NS_DECLARE_POD_TYPE();

  struct Type
  {
    using StorageType = nsUInt8;

    enum Enum
    {
      Unknown,
      Input,
      Output,
      PassThrough,

      Default = Unknown
    };
  };

  nsEnum<Type> m_Type;
  nsUInt8 m_uiInputIndex = 0xFF;
  nsUInt8 m_uiOutputIndex = 0xFF;
  nsRenderPipelineNode* m_pParent = nullptr;
};

struct nsRenderPipelineNodeInputPin : public nsRenderPipelineNodePin
{
  NS_DECLARE_POD_TYPE();

  NS_ALWAYS_INLINE nsRenderPipelineNodeInputPin() { m_Type = Type::Input; }
};

struct nsRenderPipelineNodeOutputPin : public nsRenderPipelineNodePin
{
  NS_DECLARE_POD_TYPE();

  NS_ALWAYS_INLINE nsRenderPipelineNodeOutputPin() { m_Type = Type::Output; }
};

struct nsRenderPipelineNodePassThrougPin : public nsRenderPipelineNodePin
{
  NS_DECLARE_POD_TYPE();

  NS_ALWAYS_INLINE nsRenderPipelineNodePassThrougPin() { m_Type = Type::PassThrough; }
};

class NS_RENDERERCORE_DLL nsRenderPipelineNode : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsRenderPipelineNode, nsReflectedClass);

public:
  virtual ~nsRenderPipelineNode() = default;

  void InitializePins();

  nsHashedString GetPinName(const nsRenderPipelineNodePin* pPin) const;
  const nsRenderPipelineNodePin* GetPinByName(const char* szName) const;
  const nsRenderPipelineNodePin* GetPinByName(nsHashedString sName) const;
  const nsArrayPtr<const nsRenderPipelineNodePin* const> GetInputPins() const { return m_InputPins; }
  const nsArrayPtr<const nsRenderPipelineNodePin* const> GetOutputPins() const { return m_OutputPins; }

private:
  nsDynamicArray<const nsRenderPipelineNodePin*> m_InputPins;
  nsDynamicArray<const nsRenderPipelineNodePin*> m_OutputPins;
  nsHashTable<nsHashedString, const nsRenderPipelineNodePin*> m_NameToPin;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsRenderPipelineNodePin);
NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsRenderPipelineNodeInputPin);
NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsRenderPipelineNodeOutputPin);
NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsRenderPipelineNodePassThrougPin);
