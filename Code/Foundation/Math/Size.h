#pragma once

#include <Foundation/Basics.h>

/// \brief A simple size class templated on the type for width and height.
///
template <typename Type>
class nsSizeTemplate
{
public:
  // Means this object can be copied using memcpy instead of copy construction.
  NS_DECLARE_POD_TYPE();

  // *** Data ***
public:
  Type width;
  Type height;

  // *** Constructors ***
public:
  /// \brief Default constructor does not initialize the data.
  nsSizeTemplate();

  /// \brief Constructor to set all values.
  nsSizeTemplate(Type width, Type height);

  // *** Common Functions ***
public:
  /// \brief Returns true if the area described by the size is non zero
  bool HasNonZeroArea() const;
};

template <typename Type>
bool operator==(const nsSizeTemplate<Type>& v1, const nsSizeTemplate<Type>& v2);

template <typename Type>
bool operator!=(const nsSizeTemplate<Type>& v1, const nsSizeTemplate<Type>& v2);

#include <Foundation/Math/Implementation/Size_inl.h>

using nsSizeU32 = nsSizeTemplate<nsUInt32>;
using nsSizeFloat = nsSizeTemplate<float>;
using nsSizeDouble = nsSizeTemplate<double>;
