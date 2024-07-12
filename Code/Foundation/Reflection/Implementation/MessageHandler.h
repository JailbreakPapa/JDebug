#pragma once

/// \file

#include <Foundation/Basics.h>
#include <Foundation/Reflection/Implementation/RTTI.h>

class nsMessage;

/// \brief The base class for all message handlers that a type provides.
class NS_FOUNDATION_DLL nsAbstractMessageHandler
{
public:
  virtual ~nsAbstractMessageHandler() = default;

  NS_ALWAYS_INLINE void operator()(void* pInstance, nsMessage& ref_msg) { (*m_DispatchFunc)(this, pInstance, ref_msg); }

  NS_FORCE_INLINE void operator()(const void* pInstance, nsMessage& ref_msg)
  {
    NS_ASSERT_DEV(m_bIsConst, "Calling a non const message handler with a const instance.");
    (*m_ConstDispatchFunc)(this, pInstance, ref_msg);
  }

  NS_ALWAYS_INLINE nsMessageId GetMessageId() const { return m_Id; }

  NS_ALWAYS_INLINE bool IsConst() const { return m_bIsConst; }

protected:
  using DispatchFunc = void (*)(nsAbstractMessageHandler* pSelf, void* pInstance, nsMessage&);
  using ConstDispatchFunc = void (*)(nsAbstractMessageHandler* pSelf, const void* pInstance, nsMessage&);

  union
  {
    DispatchFunc m_DispatchFunc = nullptr;
    ConstDispatchFunc m_ConstDispatchFunc;
  };
  nsMessageId m_Id = nsSmallInvalidIndex;
  bool m_bIsConst = false;
};

struct nsMessageSenderInfo
{
  const char* m_szName;
  const nsRTTI* m_pMessageType;
};

namespace nsInternal
{
  template <typename Class, typename MessageType>
  struct MessageHandlerTraits
  {
    static nsCompileTimeTrueType IsConst(void (Class::*)(MessageType&) const);
    static nsCompileTimeFalseType IsConst(...);
  };

  template <bool bIsConst>
  struct MessageHandler
  {
    template <typename Class, typename MessageType, void (Class::*Method)(MessageType&)>
    class Impl : public nsAbstractMessageHandler
    {
    public:
      Impl()
      {
        m_DispatchFunc = &Dispatch;
        m_Id = MessageType::GetTypeMsgId();
        m_bIsConst = false;
      }

      static void Dispatch(nsAbstractMessageHandler* pSelf, void* pInstance, nsMessage& ref_msg)
      {
        Class* pTargetInstance = static_cast<Class*>(pInstance);
        (pTargetInstance->*Method)(static_cast<MessageType&>(ref_msg));
      }
    };
  };

  template <>
  struct MessageHandler<true>
  {
    template <typename Class, typename MessageType, void (Class::*Method)(MessageType&) const>
    class Impl : public nsAbstractMessageHandler
    {
    public:
      Impl()
      {
        m_ConstDispatchFunc = &Dispatch;
        m_Id = MessageType::GetTypeMsgId();
        m_bIsConst = true;
      }

      /// \brief Casts the given message to the type of this message handler, then passes that to the class instance.
      static void Dispatch(nsAbstractMessageHandler* pSelf, const void* pInstance, nsMessage& ref_msg)
      {
        const Class* pTargetInstance = static_cast<const Class*>(pInstance);
        (pTargetInstance->*Method)(static_cast<MessageType&>(ref_msg));
      }
    };
  };
} // namespace nsInternal

#define NS_IS_CONST_MESSAGE_HANDLER(Class, MessageType, Method) \
  (sizeof(nsInternal::MessageHandlerTraits<Class, MessageType>::IsConst(Method)) == sizeof(nsCompileTimeTrueType))
