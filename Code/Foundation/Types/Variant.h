#pragma once

#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Math/Declarations.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Threading/AtomicInteger.h>
#include <Foundation/Types/TypedPointer.h>
#include <Foundation/Types/Types.h>
#include <Foundation/Types/VariantType.h>

#include <Foundation/Reflection/Implementation/DynamicRTTI.h>
#include <Foundation/Utilities/ConversionUtils.h>

class nsRTTI;

/// \brief Defines a reference to an immutable object owned by an nsVariant.
///
/// Used to store custom types inside an nsVariant. As lifetime is governed by the nsVariant, it is generally not safe to store an nsTypedObject.
/// This class is needed to be able to differentiate between nsVariantType::TypedPointer and nsVariantType::TypedObject e.g. in nsVariant::DispatchTo.
/// \sa nsVariant, NS_DECLARE_CUSTOM_VARIANT_TYPE
struct nsTypedObject
{
  NS_DECLARE_POD_TYPE();
  const void* m_pObject = nullptr;
  const nsRTTI* m_pType = nullptr;

  bool operator==(const nsTypedObject& rhs) const
  {
    return m_pObject == rhs.m_pObject;
  }
  NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(const nsTypedObject&);
};

/// \brief nsVariant is a class that can store different types of variables, which is useful in situations where it is not clear up front,
/// which type of data will be passed around.
///
/// The variant supports a fixed list of types that it can store (\see nsVariant::Type). All types of 16 bytes or less in size can be stored
/// without requiring a heap allocation. For larger types memory is allocated on the heap. In general variants should be used for code that
/// needs to be flexible. Although nsVariant is implemented very efficiently, it should be avoided to use nsVariant in code that needs to be
/// fast.
class NS_FOUNDATION_DLL nsVariant
{
public:
  using Type = nsVariantType;
  template <typename T>
  using TypeDeduction = nsVariantTypeDeduction<T>;

  /// \brief helper struct to wrap a string pointer
  struct StringWrapper
  {
    NS_ALWAYS_INLINE StringWrapper(const char* szStr)
      : m_str(szStr)
    {
    }
    const char* m_str;
  };

  /// \brief Initializes the variant to be 'Invalid'
  nsVariant(); // [tested]

  /// \brief Copies the data from the other variant.
  ///
  /// \note If the data of the variant needed to be allocated on the heap, it will be shared among variants.
  /// Thus, once you have stored such a type inside a variant, you can copy it to other variants, without introducing
  /// additional memory allocations.
  nsVariant(const nsVariant& other); // [tested]

  /// \brief Moves the data from the other variant.
  nsVariant(nsVariant&& other) noexcept; // [tested]

  nsVariant(const bool& value);
  nsVariant(const nsInt8& value);
  nsVariant(const nsUInt8& value);
  nsVariant(const nsInt16& value);
  nsVariant(const nsUInt16& value);
  nsVariant(const nsInt32& value);
  nsVariant(const nsUInt32& value);
  nsVariant(const nsInt64& value);
  nsVariant(const nsUInt64& value);
  nsVariant(const float& value);
  nsVariant(const double& value);
  nsVariant(const nsColor& value);
  nsVariant(const nsVec2& value);
  nsVariant(const nsVec3& value);
  nsVariant(const nsVec4& value);
  nsVariant(const nsVec2I32& value);
  nsVariant(const nsVec3I32& value);
  nsVariant(const nsVec4I32& value);
  nsVariant(const nsVec2U32& value);
  nsVariant(const nsVec3U32& value);
  nsVariant(const nsVec4U32& value);
  nsVariant(const nsQuat& value);
  nsVariant(const nsMat3& value);
  nsVariant(const nsMat4& value);
  nsVariant(const nsTransform& value);
  nsVariant(const char* value);
  nsVariant(const nsString& value);
  nsVariant(const nsUntrackedString& value);
  nsVariant(const nsStringView& value, bool bCopyString = true);
  nsVariant(const nsDataBuffer& value);
  nsVariant(const nsTime& value);
  nsVariant(const nsUuid& value);
  nsVariant(const nsAngle& value);
  nsVariant(const nsColorGammaUB& value);
  nsVariant(const nsHashedString& value);
  nsVariant(const nsTempHashedString& value);

