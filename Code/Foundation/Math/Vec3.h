#pragma once

#include <Foundation/Math/Math.h>
#include <Foundation/Math/Vec2.h>

/// \brief A 3-component vector class.
template <typename Type>
class wdVec3Template
{
public:
  // Means that vectors can be copied using memcpy instead of copy construction.
  WD_DECLARE_POD_TYPE();

  typedef Type ComponentType;

  // *** Data ***
public:
  Type x, y, z;

  // *** Constructors ***
public:
  /// \brief default-constructed vector is uninitialized (for speed)
  wdVec3Template(); // [tested]

  /// \brief Initializes the vector with x,y,z
  wdVec3Template(Type x, Type y, Type z); // [tested]

  /// \brief Initializes all 3 components with xyz
  explicit wdVec3Template(Type v); // [tested]
  // no copy-constructor and operator= since the default-generated ones will be faster

  /// \brief Returns a vector with all components set to zero.
  static wdVec3Template<Type> ZeroVector() { return wdVec3Template(0); } // [tested]
  /// \brief Returns a vector with all components set to one.
  static wdVec3Template<Type> OneVector() { return wdVec3Template(1); }

  /// \brief Returns a vector initialized to the x unit vector (1, 0, 0).
  static const wdVec3Template<Type> UnitXAxis() { return wdVec3Template(1, 0, 0); }
  /// \brief Returns a vector initialized to the y unit vector (0, 1, 0).
  static const wdVec3Template<Type> UnitYAxis() { return wdVec3Template(0, 1, 0); }
  /// \brief Returns a vector initialized to the z unit vector (0, 0, 1).
  static const wdVec3Template<Type> UnitZAxis() { return wdVec3Template(0, 0, 1); }

#if WD_ENABLED(WD_MATH_CHECK_FOR_NAN)
  void AssertNotNaN() const
  {
    WD_ASSERT_ALWAYS(!IsNaN(), "This object contains NaN values. This can happen when you forgot to initialize it before using it. Please "
                               "check that all code-paths properly initialize this object.");
  }
#endif

  // *** Conversions ***
public:
  /// \brief Returns an wdVec2Template with x and y from this vector.
  const wdVec2Template<Type> GetAsVec2() const; // [tested]

  /// \brief Returns an wdVec4Template with x,y,z from this vector and w set to the parameter.
  const wdVec4Template<Type> GetAsVec4(Type w) const; // [tested]

  /// \brief Returns an wdVec4Template with x,y,z from this vector and w set 1.
  const wdVec4Template<Type> GetAsPositionVec4() const; // [tested]

  /// \brief Returns an wdVec4Template with x,y,z from this vector and w set 0.
  const wdVec4Template<Type> GetAsDirectionVec4() const; // [tested]

  /// \brief Returns the data as an array.
  const Type* GetData() const { return &x; }

  /// \brief Returns the data as an array.
  Type* GetData() { return &x; }

  // *** Functions to set the vector to specific values ***
public:
  /// \brief Sets all 3 components to this value.
  void Set(Type xyz); // [tested]

  /// \brief Sets the vector to these values.
  void Set(Type x, Type y, Type z); // [tested]

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
  const wdVec3Template<Type> GetNormalized() const; // [tested]

  /// \brief Normalizes this vector.
  void Normalize(); // [tested]

  /// \brief Tries to normalize this vector. If the vector is too close to zero, WD_FAILURE is returned and the vector is set to the given
  /// fallback value.
  wdResult NormalizeIfNotZero(
    const wdVec3Template<Type>& vFallback = wdVec3Template(1, 0, 0), Type fEpsilon = wdMath::SmallEpsilon<Type>()); // [tested]

  /// \brief Returns, whether this vector is (0, 0, 0).
  bool IsZero() const; // [tested]

  /// \brief Returns, whether this vector is (0, 0, 0) within a given epsilon.
  bool IsZero(Type fEpsilon) const; // [tested]

  /// \brief Returns, whether the squared length of this vector is between 0.999f and 1.001f.
  bool IsNormalized(Type fEpsilon = wdMath::HugeEpsilon<Type>()) const; // [tested]

  /// \brief Returns true, if any of x, y or z is NaN
  bool IsNaN() const; // [tested]

  /// \brief Checks that all components are finite numbers.
  bool IsValid() const; // [tested]


  // *** Operators ***
public:
  /// \brief Returns the negation of this vector.
  const wdVec3Template<Type> operator-() const; // [tested]

  /// \brief Adds rhs component-wise to this vector
  void operator+=(const wdVec3Template<Type>& rhs); // [tested]

  /// \brief Subtracts rhs component-wise from this vector
  void operator-=(const wdVec3Template<Type>& rhs); // [tested]

  /// \brief Multiplies rhs component-wise to this vector
  void operator*=(const wdVec3Template<Type>& rhs);

  /// \brief Divides this vector component-wise by rhs
  void operator/=(const wdVec3Template<Type>& rhs);

  /// \brief Multiplies all components of this vector with f
  void operator*=(Type f); // [tested]

  /// \brief Divides all components of this vector by f
  void operator/=(Type f); // [tested]

  /// \brief Equality Check (bitwise)
  bool IsIdentical(const wdVec3Template<Type>& rhs) const; // [tested]

  /// \brief Equality Check with epsilon
  bool IsEqual(const wdVec3Template<Type>& rhs, Type fEpsilon) const; // [tested]


  // *** Common vector operations ***
public:
  /// \brief Returns the positive angle between *this and rhs.
  /// Both this and rhs must be normalized
  wdAngle GetAngleBetween(const wdVec3Template<Type>& rhs) const; // [tested]

