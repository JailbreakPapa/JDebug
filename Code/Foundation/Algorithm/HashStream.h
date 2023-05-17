#pragma once

#include <Foundation/Basics.h>
#include <Foundation/IO/Stream.h>

/// \brief A stream writer that hashes the data written to it.
///
/// This stream writer allows to conveniently generate a 32 bit hash value for any kind of data.
class WD_FOUNDATION_DLL wdHashStreamWriter32 : public wdStreamWriter
{
public:
  /// \brief Pass an initial seed for the hash calculation.
  wdHashStreamWriter32(wdUInt32 uiSeed = 0);
  ~wdHashStreamWriter32();

  /// \brief Writes bytes directly to the stream.
  virtual wdResult WriteBytes(const void* pWriteBuffer, wdUInt64 uiBytesToWrite) override;

  /// \brief Returns the current hash value. You can read this at any time between write operations, or after writing is done to get the final hash
  /// value.
  wdUInt32 GetHashValue() const;

private:
  void* m_pState = nullptr;
};


/// \brief A stream writer that hashes the data written to it.
///
/// This stream writer allows to conveniently generate a 64 bit hash value for any kind of data.
class WD_FOUNDATION_DLL wdHashStreamWriter64 : public wdStreamWriter
{
public:
  /// \brief Pass an initial seed for the hash calculation.
  wdHashStreamWriter64(wdUInt64 uiSeed = 0);
  ~wdHashStreamWriter64();

  /// \brief Writes bytes directly to the stream.
  virtual wdResult WriteBytes(const void* pWriteBuffer, wdUInt64 uiBytesToWrite) override;

  /// \brief Returns the current hash value. You can read this at any time between write operations, or after writing is done to get the final hash
  /// value.
  wdUInt64 GetHashValue() const;

private:
  void* m_pState = nullptr;
};
