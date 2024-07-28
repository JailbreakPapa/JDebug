
inline nsBitflags<nsGALShaderResourceCategory> nsGALShaderResourceCategory::MakeFromShaderDescriptorType(nsGALShaderResourceType::Enum type)
{
  switch (type)
  {
    case nsGALShaderResourceType::Sampler:
      return nsGALShaderResourceCategory::Sampler;
    case nsGALShaderResourceType::ConstantBuffer:
    case nsGALShaderResourceType::PushConstants:
      return nsGALShaderResourceCategory::ConstantBuffer;
    case nsGALShaderResourceType::Texture:
      return nsGALShaderResourceCategory::TextureSRV;
    case nsGALShaderResourceType::TexelBuffer:
    case nsGALShaderResourceType::StructuredBuffer:
      return nsGALShaderResourceCategory::BufferSRV;
    case nsGALShaderResourceType::TextureRW:
      return nsGALShaderResourceCategory::TextureUAV;
    case nsGALShaderResourceType::TexelBufferRW:
    case nsGALShaderResourceType::StructuredBufferRW:
      return nsGALShaderResourceCategory::BufferUAV;
    case nsGALShaderResourceType::TextureAndSampler:
      return nsGALShaderResourceCategory::TextureSRV | nsGALShaderResourceCategory::Sampler;
    default:
      NS_REPORT_FAILURE("Missing enum");
      return {};
  }
}

inline bool nsGALShaderTextureType::IsArray(nsGALShaderTextureType::Enum format)
{
  switch (format)
  {
    case nsGALShaderTextureType::Texture1DArray:
    case nsGALShaderTextureType::Texture2DArray:
    case nsGALShaderTextureType::Texture2DMSArray:
    case nsGALShaderTextureType::TextureCubeArray:
      return true;
    default:
      return false;
  }
}