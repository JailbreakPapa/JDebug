#pragma once

#include <Foundation/Math/Math.h>
#include <Foundation/Math/Vec3.h>

/// \brief A 4-component vector class.
template <typename Type>
class nsVec4Template
{
public:
  // Means that vectors can be copied using memcpy instead of copy construction.
  NS_DECLARE_POD_TYPE();

  using ComponentType = Type;

  // *** Data ***
public:
  Type x, y, z, w;

  // *** Constructors ***
public:
  /// \brief Default-constructed vector is uninitialized (for speed)
  nsVec4Template(); // [tested]

  /// \brief Initializes the vector with x,y,z,w
  nsVec4Template(Type x, Type y, Type z, Type w); // [tested]

  /// \brief Initializes the vector from a vec3 and a float.
  nsVec4Template(nsVec3Template<Type> vXyz, Type w);

  /// \brief Initializes all 4 components with xyzw
  explicit nsVec4Template(Type v); // [tested]
  // no copy-constructor and operator= since the default-generated ones will be faster

  /// \brief Returns a vector with all components set to Not-a-Number (NaN).
  NS_DECLARE_IF_FLOAT_TYPE
  [[nodiscard]] static nsVec4Template<Type>
  MakeNaN() { return nsVec4Template<Type>(nsMath::NaN<Type>()); }

  /// \brief Returns a vector with all components set to zero.
  [[nodiscard]] static nsVec4Template<Type> MakeZero() { return nsVec4Template<Type>(0); } // [tested]

#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  void AssertNotNaN() const
  {
    NS_ASSERT_ALWAYS(!IsNaN(), "This object contains NaN values. This can happen when you forgot to initialize it before using it. Please "
                               "check that all code-paths properly initialize this object.");
  }
#endif

  // *** Conversions ***
public:
  /// \brief Returns an nsVec2Template with x and y from this vector.
  const nsVec2Template<Type> GetAsVec2() const; // [tested]

  /// \brief Returns an nsVec3Template with x,y and z from this vector.
  const nsVec3Template<Type> GetAsVec3() const; // [tested]

  /// \brief Returns the data as an array.
  const Type* GetData() const { return &x; }

  /// \brief Returns the data as an array.
  Type* GetData() { return &x; }

  // *** Functions to set the vector to specific values ***
public:
  /// \brief Sets all 4 components to this value.
  void Set(Type xyzw); // [tested]

  /// \brief Sets the vector to these values.
  void Set(Type x, Type y, Type z, Type w); // [tested]

  /// \brief Sets the vector to all zero.
  void SetZero(); // [tested]

  // *** Functions dealing with length ***
public:
  /// \brief Returns the length of the vector.
  NS_DECLARE_IF_FLOAT_TYPE
  Type GetLength() const; // [tested]

  /// \brief Returns the squared length. Faster, since no square-root is taken. Useful, if one only wants to compare the lengths of two
  /// vectors.
  Type GetLengthSquared() const; // [tested]

  /// \brief Normalizes this vector and returns its previous length in one operation. More efficient than calling GetLength and then
  /// Normalize.
  NS_DECLARE_IF_FLOAT_TYPE
  Type GetLengthAndNormalize(); // [tested]

  /// \brief Returns a normalized version of this vector, leaves the vector itself unchanged.
  NS_DECLARE_IF_FLOAT_TYPE
  const nsVec4Template<Type> GetNormalized() const; // [tested]

  /// \brief Normalizes this vector.
  NS_DECLARE_IF_FLOAT_TYPE
  void Normalize(); // [tested]

  /// \brief Tries to normalize this vector. If the vector is too close to zero, NS_FAILURE is returned and the vector is set to the given
  /// fallback value.
  NS_DECLARE_IF_FLOAT_TYPE
  nsResult NormalizeIfNotZero(const nsVec4Template<Type>& vFallback = nsVec4Template<Type>(1, 0, 0, 0), Type fEpsilon = nsMath::SmallEpsilon<Type>()); // [tested]

