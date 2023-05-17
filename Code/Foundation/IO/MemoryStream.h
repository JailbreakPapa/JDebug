#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/RefCounted.h>

class wdMemoryStreamReader;
class wdMemoryStreamWriter;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief Instances of this class act as storage for memory streams
class WD_FOUNDATION_DLL wdMemoryStreamStorageInterface
{
public:
  wdMemoryStreamStorageInterface();
  virtual ~wdMemoryStreamStorageInterface();

  /// \brief Returns the number of bytes that are currently stored. Asserts that the stored amount is less than 4GB.
  wdUInt32 GetStorageSize32() const
  {
    WD_ASSERT_ALWAYS(GetStorageSize64() <= wdMath::MaxValue<wdUInt32>(), "The memory stream storage object has grown beyond 4GB. The code using it has to be adapted to support this.");
    return (wdUInt32)GetStorageSize64();
  }

  /// \brief Returns the number of bytes that are currently stored.
  virtual wdUInt64 GetStorageSize64() const = 0; // [tested]

  /// \brief Clears the entire storage. All readers and writers must be reset to start from the beginning again.
  virtual void Clear() = 0;

  /// \brief Deallocates any allocated memory that's not needed to hold the currently stored data.
  virtual void Compact() = 0;

  /// \brief Returns the amount of bytes that are currently allocated on the heap.
  virtual wdUInt64 GetHeapMemoryUsage() const = 0;

  /// \brief Copies all data from the given stream into the storage.
  void ReadAll(wdStreamReader& inout_stream, wdUInt64 uiMaxBytes = wdMath::MaxValue<wdUInt64>());

  /// \brief Reserves N bytes of storage.
  virtual void Reserve(wdUInt64 uiBytes) = 0;

  /// \brief Writes the entire content of the storage to the provided stream.
  virtual wdResult CopyToStream(wdStreamWriter& inout_stream) const = 0;

  /// \brief Returns a read-only wdArrayPtr that represents a contiguous area in memory which starts at the given first byte.
  ///
  /// This piece of memory can be read/copied/modified in one operation (memcpy etc).
  /// The next byte after this slice may be located somewhere entirely different in memory.
  /// Call GetContiguousMemoryRange() again with the next byte after this range, to get access to the next memory area.
  ///
  /// Chunks may differ in size.
  virtual wdArrayPtr<const wdUInt8> GetContiguousMemoryRange(wdUInt64 uiStartByte) const = 0;

  /// Non-const overload of GetContiguousMemoryRange().
  virtual wdArrayPtr<wdUInt8> GetContiguousMemoryRange(wdUInt64 uiStartByte) = 0;

private:
  virtual void SetInternalSize(wdUInt64 uiSize) = 0;

  friend class wdMemoryStreamReader;
  friend class wdMemoryStreamWriter;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief Templated implementation of wdMemoryStreamStorageInterface that adapts most standard wd containers to the interface.
///
/// Note that wdMemoryStreamContainerStorage assumes contiguous storage, so using an wdDeque for storage will not work.
template <typename CONTAINER>
class wdMemoryStreamContainerStorage : public wdMemoryStreamStorageInterface
{
public:
  /// \brief Creates the storage object for a memory stream. Use \a uiInitialCapacity to reserve some memory up front.
  wdMemoryStreamContainerStorage(wdUInt32 uiInitialCapacity = 0, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator())
    : m_Storage(pAllocator)
  {
    m_Storage.Reserve(uiInitialCapacity);
  }

  virtual wdUInt64 GetStorageSize64() const override { return m_Storage.GetCount(); }
  virtual void Clear() override { m_Storage.Clear(); }
  virtual void Compact() override { m_Storage.Compact(); }
  virtual wdUInt64 GetHeapMemoryUsage() const override { return m_Storage.GetHeapMemoryUsage(); }

