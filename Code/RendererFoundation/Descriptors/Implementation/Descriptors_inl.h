

inline nsGALShaderCreationDescription::nsGALShaderCreationDescription()
  : nsHashableStruct()
{
}

inline nsGALShaderCreationDescription::~nsGALShaderCreationDescription()
{
  for (nsUInt32 i = 0; i < nsGALShaderStage::ENUM_COUNT; ++i)
  {
    m_ByteCodes[i] = nullptr;
  }
}

inline bool nsGALShaderCreationDescription::HasByteCodeForStage(nsGALShaderStage::Enum stage) const
{
  return m_ByteCodes[stage] != nullptr && m_ByteCodes[stage]->IsValid();
}

inline void nsGALTextureCreationDescription::SetAsRenderTarget(
  nsUInt32 uiWidth, nsUInt32 uiHeight, nsGALResourceFormat::Enum format, nsGALMSAASampleCount::Enum sampleCount /*= nsGALMSAASampleCount::None*/)
{
  m_uiWidth = uiWidth;
  m_uiHeight = uiHeight;
  m_uiDepth = 1;
  m_uiMipLevelCount = 1;
  m_uiArraySize = 1;
  m_SampleCount = sampleCount;
  m_Format = format;
  m_Type = nsGALTextureType::Texture2D;
  m_bAllowShaderResourceView = true;
  m_bAllowUAV = false;
  m_bCreateRenderTarget = true;
  m_bAllowDynamicMipGeneration = false;
  m_ResourceAccess.m_bReadBack = false;
  m_ResourceAccess.m_bImmutable = true;
  m_pExisitingNativeObject = nullptr;
}

NS_FORCE_INLINE nsGALVertexAttribute::nsGALVertexAttribute(
  nsGALVertexAttributeSemantic::Enum semantic, nsGALResourceFormat::Enum format, nsUInt16 uiOffset, nsUInt8 uiVertexBufferSlot, bool bInstanceData)
  : m_eSemantic(semantic)
  , m_eFormat(format)
  , m_uiOffset(uiOffset)
  , m_uiVertexBufferSlot(uiVertexBufferSlot)
  , m_bInstanceData(bInstanceData)
{
}
