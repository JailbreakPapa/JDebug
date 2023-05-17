#pragma once

#include <Foundation/Math/Math.h>

#if WD_ENABLED(WD_MATH_CHECK_FOR_NAN)
#  define WD_VEC2_CHECK_FOR_NAN(obj) (obj)->AssertNotNaN();
#else
#  define WD_VEC2_CHECK_FOR_NAN(obj)
#endif

/// \brief A 2-component vector class.
template <typename Type>
class wdVec2Template
{
public:
  // Means that vectors can be copied using memcpy instead of copy construction.
  WD_DECLARE_POD_TYPE();

  using ComponentType = Type;


  // *** Data ***
public:
  Type x;
  Type y;

  // *** Constructors ***
public:
  /// \brief default-constructed vector is uninitialized (for speed)
  wdVec2Template(); // [tested]

  /// \brief Initializes the vector with x,y
  wdVec2Template(Type x, Type y); // [tested]

  /// \brief Initializes all components with xy
  explicit wdVec2Template(Type v); // [tested]

  // no copy-constructor and operator= since the default-generated ones will be faster

  /// \brief Static function that returns a zero-vector.
  static const wdVec2Template<Type> ZeroVector() { return wdVec2Template(0); } // [tested]

#if WD_ENABLED(WD_MATH_CHECK_FOR_NAN)
  void AssertNotNaN() const
  {
    WD_ASSERT_ALWAYS(!IsNaN(), "This object contains NaN values. This can happen when you forgot to initialize it before using it. Please "
                               "check that all code-paths properly initialize this object.");
  }
#endif

  // *** Conversions ***
public:
  /// \brief Returns an wdVec3Template with x,y from this vector and z set by the parameter.
  const wdVec3Template<Type> GetAsVec3(Type z) const; // [tested]

  /// \brief Returns an wdVec4Template with x,y from this vector and z and w set by the parameters.
  const wdVec4Template<Type> GetAsVec4(Type z, Type w) const; // [tested]

  /// \brief Returns the data as an array.
  const Type* GetData() const { return &x; }

  /// \brief Returns the data as an array.
  Type* GetData() { return &x; }

  // *** Functions to set the vector to specific values ***
public:
  /// \brief Sets all components to this value.
  void Set(Type xy); // [tested]

  /// \brief Sets the vector to these values.
  void Set(Type x, Type y); // [tested]

  /// \brief Sets the vector to all zero.
  void SetZero(); // [tested]

  // *** Functions dealing with length ***
public:
  /// \brief Returns the length of the vector.
  Type GetLength() const; // [tested]

  /// \brief Tries to rescale the vector to the given length. If the vector is too close to zero, WD_FAILURE is returned and the vector is
  /// set to zero.
  wdResult SetLength(Type fNewLength, Type fEpsilon = wdMath::DefaultEpsilon<Type>()); // [tested]

  /// \brief Returns the squared length. Faster, since no square-root is taken. Useful, if one only wants to compare the lengths of two
  /// vectors.
  Type GetLengthSquared() const; // [tested]

  /// \brief Normalizes this vector and returns its previous length in one operation. More efficient than calling GetLength and then
  /// Normalize.
  Type GetLengthAndNormalize(); // [tested]

  /// \brief Returns a normalized version of this vector, leaves the vector itself unchanged.
  const wdVec2Template<Type> GetNormalized() const; // [tested]

  /// \brief Normalizes this vector.
  void Normalize(); // [tested]

  /// \brief Tries to normalize this vector. If the vector is too close to zero, WD_FAILURE is returned and the vector is set to the given
  /// fallback value.
  wdResult NormalizeIfNotZero(
    const wdVec2Template<Type>& vFallback = wdVec2Template<Type>(1, 0), Type fEpsilon = wdMath::DefaultEpsilon<Type>()); // [tested]

  /// \brief Returns, whether this vector is (0, 0).
  bool IsZero() const; // [tested]

  /// \brief Returns, whether this vector is (0, 0) within a certain threshold.
  bool IsZero(Type fEpsilon) const; // [tested]

  /// \brief Returns, whether the squared length of this vector is between 0.999f and 1.001f.
  bool IsNormalized(Type fEpsilon = wdMath::HugeEpsilon<Type>()) const; // [tested]

  /// \brief Returns true, if any of x or y is NaN
  bool IsNaN() const; // [tested]

  /// \brief Checks that all components are finite numbers.
  bool IsValid() const; // [tested]


  // *** Operators ***
public:
  /// \brief Returns the negation of this vector.
  const wdVec2Template<Type> operator-() const; // [tested]