  virtual void Reserve(wdUInt64 uiBytes) override
  {
    WD_ASSERT_DEV(uiBytes <= wdMath::MaxValue<wdUInt32>(), "wdMemoryStreamContainerStorage only supports 32 bit addressable sizes.");
    m_Storage.Reserve(static_cast<wdUInt32>(uiBytes));
  }

  virtual wdResult CopyToStream(wdStreamWriter& inout_stream) const override
  {
    return inout_stream.WriteBytes(m_Storage.GetData(), m_Storage.GetCount());
  }

  virtual wdArrayPtr<const wdUInt8> GetContiguousMemoryRange(wdUInt64 uiStartByte) const override
  {
    if (uiStartByte >= m_Storage.GetCount())
      return {};

    return wdArrayPtr<const wdUInt8>(m_Storage.GetData() + uiStartByte, m_Storage.GetCount() - static_cast<wdUInt32>(uiStartByte));
  }

  virtual wdArrayPtr<wdUInt8> GetContiguousMemoryRange(wdUInt64 uiStartByte) override
  {
    if (uiStartByte >= m_Storage.GetCount())
      return {};

    return wdArrayPtr<wdUInt8>(m_Storage.GetData() + uiStartByte, m_Storage.GetCount() - static_cast<wdUInt32>(uiStartByte));
  }

  /// \brief The data is guaranteed to be contiguous.
  const wdUInt8* GetData() const { return &m_Storage[0]; }

private:
  virtual void SetInternalSize(wdUInt64 uiSize) override
  {
    WD_ASSERT_DEV(uiSize <= wdMath::MaxValue<wdUInt32>(), "Storage that large is not supported.");
    m_Storage.SetCountUninitialized(static_cast<wdUInt32>(uiSize));
  }

  CONTAINER m_Storage;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


/// wdContiguousMemoryStreamStorage holds internally an wdHybridArray<wdUInt8, 256>, to prevent allocations when only small temporary memory streams
/// are needed. That means it will have a memory overhead of that size.
/// Also it reallocates memory on demand, and the data is guaranteed to be contiguous. This may be desirable,
/// but can have a high performance overhead when data grows very large.
class WD_FOUNDATION_DLL wdContiguousMemoryStreamStorage : public wdMemoryStreamContainerStorage<wdHybridArray<wdUInt8, 256>>
{
public:
  wdContiguousMemoryStreamStorage(wdUInt32 uiInitialCapacity = 0, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator())
    : wdMemoryStreamContainerStorage<wdHybridArray<wdUInt8, 256>>(uiInitialCapacity, pAllocator)
  {
  }
};

/// \brief The default implementation for memory stream storage.
///
/// This implementation of wdMemoryStreamStorageInterface handles use cases both from very small to extremely large storage needs.
/// It starts out with some inplace memory that can accommodate small amounts of data.
/// To grow, additional chunks of data are allocated. No memory ever needs to be copied to grow the container.
/// However, that also means that the memory isn't stored in one contiguous array, therefore data has to be accessed piece-wise
/// through GetContiguousMemoryRange().
class WD_FOUNDATION_DLL wdDefaultMemoryStreamStorage : public wdMemoryStreamStorageInterface
{
public:
  wdDefaultMemoryStreamStorage(wdUInt32 uiInitialCapacity = 0, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());
  ~wdDefaultMemoryStreamStorage();

  virtual void Reserve(wdUInt64 uiBytes) override; // [tested]

  virtual wdUInt64 GetStorageSize64() const override; // [tested]
  virtual void Clear() override;
  virtual void Compact() override;
  virtual wdUInt64 GetHeapMemoryUsage() const override;
  virtual wdResult CopyToStream(wdStreamWriter& inout_stream) const override;
  virtual wdArrayPtr<const wdUInt8> GetContiguousMemoryRange(wdUInt64 uiStartByte) const override; // [tested]
  virtual wdArrayPtr<wdUInt8> GetContiguousMemoryRange(wdUInt64 uiStartByte) override;             // [tested]

private:
  virtual void SetInternalSize(wdUInt64 uiSize) override;

