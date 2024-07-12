#pragma once

/// \file

#include <Foundation/Time/Time.h>
#include <Foundation/Types/ArrayPtr.h>
#include <Foundation/Types/Id.h>
#include <utility>


#ifdef new
#  undef new
#endif

#ifdef delete
#  undef delete
#endif

using nsAllocatorId = nsGenericId<24, 8>;

/// \brief Base class for all memory allocators.
class NS_FOUNDATION_DLL nsAllocator
{
public:
  struct Stats
  {
    NS_DECLARE_POD_TYPE();

    nsUInt64 m_uiNumAllocations = 0;         ///< total number of allocations
    nsUInt64 m_uiNumDeallocations = 0;       ///< total number of deallocations
    nsUInt64 m_uiAllocationSize = 0;         ///< total allocation size in bytes

    nsUInt64 m_uiPerFrameAllocationSize = 0; ///< allocation size in bytes in this frame
    nsTime m_PerFrameAllocationTime;         ///< time spend on allocations in this frame
  };

  nsAllocator();
  virtual ~nsAllocator();

  /// \brief Interface, do not use this directly, always use the new/delete macros below
  virtual void* Allocate(size_t uiSize, size_t uiAlign, nsMemoryUtils::DestructorFunction destructorFunc = nullptr) = 0;
  virtual void Deallocate(void* pPtr) = 0;
  virtual void* Reallocate(void* pPtr, size_t uiCurrentSize, size_t uiNewSize, size_t uiAlign);

  /// \brief Returns the number of bytes allocated at this address.
  ///
  /// \note Careful! This information is only available, if allocation tracking is enabled!
  /// Otherwise 0 is returned.
  /// See nsAllocatorTrackingMode and NS_ALLOC_TRACKING_DEFAULT.
  virtual size_t AllocatedSize(const void* pPtr) = 0;

  virtual nsAllocatorId GetId() const = 0;
  virtual Stats GetStats() const = 0;

private:
  NS_DISALLOW_COPY_AND_ASSIGN(nsAllocator);
};

#include <Foundation/Memory/Implementation/Allocator_inl.h>

/// \brief creates a new instance of type using the given allocator
#define NS_NEW(allocator, type, ...) \
  nsInternal::NewInstance<type>(     \
    new ((allocator)->Allocate(sizeof(type), NS_ALIGNMENT_OF(type), nsMemoryUtils::MakeDestructorFunction<type>())) type(__VA_ARGS__), (allocator))

/// \brief deletes the instance stored in ptr using the given allocator and sets ptr to nullptr
#define NS_DELETE(allocator, ptr)       \
  {                                     \
    nsInternal::Delete(allocator, ptr); \
    ptr = nullptr;                      \
  }

/// \brief creates a new array of type using the given allocator with count elements, calls default constructor for non-POD types
#define NS_NEW_ARRAY(allocator, type, count) nsInternal::CreateArray<type>(allocator, count)

/// \brief Calls destructor on every element for non-POD types and deletes the array stored in arrayPtr using the given allocator
#define NS_DELETE_ARRAY(allocator, arrayPtr)      \
  {                                               \
    nsInternal::DeleteArray(allocator, arrayPtr); \
    arrayPtr.Clear();                             \
  }

/// \brief creates a raw buffer of type using the given allocator with count elements, but does NOT call the default constructor
#define NS_NEW_RAW_BUFFER(allocator, type, count) nsInternal::CreateRawBuffer<type>(allocator, count)

/// \brief deletes a raw buffer stored in ptr using the given allocator, but does NOT call destructor
#define NS_DELETE_RAW_BUFFER(allocator, ptr)     \
  {                                              \
    nsInternal::DeleteRawBuffer(allocator, ptr); \
    ptr = nullptr;                               \
  }

/// \brief extends a given raw buffer to the new size, taking care of calling constructors / assignment operators.
#define NS_EXTEND_RAW_BUFFER(allocator, ptr, oldSize, newSize) nsInternal::ExtendRawBuffer(ptr, allocator, oldSize, newSize)



/// \brief creates a new instance of type using the default allocator
#define NS_DEFAULT_NEW(type, ...) NS_NEW(nsFoundation::GetDefaultAllocator(), type, __VA_ARGS__)

/// \brief deletes the instance stored in ptr using the default allocator and sets ptr to nullptr
#define NS_DEFAULT_DELETE(ptr) NS_DELETE(nsFoundation::GetDefaultAllocator(), ptr)

/// \brief creates a new array of type using the default allocator with count elements, calls default constructor for non-POD types
#define NS_DEFAULT_NEW_ARRAY(type, count) NS_NEW_ARRAY(nsFoundation::GetDefaultAllocator(), type, count)

/// \brief calls destructor on every element for non-POD types and deletes the array stored in arrayPtr using the default allocator
#define NS_DEFAULT_DELETE_ARRAY(arrayPtr) NS_DELETE_ARRAY(nsFoundation::GetDefaultAllocator(), arrayPtr)

/// \brief creates a raw buffer of type using the default allocator with count elements, but does NOT call the default constructor
#define NS_DEFAULT_NEW_RAW_BUFFER(type, count) NS_NEW_RAW_BUFFER(nsFoundation::GetDefaultAllocator(), type, count)

/// \brief deletes a raw buffer stored in ptr using the default allocator, but does NOT call destructor
#define NS_DEFAULT_DELETE_RAW_BUFFER(ptr) NS_DELETE_RAW_BUFFER(nsFoundation::GetDefaultAllocator(), ptr)
