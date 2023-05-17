#pragma once

/// \brief A 16 bit IEEE float class. Often called "half"
///
/// This class only contains functions to convert between float and float16. It does not support any mathematical operations.
/// It is only intended for conversion, always do all mathematical operations on regular floats (or let the GPU do them on halfs).
class WD_FOUNDATION_DLL wdFloat16
{
public:
  // Means that vectors can be copied using memcpy instead of copy construction.
  WD_DECLARE_POD_TYPE();

  /// \brief Default constructor does not initialize the value.
  wdFloat16() = default;

  /// \brief Create float16 from float.
  wdFloat16(float f); // [tested]

  /// \brief Create float16 from float.
  void operator=(float f); // [tested]

  /// \brief Create float16 from raw data.
  void SetRawData(wdUInt16 uiData) { m_uiData = uiData; } // [tested]

  /// \brief Returns the raw 16 Bit data.
  wdUInt16 GetRawData() const { return m_uiData; } // [tested]

  /// \brief Convert float16 to float.
  operator float() const; // [tested]

  /// \brief Returns true, if both values are identical.
  bool operator==(const wdFloat16& c2) { return m_uiData == c2.m_uiData; } // [tested]

  /// \brief Returns true, if both values are not identical.
  bool operator!=(const wdFloat16& c2) { return m_uiData != c2.m_uiData; } // [tested]

private:
  /// Raw 16 float data.
  wdUInt16 m_uiData;
};

/// \brief A simple helper class to use half-precision floats (wdFloat16) as vectors
class WD_FOUNDATION_DLL wdFloat16Vec2
{
public:
  // Means that vectors can be copied using memcpy instead of copy construction.
  WD_DECLARE_POD_TYPE();

  wdFloat16Vec2() = default;
  wdFloat16Vec2(const wdVec2& vVec);

  void operator=(const wdVec2& vVec);
  operator wdVec2() const;

  wdFloat16 x, y;
};

/// \brief A simple helper class to use half-precision floats (wdFloat16) as vectors
class WD_FOUNDATION_DLL wdFloat16Vec3
{
public:
  // Means that vectors can be copied using memcpy instead of copy construction.
  WD_DECLARE_POD_TYPE();

  wdFloat16Vec3() = default;
  wdFloat16Vec3(const wdVec3& vVec);

  void operator=(const wdVec3& vVec);
  operator wdVec3() const;

  wdFloat16 x, y, z;
};

/// \brief A simple helper class to use half-precision floats (wdFloat16) as vectors
class WD_FOUNDATION_DLL wdFloat16Vec4
{
public:
  // Means that vectors can be copied using memcpy instead of copy construction.
  WD_DECLARE_POD_TYPE();

  wdFloat16Vec4() = default;
  wdFloat16Vec4(const wdVec4& vVec);

  void operator=(const wdVec4& vVec);
  operator wdVec4() const;

  wdFloat16 x, y, z, w;
};
