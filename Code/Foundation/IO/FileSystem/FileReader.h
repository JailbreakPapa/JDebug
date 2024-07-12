#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/IO/FileSystem/Implementation/FileReaderWriterBase.h>
#include <Foundation/IO/Stream.h>

/// \brief The default class to use to read data from a file, implements the nsStreamReader interface.
///
/// This file reader buffers reads up to a certain amount of bytes (configurable).
/// It closes the file automatically once it goes out of scope.
class NS_FOUNDATION_DLL nsFileReader : public nsFileReaderBase
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsFileReader);

public:
  /// \brief Constructor, does nothing.
  nsFileReader()

    = default;

  /// \brief Destructor, closes the file, if it is still open (RAII).
  ~nsFileReader() { Close(); }

  /// \brief Opens the given file for reading. Returns NS_SUCCESS if the file could be opened. A cache is created to speed up small reads.
  ///
  /// You should typically not disable bAllowFileEvents, unless you need to prevent recursive file events,
  /// which is only the case, if you are doing file accesses from within a File Event Handler.
  nsResult Open(nsStringView sFile, nsUInt32 uiCacheSize = 1024 * 64, nsFileShareMode::Enum fileShareMode = nsFileShareMode::Default, bool bAllowFileEvents = true);

  /// \brief Closes the file, if it is open.
  void Close();

  /// \brief Attempts to read the given number of bytes into the buffer. Returns the actual number of bytes read.
  virtual nsUInt64 ReadBytes(void* pReadBuffer, nsUInt64 uiBytesToRead) override;

  /// \brief Helper method to skip a number of bytes. Returns the actual number of bytes skipped.
  virtual nsUInt64 SkipBytes(nsUInt64 uiBytesToSkip) override;
  /// \brief Whether the end of the file was reached during reading.
  ///
  /// \note This is not 100% accurate, it does not guarantee that if it returns false, that the next read will return any data.
  bool IsEOF() const { return m_bEOF; }

private:
  nsUInt64 m_uiBytesCached = 0;
  nsUInt64 m_uiCacheReadPosition = 0;
  nsDynamicArray<nsUInt8> m_Cache;
  bool m_bEOF = true;
};
