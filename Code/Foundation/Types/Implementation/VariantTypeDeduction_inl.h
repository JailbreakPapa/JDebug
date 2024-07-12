


/// \cond

template <>
struct nsVariantTypeDeduction<bool>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Bool;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = bool;
  using ReturnType = bool;
};

template <>
struct nsVariantTypeDeduction<nsInt8>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Int8;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsInt8;
};

template <>
struct nsVariantTypeDeduction<nsUInt8>
{
  static constexpr nsVariantType::Enum value = nsVariantType::UInt8;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsUInt8;
};

template <>
struct nsVariantTypeDeduction<nsInt16>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Int16;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsInt16;
};

template <>
struct nsVariantTypeDeduction<nsUInt16>
{
  static constexpr nsVariantType::Enum value = nsVariantType::UInt16;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsUInt16;
};

template <>
struct nsVariantTypeDeduction<nsInt32>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Int32;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsInt32;
};

template <>
struct nsVariantTypeDeduction<nsUInt32>
{
  static constexpr nsVariantType::Enum value = nsVariantType::UInt32;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsUInt32;
};

template <>
struct nsVariantTypeDeduction<nsInt64>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Int64;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsInt64;
};

template <>
struct nsVariantTypeDeduction<nsUInt64>
{
  static constexpr nsVariantType::Enum value = nsVariantType::UInt64;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsUInt64;
};

template <>
struct nsVariantTypeDeduction<float>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Float;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = float;
};

template <>
struct nsVariantTypeDeduction<double>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Double;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = double;
};

template <>
struct nsVariantTypeDeduction<nsColor>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Color;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = true;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsColor;
};

template <>
struct nsVariantTypeDeduction<nsColorGammaUB>
{
  static constexpr nsVariantType::Enum value = nsVariantType::ColorGamma;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = true;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsColorGammaUB;
};

template <>
struct nsVariantTypeDeduction<nsVec2>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Vector2;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = true;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsVec2;
};

template <>
struct nsVariantTypeDeduction<nsVec3>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Vector3;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = true;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsVec3;
};

template <>
struct nsVariantTypeDeduction<nsVec4>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Vector4;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = true;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsVec4;
};

template <>
struct nsVariantTypeDeduction<nsVec2I32>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Vector2I;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = true;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsVec2I32;
};

template <>
struct nsVariantTypeDeduction<nsVec3I32>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Vector3I;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = true;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsVec3I32;
};

template <>
struct nsVariantTypeDeduction<nsVec4I32>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Vector4I;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = true;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsVec4I32;
};

template <>
struct nsVariantTypeDeduction<nsVec2U32>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Vector2U;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = true;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsVec2U32;
};

template <>
struct nsVariantTypeDeduction<nsVec3U32>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Vector3U;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = true;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsVec3U32;
};

template <>
struct nsVariantTypeDeduction<nsVec4U32>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Vector4U;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = true;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsVec4U32;
};

template <>
struct nsVariantTypeDeduction<nsQuat>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Quaternion;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = true;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsQuat;
};

template <>
struct nsVariantTypeDeduction<nsMat3>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Matrix3;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsMat3;
};

template <>
struct nsVariantTypeDeduction<nsMat4>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Matrix4;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsMat4;
};

template <>
struct nsVariantTypeDeduction<nsTransform>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Transform;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsTransform;
};

template <>
struct nsVariantTypeDeduction<nsString>
{
  static constexpr nsVariantType::Enum value = nsVariantType::String;
  static constexpr bool forceSharing = true;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsString;
};

template <>
struct nsVariantTypeDeduction<nsUntrackedString>
{
  static constexpr nsVariantType::Enum value = nsVariantType::String;
  static constexpr bool forceSharing = true;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsString;
};

template <>
struct nsVariantTypeDeduction<nsStringView>
{
  static constexpr nsVariantType::Enum value = nsVariantType::StringView;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsStringView;
};

template <>
struct nsVariantTypeDeduction<nsDataBuffer>
{
  static constexpr nsVariantType::Enum value = nsVariantType::DataBuffer;
  static constexpr bool forceSharing = true;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsDataBuffer;
};

template <>
struct nsVariantTypeDeduction<char*>
{
  static constexpr nsVariantType::Enum value = nsVariantType::String;
  static constexpr bool forceSharing = true;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsString;
};

template <>
struct nsVariantTypeDeduction<const char*>
{
  static constexpr nsVariantType::Enum value = nsVariantType::String;
  static constexpr bool forceSharing = true;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsString;
};

template <size_t N>
struct nsVariantTypeDeduction<char[N]>
{
  static constexpr nsVariantType::Enum value = nsVariantType::String;
  static constexpr bool forceSharing = true;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsString;
};

template <size_t N>
struct nsVariantTypeDeduction<const char[N]>
{
  static constexpr nsVariantType::Enum value = nsVariantType::String;
  static constexpr bool forceSharing = true;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsString;
};

template <>
struct nsVariantTypeDeduction<nsTime>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Time;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsTime;
};

template <>
struct nsVariantTypeDeduction<nsUuid>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Uuid;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsUuid;
};

template <>
struct nsVariantTypeDeduction<nsAngle>
{
  static constexpr nsVariantType::Enum value = nsVariantType::Angle;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsAngle;
};

template <>
struct nsVariantTypeDeduction<nsHashedString>
{
  static constexpr nsVariantType::Enum value = nsVariantType::HashedString;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsHashedString;
};

template <>
struct nsVariantTypeDeduction<nsTempHashedString>
{
  static constexpr nsVariantType::Enum value = nsVariantType::TempHashedString;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsTempHashedString;
};

template <>
struct nsVariantTypeDeduction<nsVariantArray>
{
  static constexpr nsVariantType::Enum value = nsVariantType::VariantArray;
  static constexpr bool forceSharing = true;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsVariantArray;
};

template <>
struct nsVariantTypeDeduction<nsArrayPtr<nsVariant>>
{
  static constexpr nsVariantType::Enum value = nsVariantType::VariantArray;
  static constexpr bool forceSharing = true;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsVariantArray;
};


template <>
struct nsVariantTypeDeduction<nsVariantDictionary>
{
  static constexpr nsVariantType::Enum value = nsVariantType::VariantDictionary;
  static constexpr bool forceSharing = true;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsVariantDictionary;
};

namespace nsInternal
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
    using StorageType = nsReflectedClass*;
  };
} // namespace nsInternal

template <>
struct nsVariantTypeDeduction<nsTypedPointer>
{
  static constexpr nsVariantType::Enum value = nsVariantType::TypedPointer;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = true;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::DirectCast;

  using StorageType = nsTypedPointer;
};

template <typename T>
struct nsVariantTypeDeduction<T*>
{
  static constexpr nsVariantType::Enum value = nsVariantType::TypedPointer;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = true;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::PointerCast;

  using StorageType = nsTypedPointer;
};

template <>
struct nsVariantTypeDeduction<nsTypedObject>
{
  static constexpr nsVariantType::Enum value = nsVariantType::TypedObject;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = true;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::TypedObject;

  using StorageType = nsTypedObject;
};

/// \endcond
