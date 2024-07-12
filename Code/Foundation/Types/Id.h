#pragma once

/// \file

#include <Foundation/Basics.h>

/// \brief Declares an id type, see generic id below how to use this
#define NS_DECLARE_ID_TYPE(name, instanceIndexBits, generationBits)                                        \
  static const StorageType MAX_INSTANCES = (1ULL << instanceIndexBits);                                    \
  static const StorageType INVALID_INSTANCE_INDEX = MAX_INSTANCES - 1;                                     \
  static const StorageType INDEX_AND_GENERATION_MASK = (1ULL << (instanceIndexBits + generationBits)) - 1; \
  NS_DECLARE_POD_TYPE();                                                                                   \
  NS_ALWAYS_INLINE name()                                                                                  \
  {                                                                                                        \
    m_Data = INVALID_INSTANCE_INDEX;                                                                       \
  }                                                                                                        \
  NS_ALWAYS_INLINE explicit name(StorageType internalData)                                                 \
  {                                                                                                        \
    m_Data = internalData;                                                                                 \
  }                                                                                                        \
  NS_ALWAYS_INLINE bool operator==(const name other) const                                                 \
  {                                                                                                        \
    return m_Data == other.m_Data;                                                                         \
  }                                                                                                        \
  NS_ALWAYS_INLINE bool operator!=(const name other) const                                                 \
  {                                                                                                        \
    return m_Data != other.m_Data;                                                                         \
  }                                                                                                        \
  NS_ALWAYS_INLINE bool operator<(const name other) const                                                  \
  {                                                                                                        \
    return m_Data < other.m_Data;                                                                          \
  }                                                                                                        \
  NS_ALWAYS_INLINE void Invalidate()                                                                       \
  {                                                                                                        \
    m_Data = INVALID_INSTANCE_INDEX;                                                                       \
  }                                                                                                        \
  NS_ALWAYS_INLINE bool IsInvalidated() const                                                              \
  {                                                                                                        \
    return m_Data == INVALID_INSTANCE_INDEX;                                                               \
  }                                                                                                        \
  NS_ALWAYS_INLINE bool IsIndexAndGenerationEqual(const name other) const                                  \
  {                                                                                                        \
    return (m_Data & INDEX_AND_GENERATION_MASK) == (other.m_Data & INDEX_AND_GENERATION_MASK);             \
  }


/// \brief A generic id class that holds an id combined of an instance index and a generation counter.
///
/// \todo Document this better.
template <nsUInt32 InstanceIndexBits, nsUInt32 GenerationBits>
struct nsGenericId
{
  enum
  {
    STORAGE_SIZE = ((InstanceIndexBits + GenerationBits - 1) / 8) + 1
  };
  using StorageType = typename nsSizeToType<STORAGE_SIZE>::Type;

  NS_DECLARE_ID_TYPE(nsGenericId, InstanceIndexBits, GenerationBits);

  NS_ALWAYS_INLINE nsGenericId(StorageType instanceIndex, StorageType generation)
  {
    m_Data = 0;
    m_InstanceIndex = instanceIndex;
    m_Generation = generation;
  }

  union
  {
    StorageType m_Data;
    struct
    {
      StorageType m_InstanceIndex : InstanceIndexBits;
      StorageType m_Generation : GenerationBits;
    };
  };
};

#define NS_DECLARE_HANDLE_TYPE(name, idType)               \
public:                                                    \
  NS_DECLARE_POD_TYPE();                                   \
  NS_ALWAYS_INLINE name() {}                               \
  NS_ALWAYS_INLINE explicit name(idType internalId)        \
    : m_InternalId(internalId)                             \
  {                                                        \
  }                                                        \
  NS_ALWAYS_INLINE bool operator==(const name other) const \
  {                                                        \
    return m_InternalId == other.m_InternalId;             \
  }                                                        \
  NS_ALWAYS_INLINE bool operator!=(const name other) const \
  {                                                        \
    return m_InternalId != other.m_InternalId;             \
  }                                                        \
  NS_ALWAYS_INLINE bool operator<(const name other) const  \
  {                                                        \
    return m_InternalId < other.m_InternalId;              \
  }                                                        \
  NS_ALWAYS_INLINE void Invalidate()                       \
  {                                                        \
    m_InternalId.Invalidate();                             \
  }                                                        \
  NS_ALWAYS_INLINE bool IsInvalidated() const              \
  {                                                        \
    return m_InternalId.IsInvalidated();                   \
  }                                                        \
  NS_ALWAYS_INLINE idType GetInternalID() const            \
  {                                                        \
    return m_InternalId;                                   \
  }                                                        \
  using IdType = idType;                                   \
                                                           \
protected:                                                 \
  idType m_InternalId;                                     \
  operator idType()                                        \
  {                                                        \
    return m_InternalId;                                   \
  }                                                        \
  operator const idType() const                            \
  {                                                        \
    return m_InternalId;                                   \
  }
