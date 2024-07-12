#include <Texture/TexturePCH.h>

#include <Texture/Image/Formats/ImageFormatMappings.h>

#define MAKE_FOURCC(a, b, c, d) (a) | ((b) << 8) | ((c) << 16) | ((d) << 24)

using DXGI_FORMAT = enum DXGI_FORMAT {
  DXGI_FORMAT_UNKNOWN = 0,
  DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
  DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
  DXGI_FORMAT_R32G32B32A32_UINT = 3,
  DXGI_FORMAT_R32G32B32A32_SINT = 4,
  DXGI_FORMAT_R32G32B32_TYPELESS = 5,
  DXGI_FORMAT_R32G32B32_FLOAT = 6,
  DXGI_FORMAT_R32G32B32_UINT = 7,
  DXGI_FORMAT_R32G32B32_SINT = 8,
  DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
  DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
  DXGI_FORMAT_R16G16B16A16_UNORM = 11,
  DXGI_FORMAT_R16G16B16A16_UINT = 12,
  DXGI_FORMAT_R16G16B16A16_SNORM = 13,
  DXGI_FORMAT_R16G16B16A16_SINT = 14,
  DXGI_FORMAT_R32G32_TYPELESS = 15,
  DXGI_FORMAT_R32G32_FLOAT = 16,
  DXGI_FORMAT_R32G32_UINT = 17,
  DXGI_FORMAT_R32G32_SINT = 18,
  DXGI_FORMAT_R32G8X24_TYPELESS = 19,
  DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
  DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
  DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
  DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
  DXGI_FORMAT_R10G10B10A2_UNORM = 24,
  DXGI_FORMAT_R10G10B10A2_UINT = 25,
  DXGI_FORMAT_R11G11B10_FLOAT = 26,
  DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
  DXGI_FORMAT_R8G8B8A8_UNORM = 28,
  DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
  DXGI_FORMAT_R8G8B8A8_UINT = 30,
  DXGI_FORMAT_R8G8B8A8_SNORM = 31,
  DXGI_FORMAT_R8G8B8A8_SINT = 32,
  DXGI_FORMAT_R16G16_TYPELESS = 33,
  DXGI_FORMAT_R16G16_FLOAT = 34,
  DXGI_FORMAT_R16G16_UNORM = 35,
  DXGI_FORMAT_R16G16_UINT = 36,
  DXGI_FORMAT_R16G16_SNORM = 37,
  DXGI_FORMAT_R16G16_SINT = 38,
  DXGI_FORMAT_R32_TYPELESS = 39,
  DXGI_FORMAT_D32_FLOAT = 40,
  DXGI_FORMAT_R32_FLOAT = 41,
  DXGI_FORMAT_R32_UINT = 42,
  DXGI_FORMAT_R32_SINT = 43,
  DXGI_FORMAT_R24G8_TYPELESS = 44,
  DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
  DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
  DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
  DXGI_FORMAT_R8G8_TYPELESS = 48,
  DXGI_FORMAT_R8G8_UNORM = 49,
  DXGI_FORMAT_R8G8_UINT = 50,
  DXGI_FORMAT_R8G8_SNORM = 51,
  DXGI_FORMAT_R8G8_SINT = 52,
  DXGI_FORMAT_R16_TYPELESS = 53,
  DXGI_FORMAT_R16_FLOAT = 54,
  DXGI_FORMAT_D16_UNORM = 55,
  DXGI_FORMAT_R16_UNORM = 56,
  DXGI_FORMAT_R16_UINT = 57,
  DXGI_FORMAT_R16_SNORM = 58,
  DXGI_FORMAT_R16_SINT = 59,
  DXGI_FORMAT_R8_TYPELESS = 60,
  DXGI_FORMAT_R8_UNORM = 61,
  DXGI_FORMAT_R8_UINT = 62,
  DXGI_FORMAT_R8_SNORM = 63,
  DXGI_FORMAT_R8_SINT = 64,
  DXGI_FORMAT_A8_UNORM = 65,
  DXGI_FORMAT_R1_UNORM = 66,
  DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
  DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
  DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
  DXGI_FORMAT_BC1_TYPELESS = 70,
  DXGI_FORMAT_BC1_UNORM = 71,
  DXGI_FORMAT_BC1_UNORM_SRGB = 72,
  DXGI_FORMAT_BC2_TYPELESS = 73,
  DXGI_FORMAT_BC2_UNORM = 74,
  DXGI_FORMAT_BC2_UNORM_SRGB = 75,
  DXGI_FORMAT_BC3_TYPELESS = 76,
  DXGI_FORMAT_BC3_UNORM = 77,
  DXGI_FORMAT_BC3_UNORM_SRGB = 78,
  DXGI_FORMAT_BC4_TYPELESS = 79,
  DXGI_FORMAT_BC4_UNORM = 80,
  DXGI_FORMAT_BC4_SNORM = 81,
  DXGI_FORMAT_BC5_TYPELESS = 82,
  DXGI_FORMAT_BC5_UNORM = 83,
  DXGI_FORMAT_BC5_SNORM = 84,
  DXGI_FORMAT_B5G6R5_UNORM = 85,
  DXGI_FORMAT_B5G5R5A1_UNORM = 86,
  DXGI_FORMAT_B8G8R8A8_UNORM = 87,
  DXGI_FORMAT_B8G8R8X8_UNORM = 88,
  DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
  DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
  DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
  DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
  DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
  DXGI_FORMAT_BC6H_TYPELESS = 94,
  DXGI_FORMAT_BC6H_UF16 = 95,
  DXGI_FORMAT_BC6H_SF16 = 96,
  DXGI_FORMAT_BC7_TYPELESS = 97,
  DXGI_FORMAT_BC7_UNORM = 98,
  DXGI_FORMAT_BC7_UNORM_SRGB = 99,
  DXGI_FORMAT_AYUV = 100,
  DXGI_FORMAT_Y410 = 101,
  DXGI_FORMAT_Y416 = 102,
  DXGI_FORMAT_NV12 = 103,
  DXGI_FORMAT_P010 = 104,
  DXGI_FORMAT_P016 = 105,
  DXGI_FORMAT_420_OPAQUE = 106,
  DXGI_FORMAT_YUY2 = 107,
  DXGI_FORMAT_Y210 = 108,
  DXGI_FORMAT_Y216 = 109,
  DXGI_FORMAT_NV11 = 110,
  DXGI_FORMAT_AI44 = 111,
  DXGI_FORMAT_IA44 = 112,
  DXGI_FORMAT_P8 = 113,
  DXGI_FORMAT_A8P8 = 114,
  DXGI_FORMAT_B4G4R4A4_UNORM = 115,
  DXGI_FORMAT_FORCE_UINT = 0xffffffffUL
};

