#pragma once

/// \brief Operator to serialize nsIAllocator::Stats objects.
NS_FOUNDATION_DLL void operator<<(nsStreamWriter& inout_stream, const nsAllocator::Stats& rhs);

/// \brief Operator to serialize nsIAllocator::Stats objects.
NS_FOUNDATION_DLL void operator>>(nsStreamReader& inout_stream, nsAllocator::Stats& rhs);

struct nsTime;

/// \brief Operator to serialize nsTime objects.
NS_FOUNDATION_DLL void operator<<(nsStreamWriter& inout_stream, nsTime value);

/// \brief Operator to serialize nsTime objects.
NS_FOUNDATION_DLL void operator>>(nsStreamReader& inout_stream, nsTime& ref_value);


class nsUuid;

/// \brief Operator to serialize nsUuid objects. [tested]
NS_FOUNDATION_DLL void operator<<(nsStreamWriter& inout_stream, const nsUuid& value);

/// \brief Operator to serialize nsUuid objects. [tested]
NS_FOUNDATION_DLL void operator>>(nsStreamReader& inout_stream, nsUuid& ref_value);

class nsHashedString;

/// \brief Operator to serialize nsHashedString objects. [tested]
NS_FOUNDATION_DLL void operator<<(nsStreamWriter& inout_stream, const nsHashedString& sValue);

/// \brief Operator to serialize nsHashedString objects. [tested]
NS_FOUNDATION_DLL void operator>>(nsStreamReader& inout_stream, nsHashedString& ref_sValue);

class nsTempHashedString;

/// \brief Operator to serialize nsHashedString objects.
NS_FOUNDATION_DLL void operator<<(nsStreamWriter& inout_stream, const nsTempHashedString& sValue);

/// \brief Operator to serialize nsHashedString objects.
NS_FOUNDATION_DLL void operator>>(nsStreamReader& inout_stream, nsTempHashedString& ref_sValue);

class nsVariant;

/// \brief Operator to serialize nsVariant objects.
NS_FOUNDATION_DLL void operator<<(nsStreamWriter& inout_stream, const nsVariant& value);

/// \brief Operator to serialize nsVariant objects.
NS_FOUNDATION_DLL void operator>>(nsStreamReader& inout_stream, nsVariant& ref_value);

class nsTimestamp;

/// \brief Operator to serialize nsTimestamp objects.
NS_FOUNDATION_DLL void operator<<(nsStreamWriter& inout_stream, nsTimestamp value);

/// \brief Operator to serialize nsTimestamp objects.
NS_FOUNDATION_DLL void operator>>(nsStreamReader& inout_stream, nsTimestamp& ref_value);

struct nsVarianceTypeFloat;

/// \brief Operator to serialize nsTimestamp objects.
NS_FOUNDATION_DLL void operator<<(nsStreamWriter& inout_stream, const nsVarianceTypeFloat& value);

/// \brief Operator to serialize nsTimestamp objects.
NS_FOUNDATION_DLL void operator>>(nsStreamReader& inout_stream, nsVarianceTypeFloat& ref_value);

struct nsVarianceTypeTime;

/// \brief Operator to serialize nsTimestamp objects.
NS_FOUNDATION_DLL void operator<<(nsStreamWriter& inout_stream, const nsVarianceTypeTime& value);

/// \brief Operator to serialize nsTimestamp objects.
NS_FOUNDATION_DLL void operator>>(nsStreamReader& inout_stream, nsVarianceTypeTime& ref_value);

struct nsVarianceTypeAngle;

/// \brief Operator to serialize nsTimestamp objects.
NS_FOUNDATION_DLL void operator<<(nsStreamWriter& inout_stream, const nsVarianceTypeAngle& value);

/// \brief Operator to serialize nsTimestamp objects.
NS_FOUNDATION_DLL void operator>>(nsStreamReader& inout_stream, nsVarianceTypeAngle& ref_value);
