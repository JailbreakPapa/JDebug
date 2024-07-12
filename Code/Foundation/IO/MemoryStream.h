#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/RefCounted.h>

class nsMemoryStreamReader;
class nsMemoryStreamWriter;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief Instances of this class act as storage for memory streams
class NS_FOUNDATION_DLL nsMemoryStreamStorageInterface
{
public:
  nsMemoryStreamStorageInterface();
  virtual ~nsMemoryStreamStorageInterface();

  /// \brief Returns the number of bytes that are currently stored. Asserts that the stored amount is less than 4GB.
  nsUInt32 GetStorageSize32() const
  {
    NS_ASSERT_ALWAYS(GetStorageSize64() <= nsMath::MaxValue<nsUInt32>(), "The memory stream storage object has grown beyond 4GB. The code using it has to be adapted to support this.");
    return (nsUInt32)GetStorageSize64();
  }

  /// \brief Returns the number of bytes that are currently stored.
  virtual nsUInt64 GetStorageSize64() const = 0; // [tested]

  /// \brief Clears the entire storage. All readers and writers must be reset to start from the beginning again.
  virtual void Clear() = 0;

  /// \brief Deallocates any allocated memory that's not needed to hold the currently stored data.
  virtual void Compact() = 0;

  /// \brief Returns the amount of bytes that are currently allocated on the heap.
  virtual nsUInt64 GetHeapMemoryUsage() const = 0;

  /// \brief Copies all data from the given stream into the storage.
  void ReadAll(nsStreamReader& inout_stream, nsUInt64 uiMaxBytes = nsMath::MaxValue<nsUInt64>());

  /// \brief Reserves N bytes of storage.
  virtual void Reserve(nsUInt64 uiBytes) = 0;

  /// \brief Writes the entire content of the storage to the provided stream.
  virtual nsResult CopyToStream(nsStreamWriter& inout_stream) const = 0;

  /// \brief Returns a read-only nsArrayPtr that represents a contiguous area in memory which starts at the given first byte.
  ///
  /// This piece of memory can be read/copied/modified in one operation (memcpy etc).
  /// The next byte after this slice may be located somewhere entirely different in memory.
  /// Call GetContiguousMemoryRange() again with the next byte after this range, to get access to the next memory area.
  ///
  /// Chunks may differ in size.
  virtual nsArrayPtr<const nsUInt8> GetContiguousMemoryRange(nsUInt64 uiStartByte) const = 0;

  /// Non-const overload of GetContiguousMemoryRange().
  virtual nsArrayPtr<nsUInt8> GetContiguousMemoryRange(nsUInt64 uiStartByte) = 0;

private:
  virtual void SetInternalSize(nsUInt64 uiSize) = 0;

  friend class nsMemoryStreamReader;
  friend class nsMemoryStreamWriter;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief Templated implementation of nsMemoryStreamStorageInterface that adapts most standard ns containers to the interface.
///
/// Note that nsMemoryStreamContainerStorage assumes contiguous storage, so using an nsDeque for storage will not work.
template <typename CONTAINER>
class nsMemoryStreamContainerStorage : public nsMemoryStreamStorageInterface
{
public:
  /// \brief Creates the storage object for a memory stream. Use \a uiInitialCapacity to reserve some memory up front.
  nsMemoryStreamContainerStorage(nsUInt32 uiInitialCapacity = 0, nsAllocator* pAllocator = nsFoundation::GetDefaultAllocator())
    : m_Storage(pAllocator)
  {
    m_Storage.Reserve(uiInitialCapacity);
  }

  virtual nsUInt64 GetStorageSize64() const override { return m_Storage.GetCount(); }
  virtual void Clear() override { m_Storage.Clear(); }
  virtual void Compact() override { m_Storage.Compact(); }
  virtual nsUInt64 GetHeapMemoryUsage() const override { return m_Storage.GetHeapMemoryUsage(); }