nsUInt32 nsImageFormatMappings::ToDxgiFormat(nsImageFormat::Enum format)
{

#define CASE_NS2DXGI(ns)  \
  case nsImageFormat::ns: \
    return DXGI_FORMAT_##ns
  switch (format)
  {
    default:
      return DXGI_FORMAT_UNKNOWN;

      CASE_NS2DXGI(R32G32B32A32_FLOAT);
      CASE_NS2DXGI(R32G32B32A32_UINT);
      CASE_NS2DXGI(R32G32B32A32_SINT);
      CASE_NS2DXGI(R32G32B32_FLOAT);
      CASE_NS2DXGI(R32G32B32_UINT);
      CASE_NS2DXGI(R32G32B32_SINT);
      CASE_NS2DXGI(R16G16B16A16_FLOAT);
      CASE_NS2DXGI(R16G16B16A16_UNORM);
      CASE_NS2DXGI(R16G16B16A16_UINT);
      CASE_NS2DXGI(R16G16B16A16_SNORM);
      CASE_NS2DXGI(R16G16B16A16_SINT);
      CASE_NS2DXGI(R32G32_FLOAT);
      CASE_NS2DXGI(R32G32_UINT);
      CASE_NS2DXGI(R32G32_SINT);
      CASE_NS2DXGI(D32_FLOAT_S8X24_UINT);
      CASE_NS2DXGI(R10G10B10A2_UNORM);
      CASE_NS2DXGI(R10G10B10A2_UINT);
      CASE_NS2DXGI(R11G11B10_FLOAT);
      CASE_NS2DXGI(R8G8B8A8_UNORM);
      CASE_NS2DXGI(R8G8B8A8_UNORM_SRGB);
      CASE_NS2DXGI(R8G8B8A8_UINT);
      CASE_NS2DXGI(R8G8B8A8_SNORM);
      CASE_NS2DXGI(R8G8B8A8_SINT);
      CASE_NS2DXGI(R16G16_FLOAT);
      CASE_NS2DXGI(R16G16_UNORM);
      CASE_NS2DXGI(R16G16_UINT);
      CASE_NS2DXGI(R16G16_SNORM);
      CASE_NS2DXGI(R16G16_SINT);
      CASE_NS2DXGI(D32_FLOAT);
      CASE_NS2DXGI(R32_FLOAT);
      CASE_NS2DXGI(R32_UINT);
      CASE_NS2DXGI(R32_SINT);
      CASE_NS2DXGI(D24_UNORM_S8_UINT);
      CASE_NS2DXGI(R8G8_UNORM);
      CASE_NS2DXGI(R8G8_UINT);
      CASE_NS2DXGI(R8G8_SNORM);
      CASE_NS2DXGI(R8G8_SINT);
      CASE_NS2DXGI(R16_FLOAT);
      CASE_NS2DXGI(D16_UNORM);
      CASE_NS2DXGI(R16_UNORM);
      CASE_NS2DXGI(R16_UINT);
      CASE_NS2DXGI(R16_SNORM);
      CASE_NS2DXGI(R16_SINT);
      CASE_NS2DXGI(R8_UNORM);
      CASE_NS2DXGI(R8_UINT);
      CASE_NS2DXGI(R8_SNORM);
      CASE_NS2DXGI(R8_SINT);
      CASE_NS2DXGI(BC1_UNORM);
      CASE_NS2DXGI(BC1_UNORM_SRGB);
      CASE_NS2DXGI(BC2_UNORM);
      CASE_NS2DXGI(BC2_UNORM_SRGB);
      CASE_NS2DXGI(BC3_UNORM);
      CASE_NS2DXGI(BC3_UNORM_SRGB);
      CASE_NS2DXGI(BC4_UNORM);
      CASE_NS2DXGI(BC4_SNORM);
      CASE_NS2DXGI(BC5_UNORM);
      CASE_NS2DXGI(BC5_SNORM);
      CASE_NS2DXGI(B5G6R5_UNORM);
      CASE_NS2DXGI(B5G5R5A1_UNORM);
      CASE_NS2DXGI(B8G8R8A8_UNORM);
      CASE_NS2DXGI(B8G8R8X8_UNORM);
      CASE_NS2DXGI(B8G8R8A8_UNORM_SRGB);
      CASE_NS2DXGI(B8G8R8X8_UNORM_SRGB);
      CASE_NS2DXGI(BC6H_UF16);
      CASE_NS2DXGI(BC6H_SF16);
      CASE_NS2DXGI(BC7_UNORM);
      CASE_NS2DXGI(BC7_UNORM_SRGB);
      CASE_NS2DXGI(B4G4R4A4_UNORM);
      CASE_NS2DXGI(NV12);
  }
}