  void AddChunk(wdUInt32 uiMinimumSize);

  struct Chunk
  {
    wdUInt64 m_uiStartOffset = 0;
    wdArrayPtr<wdUInt8> m_Bytes;
  };

  wdHybridArray<Chunk, 16> m_Chunks;

  wdUInt64 m_uiCapacity = 0;
  wdUInt64 m_uiInternalSize = 0;
  wdUInt8 m_InplaceMemory[512]; // used for the very first bytes, might cover small memory streams without an allocation
  mutable wdUInt32 m_uiLastChunkAccessed = 0;
  mutable wdUInt64 m_uiLastByteAccessed = 0;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief Wrapper around an existing container to implement wdMemoryStreamStorageInterface
template <typename CONTAINER>
class wdMemoryStreamContainerWrapperStorage : public wdMemoryStreamStorageInterface
{
public:
  wdMemoryStreamContainerWrapperStorage(CONTAINER* pContainer) { m_pStorage = pContainer; }

  virtual wdUInt64 GetStorageSize64() const override { return m_pStorage->GetCount(); }
  virtual void Clear() override { m_pStorage->Clear(); }
  virtual void Compact() override { m_pStorage->Compact(); }
  virtual wdUInt64 GetHeapMemoryUsage() const override { return m_pStorage->GetHeapMemoryUsage(); }

  virtual void Reserve(wdUInt64 uiBytes) override
  {
    WD_ASSERT_DEV(uiBytes <= wdMath::MaxValue<wdUInt32>(), "wdMemoryStreamContainerWrapperStorage only supports 32 bit addressable sizes.");
    m_pStorage->Reserve(static_cast<wdUInt32>(uiBytes));
  }

  virtual wdResult CopyToStream(wdStreamWriter& inout_stream) const override
  {
    return inout_stream.WriteBytes(m_pStorage->GetData(), m_pStorage->GetCount());
  }

  virtual wdArrayPtr<const wdUInt8> GetContiguousMemoryRange(wdUInt64 uiStartByte) const override
  {
    if (uiStartByte >= m_pStorage->GetCount())
      return {};

    return wdArrayPtr<const wdUInt8>(m_pStorage->GetData() + uiStartByte, m_pStorage->GetCount() - static_cast<wdUInt32>(uiStartByte));
  }

  virtual wdArrayPtr<wdUInt8> GetContiguousMemoryRange(wdUInt64 uiStartByte) override
  {
    if (uiStartByte >= m_pStorage->GetCount())
      return {};

    return wdArrayPtr<wdUInt8>(m_pStorage->GetData() + uiStartByte, m_pStorage->GetCount() - static_cast<wdUInt32>(uiStartByte));
  }

private:
  virtual void SetInternalSize(wdUInt64 uiSize) override
  {
    WD_ASSERT_DEV(uiSize <= wdMath::MaxValue<wdUInt32>(), "wdMemoryStreamContainerWrapperStorage only supports up to 4GB sizes.");
    m_pStorage->SetCountUninitialized(static_cast<wdUInt32>(uiSize));
  }

  CONTAINER* m_pStorage;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief A reader which can access a memory stream.
///
/// Please note that the functions exposed by this object are not thread safe! If access to the same wdMemoryStreamStorage object from
/// multiple threads is desired please create one instance of wdMemoryStreamReader per thread.
class WD_FOUNDATION_DLL wdMemoryStreamReader : public wdStreamReader
{
public:
  /// \brief Pass the memory storage object from which to read from.
  /// Pass nullptr if you are going to set the storage stream later via SetStorage().
  wdMemoryStreamReader(const wdMemoryStreamStorageInterface* pStreamStorage = nullptr);

  ~wdMemoryStreamReader();

