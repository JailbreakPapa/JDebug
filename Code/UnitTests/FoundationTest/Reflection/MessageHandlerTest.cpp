#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Communication/Message.h>
#include <Foundation/Reflection/Reflection.h>

#ifdef GetMessage
#  undef GetMessage
#endif

namespace
{

  struct nsMsgTest : public nsMessage
  {
    NS_DECLARE_MESSAGE_TYPE(nsMsgTest, nsMessage);
  };

  NS_IMPLEMENT_MESSAGE_TYPE(nsMsgTest);
  NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgTest, 1, nsRTTIDefaultAllocator<nsMsgTest>)
  NS_END_DYNAMIC_REFLECTED_TYPE;

  struct AddMessage : public nsMsgTest
  {
    NS_DECLARE_MESSAGE_TYPE(AddMessage, nsMsgTest);

    nsInt32 m_iValue;
  };
  NS_IMPLEMENT_MESSAGE_TYPE(AddMessage);
  NS_BEGIN_DYNAMIC_REFLECTED_TYPE(AddMessage, 1, nsRTTIDefaultAllocator<AddMessage>)
  NS_END_DYNAMIC_REFLECTED_TYPE;

  struct SubMessage : public nsMsgTest
  {
    NS_DECLARE_MESSAGE_TYPE(SubMessage, nsMsgTest);

    nsInt32 m_iValue;
  };
  NS_IMPLEMENT_MESSAGE_TYPE(SubMessage);
  NS_BEGIN_DYNAMIC_REFLECTED_TYPE(SubMessage, 1, nsRTTIDefaultAllocator<SubMessage>)
  NS_END_DYNAMIC_REFLECTED_TYPE;

  struct MulMessage : public nsMsgTest
  {
    NS_DECLARE_MESSAGE_TYPE(MulMessage, nsMsgTest);

    nsInt32 m_iValue;
  };
  NS_IMPLEMENT_MESSAGE_TYPE(MulMessage);
  NS_BEGIN_DYNAMIC_REFLECTED_TYPE(MulMessage, 1, nsRTTIDefaultAllocator<MulMessage>)
  NS_END_DYNAMIC_REFLECTED_TYPE;

  struct GetMessage : public nsMsgTest
  {
    NS_DECLARE_MESSAGE_TYPE(GetMessage, nsMsgTest);

    nsInt32 m_iValue;
  };
  NS_IMPLEMENT_MESSAGE_TYPE(GetMessage);
  NS_BEGIN_DYNAMIC_REFLECTED_TYPE(GetMessage, 1, nsRTTIDefaultAllocator<GetMessage>)
  NS_END_DYNAMIC_REFLECTED_TYPE;
} // namespace

class BaseHandler : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(BaseHandler, nsReflectedClass);

public:
  BaseHandler()

    = default;

  void OnAddMessage(AddMessage& ref_msg) { m_iValue += ref_msg.m_iValue; }

  void OnMulMessage(MulMessage& ref_msg) { m_iValue *= ref_msg.m_iValue; }

  void OnGetMessage(GetMessage& ref_msg) const { ref_msg.m_iValue = m_iValue; }

  nsInt32 m_iValue = 0;
};

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(BaseHandler, 1, nsRTTINoAllocator)
{
  NS_BEGIN_MESSAGEHANDLERS{
      NS_MESSAGE_HANDLER(AddMessage, OnAddMessage),
      NS_MESSAGE_HANDLER(MulMessage, OnMulMessage),
      NS_MESSAGE_HANDLER(GetMessage, OnGetMessage),
  } NS_END_MESSAGEHANDLERS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

class DerivedHandler : public BaseHandler
{
  NS_ADD_DYNAMIC_REFLECTION(DerivedHandler, BaseHandler);

public:
  void OnAddMessage(AddMessage& ref_msg) { m_iValue += ref_msg.m_iValue * 2; }

  void OnSubMessage(SubMessage& ref_msg) { m_iValue -= ref_msg.m_iValue; }
};

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(DerivedHandler, 1, nsRTTINoAllocator)
{
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(AddMessage, OnAddMessage),
    NS_MESSAGE_HANDLER(SubMessage, OnSubMessage),
  }
  NS_END_MESSAGEHANDLERS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

NS_CREATE_SIMPLE_TEST(Reflection, MessageHandler)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Simple Dispatch")
  {
    BaseHandler test;
    const nsRTTI* pRTTI = test.GetStaticRTTI();

    NS_TEST_BOOL(pRTTI->CanHandleMessage<AddMessage>());
    NS_TEST_BOOL(!pRTTI->CanHandleMessage<SubMessage>());
    NS_TEST_BOOL(pRTTI->CanHandleMessage<MulMessage>());
    NS_TEST_BOOL(pRTTI->CanHandleMessage<GetMessage>());

    AddMessage addMsg;
    addMsg.m_iValue = 4;
    bool handled = pRTTI->DispatchMessage(&test, addMsg);
    NS_TEST_BOOL(handled);

    NS_TEST_INT(test.m_iValue, 4);

    SubMessage subMsg;
    subMsg.m_iValue = 4;
    handled = pRTTI->DispatchMessage(&test, subMsg); // should do nothing
    NS_TEST_BOOL(!handled);

    NS_TEST_INT(test.m_iValue, 4);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Simple Dispatch const")
  {
    const BaseHandler test;
    const nsRTTI* pRTTI = test.GetStaticRTTI();

    AddMessage addMsg;
    addMsg.m_iValue = 4;
    bool handled = pRTTI->DispatchMessage(&test, addMsg);
    NS_TEST_BOOL(!handled); // should do nothing since object is const and the add message handler is non-const

    NS_TEST_INT(test.m_iValue, 0);

    GetMessage getMsg;
    getMsg.m_iValue = 12;
    handled = pRTTI->DispatchMessage(&test, getMsg);
    NS_TEST_BOOL(handled);
    NS_TEST_INT(getMsg.m_iValue, 0);

    NS_TEST_INT(test.m_iValue, 0); // object must not be modified
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Dispatch with inheritance")
  {
    DerivedHandler test;
    const nsRTTI* pRTTI = test.GetStaticRTTI();

    NS_TEST_BOOL(pRTTI->CanHandleMessage<AddMessage>());
    NS_TEST_BOOL(pRTTI->CanHandleMessage<SubMessage>());
    NS_TEST_BOOL(pRTTI->CanHandleMessage<MulMessage>());

    // message handler overridden by derived class
    AddMessage addMsg;
    addMsg.m_iValue = 4;
    bool handled = pRTTI->DispatchMessage(&test, addMsg);
    NS_TEST_BOOL(handled);

    NS_TEST_INT(test.m_iValue, 8);

    SubMessage subMsg;
    subMsg.m_iValue = 4;
    handled = pRTTI->DispatchMessage(&test, subMsg);
    NS_TEST_BOOL(handled);

    NS_TEST_INT(test.m_iValue, 4);

    // message handled by base class
    MulMessage mulMsg;
    mulMsg.m_iValue = 4;
    handled = pRTTI->DispatchMessage(&test, mulMsg);
    NS_TEST_BOOL(handled);

    NS_TEST_INT(test.m_iValue, 16);
  }
}