  nsVariant(const nsVariantArray& value);
  nsVariant(const nsVariantDictionary& value);

  nsVariant(const nsTypedPointer& value);
  nsVariant(const nsTypedObject& value);

  template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::CustomTypeCast, int> = 0>
  nsVariant(const T& value);

  template <typename T>
  nsVariant(const T* value);

  /// \brief Initializes to a TypedPointer of the given object and type.
  nsVariant(void* value, const nsRTTI* pType);

  /// \brief Initializes to a TypedObject by cloning the given object and type.
  void CopyTypedObject(const void* value, const nsRTTI* pType); // [tested]

  /// \brief Initializes to a TypedObject by taking ownership of the given object and type.
  void MoveTypedObject(void* value, const nsRTTI* pType); // [tested]

  /// \brief If necessary, this will deallocate any heap memory that is not in use any more.
  ~nsVariant();

  /// \brief Copies the data from the \a other variant into this one.
  void operator=(const nsVariant& other); // [tested]

  /// \brief Moves the data from the \a other variant into this one.
  void operator=(nsVariant&& other) noexcept; // [tested]

  /// \brief Deduces the type of \a T and stores \a value.
  ///
  /// If the type to be stored in the variant is not supported, a compile time error will occur.
  template <typename T>
  void operator=(const T& value); // [tested]

  /// \brief Will compare the value of this variant to that of \a other.
  ///
  /// If both variants store 'numbers' (float, double, int types) the comparison will work, even if the types are not identical.
  ///
  /// \note If the two types are not numbers and not equal, an assert will occur. So be careful to only compare variants
  /// that can either both be converted to double (\see CanConvertTo()) or whose types are equal.
  bool operator==(const nsVariant& other) const; // [tested]

  NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(const nsVariant&);

  /// \brief See non-templated operator==
  template <typename T>
  bool operator==(const T& other) const; // [tested]

#if NS_DISABLED(NS_USE_CPP20_OPERATORS)
  /// \brief See non-templated operator!=
  template <typename T>
  bool operator!=(const T& other) const // [tested]
  {
    return !(*this == other);
  }
#endif

  /// \brief Returns whether this variant stores any other type than 'Invalid'.
  bool IsValid() const; // [tested]

  /// \brief Returns whether the stored type is numerical type either integer or floating point.
  ///
  /// Bool counts as number.
  bool IsNumber() const; // [tested]

  /// \brief Returns whether the stored type is floating point (float or double).
  bool IsFloatingPoint() const; // [tested]

  /// \brief Returns whether the stored type is a string (nsString or nsStringView).
  bool IsString() const; // [tested]

  /// \brief Returns whether the stored type is a hashed string (nsHashedString or nsTempHashedString).
  bool IsHashedString() const;

  /// \brief Returns whether the stored type is exactly the given type.
  ///
  /// \note This explicitly also differentiates between the different integer types.
  /// So when the variant stores an Int32, IsA<Int64>() will return false, even though the types could be converted.
  template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::DirectCast, int> = 0>
  bool IsA() const; // [tested]

  template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::PointerCast, int> = 0>
  bool IsA() const; // [tested]

  template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::TypedObject, int> = 0>
  bool IsA() const; // [tested]

  template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::CustomTypeCast, int> = 0>
  bool IsA() const; // [tested]

  /// \brief Returns the exact nsVariant::Type value.
  Type::Enum GetType() const; // [tested]

  /// \brief Returns the variants value as the provided type.
  ///
  /// \note This function does not do ANY type of conversion from the stored type to the given type. Not even integer conversions!
  /// If the types don't match, this function will assert!
  /// So be careful to use this function only when you know exactly that the stored type matches the expected type.
  ///
  /// Prefer to use ConvertTo() when you can instead.
  template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::DirectCast, int> = 0>
  const T& Get() const; // [tested]

  template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::PointerCast, int> = 0>
  T Get() const;        // [tested]

  template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::TypedObject, int> = 0>
  const T Get() const;  // [tested]

  template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::CustomTypeCast, int> = 0>
  const T& Get() const; // [tested]

  /// \brief Returns an writable nsTypedPointer to the internal data.
  /// If the data is currently shared a clone will be made to ensure we hold the only reference.
  nsTypedPointer GetWriteAccess(); // [tested]

