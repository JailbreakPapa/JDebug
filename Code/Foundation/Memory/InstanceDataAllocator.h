#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/Blob.h>
#include <Foundation/Containers/DynamicArray.h>

/// \brief Structure to describe an instance data type.
///
/// Many resources, such as VMs, state machines and visual scripts of various types have shared state (their configuration)
/// as well as per-instance state (for their execution).
///
/// This structure describes the type of instance data used by a such a resource (or a node inside it).
/// Instance data is allocated through the nsInstanceDataAllocator.
///
/// Use the templated Fill() method to fill the desc from a data type.
struct NS_FOUNDATION_DLL nsInstanceDataDesc
{
  nsUInt32 m_uiTypeSize = 0;
  nsUInt32 m_uiTypeAlignment = 0;
  nsMemoryUtils::ConstructorFunction m_ConstructorFunction = nullptr;
  nsMemoryUtils::DestructorFunction m_DestructorFunction = nullptr;

  template <typename T>
  NS_ALWAYS_INLINE void FillFromType()
  {
    m_uiTypeSize = sizeof(T);
    m_uiTypeAlignment = NS_ALIGNMENT_OF(T);
    m_ConstructorFunction = nsMemoryUtils::MakeConstructorFunction<SkipTrivialTypes, T>();
    m_DestructorFunction = nsMemoryUtils::MakeDestructorFunction<T>();
  }
};

/// \brief Helper class to manager instance data allocation, construction and destruction
class NS_FOUNDATION_DLL nsInstanceDataAllocator
{
public:
  /// \brief Adds the given desc to internal list of data that needs to be allocated and returns the byte offset.
  [[nodiscard]] nsUInt32 AddDesc(const nsInstanceDataDesc& desc);

  /// \brief Resets all internal state.
  void ClearDescs();

  /// \brief Constructs the instance data objects, within the pre-allocated memory block.
  void Construct(nsByteBlobPtr blobPtr) const;

  /// \brief Destructs the instance data objects.
  void Destruct(nsByteBlobPtr blobPtr) const;

  /// \brief Allocates memory and constructs the instance data objects inside it. The returned nsBlob must be stored somewhere.
  [[nodiscard]] nsBlob AllocateAndConstruct() const;

  /// \brief Destructs and deallocates the instance data objects and the given memory block.
  void DestructAndDeallocate(nsBlob& ref_blob) const;

  /// \brief The total size in bytes taken up by all instance data objects that were added.
  nsUInt32 GetTotalDataSize() const { return m_uiTotalDataSize; }

  /// \brief Retrieves a void pointer to the instance data within the given blob at the given offset, or nullptr if the offset is invalid.
  NS_ALWAYS_INLINE static void* GetInstanceData(const nsByteBlobPtr& blobPtr, nsUInt32 uiOffset)
  {
    return (uiOffset != nsInvalidIndex) ? blobPtr.GetPtr() + uiOffset : nullptr;
  }

private:
  nsDynamicArray<nsInstanceDataDesc> m_Descs;
  nsUInt32 m_uiTotalDataSize = 0;
};
