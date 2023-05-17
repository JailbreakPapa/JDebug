#pragma once

#include <Foundation/Basics.h>

/// \brief A simple size class templated on the type for width and height.
///
template <typename Type>
class wdSizeTemplate
{
public:
  // Means this object can be copied using memcpy instead of copy construction.
  WD_DECLARE_POD_TYPE();

  // *** Data ***
public:
  Type width;
  Type height;

  // *** Constructors ***
public:
  /// \brief Default constructor does not initialize the data.
  wdSizeTemplate();

  /// \brief Constructor to set all values.
  wdSizeTemplate(Type width, Type height);

  // *** Common Functions ***
public:
  /// \brief Returns true if the area described by the size is non zero
  bool HasNonZeroArea() const;
};

template <typename Type>
bool operator==(const wdSizeTemplate<Type>& v1, const wdSizeTemplate<Type>& v2);

template <typename Type>
bool operator!=(const wdSizeTemplate<Type>& v1, const wdSizeTemplate<Type>& v2);

#include <Foundation/Math/Implementation/Size_inl.h>

typedef wdSizeTemplate<wdUInt32> wdSizeU32;
typedef wdSizeTemplate<float> wdSizeFloat;
typedef wdSizeTemplate<double> wdSizeDouble;