  template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::DirectCast, int> = 0>
  T& GetWritable();                // [tested]

  template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::PointerCast, int> = 0>
  T GetWritable();                 // [tested]

  template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::CustomTypeCast, int> = 0>
  T& GetWritable();                // [tested]


  /// \brief Returns a const void* to the internal data.
  /// For TypedPointer and TypedObject this will return a pointer to the target object.
  const void* GetData() const; // [tested]

  /// \brief Returns the nsRTTI type of the held value.
  /// For TypedPointer and TypedObject this will return the type of the target object.
  const nsRTTI* GetReflectedType() const; // [tested]

  /// \brief Returns the sub value at iIndex. This could be an element in an array or a member property inside a reflected type.
  ///
  /// Out of bounds access is handled gracefully and will return an invalid variant.
  const nsVariant operator[](nsUInt32 uiIndex) const; // [tested]

  /// \brief Returns the sub value with szKey. This could be a value in a dictionary or a member property inside a reflected type.
  ///
  /// This function will return an invalid variant if no corresponding sub value is found.
  const nsVariant operator[](StringWrapper key) const; // [tested]

  /// \brief Returns whether the stored type can generally be converted to the desired type.
  ///
  /// This function will return true for all number conversions, as float / double / int / etc. can generally be converted into each
  /// other. It will also return true for all conversion from string to number types, and from all 'simple' types (not array or dictionary)
  /// to string.
  ///
  /// \note This function only returns whether a conversion between the stored TYPE and the desired TYPE is generally possible. It does NOT
  /// return whether the stored VALUE is indeed convertible to the desired type. For example, a string is generally convertible to float, if
  /// it stores a string representation of a float value. If, however, it stores anything else, the conversion can still fail.
  ///
  /// The only way to figure out whether the stored data can be converted to some type, is to actually convert it, using ConvertTo(), and
  /// then to check the conversion status.
  template <typename T>
  bool CanConvertTo() const; // [tested]

  /// \brief Same as the templated CanConvertTo function.
  bool CanConvertTo(Type::Enum type) const; // [tested]

  /// \brief Tries to convert the stored value to the given type. The optional status parameter can be used to check whether the conversion
  /// succeeded.
  ///
  /// When CanConvertTo() returns false, ConvertTo() will also always fail. However, when CanConvertTo() returns true, this is no guarantee
  /// that ConvertTo() will succeed. Conversion between numbers and to strings will generally succeed. However, converting from a string to
  /// another type can fail or succeed, depending on the exact string value.
  template <typename T>
  T ConvertTo(nsResult* out_pConversionStatus = nullptr) const; // [tested]

  /// \brief Same as the templated function.
  nsVariant ConvertTo(Type::Enum type, nsResult* out_pConversionStatus = nullptr) const; // [tested]

  /// \brief This will call the overloaded operator() (function call operator) of the provided functor.
  ///
  /// This allows to implement a functor that overloads operator() for different types and then call the proper version of that operator,
  /// depending on the provided runtime type. Note that the proper overload of operator() is selected by providing a dummy type, but it will
  /// contain no useful value. Instead, store the other necessary data inside the functor object, before calling this function. For example,
  /// store a pointer to a variant inside the functor object and then call DispatchTo to execute the function that will handle the given
  /// type of the variant.
  template <typename Functor, class... Args>
  static auto DispatchTo(Functor& ref_functor, Type::Enum type, Args&&... args); // [tested]

  /// \brief Computes the hash value of the stored data. Returns uiSeed (unchanged) for an invalid Variant.
  nsUInt64 ComputeHash(nsUInt64 uiSeed = 0) const;

private:
  friend class nsVariantHelper;
  friend struct CompareFunc;
  friend struct GetTypeFromVariantFunc;

  struct SharedData
  {
    void* m_Ptr;
    const nsRTTI* m_pType;
    nsAtomicInteger32 m_uiRef = 1;
    NS_ALWAYS_INLINE SharedData(void* pPtr, const nsRTTI* pType)
      : m_Ptr(pPtr)
      , m_pType(pType)
    {
    }
    virtual ~SharedData() = default;
    virtual SharedData* Clone() const = 0;
  };

