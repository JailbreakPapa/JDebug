
// static
NS_ALWAYS_INLINE nsUInt32 nsGALResourceFormat::GetBitsPerElement(nsGALResourceFormat::Enum format)
{
  return s_BitsPerElement[format];
}

// static
NS_ALWAYS_INLINE nsUInt8 nsGALResourceFormat::GetChannelCount(nsGALResourceFormat::Enum format)
{
  return s_ChannelCount[format];
}

// static
NS_FORCE_INLINE bool nsGALResourceFormat::IsDepthFormat(nsGALResourceFormat::Enum format)
{
  return format == DFloat || format == D16 || format == D24S8;
}

// static
NS_FORCE_INLINE bool nsGALResourceFormat::IsStencilFormat(Enum format)
{
  return format == D24S8;
}

// static
NS_FORCE_INLINE bool nsGALResourceFormat::IsSrgb(nsGALResourceFormat::Enum format)
{
  return format == BGRAUByteNormalizedsRGB || format == RGBAUByteNormalizedsRGB || format == BC1sRGB || format == BC2sRGB || format == BC3sRGB ||
         format == BC7UNormalizedsRGB;
}

NS_FORCE_INLINE bool nsGALResourceFormat::IsIntegerFormat(Enum format)
{
  switch (format)
  {
    // D16 is actually a 16 bit unorm format
    case nsGALResourceFormat::D16:
    // 32bit, 4 channel
    case nsGALResourceFormat::RGBAUInt:
    case nsGALResourceFormat::RGBAInt:
    // 32bit, 3 channel
    case nsGALResourceFormat::RGBUInt:
    case nsGALResourceFormat::RGBInt:
    // 16bit, 4 channel
    case nsGALResourceFormat::RGBAUShort:
    case nsGALResourceFormat::RGBAShort:
    // 16bit, 2 channel
    case nsGALResourceFormat::RGUInt:
    case nsGALResourceFormat::RGInt:
    // packed 32bit, 4 channel
    case nsGALResourceFormat::RGB10A2UInt:
    // 8bit, 4 channel
    case nsGALResourceFormat::RGBAUByte:
    case nsGALResourceFormat::RGBAByte:
    // 16bit, 2 channel
    case nsGALResourceFormat::RGUShort:
    case nsGALResourceFormat::RGShort:
    // 8bit, 2 channel
    case nsGALResourceFormat::RGUByte:
    case nsGALResourceFormat::RGByte:
    // 32bit, 1 channel
    case nsGALResourceFormat::RUInt:
    case nsGALResourceFormat::RInt:
    // 16bit, 1 channel
    case nsGALResourceFormat::RUShort:
    case nsGALResourceFormat::RShort:
    // 8bit, 1 channel
    case nsGALResourceFormat::RUByte:
    case nsGALResourceFormat::RByte:
      return true;
    default:
      return false;
  }
}

NS_FORCE_INLINE bool nsGALResourceFormat::IsSignedFormat(Enum format)
{
  switch (format)
  {
    case RGBAFloat:
    case RGBAInt:
    case RGBFloat:
    case RGBInt:
    case RGBAHalf:
    case RGBAShort:
    case RGBAShortNormalized:
    case RGFloat:
    case RGInt:
    case RGB10A2UIntNormalized:
    case RG11B10Float:
    case RGBAByteNormalized:
    case RGBAByte:
    case RGHalf:
    case RGShort:
    case RGShortNormalized:
    case RGByte:
    case RGByteNormalized:
    case RFloat:
    case RInt:
    case RHalf:
    case RShort:
    case RShortNormalized:
    case RByte:
    case RByteNormalized:
    case BC4Normalized:
    case BC5Normalized:
    case BC6Float:
      return true;
    default:
      return false;
  }
}

template <typename NativeFormatType, NativeFormatType InvalidFormat>
nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>::nsGALFormatLookupEntry()
  : m_eStorage(InvalidFormat)
  , m_eRenderTarget(InvalidFormat)
  , m_eDepthOnlyType(InvalidFormat)
  , m_eStencilOnlyType(InvalidFormat)
  , m_eDepthStencilType(InvalidFormat)
  , m_eVertexAttributeType(InvalidFormat)
  , m_eResourceViewType(InvalidFormat)
{
}


template <typename NativeFormatType, NativeFormatType InvalidFormat>
nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>::nsGALFormatLookupEntry(NativeFormatType storage)
  : m_eStorage(storage)
  , m_eRenderTarget(InvalidFormat)
  , m_eDepthOnlyType(InvalidFormat)
  , m_eStencilOnlyType(InvalidFormat)
  , m_eDepthStencilType(InvalidFormat)
  , m_eVertexAttributeType(InvalidFormat)
  , m_eResourceViewType(InvalidFormat)
{
}

template <typename NativeFormatType, NativeFormatType InvalidFormat>
nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>& nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>::RT(
  NativeFormatType renderTargetType)
{
  m_eRenderTarget = renderTargetType;
  return *this;
}

template <typename NativeFormatType, NativeFormatType InvalidFormat>
nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>& nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>::D(NativeFormatType depthOnlyType)
{
  m_eDepthOnlyType = depthOnlyType;
  return *this;
}

template <typename NativeFormatType, NativeFormatType InvalidFormat>
nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>& nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>::S(NativeFormatType stencilOnlyType)
{
  m_eStencilOnlyType = stencilOnlyType;
  return *this;
}

template <typename NativeFormatType, NativeFormatType InvalidFormat>
nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>& nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>::DS(
  NativeFormatType depthStencilType)
{
  m_eDepthStencilType = depthStencilType;
  return *this;
}

template <typename NativeFormatType, NativeFormatType InvalidFormat>
nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>& nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>::VA(
  NativeFormatType vertexAttributeType)
{
  m_eVertexAttributeType = vertexAttributeType;
  return *this;
}

template <typename NativeFormatType, NativeFormatType InvalidFormat>
nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>& nsGALFormatLookupEntry<NativeFormatType, InvalidFormat>::RV(
  NativeFormatType resourceViewType)
{
  m_eResourceViewType = resourceViewType;
  return *this;
}


template <typename FormatClass>
nsGALFormatLookupTable<FormatClass>::nsGALFormatLookupTable()
{
  for (nsUInt32 i = 0; i < nsGALResourceFormat::ENUM_COUNT; i++)
  {
    m_Formats[i] = FormatClass();
  }
}

template <typename FormatClass>
const FormatClass& nsGALFormatLookupTable<FormatClass>::GetFormatInfo(nsGALResourceFormat::Enum format) const
{
  return m_Formats[format];
}

template <typename FormatClass>
void nsGALFormatLookupTable<FormatClass>::SetFormatInfo(nsGALResourceFormat::Enum format, const FormatClass& newFormatInfo)
{
  m_Formats[format] = newFormatInfo;
}