nsImageFormat::Enum nsImageFormatMappings::FromDxgiFormat(nsUInt32 uiDxgiFormat)
{
#define CASE_DXGI2NS(ns) \
  case DXGI_FORMAT_##ns: \
    return nsImageFormat::ns
  switch (uiDxgiFormat)
  {
    default:
      return nsImageFormat::UNKNOWN;

      CASE_DXGI2NS(R32G32B32A32_FLOAT);
      CASE_DXGI2NS(R32G32B32A32_UINT);
      CASE_DXGI2NS(R32G32B32A32_SINT);
      CASE_DXGI2NS(R32G32B32_FLOAT);
      CASE_DXGI2NS(R32G32B32_UINT);
      CASE_DXGI2NS(R32G32B32_SINT);
      CASE_DXGI2NS(R16G16B16A16_FLOAT);
      CASE_DXGI2NS(R16G16B16A16_UNORM);
      CASE_DXGI2NS(R16G16B16A16_UINT);
      CASE_DXGI2NS(R16G16B16A16_SNORM);
      CASE_DXGI2NS(R16G16B16A16_SINT);
      CASE_DXGI2NS(R32G32_FLOAT);
      CASE_DXGI2NS(R32G32_UINT);
      CASE_DXGI2NS(R32G32_SINT);
      CASE_DXGI2NS(D32_FLOAT_S8X24_UINT);
      CASE_DXGI2NS(R10G10B10A2_UNORM);
      CASE_DXGI2NS(R10G10B10A2_UINT);
      CASE_DXGI2NS(R11G11B10_FLOAT);
      CASE_DXGI2NS(R8G8B8A8_UNORM);
      CASE_DXGI2NS(R8G8B8A8_UNORM_SRGB);
      CASE_DXGI2NS(R8G8B8A8_UINT);
      CASE_DXGI2NS(R8G8B8A8_SNORM);
      CASE_DXGI2NS(R8G8B8A8_SINT);
      CASE_DXGI2NS(R16G16_FLOAT);
      CASE_DXGI2NS(R16G16_UNORM);
      CASE_DXGI2NS(R16G16_UINT);
      CASE_DXGI2NS(R16G16_SNORM);
      CASE_DXGI2NS(R16G16_SINT);
      CASE_DXGI2NS(D32_FLOAT);
      CASE_DXGI2NS(R32_FLOAT);
      CASE_DXGI2NS(R32_UINT);
      CASE_DXGI2NS(R32_SINT);
      CASE_DXGI2NS(D24_UNORM_S8_UINT);
      CASE_DXGI2NS(R8G8_UNORM);
      CASE_DXGI2NS(R8G8_UINT);
      CASE_DXGI2NS(R8G8_SNORM);
      CASE_DXGI2NS(R8G8_SINT);
      CASE_DXGI2NS(R16_FLOAT);
      CASE_DXGI2NS(D16_UNORM);
      CASE_DXGI2NS(R16_UNORM);
      CASE_DXGI2NS(R16_UINT);
      CASE_DXGI2NS(R16_SNORM);
      CASE_DXGI2NS(R16_SINT);
      CASE_DXGI2NS(R8_UNORM);
      CASE_DXGI2NS(R8_UINT);
      CASE_DXGI2NS(R8_SNORM);
      CASE_DXGI2NS(R8_SINT);
      CASE_DXGI2NS(BC1_UNORM);
      CASE_DXGI2NS(BC1_UNORM_SRGB);
      CASE_DXGI2NS(BC2_UNORM);
      CASE_DXGI2NS(BC2_UNORM_SRGB);
      CASE_DXGI2NS(BC3_UNORM);
      CASE_DXGI2NS(BC3_UNORM_SRGB);
      CASE_DXGI2NS(BC4_UNORM);
      CASE_DXGI2NS(BC4_SNORM);
      CASE_DXGI2NS(BC5_UNORM);
      CASE_DXGI2NS(BC5_SNORM);
      CASE_DXGI2NS(B5G6R5_UNORM);
      CASE_DXGI2NS(B5G5R5A1_UNORM);
      CASE_DXGI2NS(B8G8R8A8_UNORM);
      CASE_DXGI2NS(B8G8R8X8_UNORM);
      CASE_DXGI2NS(B8G8R8A8_UNORM_SRGB);
      CASE_DXGI2NS(B8G8R8X8_UNORM_SRGB);
      CASE_DXGI2NS(BC6H_UF16);
      CASE_DXGI2NS(BC6H_SF16);
      CASE_DXGI2NS(BC7_UNORM);
      CASE_DXGI2NS(BC7_UNORM_SRGB);
      CASE_DXGI2NS(B4G4R4A4_UNORM);
      CASE_DXGI2NS(NV12);
  }
}