  /// \brief Sets the storage object upon which to operate. Resets the read position to zero.
  /// Pass nullptr if you want to detach from any previous storage stream, for example to ensure its reference count gets properly reduced.
  void SetStorage(const wdMemoryStreamStorageInterface* pStreamStorage)
  {
    m_pStreamStorage = pStreamStorage;
    m_uiReadPosition = 0;
  }

  /// \brief Reads either uiBytesToRead or the amount of remaining bytes in the stream into pReadBuffer.
  ///
  /// It is valid to pass nullptr for pReadBuffer, in this case the memory stream position is only advanced by the given number of bytes.
  virtual wdUInt64 ReadBytes(void* pReadBuffer, wdUInt64 uiBytesToRead) override; // [tested]

  /// \brief Skips bytes in the stream (e.g. for skipping objects which can't be serialized due to missing information etc.)
  virtual wdUInt64 SkipBytes(wdUInt64 uiBytesToSkip) override; // [tested]

  /// \brief Sets the read position to be used
  void SetReadPosition(wdUInt64 uiReadPosition); // [tested]

  /// \brief Returns the current read position
  wdUInt64 GetReadPosition() const { return m_uiReadPosition; }

  /// \brief Returns the total available bytes in the memory stream
  wdUInt32 GetByteCount32() const; // [tested]
  wdUInt64 GetByteCount64() const; // [tested]

  /// \brief Allows to set a string as the source of information in the memory stream for debug purposes.
  void SetDebugSourceInformation(wdStringView sDebugSourceInformation);

private:
  const wdMemoryStreamStorageInterface* m_pStreamStorage = nullptr;

  wdString m_sDebugSourceInformation;

  wdUInt64 m_uiReadPosition = 0;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief A writer which can access a memory stream
///
/// Please note that the functions exposed by this object are not thread safe!
class WD_FOUNDATION_DLL wdMemoryStreamWriter : public wdStreamWriter
{
public:
  /// \brief Pass the memory storage object to which to write to.
  wdMemoryStreamWriter(wdMemoryStreamStorageInterface* pStreamStorage = nullptr);

  ~wdMemoryStreamWriter();

  /// \brief Sets the storage object upon which to operate. Resets the write position to the end of the storage stream.
  /// Pass nullptr if you want to detach from any previous storage stream, for example to ensure its reference count gets properly reduced.
  void SetStorage(wdMemoryStreamStorageInterface* pStreamStorage)
  {
    m_pStreamStorage = pStreamStorage;
    m_uiWritePosition = 0;
    if (m_pStreamStorage)
      m_uiWritePosition = m_pStreamStorage->GetStorageSize64();
  }

  /// \brief Copies uiBytesToWrite from pWriteBuffer into the memory stream.
  ///
  /// pWriteBuffer must be a valid buffer and must hold that much data.
  virtual wdResult WriteBytes(const void* pWriteBuffer, wdUInt64 uiBytesToWrite) override; // [tested]

  /// \brief Sets the write position to be used
  void SetWritePosition(wdUInt64 uiWritePosition); // [tested]

  /// \brief Returns the current write position
  wdUInt64 GetWritePosition() const { return m_uiWritePosition; }

  /// \brief Returns the total stored bytes in the memory stream
  wdUInt32 GetByteCount32() const; // [tested]
  wdUInt64 GetByteCount64() const; // [tested]

private:
  wdMemoryStreamStorageInterface* m_pStreamStorage = nullptr;

  wdUInt64 m_uiWritePosition = 0;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief Maps a raw chunk of memory to the wdStreamReader interface.
class WD_FOUNDATION_DLL wdRawMemoryStreamReader : public wdStreamReader
{
public:
  wdRawMemoryStreamReader();

  /// \brief Initialize the raw memory reader with the chunk of memory that is the data storage.
  wdRawMemoryStreamReader(const void* pData, wdUInt64 uiDataSize); // [tested]

