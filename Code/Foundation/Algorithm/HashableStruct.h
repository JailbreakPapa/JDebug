
#pragma once

#include <Foundation/Basics.h>


/// \brief This class provides a base class for hashable structs (e.g. descriptor objects).
///
/// To help with this there are two parts:
///   1) memclear on initialization.
///   2) a CalculateHash() function calculating the 32 bit hash of the object.
///
/// You can make your own struct hashable by deriving from nsHashableStruct providing the type of
/// your class / struct as the template parameter.
template <typename DERIVED>
class nsHashableStruct
{
public:
  nsHashableStruct();                                       // [tested]
  nsHashableStruct(const nsHashableStruct<DERIVED>& other); // [tested]

  void operator=(const nsHashableStruct<DERIVED>& other);   // [tested]

  /// \brief Calculates the 32 bit hash of the struct and returns it
  nsUInt32 CalculateHash() const; // [tested]
};

#include <Foundation/Algorithm/Implementation/HashableStruct_inl.h>
