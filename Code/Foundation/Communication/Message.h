#pragma once

#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/UniquePtr.h>

using nsMessageId = nsUInt16;
class nsStreamWriter;
class nsStreamReader;

/// \brief Base class for all message types. Each message type has it's own id which is used to dispatch messages efficiently.
///
/// To implement a custom message type derive from nsMessage and add NS_DECLARE_MESSAGE_TYPE to the type declaration.
/// NS_IMPLEMENT_MESSAGE_TYPE needs to be added to a cpp.
/// \see nsRTTI
///
/// For the automatic cloning to work and for efficiency the messages must only contain simple data members.
/// For instance, everything that allocates internally (strings, arrays) should be avoided.
/// Instead, such objects should be located somewhere else and the message should only contain pointers to the data.
///
class NS_FOUNDATION_DLL nsMessage : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsMessage, nsReflectedClass);

protected:
  explicit nsMessage(size_t messageSize)
  {
    const auto sizeOffset = (reinterpret_cast<uintptr_t>(&m_Id) - reinterpret_cast<uintptr_t>(this)) + sizeof(m_Id);
    memset((void*)nsMemoryUtils::AddByteOffset(this, sizeOffset), 0, messageSize - sizeOffset);
    m_uiSize = static_cast<nsUInt16>(messageSize);
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    m_uiDebugMessageRouting = 0;
#endif
  }

public:
  NS_ALWAYS_INLINE nsMessage()
    : nsMessage(sizeof(nsMessage))
  {
  }

  virtual ~nsMessage() = default;

  /// \brief Derived message types can override this method to influence sorting order. Smaller keys are processed first.
  virtual nsInt32 GetSortingKey() const { return 0; }

  /// \brief Returns the id for this message type.
  NS_ALWAYS_INLINE nsMessageId GetId() const { return m_Id; }

  /// \brief Returns the size in byte of this message.
  NS_ALWAYS_INLINE nsUInt16 GetSize() const { return m_uiSize; }

  /// \brief Calculates a hash of the message.
  NS_ALWAYS_INLINE nsUInt64 GetHash() const { return nsHashingUtils::xxHash64(this, m_uiSize); }

  /// \brief Implement this for efficient transmission across process boundaries (e.g. network transfer etc.)
  ///
  /// If the message is only ever sent within the same process between nodes of the same nsWorld,
  /// this does not need to be implemented.
  ///
  /// Note that PackageForTransfer() will automatically include the nsRTTI type version into the stream
  /// and ReplicatePackedMessage() will pass this into Deserialize(). Use this if the serialization changes.
  virtual void Serialize(nsStreamWriter& inout_stream) const { NS_ASSERT_NOT_IMPLEMENTED; }

  /// \see Serialize()
  virtual void Deserialize(nsStreamReader& inout_stream, nsUInt8 uiTypeVersion) { NS_ASSERT_NOT_IMPLEMENTED; }

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  /// set to true while debugging a message routing problem
  /// if the message is not delivered to any recipient at all, information about why that is will be written to nsLog
  NS_ALWAYS_INLINE void SetDebugMessageRouting(bool bDebug) { m_uiDebugMessageRouting = bDebug; }

  NS_ALWAYS_INLINE bool GetDebugMessageRouting() const { return m_uiDebugMessageRouting; }
#endif

protected:
  NS_ALWAYS_INLINE static nsMessageId GetNextMsgId() { return s_NextMsgId++; }

  nsMessageId m_Id;

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  nsUInt16 m_uiSize : 15;
  nsUInt16 m_uiDebugMessageRouting : 1;
#else
  nsUInt16 m_uiSize;
#endif

  static nsMessageId s_NextMsgId;


  //////////////////////////////////////////////////////////////////////////
  // Transferring and replicating messages
  //

public:
  /// \brief Writes msg to stream in such a way that ReplicatePackedMessage() can restore it even in another process
  ///
  /// For this to work the message type has to have the Serialize and Deserialize functions implemented.
  ///
  /// \note This is NOT used by nsWorld. Within the same process messages can be dispatched more efficiently.
  static void PackageForTransfer(const nsMessage& msg, nsStreamWriter& inout_stream);

  /// \brief Restores a message that was written by PackageForTransfer()
  ///
  /// If the message type is unknown, nullptr is returned.
  /// \see PackageForTransfer()
  static nsUniquePtr<nsMessage> ReplicatePackedMessage(nsStreamReader& inout_stream);

private:
};

/// \brief Add this macro to the declaration of your custom message type.
#define NS_DECLARE_MESSAGE_TYPE(messageType, baseType)      \
private:                                                    \
  NS_ADD_DYNAMIC_REFLECTION(messageType, baseType);         \
  static nsMessageId MSG_ID;                                \
                                                            \
protected:                                                  \
  NS_ALWAYS_INLINE explicit messageType(size_t messageSize) \
    : baseType(messageSize)                                 \
  {                                                         \
    m_Id = messageType::MSG_ID;                             \
  }                                                         \
                                                            \
public:                                                     \
  static nsMessageId GetTypeMsgId()                         \
  {                                                         \
    static nsMessageId id = nsMessage::GetNextMsgId();      \
    return id;                                              \
  }                                                         \
                                                            \
  NS_ALWAYS_INLINE messageType()                            \
    : messageType(sizeof(messageType))                      \
  {                                                         \
  }

/// \brief Implements the given message type. Add this macro to a cpp outside of the type declaration.
#define NS_IMPLEMENT_MESSAGE_TYPE(messageType) nsMessageId messageType::MSG_ID = messageType::GetTypeMsgId();


/// \brief Base class for all message senders.
template <typename T>
struct nsMessageSenderBase
{
  using MessageType = T;
};