  virtual void Reserve(nsUInt64 uiBytes) override
  {
    NS_ASSERT_DEV(uiBytes <= nsMath::MaxValue<nsUInt32>(), "nsMemoryStreamContainerStorage only supports 32 bit addressable sizes.");
    m_Storage.Reserve(static_cast<nsUInt32>(uiBytes));
  }

  virtual nsResult CopyToStream(nsStreamWriter& inout_stream) const override
  {
    return inout_stream.WriteBytes(m_Storage.GetData(), m_Storage.GetCount());
  }

  virtual nsArrayPtr<const nsUInt8> GetContiguousMemoryRange(nsUInt64 uiStartByte) const override
  {
    if (uiStartByte >= m_Storage.GetCount())
      return {};

    return nsArrayPtr<const nsUInt8>(m_Storage.GetData() + uiStartByte, m_Storage.GetCount() - static_cast<nsUInt32>(uiStartByte));
  }

  virtual nsArrayPtr<nsUInt8> GetContiguousMemoryRange(nsUInt64 uiStartByte) override
  {
    if (uiStartByte >= m_Storage.GetCount())
      return {};

    return nsArrayPtr<nsUInt8>(m_Storage.GetData() + uiStartByte, m_Storage.GetCount() - static_cast<nsUInt32>(uiStartByte));
  }

  /// \brief The data is guaranteed to be contiguous.
  const nsUInt8* GetData() const { return m_Storage.GetData(); }

private:
  virtual void SetInternalSize(nsUInt64 uiSize) override
  {
    NS_ASSERT_DEV(uiSize <= nsMath::MaxValue<nsUInt32>(), "Storage that large is not supported.");
    m_Storage.SetCountUninitialized(static_cast<nsUInt32>(uiSize));
  }

  CONTAINER m_Storage;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


/// nsContiguousMemoryStreamStorage holds internally an nsHybridArray<nsUInt8, 256>, to prevent allocations when only small temporary memory streams
/// are needed. That means it will have a memory overhead of that size.
/// Also it reallocates memory on demand, and the data is guaranteed to be contiguous. This may be desirable,
/// but can have a high performance overhead when data grows very large.
class NS_FOUNDATION_DLL nsContiguousMemoryStreamStorage : public nsMemoryStreamContainerStorage<nsHybridArray<nsUInt8, 256>>
{
public:
  nsContiguousMemoryStreamStorage(nsUInt32 uiInitialCapacity = 0, nsAllocator* pAllocator = nsFoundation::GetDefaultAllocator())
    : nsMemoryStreamContainerStorage<nsHybridArray<nsUInt8, 256>>(uiInitialCapacity, pAllocator)
  {
  }
};

/// \brief The default implementation for memory stream storage.
///
/// This implementation of nsMemoryStreamStorageInterface handles use cases both from very small to extremely large storage needs.
/// It starts out with some inplace memory that can accommodate small amounts of data.
/// To grow, additional chunks of data are allocated. No memory ever needs to be copied to grow the container.
/// However, that also means that the memory isn't stored in one contiguous array, therefore data has to be accessed piece-wise
/// through GetContiguousMemoryRange().
class NS_FOUNDATION_DLL nsDefaultMemoryStreamStorage final : public nsMemoryStreamStorageInterface
{
public:
  nsDefaultMemoryStreamStorage(nsUInt32 uiInitialCapacity = 0, nsAllocator* pAllocator = nsFoundation::GetDefaultAllocator());
  ~nsDefaultMemoryStreamStorage();

  virtual void Reserve(nsUInt64 uiBytes) override;    // [tested]

  virtual nsUInt64 GetStorageSize64() const override; // [tested]
  virtual void Clear() override;
  virtual void Compact() override;
  virtual nsUInt64 GetHeapMemoryUsage() const override;
  virtual nsResult CopyToStream(nsStreamWriter& inout_stream) const override;
  virtual nsArrayPtr<const nsUInt8> GetContiguousMemoryRange(nsUInt64 uiStartByte) const override; // [tested]
  virtual nsArrayPtr<nsUInt8> GetContiguousMemoryRange(nsUInt64 uiStartByte) override;             // [tested]

private:
  virtual void SetInternalSize(nsUInt64 uiSize) override;