  /// \brief Initialize the raw memory reader with the chunk of memory from a standard wd container.
  /// \note The container must store the data in a contiguous array.
  template <typename CONTAINER>
  wdRawMemoryStreamReader(const CONTAINER& container) // [tested]
  {
    Reset(container);
  }

  ~wdRawMemoryStreamReader();

  void Reset(const void* pData, wdUInt64 uiDataSize); // [tested]

  template <typename CONTAINER>
  void Reset(const CONTAINER& container) // [tested]
  {
    Reset(static_cast<const wdUInt8*>(container.GetData()), container.GetCount());
  }

  /// \brief Reads either uiBytesToRead or the amount of remaining bytes in the stream into pReadBuffer.
  ///
  /// It is valid to pass nullptr for pReadBuffer, in this case the memory stream position is only advanced by the given number of bytes.
  virtual wdUInt64 ReadBytes(void* pReadBuffer, wdUInt64 uiBytesToRead) override; // [tested]

  /// \brief Skips bytes in the stream (e.g. for skipping objects which can't be serialized due to missing information etc.)
  virtual wdUInt64 SkipBytes(wdUInt64 uiBytesToSkip) override; // [tested]

  /// \brief Sets the read position to be used
  void SetReadPosition(wdUInt64 uiReadPosition); // [tested]

  /// \brief Returns the current read position in the raw memory block
  wdUInt64 GetReadPosition() const { return m_uiReadPosition; }

  /// \brief Returns the total available bytes in the memory stream
  wdUInt64 GetByteCount() const; // [tested]

  /// \brief Allows to set a string as the source of information in the memory stream for debug purposes.
  void SetDebugSourceInformation(wdStringView sDebugSourceInformation);

private:
  const wdUInt8* m_pRawMemory = nullptr;

  wdUInt64 m_uiChunkSize = 0;
  wdUInt64 m_uiReadPosition = 0;

  wdString m_sDebugSourceInformation;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


/// \brief Maps a raw chunk of memory to the wdStreamReader interface.
class WD_FOUNDATION_DLL wdRawMemoryStreamWriter : public wdStreamWriter
{
public:
  wdRawMemoryStreamWriter(); // [tested]

  /// \brief Initialize the raw memory reader with the chunk of memory that is the data storage.
  wdRawMemoryStreamWriter(void* pData, wdUInt64 uiDataSize); // [tested]

  /// \brief Initialize the raw memory reader with the chunk of memory from a standard wd container.
  /// \note The container must store the data in a contiguous array.
  template <typename CONTAINER>
  wdRawMemoryStreamWriter(CONTAINER& ref_container) // [tested]
  {
    Reset(ref_container);
  }

  ~wdRawMemoryStreamWriter(); // [tested]

  void Reset(void* pData, wdUInt64 uiDataSize); // [tested]

  template <typename CONTAINER>
  void Reset(CONTAINER& ref_container) // [tested]
  {
    Reset(static_cast<wdUInt8*>(ref_container.GetData()), ref_container.GetCount());
  }

  /// \brief Returns the total available bytes in the memory stream
  wdUInt64 GetStorageSize() const; // [tested]

  /// \brief Returns the number of bytes written to the storage
  wdUInt64 GetNumWrittenBytes() const; // [tested]

  /// \brief Allows to set a string as the source of information in the memory stream for debug purposes.
  void SetDebugSourceInformation(wdStringView sDebugSourceInformation);

  /// \brief Copies uiBytesToWrite from pWriteBuffer into the memory stream.
  ///
  /// pWriteBuffer must be a valid buffer and must hold that much data.
  virtual wdResult WriteBytes(const void* pWriteBuffer, wdUInt64 uiBytesToWrite) override; // [tested]

private:
  wdUInt8* m_pRawMemory = nullptr;

  wdUInt64 m_uiChunkSize = 0;
  wdUInt64 m_uiWritePosition = 0;

  wdString m_sDebugSourceInformation;
};
