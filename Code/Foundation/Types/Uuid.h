
#pragma once

#include <Foundation/Algorithm/HashingUtils.h>

class nsStreamReader;
class nsStreamWriter;

/// \brief This data type is the abstraction for 128-bit Uuid (also known as GUID) instances.
class NS_FOUNDATION_DLL nsUuid
{
public:
  NS_DECLARE_POD_TYPE();

  /// \brief Default constructor. Constructed Uuid will be invalid.
  NS_ALWAYS_INLINE nsUuid(); // [tested]

  /// \brief Constructs the Uuid from existing values
  NS_ALWAYS_INLINE nsUuid(nsUInt64 uiLow, nsUInt64 uiHigh)
  {
    m_uiLow = uiLow;
    m_uiHigh = uiHigh;
  }

  /// \brief Comparison operator. [tested]
  NS_ALWAYS_INLINE bool operator==(const nsUuid& other) const;

  /// \brief Comparison operator. [tested]
  NS_ALWAYS_INLINE bool operator!=(const nsUuid& other) const;

  /// \brief Comparison operator.
  NS_ALWAYS_INLINE bool operator<(const nsUuid& other) const;

  /// \brief Returns true if this is a valid Uuid.
  NS_ALWAYS_INLINE bool IsValid() const;

  /// \brief Returns an invalid UUID.
  [[nodiscard]] NS_ALWAYS_INLINE static nsUuid MakeInvalid() { return nsUuid(0, 0); }

  /// \brief Returns a new Uuid.
  [[nodiscard]] static nsUuid MakeUuid();

  /// \brief Returns the internal 128 Bit of data
  void GetValues(nsUInt64& ref_uiLow, nsUInt64& ref_uiHigh) const
  {
    ref_uiHigh = m_uiHigh;
    ref_uiLow = m_uiLow;
  }

  /// \brief Creates a uuid from a string. The result is always the same for the same string.
  [[nodiscard]] static nsUuid MakeStableUuidFromString(nsStringView sString);

  /// \brief Creates a uuid from an integer. The result is always the same for the same input.
  [[nodiscard]] static nsUuid MakeStableUuidFromInt(nsInt64 iInt);

  /// \brief Adds the given seed value to this guid, creating a new guid. The process is reversible.
  NS_ALWAYS_INLINE void CombineWithSeed(const nsUuid& seed);

  /// \brief Subtracts the given seed from this guid, restoring the original guid.
  NS_ALWAYS_INLINE void RevertCombinationWithSeed(const nsUuid& seed);

  /// \brief Combines two guids using hashing, irreversible and order dependent.
  NS_ALWAYS_INLINE void HashCombine(const nsUuid& hash);

private:
  friend NS_FOUNDATION_DLL_FRIEND void operator>>(nsStreamReader& inout_stream, nsUuid& ref_value);
  friend NS_FOUNDATION_DLL_FRIEND void operator<<(nsStreamWriter& inout_stream, const nsUuid& value);

  nsUInt64 m_uiHigh;
  nsUInt64 m_uiLow;
};

#include <Foundation/Types/Implementation/Uuid_inl.h>
