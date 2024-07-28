#include <RendererFoundation/RendererFoundationPCH.h>

#include <RendererFoundation/RendererFoundationDLL.h>

const nsUInt8 nsGALIndexType::s_Size[nsGALIndexType::ENUM_COUNT] = {
  0,               // None
  sizeof(nsInt16), // UShort
  sizeof(nsInt32)  // UInt
};

const char* nsGALShaderStage::Names[ENUM_COUNT] = {
  "VertexShader",
  "HullShader",
  "DomainShader",
  "GeometryShader",
  "PixelShader",
  "ComputeShader",
};