nsUInt32 nsImageFormatMappings::ToFourCc(nsImageFormat::Enum format)
{
  switch (format)
  {
    case nsImageFormat::BC1_UNORM:
      return MAKE_FOURCC('D', 'X', 'T', '1');

    case nsImageFormat::BC2_UNORM:
      return MAKE_FOURCC('D', 'X', 'T', '3');

    case nsImageFormat::BC3_UNORM:
      return MAKE_FOURCC('D', 'X', 'T', '5');

    case nsImageFormat::BC4_UNORM:
      return MAKE_FOURCC('A', 'T', 'I', '1');

    case nsImageFormat::BC5_UNORM:
      return MAKE_FOURCC('A', 'T', 'I', '2');

    default:
      return 0;
  }
}

nsImageFormat::Enum nsImageFormatMappings::FromFourCc(nsUInt32 uiFourCc)
{
  switch (uiFourCc)
  {
    case MAKE_FOURCC('D', 'X', 'T', '1'):
      return nsImageFormat::BC1_UNORM;

    case MAKE_FOURCC('D', 'X', 'T', '2'):
    case MAKE_FOURCC('D', 'X', 'T', '3'):
      return nsImageFormat::BC2_UNORM;

    case MAKE_FOURCC('D', 'X', 'T', '4'):
    case MAKE_FOURCC('D', 'X', 'T', '5'):
      return nsImageFormat::BC3_UNORM;

    case MAKE_FOURCC('A', 'T', 'I', '1'):
    case MAKE_FOURCC('B', 'C', '4', 'U'):
      return nsImageFormat::BC4_UNORM;

    case MAKE_FOURCC('A', 'T', 'I', '2'):
    case MAKE_FOURCC('B', 'C', '5', 'U'):
      return nsImageFormat::BC5_UNORM;

    // old legacy DirectX formats
    case 116: // D3DFMT_A32B32G32R32F
      return nsImageFormat::R32G32B32A32_FLOAT;

    case 115: // D3DFMT_G32R32F
      return nsImageFormat::R32G32_FLOAT;

    case 114: // D3DFMT_R32F
      return nsImageFormat::R32_FLOAT;

    case 113: // D3DFMT_A16B16G16R16F
      return nsImageFormat::R16G16B16A16_FLOAT;

    case 112: // D3DFMT_G16R16F
      return nsImageFormat::R16G16_FLOAT;

    case 111: // D3DFMT_R16F
      return nsImageFormat::R16_FLOAT;

    case 110: // D3DFMT_Q16W16V16U16
      return nsImageFormat::R16G16B16A16_SNORM;

    case 36:  // D3DFMT_A16B16G16R16
      return nsImageFormat::R16G16B16A16_UNORM;

    default:
      return nsImageFormat::UNKNOWN;
  }
}