  void AddChunk(nsUInt32 uiMinimumSize);

  struct Chunk
  {
    nsUInt64 m_uiStartOffset = 0;
    nsArrayPtr<nsUInt8> m_Bytes;
  };

  nsHybridArray<Chunk, 16> m_Chunks;

  nsUInt64 m_uiCapacity = 0;
  nsUInt64 m_uiInternalSize = 0;
  nsUInt8 m_InplaceMemory[512]; // used for the very first bytes, might cover small memory streams without an allocation
  mutable nsUInt32 m_uiLastChunkAccessed = 0;
  mutable nsUInt64 m_uiLastByteAccessed = 0;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief Wrapper around an existing container to implement nsMemoryStreamStorageInterface
template <typename CONTAINER>
class nsMemoryStreamContainerWrapperStorage : public nsMemoryStreamStorageInterface
{
public:
  nsMemoryStreamContainerWrapperStorage(CONTAINER* pContainer) { m_pStorage = pContainer; }

  virtual nsUInt64 GetStorageSize64() const override { return m_pStorage->GetCount(); }
  virtual void Clear() override { m_pStorage->Clear(); }
  virtual void Compact() override { m_pStorage->Compact(); }
  virtual nsUInt64 GetHeapMemoryUsage() const override { return m_pStorage->GetHeapMemoryUsage(); }

  virtual void Reserve(nsUInt64 uiBytes) override
  {
    NS_ASSERT_DEV(uiBytes <= nsMath::MaxValue<nsUInt32>(), "nsMemoryStreamContainerWrapperStorage only supports 32 bit addressable sizes.");
    m_pStorage->Reserve(static_cast<nsUInt32>(uiBytes));
  }

  virtual nsResult CopyToStream(nsStreamWriter& inout_stream) const override
  {
    return inout_stream.WriteBytes(m_pStorage->GetData(), m_pStorage->GetCount());
  }

  virtual nsArrayPtr<const nsUInt8> GetContiguousMemoryRange(nsUInt64 uiStartByte) const override
  {
    if (uiStartByte >= m_pStorage->GetCount())
      return {};

    return nsArrayPtr<const nsUInt8>(m_pStorage->GetData() + uiStartByte, m_pStorage->GetCount() - static_cast<nsUInt32>(uiStartByte));
  }

  virtual nsArrayPtr<nsUInt8> GetContiguousMemoryRange(nsUInt64 uiStartByte) override
  {
    if (uiStartByte >= m_pStorage->GetCount())
      return {};

    return nsArrayPtr<nsUInt8>(m_pStorage->GetData() + uiStartByte, m_pStorage->GetCount() - static_cast<nsUInt32>(uiStartByte));
  }

private:
  virtual void SetInternalSize(nsUInt64 uiSize) override
  {
    NS_ASSERT_DEV(uiSize <= nsMath::MaxValue<nsUInt32>(), "nsMemoryStreamContainerWrapperStorage only supports up to 4GB sizes.");
    m_pStorage->SetCountUninitialized(static_cast<nsUInt32>(uiSize));
  }

  CONTAINER* m_pStorage;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief A reader which can access a memory stream.
///
/// Please note that the functions exposed by this object are not thread safe! If access to the same nsMemoryStreamStorage object from
/// multiple threads is desired please create one instance of nsMemoryStreamReader per thread.
class NS_FOUNDATION_DLL nsMemoryStreamReader : public nsStreamReader
{
public:
  /// \brief Pass the memory storage object from which to read from.
  /// Pass nullptr if you are going to set the storage stream later via SetStorage().
  nsMemoryStreamReader(const nsMemoryStreamStorageInterface* pStreamStorage = nullptr);

