#pragma once

#include <Foundation/Basics.h>
#include <Foundation/IO/Stream.h>

/// \brief A stream writer that hashes the data written to it.
///
/// This stream writer allows to conveniently generate a 32 bit hash value for any kind of data.
class NS_FOUNDATION_DLL nsHashStreamWriter32 : public nsStreamWriter
{
public:
  /// \brief Pass an initial seed for the hash calculation.
  nsHashStreamWriter32(nsUInt32 uiSeed = 0);
  ~nsHashStreamWriter32();

  /// \brief Writes bytes directly to the stream.
  virtual nsResult WriteBytes(const void* pWriteBuffer, nsUInt64 uiBytesToWrite) override;

  /// \brief Returns the current hash value. You can read this at any time between write operations, or after writing is done to get the final hash
  /// value.
  nsUInt32 GetHashValue() const;

private:
  void* m_pState = nullptr;
};


/// \brief A stream writer that hashes the data written to it.
///
/// This stream writer allows to conveniently generate a 64 bit hash value for any kind of data.
class NS_FOUNDATION_DLL nsHashStreamWriter64 : public nsStreamWriter
{
public:
  /// \brief Pass an initial seed for the hash calculation.
  nsHashStreamWriter64(nsUInt64 uiSeed = 0);
  ~nsHashStreamWriter64();

  /// \brief Writes bytes directly to the stream.
  virtual nsResult WriteBytes(const void* pWriteBuffer, nsUInt64 uiBytesToWrite) override;

  /// \brief Returns the current hash value. You can read this at any time between write operations, or after writing is done to get the final hash
  /// value.
  nsUInt64 GetHashValue() const;

private:
  void* m_pState = nullptr;
};
