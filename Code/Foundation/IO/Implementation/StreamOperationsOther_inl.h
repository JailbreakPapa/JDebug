#pragma once

/// \brief Operator to serialize wdIAllocator::Stats objects.
WD_FOUNDATION_DLL void operator<<(wdStreamWriter& inout_stream, const wdAllocatorBase::Stats& rhs);

/// \brief Operator to serialize wdIAllocator::Stats objects.
WD_FOUNDATION_DLL void operator>>(wdStreamReader& inout_stream, wdAllocatorBase::Stats& rhs);

struct wdTime;

/// \brief Operator to serialize wdTime objects.
WD_FOUNDATION_DLL void operator<<(wdStreamWriter& inout_stream, wdTime value);

/// \brief Operator to serialize wdTime objects.
WD_FOUNDATION_DLL void operator>>(wdStreamReader& inout_stream, wdTime& ref_value);


class wdUuid;

/// \brief Operator to serialize wdUuid objects. [tested]
WD_FOUNDATION_DLL void operator<<(wdStreamWriter& inout_stream, const wdUuid& value);

/// \brief Operator to serialize wdUuid objects. [tested]
WD_FOUNDATION_DLL void operator>>(wdStreamReader& inout_stream, wdUuid& ref_value);

class wdHashedString;

/// \brief Operator to serialize wdHashedString objects. [tested]
WD_FOUNDATION_DLL void operator<<(wdStreamWriter& inout_stream, const wdHashedString& sValue);

/// \brief Operator to serialize wdHashedString objects. [tested]
WD_FOUNDATION_DLL void operator>>(wdStreamReader& inout_stream, wdHashedString& ref_sValue);

class wdTempHashedString;

/// \brief Operator to serialize wdHashedString objects.
WD_FOUNDATION_DLL void operator<<(wdStreamWriter& inout_stream, const wdTempHashedString& sValue);

/// \brief Operator to serialize wdHashedString objects.
WD_FOUNDATION_DLL void operator>>(wdStreamReader& inout_stream, wdTempHashedString& ref_sValue);

class wdVariant;

/// \brief Operator to serialize wdVariant objects.
WD_FOUNDATION_DLL void operator<<(wdStreamWriter& inout_stream, const wdVariant& value);

/// \brief Operator to serialize wdVariant objects.
WD_FOUNDATION_DLL void operator>>(wdStreamReader& inout_stream, wdVariant& ref_value);

class wdTimestamp;

/// \brief Operator to serialize wdTimestamp objects.
WD_FOUNDATION_DLL void operator<<(wdStreamWriter& inout_stream, wdTimestamp value);

/// \brief Operator to serialize wdTimestamp objects.
WD_FOUNDATION_DLL void operator>>(wdStreamReader& inout_stream, wdTimestamp& ref_value);

struct wdVarianceTypeFloat;

/// \brief Operator to serialize wdTimestamp objects.
WD_FOUNDATION_DLL void operator<<(wdStreamWriter& inout_stream, const wdVarianceTypeFloat& value);

/// \brief Operator to serialize wdTimestamp objects.
WD_FOUNDATION_DLL void operator>>(wdStreamReader& inout_stream, wdVarianceTypeFloat& ref_value);

struct wdVarianceTypeTime;

/// \brief Operator to serialize wdTimestamp objects.
WD_FOUNDATION_DLL void operator<<(wdStreamWriter& inout_stream, const wdVarianceTypeTime& value);

/// \brief Operator to serialize wdTimestamp objects.
WD_FOUNDATION_DLL void operator>>(wdStreamReader& inout_stream, wdVarianceTypeTime& ref_value);

struct wdVarianceTypeAngle;

/// \brief Operator to serialize wdTimestamp objects.
WD_FOUNDATION_DLL void operator<<(wdStreamWriter& inout_stream, const wdVarianceTypeAngle& value);

/// \brief Operator to serialize wdTimestamp objects.
WD_FOUNDATION_DLL void operator>>(wdStreamReader& inout_stream, wdVarianceTypeAngle& ref_value);
