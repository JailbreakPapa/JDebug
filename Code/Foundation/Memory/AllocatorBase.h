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

typedef wdGenericId<24, 8> wdAllocatorId;

/// \brief Base class for all memory allocators.
class WD_FOUNDATION_DLL wdAllocatorBase
{
public:
  struct Stats
  {
    WD_DECLARE_POD_TYPE();

    wdUInt64 m_uiNumAllocations = 0;   ///< total number of allocations
    wdUInt64 m_uiNumDeallocations = 0; ///< total number of deallocations
    wdUInt64 m_uiAllocationSize = 0;   ///< total allocation size in bytes

    wdUInt64 m_uiPerFrameAllocationSize = 0; ///< allocation size in bytes in this frame
    wdTime m_PerFrameAllocationTime;         ///< time spend on allocations in this frame
  };

  wdAllocatorBase();
  virtual ~wdAllocatorBase();

  /// \brief Interface, do not use this directly, always use the new/delete macros below
  virtual void* Allocate(size_t uiSize, size_t uiAlign, wdMemoryUtils::DestructorFunction destructorFunc = nullptr) = 0;
  virtual void Deallocate(void* pPtr) = 0;
  virtual void* Reallocate(void* pPtr, size_t uiCurrentSize, size_t uiNewSize, size_t uiAlign);

  /// \brief Returns the number of bytes allocated at this address.
  /// 
  /// \note Careful! This information is only available, if allocation tracking is enabled!
  /// Otherwise 0 is returned.
  /// See wdMemoryTrackingFlags::EnableAllocationTracking and WD_USE_ALLOCATION_TRACKING.
  virtual size_t AllocatedSize(const void* pPtr) = 0;

  virtual wdAllocatorId GetId() const = 0;
  virtual Stats GetStats() const = 0;

private:
  WD_DISALLOW_COPY_AND_ASSIGN(wdAllocatorBase);
};

#include <Foundation/Memory/Implementation/AllocatorBase_inl.h>

/// \brief creates a new instance of type using the given allocator
#define WD_NEW(allocator, type, ...) \
  wdInternal::NewInstance<type>(     \
    new ((allocator)->Allocate(sizeof(type), WD_ALIGNMENT_OF(type), wdMemoryUtils::MakeDestructorFunction<type>())) type(__VA_ARGS__), (allocator))

/// \brief deletes the instance stored in ptr using the given allocator and sets ptr to nullptr
#define WD_DELETE(allocator, ptr)       \
  {                                     \
    wdInternal::Delete(allocator, ptr); \
    ptr = nullptr;                      \
  }

/// \brief creates a new array of type using the given allocator with count elements, calls default constructor for non-POD types
#define WD_NEW_ARRAY(allocator, type, count) wdInternal::CreateArray<type>(allocator, count)

/// \brief Calls destructor on every element for non-POD types and deletes the array stored in arrayPtr using the given allocator
#define WD_DELETE_ARRAY(allocator, arrayPtr)      \
  {                                               \
    wdInternal::DeleteArray(allocator, arrayPtr); \
    arrayPtr.Clear();                             \
  }

/// \brief creates a raw buffer of type using the given allocator with count elements, but does NOT call the default constructor
#define WD_NEW_RAW_BUFFER(allocator, type, count) wdInternal::CreateRawBuffer<type>(allocator, count)

/// \brief deletes a raw buffer stored in ptr using the given allocator, but does NOT call destructor
#define WD_DELETE_RAW_BUFFER(allocator, ptr)     \
  {                                              \
    wdInternal::DeleteRawBuffer(allocator, ptr); \
    ptr = nullptr;                               \
  }

/// \brief extends a given raw buffer to the new size, taking care of calling constructors / assignment operators.
#define WD_EXTEND_RAW_BUFFER(allocator, ptr, oldSize, newSize) wdInternal::ExtendRawBuffer(ptr, allocator, oldSize, newSize)



/// \brief creates a new instance of type using the default allocator
#define WD_DEFAULT_NEW(type, ...) WD_NEW(wdFoundation::GetDefaultAllocator(), type, __VA_ARGS__)

/// \brief deletes the instance stored in ptr using the default allocator and sets ptr to nullptr
#define WD_DEFAULT_DELETE(ptr) WD_DELETE(wdFoundation::GetDefaultAllocator(), ptr)

/// \brief creates a new array of type using the default allocator with count elements, calls default constructor for non-POD types
#define WD_DEFAULT_NEW_ARRAY(type, count) WD_NEW_ARRAY(wdFoundation::GetDefaultAllocator(), type, count)

/// \brief calls destructor on every element for non-POD types and deletes the array stored in arrayPtr using the default allocator
#define WD_DEFAULT_DELETE_ARRAY(arrayPtr) WD_DELETE_ARRAY(wdFoundation::GetDefaultAllocator(), arrayPtr)

/// \brief creates a raw buffer of type using the default allocator with count elements, but does NOT call the default constructor
#define WD_DEFAULT_NEW_RAW_BUFFER(type, count) WD_NEW_RAW_BUFFER(wdFoundation::GetDefaultAllocator(), type, count)

/// \brief deletes a raw buffer stored in ptr using the default allocator, but does NOT call destructor
#define WD_DEFAULT_DELETE_RAW_BUFFER(ptr) WD_DELETE_RAW_BUFFER(wdFoundation::GetDefaultAllocator(), ptr)
