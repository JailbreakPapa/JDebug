#pragma once

/// \file

#include <Foundation/Basics.h>
#include <Foundation/Reflection/Implementation/RTTI.h>

class wdMessage;

/// \brief The base class for all message handlers that a type provides.
class WD_FOUNDATION_DLL wdAbstractMessageHandler
{
public:
  WD_ALWAYS_INLINE void operator()(void* pInstance, wdMessage& ref_msg) { (*m_DispatchFunc)(pInstance, ref_msg); }

  WD_FORCE_INLINE void operator()(const void* pInstance, wdMessage& ref_msg)
  {
    WD_ASSERT_DEV(m_bIsConst, "Calling a non const message handler with a const instance.");
    (*m_ConstDispatchFunc)(pInstance, ref_msg);
  }

  WD_ALWAYS_INLINE wdMessageId GetMessageId() const { return m_Id; }

  WD_ALWAYS_INLINE bool IsConst() const { return m_bIsConst; }

protected:
  using DispatchFunc = void (*)(void*, wdMessage&);
  using ConstDispatchFunc = void (*)(const void*, wdMessage&);

  union
  {
    DispatchFunc m_DispatchFunc;
    ConstDispatchFunc m_ConstDispatchFunc;
  };
  wdMessageId m_Id;
  bool m_bIsConst;
};

struct wdMessageSenderInfo
{
  const char* m_szName;
  const wdRTTI* m_pMessageType;
};

namespace wdInternal
{
  template <typename Class, typename MessageType>
  struct MessageHandlerTraits
  {
    static wdCompileTimeTrueType IsConst(void (Class::*)(MessageType&) const);
    static wdCompileTimeFalseType IsConst(...);
  };

  template <bool bIsConst>
  struct MessageHandler
  {
    template <typename Class, typename MessageType, void (Class::*Method)(MessageType&)>
    class Impl : public wdAbstractMessageHandler
    {
    public:
      Impl()
      {
        m_DispatchFunc = &Dispatch;
        m_Id = MessageType::GetTypeMsgId();
        m_bIsConst = false;
      }

      static void Dispatch(void* pInstance, wdMessage& ref_msg)
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
    class Impl : public wdAbstractMessageHandler
    {
    public:
      Impl()
      {
        m_ConstDispatchFunc = &Dispatch;
        m_Id = MessageType::GetTypeMsgId();
        m_bIsConst = true;
      }

      /// \brief Casts the given message to the type of this message handler, then passes that to the class instance.
      static void Dispatch(const void* pInstance, wdMessage& ref_msg)
      {
        const Class* pTargetInstance = static_cast<const Class*>(pInstance);
        (pTargetInstance->*Method)(static_cast<MessageType&>(ref_msg));
      }
    };
  };
} // namespace wdInternal

#define WD_IS_CONST_MESSAGE_HANDLER(Class, MessageType, Method) \
  (sizeof(wdInternal::MessageHandlerTraits<Class, MessageType>::IsConst(Method)) == sizeof(wdCompileTimeTrueType))