  ~nsMemoryStreamReader();

  /// \brief Sets the storage object upon which to operate. Resets the read position to zero.
  /// Pass nullptr if you want to detach from any previous storage stream, for example to ensure its reference count gets properly reduced.
  void SetStorage(const nsMemoryStreamStorageInterface* pStreamStorage)
  {
    m_pStreamStorage = pStreamStorage;
    m_uiReadPosition = 0;
  }

  /// \brief Reads either uiBytesToRead or the amount of remaining bytes in the stream into pReadBuffer.
  ///
  /// It is valid to pass nullptr for pReadBuffer, in this case the memory stream position is only advanced by the given number of bytes.
  virtual nsUInt64 ReadBytes(void* pReadBuffer, nsUInt64 uiBytesToRead) override; // [tested]

  /// \brief Skips bytes in the stream (e.g. for skipping objects which can't be serialized due to missing information etc.)
  virtual nsUInt64 SkipBytes(nsUInt64 uiBytesToSkip) override; // [tested]

  /// \brief Sets the read position to be used
  void SetReadPosition(nsUInt64 uiReadPosition); // [tested]

  /// \brief Returns the current read position
  nsUInt64 GetReadPosition() const { return m_uiReadPosition; }

  /// \brief Returns the total available bytes in the memory stream
  nsUInt32 GetByteCount32() const; // [tested]
  nsUInt64 GetByteCount64() const; // [tested]

  /// \brief Allows to set a string as the source of information in the memory stream for debug purposes.
  void SetDebugSourceInformation(nsStringView sDebugSourceInformation);

private:
  const nsMemoryStreamStorageInterface* m_pStreamStorage = nullptr;

  nsString m_sDebugSourceInformation;

  nsUInt64 m_uiReadPosition = 0;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief A writer which can access a memory stream
///
/// Please note that the functions exposed by this object are not thread safe!
class NS_FOUNDATION_DLL nsMemoryStreamWriter : public nsStreamWriter
{
public:
  /// \brief Pass the memory storage object to which to write to.
  nsMemoryStreamWriter(nsMemoryStreamStorageInterface* pStreamStorage = nullptr);

  ~nsMemoryStreamWriter();

  /// \brief Sets the storage object upon which to operate. Resets the write position to the end of the storage stream.
  /// Pass nullptr if you want to detach from any previous storage stream, for example to ensure its reference count gets properly reduced.
  void SetStorage(nsMemoryStreamStorageInterface* pStreamStorage)
  {
    m_pStreamStorage = pStreamStorage;
    m_uiWritePosition = 0;
    if (m_pStreamStorage)
      m_uiWritePosition = m_pStreamStorage->GetStorageSize64();
  }

  /// \brief Copies uiBytesToWrite from pWriteBuffer into the memory stream.
  ///
  /// pWriteBuffer must be a valid buffer and must hold that much data.
  virtual nsResult WriteBytes(const void* pWriteBuffer, nsUInt64 uiBytesToWrite) override; // [tested]

  /// \brief Sets the write position to be used
  void SetWritePosition(nsUInt64 uiWritePosition); // [tested]

  /// \brief Returns the current write position
  nsUInt64 GetWritePosition() const { return m_uiWritePosition; }

  /// \brief Returns the total stored bytes in the memory stream
  nsUInt32 GetByteCount32() const; // [tested]
  nsUInt64 GetByteCount64() const; // [tested]

private:
  nsMemoryStreamStorageInterface* m_pStreamStorage = nullptr;

  nsUInt64 m_uiWritePosition = 0;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief Maps a raw chunk of memory to the nsStreamReader interface.
class NS_FOUNDATION_DLL nsRawMemoryStreamReader : public nsStreamReader
{
public:
  nsRawMemoryStreamReader();

  /// \brief Initialize the raw memory reader with the chunk of memory that is the data storage.
  nsRawMemoryStreamReader(const void* pData, nsUInt64 uiDataSize); // [tested]