  /// \brief Returns the Dot-product of the two vectors (commutative, order does not matter)
  Type Dot(const wdVec3Template<Type>& rhs) const; // [tested]

  /// \brief Returns the Cross-product of the two vectors (NOT commutative, order DOES matter)
  const wdVec3Template<Type> CrossRH(const wdVec3Template<Type>& rhs) const; // [tested]

  /// \brief Returns the component-wise minimum of *this and rhs
  const wdVec3Template<Type> CompMin(const wdVec3Template<Type>& rhs) const; // [tested]

  /// \brief Returns the component-wise maximum of *this and rhs
  const wdVec3Template<Type> CompMax(const wdVec3Template<Type>& rhs) const; // [tested]

  /// \brief Returns the component-wise clamped value of *this between low and high.
  const wdVec3Template<Type> CompClamp(const wdVec3Template<Type>& vLow, const wdVec3Template<Type>& vHigh) const; // [tested]

  /// \brief Returns the component-wise multiplication of *this and rhs
  const wdVec3Template<Type> CompMul(const wdVec3Template<Type>& rhs) const; // [tested]

  /// \brief Returns the component-wise division of *this and rhs
  const wdVec3Template<Type> CompDiv(const wdVec3Template<Type>& rhs) const; // [tested]

  /// brief Returns the component-wise absolute of *this.
  const wdVec3Template<Type> Abs() const; // [tested]


  // *** Other common operations ***
public:
  /// \brief Calculates the normal of the triangle defined by the three vertices. Vertices are assumed to be ordered counter-clockwise.
  wdResult CalculateNormal(const wdVec3Template<Type>& v1, const wdVec3Template<Type>& v2, const wdVec3Template<Type>& v3); // [tested]

  /// \brief Modifies this direction vector to be orthogonal to the given (normalized) direction vector. The result is NOT normalized.
  ///
  /// \note This function may fail, e.g. create a vector that is zero, if the given normal is parallel to the vector itself.
  ///       If you need to handle such cases, you should manually check afterwards, whether the result is zero, or cannot be normalized.
  void MakeOrthogonalTo(const wdVec3Template<Type>& vNormal); // [tested]

  /// \brief Returns some arbitrary vector orthogonal to this one. The vector is NOT normalized.
  const wdVec3Template<Type> GetOrthogonalVector() const; // [tested]

  /// \brief Returns this vector reflected at vNormal.
  const wdVec3Template<Type> GetReflectedVector(const wdVec3Template<Type>& vNormal) const; // [tested]

  /// \brief Returns this vector, refracted at vNormal, using the refraction index of the current medium and the medium it enters.
  const wdVec3Template<Type> GetRefractedVector(const wdVec3Template<Type>& vNormal, Type fRefIndex1, Type fRefIndex2) const;

  /// \brief Sets the vector to a random point inside a unit sphere (radius 1).
  static wdVec3Template<Type> CreateRandomPointInSphere(wdRandom& inout_rng); // [tested]

  /// \brief Creates a random direction vector. The vector is normalized.
  static wdVec3Template<Type> CreateRandomDirection(wdRandom& inout_rng); // [tested]

  /// \brief Creates a random vector around the x axis with a maximum deviation angle of \a maxDeviation. The vector is normalized.
  /// The deviation angle must be larger than zero.
  static wdVec3Template<Type> CreateRandomDeviationX(wdRandom& inout_rng, const wdAngle& maxDeviation); // [tested]

  /// \brief Creates a random vector around the y axis with a maximum deviation angle of \a maxDeviation. The vector is normalized.
  /// The deviation angle must be larger than zero.
  static wdVec3Template<Type> CreateRandomDeviationY(wdRandom& inout_rng, const wdAngle& maxDeviation); // [tested]

  /// \brief Creates a random vector around the z axis with a maximum deviation angle of \a maxDeviation. The vector is normalized.
  /// The deviation angle must be larger than zero.
  static wdVec3Template<Type> CreateRandomDeviationZ(wdRandom& inout_rng, const wdAngle& maxDeviation); // [tested]

  /// \brief Creates a random vector around the given normal with a maximum deviation.
  /// \note If you are going to do this many times with the same axis, rather than calling this function, instead manually
  /// do what this function does (see inline code) and only compute the quaternion once.
  static wdVec3Template<Type> CreateRandomDeviation(wdRandom& inout_rng, const wdAngle& maxDeviation, const wdVec3Template<Type>& vNormal); // [tested]
};

// *** Operators ***

template <typename Type>
const wdVec3Template<Type> operator+(const wdVec3Template<Type>& v1, const wdVec3Template<Type>& v2); // [tested]

template <typename Type>
const wdVec3Template<Type> operator-(const wdVec3Template<Type>& v1, const wdVec3Template<Type>& v2); // [tested]


template <typename Type>
const wdVec3Template<Type> operator*(Type f, const wdVec3Template<Type>& v); // [tested]

template <typename Type>
const wdVec3Template<Type> operator*(const wdVec3Template<Type>& v, Type f); // [tested]


template <typename Type>
const wdVec3Template<Type> operator/(const wdVec3Template<Type>& v, Type f); // [tested]


template <typename Type>
bool operator==(const wdVec3Template<Type>& v1, const wdVec3Template<Type>& v2); // [tested]

template <typename Type>
bool operator!=(const wdVec3Template<Type>& v1, const wdVec3Template<Type>& v2); // [tested]

/// \brief Strict weak ordering. Useful for sorting vertices into a map.
template <typename Type>
bool operator<(const wdVec3Template<Type>& v1, const wdVec3Template<Type>& v2); // [tested]

#include <Foundation/Math/Implementation/Vec3_inl.h>