  /// \brief Returns, whether this vector is (0, 0, 0, 0).
  bool IsZero() const; // [tested]

  /// \brief Returns, whether this vector is (0, 0, 0, 0).
  bool IsZero(Type fEpsilon) const; // [tested]

  /// \brief Returns, whether the squared length of this vector is between 0.999f and 1.001f.
  NS_DECLARE_IF_FLOAT_TYPE
  bool IsNormalized(Type fEpsilon = nsMath::HugeEpsilon<Type>()) const; // [tested]

  /// \brief Returns true, if any of x, y, z or w is NaN.
  bool IsNaN() const; // [tested]

  /// \brief Checks that all components are finite numbers.
  bool IsValid() const; // [tested]


  // *** Operators ***
public:
  /// \brief Returns the negation of this vector.
  const nsVec4Template<Type> operator-() const; // [tested]

  /// \brief Adds cc component-wise to this vector.
  void operator+=(const nsVec4Template<Type>& vCc); // [tested]

  /// \brief Subtracts cc component-wise from this vector.
  void operator-=(const nsVec4Template<Type>& vCc); // [tested]

  /// \brief Multiplies all components of this vector with f.
  void operator*=(Type f); // [tested]

  /// \brief Divides all components of this vector by f.
  void operator/=(Type f); // [tested]

  /// \brief Equality Check (bitwise).
  bool IsIdentical(const nsVec4Template<Type>& rhs) const; // [tested]

  /// \brief Equality Check with epsilon.
  bool IsEqual(const nsVec4Template<Type>& rhs, Type fEpsilon) const; // [tested]


  // *** Common vector operations ***
public:
  /// \brief Returns the dot-product of the two vectors (commutative, order does not matter).
  Type Dot(const nsVec4Template<Type>& rhs) const; // [tested]

  /// \brief Returns the component-wise minimum of *this and rhs.
  const nsVec4Template<Type> CompMin(const nsVec4Template<Type>& rhs) const; // [tested]

  /// \brief Returns the component-wise maximum of *this and rhs.
  const nsVec4Template<Type> CompMax(const nsVec4Template<Type>& rhs) const; // [tested]

  /// \brief Returns the component-wise clamped value of *this between low and high.
  const nsVec4Template<Type> CompClamp(const nsVec4Template<Type>& vLow, const nsVec4Template<Type>& vHigh) const; // [tested]

  /// \brief Returns the component-wise multiplication of *this and rhs.
  const nsVec4Template<Type> CompMul(const nsVec4Template<Type>& rhs) const; // [tested]

  /// \brief Returns the component-wise division of *this and rhs.
  const nsVec4Template<Type> CompDiv(const nsVec4Template<Type>& rhs) const; // [tested]

  /// brief Returns the component-wise absolute of *this.
  const nsVec4Template<Type> Abs() const; // [tested]
};

// *** Operators ***

template <typename Type>
const nsVec4Template<Type> operator+(const nsVec4Template<Type>& v1, const nsVec4Template<Type>& v2); // [tested]

template <typename Type>
const nsVec4Template<Type> operator-(const nsVec4Template<Type>& v1, const nsVec4Template<Type>& v2); // [tested]


template <typename Type>
const nsVec4Template<Type> operator*(Type f, const nsVec4Template<Type>& v); // [tested]

template <typename Type>
const nsVec4Template<Type> operator*(const nsVec4Template<Type>& v, Type f); // [tested]


template <typename Type>
const nsVec4Template<Type> operator/(const nsVec4Template<Type>& v, Type f); // [tested]


template <typename Type>
bool operator==(const nsVec4Template<Type>& v1, const nsVec4Template<Type>& v2); // [tested]

template <typename Type>
bool operator!=(const nsVec4Template<Type>& v1, const nsVec4Template<Type>& v2); // [tested]

/// \brief Strict weak ordering. Useful for sorting vertices into a map.
template <typename Type>
bool operator<(const nsVec4Template<Type>& v1, const nsVec4Template<Type>& v2); // [tested]

#include <Foundation/Math/Implementation/Vec4_inl.h>
