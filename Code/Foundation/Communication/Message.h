#pragma once

#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/UniquePtr.h>

using wdMessageId = wdUInt16;
class wdStreamWriter;
class wdStreamReader;

/// \brief Base class for all message types. Each message type has it's own id which is used to dispatch messages efficiently.
///
/// To implement a custom message type derive from wdMessage and add WD_DECLARE_MESSAGE_TYPE to the type declaration.
/// WD_IMPLEMENT_MESSAGE_TYPE needs to be added to a cpp.
/// \see wdRTTI
///
/// For the automatic cloning to work and for efficiency the messages must only contain simple data members.
/// For instance, everything that allocates internally (strings, arrays) should be avoided.
/// Instead, such objects should be located somewhere else and the message should only contain pointers to the data.
///
class WD_FOUNDATION_DLL wdMessage : public wdReflectedClass
{
  WD_ADD_DYNAMIC_REFLECTION(wdMessage, wdReflectedClass);

protected:
  explicit wdMessage(size_t messageSize)
  {
    const auto sizeOffset = (reinterpret_cast<uintptr_t>(&m_Id) - reinterpret_cast<uintptr_t>(this)) + sizeof(m_Id);
    memset((void*)wdMemoryUtils::AddByteOffset(this, sizeOffset), 0, messageSize - sizeOffset);
    m_uiSize = static_cast<wdUInt16>(messageSize);
#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
    m_uiDebugMessageRouting = 0;
#endif
  }

public:
  WD_ALWAYS_INLINE wdMessage()
    : wdMessage(sizeof(wdMessage))
  {
  }

  virtual ~wdMessage() {}

  /// \brief Derived message types can override this method to influence sorting order. Smaller keys are processed first.
  virtual wdInt32 GetSortingKey() const { return 0; }

  /// \brief Returns the id for this message type.
  WD_ALWAYS_INLINE wdMessageId GetId() const { return m_Id; }

  /// \brief Returns the size in byte of this message.
  WD_ALWAYS_INLINE wdUInt16 GetSize() const { return m_uiSize; }

  /// \brief Calculates a hash of the message.
  WD_ALWAYS_INLINE wdUInt64 GetHash() const { return wdHashingUtils::xxHash64(this, m_uiSize); }

  /// \brief Implement this for efficient transmission across process boundaries (e.g. network transfer etc.)
  ///
  /// If the message is only ever sent within the same process between nodes of the same wdWorld,
  /// this does not need to be implemented.
  ///
  /// Note that PackageForTransfer() will automatically include the wdRTTI type version into the stream
  /// and ReplicatePackedMessage() will pass this into Deserialize(). Use this if the serialization changes.
  virtual void Serialize(wdStreamWriter& inout_stream) const { WD_ASSERT_NOT_IMPLEMENTED; }

  /// \see Serialize()
  virtual void Deserialize(wdStreamReader& inout_stream, wdUInt8 uiTypeVersion) { WD_ASSERT_NOT_IMPLEMENTED; }

#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  /// set to true while debugging a message routing problem
  /// if the message is not delivered to any recipient at all, information about why that is will be written to wdLog
  WD_ALWAYS_INLINE void SetDebugMessageRouting(bool bDebug) { m_uiDebugMessageRouting = bDebug; }

  WD_ALWAYS_INLINE bool GetDebugMessageRouting() const { return m_uiDebugMessageRouting; }
#endif

protected:
  WD_ALWAYS_INLINE static wdMessageId GetNextMsgId() { return s_NextMsgId++; }

  wdMessageId m_Id;

#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  wdUInt16 m_uiSize : 15;
  wdUInt16 m_uiDebugMessageRouting : 1;
#else
  wdUInt16 m_uiSize;
#endif

  static wdMessageId s_NextMsgId;


  //////////////////////////////////////////////////////////////////////////
  // Transferring and replicating messages
  //

public:
  /// \brief Writes msg to stream in such a way that ReplicatePackedMessage() can restore it even in another process
  ///
  /// For this to work the message type has to have the Serialize and Deserialize functions implemented.
  ///
  /// \note This is NOT used by wdWorld. Within the same process messages can be dispatched more efficiently.
  static void PackageForTransfer(const wdMessage& msg, wdStreamWriter& inout_stream);

  /// \brief Restores a message that was written by PackageForTransfer()
  ///
  /// If the message type is unknown, nullptr is returned.
  /// \see PackageForTransfer()
  static wdUniquePtr<wdMessage> ReplicatePackedMessage(wdStreamReader& inout_stream);

private:
};

/// \brief Add this macro to the declaration of your custom message type.
#define WD_DECLARE_MESSAGE_TYPE(messageType, baseType)      \
private:                                                    \
  WD_ADD_DYNAMIC_REFLECTION(messageType, baseType);         \
  static wdMessageId MSG_ID;                                \
                                                            \
protected:                                                  \
  WD_ALWAYS_INLINE explicit messageType(size_t messageSize) \
    : baseType(messageSize)                                 \
  {                                                         \
    m_Id = messageType::MSG_ID;                             \
  }                                                         \
                                                            \
public:                                                     \
  static wdMessageId GetTypeMsgId()                         \
  {                                                         \
    static wdMessageId id = wdMessage::GetNextMsgId();      \
    return id;                                              \
  }                                                         \
                                                            \
  WD_ALWAYS_INLINE messageType()                            \
    : messageType(sizeof(messageType))                      \
  {                                                         \
  }

/// \brief Implements the given message type. Add this macro to a cpp outside of the type declaration.
#define WD_IMPLEMENT_MESSAGE_TYPE(messageType) wdMessageId messageType::MSG_ID = messageType::GetTypeMsgId();


/// \brief Base class for all message senders.
template <typename T>
struct wdMessageSenderBase
{
  typedef T MessageType;
};
