#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/IO/FileSystem/Implementation/FileReaderWriterBase.h>
#include <Foundation/IO/Stream.h>

/// \brief The default class to use to read data from a file, implements the wdStreamReader interface.
///
/// This file reader buffers reads up to a certain amount of bytes (configurable).
/// It closes the file automatically once it goes out of scope.
class WD_FOUNDATION_DLL wdFileReader : public wdFileReaderBase
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdFileReader);

public:
  /// \brief Constructor, does nothing.
  wdFileReader()
    : m_uiBytesCached(0)
    , m_uiCacheReadPosition(0)
    , m_bEOF(true)
  {
  }

  /// \brief Destructor, closes the file, if it is still open (RAII).
  ~wdFileReader() { Close(); }

  /// \brief Opens the given file for reading. Returns WD_SUCCESS if the file could be opened. A cache is created to speed up small reads.
  ///
  /// You should typically not disable bAllowFileEvents, unless you need to prevent recursive file events,
  /// which is only the case, if you are doing file accesses from within a File Event Handler.
  wdResult Open(wdStringView sFile, wdUInt32 uiCacheSize = 1024 * 64, wdFileShareMode::Enum fileShareMode = wdFileShareMode::Default,
    bool bAllowFileEvents = true);

  /// \brief Closes the file, if it is open.
  void Close();

  /// \brief Attempts to read the given number of bytes into the buffer. Returns the actual number of bytes read.
  virtual wdUInt64 ReadBytes(void* pReadBuffer, wdUInt64 uiBytesToRead) override;

private:
  wdUInt64 m_uiBytesCached;
  wdUInt64 m_uiCacheReadPosition;
  wdDynamicArray<wdUInt8> m_Cache;
  bool m_bEOF;
};
