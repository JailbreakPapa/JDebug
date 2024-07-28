
#pragma once

#include <RendererFoundation/RendererFoundationDLL.h>

struct NS_RENDERERFOUNDATION_DLL nsGALResourceFormat
{
  using StorageType = nsUInt8;

  enum Enum : nsUInt8
  {
    Invalid = 0,

    RGBAFloat,
    XYZWFloat = RGBAFloat,
    RGBAUInt,
    RGBAInt,

    RGBFloat,
    XYZFloat = RGBFloat,
    UVWFloat = RGBFloat,
    RGBUInt,
    RGBInt,

    B5G6R5UNormalized,
    BGRAUByteNormalized,
    BGRAUByteNormalizedsRGB,

    RGBAHalf,
    XYZWHalf = RGBAHalf,
    RGBAUShort,
    RGBAUShortNormalized,
    RGBAShort,
    RGBAShortNormalized,

    RGFloat,
    XYFloat = RGFloat,
    UVFloat = RGFloat,
    RGUInt,
    RGInt,

    RGB10A2UInt,
    RGB10A2UIntNormalized,
    RG11B10Float,

    RGBAUByteNormalized,
    RGBAUByteNormalizedsRGB,
    RGBAUByte,
    RGBAByteNormalized,
    RGBAByte,

    RGHalf,
    XYHalf = RGHalf,
    UVHalf = RGHalf,
    RGUShort,
    RGUShortNormalized,
    RGShort,
    RGShortNormalized,
    RGUByte,
    RGUByteNormalized,
    RGByte,
    RGByteNormalized,

    DFloat,

    RFloat,
    RUInt,
    RInt,
    RHalf,
    RUShort,
    RUShortNormalized,
    RShort,
    RShortNormalized,
    RUByte,
    RUByteNormalized,
    RByte,
    RByteNormalized,

    AUByteNormalized,

    D16,
    D24S8,

    BC1,
    BC1sRGB,
    BC2,
    BC2sRGB,
    BC3,
    BC3sRGB,
    BC4UNormalized,
    BC4Normalized,
    BC5UNormalized,
    BC5Normalized,
    BC6UFloat,
    BC6Float,
    BC7UNormalized,
    BC7UNormalizedsRGB,

    ENUM_COUNT,

    Default = RGBAUByteNormalizedsRGB
  };


  // General format Meta-Informations:

  /// \brief The size in bits per element (usually pixels, except for mesh stream elements) of a single element of the given resource format.
  static nsUInt32 GetBitsPerElement(nsGALResourceFormat::Enum format);

  /// \brief The number of color channels this format contains.
  static nsUInt8 GetChannelCount(nsGALResourceFormat::Enum format);

  /// \todo A combination of propertyflags, something like srgb, normalized, ...
  // Would be very useful for some GL stuff and Testing.

  /// \brief Returns whether the given resource format is a depth format
  static bool IsDepthFormat(nsGALResourceFormat::Enum format);
  static bool IsStencilFormat(nsGALResourceFormat::Enum format);

  static bool IsSrgb(nsGALResourceFormat::Enum format);

  /// \brief Returns whether the given resource format returns integer values when sampled (e.g. RUShort). Note that normalized formats like RGUShortNormalized are not considered integer formats as they return float values in the [0..1] range when sampled.
  static bool IsIntegerFormat(nsGALResourceFormat::Enum format);

  /// \brief Returns whether the given resource format can store negative values.
  static bool IsSignedFormat(nsGALResourceFormat::Enum format);

private:
  static const nsUInt8 s_BitsPerElement[nsGALResourceFormat::ENUM_COUNT];

  static const nsUInt8 s_ChannelCount[nsGALResourceFormat::ENUM_COUNT];
};

template <typename NativeFormatType, NativeFormatType InvalidFormat>
class nsGALFormatLookupEntry
{
public:
  inline nsGALFormatLookupEntry();

  inline nsGALFormatLookupEntry(NativeFormatType storage);

  inline nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>& RT(NativeFormatType renderTargetType);

  inline nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>& D(NativeFormatType depthOnlyType);

  inline nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>& S(NativeFormatType stencilOnlyType);

  inline nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>& DS(NativeFormatType depthStencilType);

  inline nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>& VA(NativeFormatType vertexAttributeType);

  inline nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>& RV(NativeFormatType resourceViewType);

  NativeFormatType m_eStorage;
  NativeFormatType m_eRenderTarget;
  NativeFormatType m_eDepthOnlyType;
  NativeFormatType m_eStencilOnlyType;
  NativeFormatType m_eDepthStencilType;
  NativeFormatType m_eVertexAttributeType;
  NativeFormatType m_eResourceViewType;
};

// Reusable table class to store lookup information (from nsGALResourceFormat to the various formats for texture/buffer storage, views)
template <typename FormatClass>
class nsGALFormatLookupTable
{
public:
  nsGALFormatLookupTable();

  NS_ALWAYS_INLINE const FormatClass& GetFormatInfo(nsGALResourceFormat::Enum format) const;

  NS_ALWAYS_INLINE void SetFormatInfo(nsGALResourceFormat::Enum format, const FormatClass& newFormatInfo);

private:
  FormatClass m_Formats[nsGALResourceFormat::ENUM_COUNT];
};

#include <RendererFoundation/Resources/Implementation/ResourceFormats_inl.h>