  /// \brief Adds cc component-wise to this vector
  void operator+=(const wdVec2Template<Type>& vCc); // [tested]

  /// \brief Subtracts cc component-wise from this vector
  void operator-=(const wdVec2Template<Type>& vCc); // [tested]

  /// \brief Multiplies all components of this vector with f
  void operator*=(Type f); // [tested]

  /// \brief Divides all components of this vector by f
  void operator/=(Type f); // [tested]

  /// \brief Equality Check (bitwise)
  bool IsIdentical(const wdVec2Template<Type>& rhs) const; // [tested]

  /// \brief Equality Check with epsilon
  bool IsEqual(const wdVec2Template<Type>& rhs, Type fEpsilon) const; // [tested]


  // *** Common vector operations ***
public:
  /// \brief Returns the positive angle between *this and rhs.
  wdAngle GetAngleBetween(const wdVec2Template<Type>& rhs) const; // [tested]

  /// \brief Returns the Dot-product of the two vectors (commutative, order does not matter)
  Type Dot(const wdVec2Template<Type>& rhs) const; // [tested]

  /// \brief Returns the component-wise minimum of *this and rhs
  const wdVec2Template<Type> CompMin(const wdVec2Template<Type>& rhs) const; // [tested]

  /// \brief Returns the component-wise maximum of *this and rhs
  const wdVec2Template<Type> CompMax(const wdVec2Template<Type>& rhs) const; // [tested]

  /// \brief Returns the component-wise clamped value of *this between low and high.
  const wdVec2Template<Type> CompClamp(const wdVec2Template<Type>& vLow, const wdVec2Template<Type>& vHigh) const; // [tested]

  /// \brief Returns the component-wise multiplication of *this and rhs
  const wdVec2Template<Type> CompMul(const wdVec2Template<Type>& rhs) const; // [tested]

  /// \brief Returns the component-wise division of *this and rhs
  const wdVec2Template<Type> CompDiv(const wdVec2Template<Type>& rhs) const; // [tested]

  /// brief Returns the component-wise absolute of *this.
  const wdVec2Template<Type> Abs() const; // [tested]


  // *** Other common operations ***
public:
  /// \brief Modifies this direction vector to be orthogonal to the given (normalized) direction vector. The result is NOT normalized.
  ///
  /// \note This function may fail, e.g. create a vector that is zero, if the given normal is parallel to the vector itself.
  ///       If you need to handle such cases, you should manually check afterwards, whether the result is zero, or cannot be normalized.
  void MakeOrthogonalTo(const wdVec2Template<Type>& vNormal); // [tested]

  /// \brief Returns some arbitrary vector orthogonal to this one. The vector is NOT normalized.
  const wdVec2Template<Type> GetOrthogonalVector() const; // [tested]

  /// \brief Returns this vector reflected at vNormal.
  const wdVec2Template<Type> GetReflectedVector(const wdVec2Template<Type>& vNormal) const; // [tested]
};

// *** Operators ***

/// \brief Component-wise addition.
template <typename Type>
const wdVec2Template<Type> operator+(const wdVec2Template<Type>& v1, const wdVec2Template<Type>& v2); // [tested]

/// \brief Component-wise subtraction.
template <typename Type>
const wdVec2Template<Type> operator-(const wdVec2Template<Type>& v1, const wdVec2Template<Type>& v2); // [tested]

/// \brief Returns a scaled vector.
template <typename Type>
const wdVec2Template<Type> operator*(Type f, const wdVec2Template<Type>& v); // [tested]

/// \brief Returns a scaled vector.
template <typename Type>
const wdVec2Template<Type> operator*(const wdVec2Template<Type>& v, Type f); // [tested]

/// \brief Returns a scaled vector.
template <typename Type>
const wdVec2Template<Type> operator/(const wdVec2Template<Type>& v, Type f); // [tested]

/// \brief Returns true, if both vectors are identical.
template <typename Type>
bool operator==(const wdVec2Template<Type>& v1, const wdVec2Template<Type>& v2); // [tested]

/// \brief Returns true, if both vectors are not identical.
template <typename Type>
bool operator!=(const wdVec2Template<Type>& v1, const wdVec2Template<Type>& v2); // [tested]

/// \brief Strict weak ordering. Useful for sorting vertices into a map.
template <typename Type>
bool operator<(const wdVec2Template<Type>& v1, const wdVec2Template<Type>& v2);

#include <Foundation/Math/Implementation/Vec2_inl.h>
