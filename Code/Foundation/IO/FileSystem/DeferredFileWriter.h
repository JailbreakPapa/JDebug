#pragma once

#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/MemoryStream.h>

/// \brief A file writer that caches all written data and only opens and writes to the output file when everything is finished.
/// Useful to ensure that only complete files are written, or nothing at all, in case of a crash.
class WD_FOUNDATION_DLL wdDeferredFileWriter : public wdStreamWriter
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdDeferredFileWriter);

public:
  wdDeferredFileWriter();

  /// \brief Upon destruction the file is closed and thus written, unless Discard was called before.
  ~wdDeferredFileWriter() { Close().IgnoreResult(); }

  /// \brief This must be configured before anything is written to the file.
  void SetOutput(wdStringView sFileToWriteTo, bool bOnlyWriteIfDifferent = false); // [tested]

  virtual wdResult WriteBytes(const void* pWriteBuffer, wdUInt64 uiBytesToWrite) override; // [tested]

  /// \brief Upon calling this the content is written to the file specified with SetOutput().
  /// The return value is WD_FAILURE if the file could not be opened or not completely written.
  wdResult Close(); // [tested]

  /// \brief Calling this abandons the content and a later Close or destruction of the instance
  /// will no longer write anything to file.
  void Discard(); // [tested]

private:
  bool m_bOnlyWriteIfDifferent = false;
  bool m_bAlreadyClosed = false;
  wdString m_sOutputFile;
  wdDefaultMemoryStreamStorage m_Storage;
  wdMemoryStreamWriter m_Writer;
};