  /// \brief Initialize the raw memory reader with the chunk of memory from a standard ns container.
  /// \note The container must store the data in a contiguous array.
  template <typename CONTAINER>
  nsRawMemoryStreamReader(const CONTAINER& container) // [tested]
  {
    Reset(container);
  }

  ~nsRawMemoryStreamReader();

  void Reset(const void* pData, nsUInt64 uiDataSize); // [tested]

  template <typename CONTAINER>
  void Reset(const CONTAINER& container)              // [tested]
  {
    Reset(static_cast<const nsUInt8*>(container.GetData()), container.GetCount());
  }

  /// \brief Reads either uiBytesToRead or the amount of remaining bytes in the stream into pReadBuffer.
  ///
  /// It is valid to pass nullptr for pReadBuffer, in this case the memory stream position is only advanced by the given number of bytes.
  virtual nsUInt64 ReadBytes(void* pReadBuffer, nsUInt64 uiBytesToRead) override; // [tested]

  /// \brief Skips bytes in the stream (e.g. for skipping objects which can't be serialized due to missing information etc.)
  virtual nsUInt64 SkipBytes(nsUInt64 uiBytesToSkip) override; // [tested]

  /// \brief Sets the read position to be used
  void SetReadPosition(nsUInt64 uiReadPosition); // [tested]

  /// \brief Returns the current read position in the raw memory block
  nsUInt64 GetReadPosition() const { return m_uiReadPosition; }

  /// \brief Returns the total available bytes in the memory stream
  nsUInt64 GetByteCount() const; // [tested]

  /// \brief Allows to set a string as the source of information in the memory stream for debug purposes.
  void SetDebugSourceInformation(nsStringView sDebugSourceInformation);

private:
  const nsUInt8* m_pRawMemory = nullptr;

  nsUInt64 m_uiChunkSize = 0;
  nsUInt64 m_uiReadPosition = 0;

  nsString m_sDebugSourceInformation;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


/// \brief Maps a raw chunk of memory to the nsStreamReader interface.
class NS_FOUNDATION_DLL nsRawMemoryStreamWriter : public nsStreamWriter
{
public:
  nsRawMemoryStreamWriter(); // [tested]

  /// \brief Initialize the raw memory reader with the chunk of memory that is the data storage.
  nsRawMemoryStreamWriter(void* pData, nsUInt64 uiDataSize); // [tested]

  /// \brief Initialize the raw memory reader with the chunk of memory from a standard ns container.
  /// \note The container must store the data in a contiguous array.
  template <typename CONTAINER>
  nsRawMemoryStreamWriter(CONTAINER& ref_container) // [tested]
  {
    Reset(ref_container);
  }

  ~nsRawMemoryStreamWriter();                   // [tested]

  void Reset(void* pData, nsUInt64 uiDataSize); // [tested]

  template <typename CONTAINER>
  void Reset(CONTAINER& ref_container)          // [tested]
  {
    Reset(static_cast<nsUInt8*>(ref_container.GetData()), ref_container.GetCount());
  }

  /// \brief Returns the total available bytes in the memory stream
  nsUInt64 GetStorageSize() const; // [tested]

  /// \brief Returns the number of bytes written to the storage
  nsUInt64 GetNumWrittenBytes() const; // [tested]

  /// \brief Allows to set a string as the source of information in the memory stream for debug purposes.
  void SetDebugSourceInformation(nsStringView sDebugSourceInformation);

  /// \brief Copies uiBytesToWrite from pWriteBuffer into the memory stream.
  ///
  /// pWriteBuffer must be a valid buffer and must hold that much data.
  virtual nsResult WriteBytes(const void* pWriteBuffer, nsUInt64 uiBytesToWrite) override; // [tested]

private:
  nsUInt8* m_pRawMemory = nullptr;

  nsUInt64 m_uiChunkSize = 0;
  nsUInt64 m_uiWritePosition = 0;

  nsString m_sDebugSourceInformation;
};
