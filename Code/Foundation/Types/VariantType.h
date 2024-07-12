#pragma once

#include <Foundation/Strings/String.h>
#include <Foundation/Types/Types.h>

class nsReflectedClass;
class nsVariant;
struct nsTime;
class nsUuid;
struct nsStringView;
struct nsTypedObject;
struct nsTypedPointer;

using nsDataBuffer = nsDynamicArray<nsUInt8>;
using nsVariantArray = nsDynamicArray<nsVariant>;
using nsVariantDictionary = nsHashTable<nsString, nsVariant>;

/// \brief This enum describes the type of data that is currently stored inside the variant.
struct nsVariantType
{
  using StorageType = nsUInt8;
  /// \brief This enum describes the type of data that is currently stored inside the variant.
  /// Note that changes to this enum require an increase of the reflection version and either
  /// patches to the serializer or a re-export of binary data that contains nsVariants.
  enum Enum : nsUInt8
  {
    Invalid = 0, ///< The variant stores no (valid) data at the moment.

    /// *** Types that are flagged as 'StandardTypes' (see DetermineTypeFlags) ***
    FirstStandardType = 1,
    Bool,             ///< The variant stores a bool.
    Int8,             ///< The variant stores an nsInt8.
    UInt8,            ///< The variant stores an nsUInt8.
    Int16,            ///< The variant stores an nsInt16.
    UInt16,           ///< The variant stores an nsUInt16.
    Int32,            ///< The variant stores an nsInt32.
    UInt32,           ///< The variant stores an nsUInt32.
    Int64,            ///< The variant stores an nsInt64.
    UInt64,           ///< The variant stores an nsUInt64.
    Float,            ///< The variant stores a float.
    Double,           ///< The variant stores a double.
    Color,            ///< The variant stores an nsColor.
    Vector2,          ///< The variant stores an nsVec2.
    Vector3,          ///< The variant stores an nsVec3.
    Vector4,          ///< The variant stores an nsVec4.
    Vector2I,         ///< The variant stores an nsVec2I32.
    Vector3I,         ///< The variant stores an nsVec3I32.
    Vector4I,         ///< The variant stores an nsVec4I32.
    Vector2U,         ///< The variant stores an nsVec2U32.
    Vector3U,         ///< The variant stores an nsVec3U32.
    Vector4U,         ///< The variant stores an nsVec4U32.
    Quaternion,       ///< The variant stores an nsQuat.
    Matrix3,          ///< The variant stores an nsMat3. A heap allocation is required to store this data type.
    Matrix4,          ///< The variant stores an nsMat4. A heap allocation is required to store this data type.
    Transform,        ///< The variant stores an nsTransform. A heap allocation is required to store this data type.
    String,           ///< The variant stores a string. A heap allocation is required to store this data type.
    StringView,       ///< The variant stores an nsStringView.
    DataBuffer,       ///< The variant stores an nsDataBuffer, a typedef to DynamicArray<nsUInt8>. A heap allocation is required to store this data type.
    Time,             ///< The variant stores an nsTime value.
    Uuid,             ///< The variant stores an nsUuid value.
    Angle,            ///< The variant stores an nsAngle value.
    ColorGamma,       ///< The variant stores an nsColorGammaUB value.
    HashedString,     ///< The variant stores an nsHashedString value.
    TempHashedString, ///< The variant stores an nsTempHashedString value.
    LastStandardType,
    /// *** Types that are flagged as 'StandardTypes' (see DetermineTypeFlags) ***

    FirstExtendedType = 64,
    VariantArray,      ///< The variant stores an array of nsVariant's. A heap allocation is required to store this data type.
    VariantDictionary, ///< The variant stores a dictionary (hashmap) of nsVariant's. A heap allocation is required to store this type.
    TypedPointer,      ///< The variant stores an nsTypedPointer value. Reflected type and data queries will match the pointed to object.
    TypedObject,       ///< The variant stores an nsTypedObject value. Reflected type and data queries will match the object. A heap allocation is required to store this type if it is larger than 16 bytes or not POD.
    LastExtendedType,  ///< Number of values for nsVariant::Type.

    MAX_ENUM_VALUE = LastExtendedType,
    Default = Invalid  ///< Default value used by nsEnum.
  };
};

NS_DEFINE_AS_POD_TYPE(nsVariantType::Enum);

struct nsVariantClass
{
  enum Enum
  {
    Invalid,
    DirectCast,     ///< A standard type
    PointerCast,    ///< Any cast to T*
    TypedObject,    ///< nsTypedObject cast. Needed because at no point does and nsVariant ever store a nsTypedObject so it can't be returned as a const reference.
    CustomTypeCast, ///< Custom object types
  };
};

/// \brief A helper struct to convert the C++ type, which is passed as the template argument, into one of the nsVariant::Type enum values.
template <typename T>
struct nsVariantTypeDeduction
{
  static constexpr nsVariantType::Enum value = nsVariantType::Invalid;
  static constexpr bool forceSharing = false;
  static constexpr bool hasReflectedMembers = false;
  static constexpr nsVariantClass::Enum classification = nsVariantClass::Invalid;

  using StorageType = T;
};

/// \brief Declares a custom variant type, allowing it to be stored by value inside an nsVariant.
///
/// Needs to be called from the same header that defines the type.
/// \sa NS_DEFINE_CUSTOM_VARIANT_TYPE
#define NS_DECLARE_CUSTOM_VARIANT_TYPE(TYPE)                                               \
  template <>                                                                              \
  struct nsVariantTypeDeduction<TYPE>                                                      \
  {                                                                                        \
    static constexpr nsVariantType::Enum value = nsVariantType::TypedObject;               \
    static constexpr bool forceSharing = false;                                            \
    static constexpr bool hasReflectedMembers = true;                                      \
    static constexpr nsVariantClass::Enum classification = nsVariantClass::CustomTypeCast; \
                                                                                           \
    using StorageType = TYPE;                                                              \
  };

#include <Foundation/Types/Implementation/VariantTypeDeduction_inl.h>
