#pragma once

#include <ToolsFoundation/ToolsFoundationDLL.h>

#include <Foundation/IO/Stream.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Time/Timestamp.h>
#include <Foundation/Types/Uuid.h>

#if 0 // Define to enable extensive file system profile scopes
#  define FILESYSTEM_PROFILE(szName) NS_PROFILE_SCOPE(szName)

#else
#  define FILESYSTEM_PROFILE(Name)

#endif

/// \brief Information about a single file on disk. The file might be a document or any other file found in the data directories.
struct NS_TOOLSFOUNDATION_DLL nsFileStatus
{
  enum class Status : nsUInt8
  {
    Unknown,    ///< Since the file has been tagged as 'Unknown' it has not been encountered again on disk (yet). Use internally to find stale entries in the model.
    FileLocked, ///< The file is locked, i.e. reading is currently not possible. Try again at a later date.
    Valid       ///< The file exists on disk.
  };

  nsFileStatus() = default;

  nsTimestamp m_LastModified;
  nsUInt64 m_uiHash = 0;
  nsUuid m_DocumentID; ///< If the file is linked to a document, the GUID is valid, otherwise not.
  Status m_Status = Status::Unknown;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_TOOLSFOUNDATION_DLL, nsFileStatus);

NS_ALWAYS_INLINE nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsFileStatus& value)
{
  inout_stream.WriteBytes(&value, sizeof(nsFileStatus)).IgnoreResult();
  return inout_stream;
}

NS_ALWAYS_INLINE nsStreamReader& operator>>(nsStreamReader& inout_stream, nsFileStatus& ref_value)
{
  inout_stream.ReadBytes(&ref_value, sizeof(nsFileStatus));
  return inout_stream;
}