  template <typename T>
  class TypedSharedData : public SharedData
  {
  private:
    T m_t;

  public:
    NS_ALWAYS_INLINE TypedSharedData(const T& value, const nsRTTI* pType = nullptr)
      : SharedData(&m_t, pType)
      , m_t(value)
    {
    }

    virtual SharedData* Clone() const override
    {
      return NS_DEFAULT_NEW(TypedSharedData<T>, m_t, m_pType);
    }
  };

  class RTTISharedData : public SharedData
  {
  public:
    RTTISharedData(void* pData, const nsRTTI* pType);

    ~RTTISharedData();

    virtual SharedData* Clone() const override;
  };

  struct InlinedStruct
  {
    constexpr static int DataSize = 4 * sizeof(float) - sizeof(void*);
    nsUInt8 m_Data[DataSize];
    const nsRTTI* m_pType;
  };

  union Data
  {
    float f[4];
    SharedData* shared;
    InlinedStruct inlined;
  } m_Data;

  nsUInt32 m_uiType : 31;
  nsUInt32 m_bIsShared : 1; // NOLINT(ns*)

  template <typename T>
  void InitInplace(const T& value);

  template <typename T>
  void InitShared(const T& value);

  template <typename T>
  void InitTypedObject(const T& value, nsTraitInt<0>);
  template <typename T>
  void InitTypedObject(const T& value, nsTraitInt<1>);

  void InitTypedPointer(void* value, const nsRTTI* pType);

  void Release();
  void CopyFrom(const nsVariant& other);
  void MoveFrom(nsVariant&& other);

  template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::DirectCast, int> = 0>
  const T& Cast() const;
  template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::PointerCast, int> = 0>
  T Cast() const;
  template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::TypedObject, int> = 0>
  const T Cast() const;
  template <typename T, typename std::enable_if_t<nsVariantTypeDeduction<T>::classification == nsVariantClass::CustomTypeCast, int> = 0>
  const T& Cast() const;

  static bool IsNumberStatic(nsUInt32 type);
  static bool IsFloatingPointStatic(nsUInt32 type);
  static bool IsStringStatic(nsUInt32 type);
  static bool IsHashedStringStatic(nsUInt32 type);
  static bool IsVector2Static(nsUInt32 type);
  static bool IsVector3Static(nsUInt32 type);
  static bool IsVector4Static(nsUInt32 type);

  // Needed to prevent including nsRTTI in nsVariant.h
  static bool IsDerivedFrom(const nsRTTI* pType1, const nsRTTI* pType2);
  static nsStringView GetTypeName(const nsRTTI* pType);

  template <typename T>
  T ConvertNumber() const;
};

/// \brief An overload of nsDynamicCast for dynamic casting a variant to a pointer type.
///
/// If the nsVariant stores an nsTypedPointer pointer, this pointer will be dynamically cast to T*.
/// If the nsVariant stores any other type (or nothing), nullptr is returned.
template <typename T>
NS_ALWAYS_INLINE T nsDynamicCast(const nsVariant& variant)
{
  if (variant.IsA<T>())
  {
    return variant.Get<T>();
  }

  return nullptr;
}

// Simple math operator overloads. An invalid variant is returned if the given variants have incompatible types.
NS_FOUNDATION_DLL nsVariant operator+(const nsVariant& a, const nsVariant& b);
NS_FOUNDATION_DLL nsVariant operator-(const nsVariant& a, const nsVariant& b);
NS_FOUNDATION_DLL nsVariant operator*(const nsVariant& a, const nsVariant& b);
NS_FOUNDATION_DLL nsVariant operator/(const nsVariant& a, const nsVariant& b);

namespace nsMath
{
  /// \brief An overload of nsMath::Lerp to interpolate variants. A and b must have the same type.
  ///
  /// If the type can't be interpolated like e.g. strings, a is returned for a fFactor less than 0.5, b is returned for a fFactor greater or equal to 0.5.
  NS_FOUNDATION_DLL nsVariant Lerp(const nsVariant& a, const nsVariant& b, double fFactor);
} // namespace nsMath

#include <Foundation/Types/Implementation/VariantHelper_inl.h>

#include <Foundation/Types/Implementation/Variant_inl.h>
