#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Time/Timestamp.h>

/// \brief This class represents a set of files of which one wants to know when any one of them changes.
///
/// wdDependencyFile stores a list of files that are the 'dependency set'. It can be serialized.
/// Through HasAnyFileChanged() one can detect whether any of the files has changed, since the last call to StoreCurrentTimeStamp().
/// The time stamp that is retrieved through StoreCurrentTimeStamp() will also be serialized.
class WD_FOUNDATION_DLL wdDependencyFile
{
public:
  wdDependencyFile();

  /// \brief Clears all files that were added with AddFileDependency()
  void Clear();

  /// \brief Adds one file as a dependency to the list
  void AddFileDependency(wdStringView sFile);

  /// \brief Allows read access to all currently stored file dependencies
  const wdHybridArray<wdString, 16>& GetFileDependencies() const { return m_AssetTransformDependencies; }

  /// \brief Writes the current state to a stream. Note that you probably should call StoreCurrentTimeStamp() before this, to serialize the latest
  /// file stamp
  wdResult WriteDependencyFile(wdStreamWriter& inout_stream) const;

  /// \brief Reads the state from a stream. Call HasAnyFileChanged() afterwards to determine whether anything has changed since when the data was
  /// serialized.
  wdResult ReadDependencyFile(wdStreamReader& inout_stream);

  /// \brief Writes the current state to a file. Note that you probably should call StoreCurrentTimeStamp() before this, to serialize the latest file
  /// stamp
  wdResult WriteDependencyFile(wdStringView sFile) const;

  /// \brief Reads the state from a file. Call HasAnyFileChanged() afterwards to determine whether anything has changed since when the data was
  /// serialized.
  wdResult ReadDependencyFile(wdStringView sFile);

  /// \brief Retrieves the current file time stamps from the filesystem and determines whether any file has changed since the last call to
  /// StoreCurrentTimeStamp() (or ReadDependencyFile())
  bool HasAnyFileChanged() const;

  /// \brief Retrieves the current file time stamps from the filesystem and stores it for later comparison. This value is also serialized through
  /// WriteDependencyFile(), so it should be called before that, to store the latest state.
  void StoreCurrentTimeStamp();

private:
  static wdResult RetrieveFileTimeStamp(wdStringView sFile, wdTimestamp& out_Result);

  wdHybridArray<wdString, 16> m_AssetTransformDependencies;
  wdInt64 m_iMaxTimeStampStored = 0;
  wdUInt64 m_uiSumTimeStampStored = 0;

  struct FileCheckCache
  {
    wdTimestamp m_FileTimestamp;
    wdTime m_LastCheck;
  };

  static wdMap<wdString, FileCheckCache> s_FileTimestamps;
};
