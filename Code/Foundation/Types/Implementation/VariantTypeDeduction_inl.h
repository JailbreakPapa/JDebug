


/// \cond

template <>
struct wdVariantTypeDeduction<bool>
{
  enum
  {
    value = wdVariantType::Bool,
    forceSharing = false,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = bool;
  using ReturnType = bool;
};

template <>
struct wdVariantTypeDeduction<wdInt8>
{
  enum
  {
    value = wdVariantType::Int8,
    forceSharing = false,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdInt8;
};

template <>
struct wdVariantTypeDeduction<wdUInt8>
{
  enum
  {
    value = wdVariantType::UInt8,
    forceSharing = false,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdUInt8;
};

template <>
struct wdVariantTypeDeduction<wdInt16>
{
  enum
  {
    value = wdVariantType::Int16,
    forceSharing = false,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdInt16;
};

template <>
struct wdVariantTypeDeduction<wdUInt16>
{
  enum
  {
    value = wdVariantType::UInt16,
    forceSharing = false,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdUInt16;
};

template <>
struct wdVariantTypeDeduction<wdInt32>
{
  enum
  {
    value = wdVariantType::Int32,
    forceSharing = false,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdInt32;
};

template <>
struct wdVariantTypeDeduction<wdUInt32>
{
  enum
  {
    value = wdVariantType::UInt32,
    forceSharing = false,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdUInt32;
};

template <>
struct wdVariantTypeDeduction<wdInt64>
{
  enum
  {
    value = wdVariantType::Int64,
    forceSharing = false,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdInt64;
};

template <>
struct wdVariantTypeDeduction<wdUInt64>
{
  enum
  {
    value = wdVariantType::UInt64,
    forceSharing = false,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdUInt64;
};

template <>
struct wdVariantTypeDeduction<float>
{
  enum
  {
    value = wdVariantType::Float,
    forceSharing = false,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = float;
};

template <>
struct wdVariantTypeDeduction<double>
{
  enum
  {
    value = wdVariantType::Double,
    forceSharing = false,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = double;
};

template <>
struct wdVariantTypeDeduction<wdColor>
{
  enum
  {
    value = wdVariantType::Color,
    forceSharing = false,
    hasReflectedMembers = true,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdColor;
};

template <>
struct wdVariantTypeDeduction<wdColorGammaUB>
{
  enum
  {
    value = wdVariantType::ColorGamma,
    forceSharing = false,
    hasReflectedMembers = true,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdColorGammaUB;
};

template <>
struct wdVariantTypeDeduction<wdVec2>
{
  enum
  {
    value = wdVariantType::Vector2,
    forceSharing = false,
    hasReflectedMembers = true,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdVec2;
};

template <>
struct wdVariantTypeDeduction<wdVec3>
{
  enum
  {
    value = wdVariantType::Vector3,
    forceSharing = false,
    hasReflectedMembers = true,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdVec3;
};

template <>
struct wdVariantTypeDeduction<wdVec4>
{
  enum
  {
    value = wdVariantType::Vector4,
    forceSharing = false,
    hasReflectedMembers = true,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdVec4;
};

template <>
struct wdVariantTypeDeduction<wdVec2I32>
{
  enum
  {
    value = wdVariantType::Vector2I,
    forceSharing = false,
    hasReflectedMembers = true,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdVec2I32;
};

template <>
struct wdVariantTypeDeduction<wdVec3I32>
{
  enum
  {
    value = wdVariantType::Vector3I,
    forceSharing = false,
    hasReflectedMembers = true,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdVec3I32;
};

template <>
struct wdVariantTypeDeduction<wdVec4I32>
{
  enum
  {
    value = wdVariantType::Vector4I,
    forceSharing = false,
    hasReflectedMembers = true,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdVec4I32;
};

template <>
struct wdVariantTypeDeduction<wdVec2U32>
{
  enum
  {
    value = wdVariantType::Vector2U,
    forceSharing = false,
    hasReflectedMembers = true,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdVec2U32;
};

template <>
struct wdVariantTypeDeduction<wdVec3U32>
{
  enum
  {
    value = wdVariantType::Vector3U,
    forceSharing = false,
    hasReflectedMembers = true,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdVec3U32;
};

template <>
struct wdVariantTypeDeduction<wdVec4U32>
{
  enum
  {
    value = wdVariantType::Vector4U,
    forceSharing = false,
    hasReflectedMembers = true,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdVec4U32;
};

template <>
struct wdVariantTypeDeduction<wdQuat>
{
  enum
  {
    value = wdVariantType::Quaternion,
    forceSharing = false,
    hasReflectedMembers = true,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdQuat;
};

template <>
struct wdVariantTypeDeduction<wdMat3>
{
  enum
  {
    value = wdVariantType::Matrix3,
    forceSharing = false,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdMat3;
};

template <>
struct wdVariantTypeDeduction<wdMat4>
{
  enum
  {
    value = wdVariantType::Matrix4,
    forceSharing = false,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdMat4;
};

template <>
struct wdVariantTypeDeduction<wdTransform>
{
  enum
  {
    value = wdVariantType::Transform,
    forceSharing = false,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdTransform;
};

template <>
struct wdVariantTypeDeduction<wdString>
{
  enum
  {
    value = wdVariantType::String,
    forceSharing = true,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdString;
};

template <>
struct wdVariantTypeDeduction<wdUntrackedString>
{
  enum
  {
    value = wdVariantType::String,
    forceSharing = true,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdString;
};

template <>
struct wdVariantTypeDeduction<wdStringView>
{
  enum
  {
    value = wdVariantType::StringView,
    forceSharing = false,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdStringView;
};

template <>
struct wdVariantTypeDeduction<wdDataBuffer>
{
  enum
  {
    value = wdVariantType::DataBuffer,
    forceSharing = true,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdDataBuffer;
};

template <>
struct wdVariantTypeDeduction<char*>
{
  enum
  {
    value = wdVariantType::String,
    forceSharing = true,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdString;
};

template <>
struct wdVariantTypeDeduction<const char*>
{
  enum
  {
    value = wdVariantType::String,
    forceSharing = true,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdString;
};

template <size_t N>
struct wdVariantTypeDeduction<char[N]>
{
  enum
  {
    value = wdVariantType::String,
    forceSharing = true,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdString;
};

template <size_t N>
struct wdVariantTypeDeduction<const char[N]>
{
  enum
  {
    value = wdVariantType::String,
    forceSharing = true,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdString;
};

template <>
struct wdVariantTypeDeduction<wdTime>
{
  enum
  {
    value = wdVariantType::Time,
    forceSharing = false,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdTime;
};

template <>
struct wdVariantTypeDeduction<wdUuid>
{
  enum
  {
    value = wdVariantType::Uuid,
    forceSharing = false,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdUuid;
};

template <>
struct wdVariantTypeDeduction<wdAngle>
{
  enum
  {
    value = wdVariantType::Angle,
    forceSharing = false,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdAngle;
};

template <>
struct wdVariantTypeDeduction<wdVariantArray>
{
  enum
  {
    value = wdVariantType::VariantArray,
    forceSharing = true,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdVariantArray;
};

template <>
struct wdVariantTypeDeduction<wdArrayPtr<wdVariant>>
{
  enum
  {
    value = wdVariantType::VariantArray,
    forceSharing = true,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdVariantArray;
};


template <>
struct wdVariantTypeDeduction<wdVariantDictionary>
{
  enum
  {
    value = wdVariantType::VariantDictionary,
    forceSharing = true,
    hasReflectedMembers = false,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdVariantDictionary;
};

namespace wdInternal
{
  template <int v>
  struct PointerDeductionHelper
  {
  };

  template <>
  struct PointerDeductionHelper<0>
  {
    using StorageType = void*;
  };

  template <>
  struct PointerDeductionHelper<1>
  {
    using StorageType = wdReflectedClass*;
  };
} // namespace wdInternal

template <>
struct wdVariantTypeDeduction<wdTypedPointer>
{
  enum
  {
    value = wdVariantType::TypedPointer,
    forceSharing = false,
    hasReflectedMembers = true,
    classification = wdVariantClass::DirectCast
  };

  using StorageType = wdTypedPointer;
};

template <typename T>
struct wdVariantTypeDeduction<T*>
{
  enum
  {
    value = wdVariantType::TypedPointer,
    forceSharing = false,
    hasReflectedMembers = true,
    classification = wdVariantClass::PointerCast
  };

  using StorageType = wdTypedPointer;
};

template <>
struct wdVariantTypeDeduction<wdTypedObject>
{
  enum
  {
    value = wdVariantType::TypedObject,
    forceSharing = false,
    hasReflectedMembers = true,
    classification = wdVariantClass::TypedObject
  };

  using StorageType = wdTypedObject;
};

/// \endcond
