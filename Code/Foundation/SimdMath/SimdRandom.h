#pragma once

#include <Foundation/SimdMath/SimdVec4u.h>

/// \brief Noise based random number generator that generates 4 pseudo random values at once.
///
/// Does not keep any internal state but relies on the user to provide different positions for each call.
/// The seed parameter can be used to further alter the noise function.
struct nsSimdRandom
{
  /// \brief Returns 4 random uint32 values at position, ie. ranging from 0 to (2 ^ 32) - 1
  static nsSimdVec4u UInt(const nsSimdVec4i& vPosition, const nsSimdVec4u& vSeed = nsSimdVec4u::MakeZero());

  /// \brief Returns 4 random float values in range [0.0 ; 1.0], ie. including zero and one
  static nsSimdVec4f FloatZeroToOne(const nsSimdVec4i& vPosition, const nsSimdVec4u& vSeed = nsSimdVec4u::MakeZero());

  /// \brief Returns 4 random float values in range [fMinValue ; fMaxValue]
  static nsSimdVec4f FloatMinMax(const nsSimdVec4i& vPosition, const nsSimdVec4f& vMinValue, const nsSimdVec4f& vMaxValue, const nsSimdVec4u& vSeed = nsSimdVec4u::MakeZero());
};

#include <Foundation/SimdMath/Implementation/SimdRandom_inl.h>
