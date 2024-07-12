#pragma once

/// \brief A 16 bit IEEE float class. Often called "half"
///
/// This class only contains functions to convert between float and float16. It does not support any mathematical operations.
/// It is only intended for conversion, always do all mathematical operations on regular floats (or let the GPU do them on halfs).
class NS_FOUNDATION_DLL nsFloat16
{
public:
  // Means that vectors can be copied using memcpy instead of copy construction.
  NS_DECLARE_POD_TYPE();

  /// \brief Default constructor does not initialize the value.
  nsFloat16() = default;

  /// \brief Create float16 from float.
  nsFloat16(float f); // [tested]

  /// \brief Create float16 from float.
  void operator=(float f); // [tested]

  /// \brief Create float16 from raw data.
  void SetRawData(nsUInt16 uiData) { m_uiData = uiData; } // [tested]

  /// \brief Returns the raw 16 Bit data.
  nsUInt16 GetRawData() const { return m_uiData; } // [tested]

  /// \brief Convert float16 to float.
  operator float() const; // [tested]

  /// \brief Returns true, if both values are identical.
  bool operator==(const nsFloat16& c2) { return m_uiData == c2.m_uiData; } // [tested]

  /// \brief Returns true, if both values are not identical.
  bool operator!=(const nsFloat16& c2) { return m_uiData != c2.m_uiData; } // [tested]

private:
  /// Raw 16 float data.
  nsUInt16 m_uiData;
};

/// \brief A simple helper class to use half-precision floats (nsFloat16) as vectors
class NS_FOUNDATION_DLL nsFloat16Vec2
{
public:
  // Means that vectors can be copied using memcpy instead of copy construction.
  NS_DECLARE_POD_TYPE();

  nsFloat16Vec2() = default;
  nsFloat16Vec2(const nsVec2& vVec);

  void operator=(const nsVec2& vVec);
  operator nsVec2() const;

  nsFloat16 x, y;
};

/// \brief A simple helper class to use half-precision floats (nsFloat16) as vectors
class NS_FOUNDATION_DLL nsFloat16Vec3
{
public:
  // Means that vectors can be copied using memcpy instead of copy construction.
  NS_DECLARE_POD_TYPE();

  nsFloat16Vec3() = default;
  nsFloat16Vec3(const nsVec3& vVec);

  void operator=(const nsVec3& vVec);
  operator nsVec3() const;

  nsFloat16 x, y, z;
};

/// \brief A simple helper class to use half-precision floats (nsFloat16) as vectors
class NS_FOUNDATION_DLL nsFloat16Vec4
{
public:
  // Means that vectors can be copied using memcpy instead of copy construction.
  NS_DECLARE_POD_TYPE();

  nsFloat16Vec4() = default;
  nsFloat16Vec4(const nsVec4& vVec);

  void operator=(const nsVec4& vVec);
  operator nsVec4() const;

  nsFloat16 x, y, z, w;
};
