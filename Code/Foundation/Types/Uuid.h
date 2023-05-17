
#pragma once

#include <Foundation/Algorithm/HashingUtils.h>

class wdStreamReader;
class wdStreamWriter;

/// \brief This data type is the abstraction for 128-bit Uuid (also known as GUID) instances.
class WD_FOUNDATION_DLL wdUuid
{
public:
  WD_DECLARE_POD_TYPE();

  /// \brief Default constructor. Constructed Uuid will be invalid. [tested]
  WD_ALWAYS_INLINE wdUuid();

  /// \brief Constructs the Uuid from existing values
  WD_ALWAYS_INLINE wdUuid(wdUInt64 uiLow, wdUInt64 uiHigh)
  {
    m_uiLow = uiLow;
    m_uiHigh = uiHigh;
  }

  /// \brief Comparison operator. [tested]
  WD_ALWAYS_INLINE bool operator==(const wdUuid& other) const;

  /// \brief Comparison operator. [tested]
  WD_ALWAYS_INLINE bool operator!=(const wdUuid& other) const;

  /// \brief Comparison operator.
  WD_ALWAYS_INLINE bool operator<(const wdUuid& other) const;

  /// \brief Returns true if this is a valid Uuid.
  WD_ALWAYS_INLINE bool IsValid() const;

  /// \brief Sets the Uuid to be invalid
  WD_ALWAYS_INLINE void SetInvalid();

  /// \brief Creates a new Uuid and stores is it in this object.
  void CreateNewUuid();

  /// \brief Returns a new Uuid.
  WD_ALWAYS_INLINE static wdUuid CreateUuid();

  /// \brief Returns the internal 128 Bit of data
  void GetValues(wdUInt64& ref_uiLow, wdUInt64& ref_uiHigh) const
  {
    ref_uiHigh = m_uiHigh;
    ref_uiLow = m_uiLow;
  }

  /// \brief Creates a uuid from a string. The result is always the same for the same string.
  static wdUuid StableUuidForString(wdStringView sString);

  /// \brief Creates a uuid from an integer. The result is always the same for the same input.
  static wdUuid StableUuidForInt(wdInt64 iInt);

  /// \brief Adds the given seed value to this guid, creating a new guid. The process is reversible.
  WD_ALWAYS_INLINE void CombineWithSeed(const wdUuid& seed);

  /// \brief Subtracts the given seed from this guid, restoring the original guid.
  WD_ALWAYS_INLINE void RevertCombinationWithSeed(const wdUuid& seed);

  /// \brief Combines two guids using hashing, irreversible and order dependent.
  WD_ALWAYS_INLINE void HashCombine(const wdUuid& hash);

private:
  friend WD_FOUNDATION_DLL_FRIEND void operator>>(wdStreamReader& inout_stream, wdUuid& ref_value);
  friend WD_FOUNDATION_DLL_FRIEND void operator<<(wdStreamWriter& inout_stream, const wdUuid& value);

  wdUInt64 m_uiHigh;
  wdUInt64 m_uiLow;
};

#include <Foundation/Types/Implementation/Uuid_inl.h>
