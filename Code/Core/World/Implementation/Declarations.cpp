#include <Core/CorePCH.h>

#include <Core/Messages/DeleteObjectMessage.h>
#include <Core/World/World.h>
#include <Foundation/Types/VariantTypeRegistry.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsGameObjectHandle, nsNoBase, 1, nsRTTIDefaultAllocator<nsGameObjectHandle>)
NS_END_STATIC_REFLECTED_TYPE;
NS_DEFINE_CUSTOM_VARIANT_TYPE(nsGameObjectHandle);

NS_BEGIN_STATIC_REFLECTED_TYPE(nsComponentHandle, nsNoBase, 1, nsRTTIDefaultAllocator<nsComponentHandle>)
NS_END_STATIC_REFLECTED_TYPE;
NS_DEFINE_CUSTOM_VARIANT_TYPE(nsComponentHandle);

NS_BEGIN_STATIC_REFLECTED_ENUM(nsObjectMode, 1)
  NS_ENUM_CONSTANTS(nsObjectMode::Automatic, nsObjectMode::ForceDynamic)
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_STATIC_REFLECTED_ENUM(nsOnComponentFinishedAction, 1)
  NS_ENUM_CONSTANTS(nsOnComponentFinishedAction::None, nsOnComponentFinishedAction::DeleteComponent, nsOnComponentFinishedAction::DeleteGameObject)
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_STATIC_REFLECTED_ENUM(nsOnComponentFinishedAction2, 1)
  NS_ENUM_CONSTANTS(nsOnComponentFinishedAction2::None, nsOnComponentFinishedAction2::DeleteComponent, nsOnComponentFinishedAction2::DeleteGameObject, nsOnComponentFinishedAction2::Restart)
NS_END_STATIC_REFLECTED_ENUM;
// clang-format on

//////////////////////////////////////////////////////////////////////////

void operator<<(nsStreamWriter& inout_stream, const nsGameObjectHandle& hValue)
{
  NS_ASSERT_DEV(false, "This function should not be called. Use nsWorldWriter::WriteGameObjectHandle instead.");
}

void operator>>(nsStreamReader& inout_stream, nsGameObjectHandle& ref_hValue)
{
  NS_ASSERT_DEV(false, "This function should not be called. Use nsWorldReader::ReadGameObjectHandle instead.");
}

void operator<<(nsStreamWriter& inout_stream, const nsComponentHandle& hValue)
{
  NS_ASSERT_DEV(false, "This function should not be called. Use nsWorldWriter::WriteComponentHandle instead.");
}

void operator>>(nsStreamReader& inout_stream, nsComponentHandle& ref_hValue)
{
  NS_ASSERT_DEV(false, "This function should not be called. Use nsWorldReader::ReadComponentHandle instead.");
}

//////////////////////////////////////////////////////////////////////////

namespace
{
  template <typename T>
  void HandleFinishedActionImpl(nsComponent* pComponent, typename T::Enum action)
  {
    if (action == T::DeleteGameObject)
    {
      // Send a message to the owner object to check whether another component wants to delete this object later.
      // Can't use nsGameObject::SendMessage because the object would immediately delete itself and furthermore the sender component needs to be
      // filtered out here.
      nsMsgDeleteGameObject msg;

      for (nsComponent* pComp : pComponent->GetOwner()->GetComponents())
      {
        if (pComp == pComponent)
          continue;

        pComp->SendMessage(msg);
        if (msg.m_bCancel)
        {
          action = T::DeleteComponent;
          break;
        }
      }

      if (action == T::DeleteGameObject)
      {
        pComponent->GetWorld()->DeleteObjectDelayed(pComponent->GetOwner()->GetHandle());
        return;
      }
    }

    if (action == T::DeleteComponent)
    {
      pComponent->GetOwningManager()->DeleteComponent(pComponent->GetHandle());
    }
  }

  template <typename T>
  void HandleDeleteObjectMsgImpl(nsMsgDeleteGameObject& ref_msg, nsEnum<T>& ref_action)
  {
    if (ref_action == T::DeleteComponent)
    {
      ref_msg.m_bCancel = true;
      ref_action = T::DeleteGameObject;
    }
    else if (ref_action == T::DeleteGameObject)
    {
      ref_msg.m_bCancel = true;
    }
  }
} // namespace

//////////////////////////////////////////////////////////////////////////

void nsOnComponentFinishedAction::HandleFinishedAction(nsComponent* pComponent, nsOnComponentFinishedAction::Enum action)
{
  HandleFinishedActionImpl<nsOnComponentFinishedAction>(pComponent, action);
}

void nsOnComponentFinishedAction::HandleDeleteObjectMsg(nsMsgDeleteGameObject& ref_msg, nsEnum<nsOnComponentFinishedAction>& ref_action)
{
  HandleDeleteObjectMsgImpl(ref_msg, ref_action);
}

//////////////////////////////////////////////////////////////////////////

void nsOnComponentFinishedAction2::HandleFinishedAction(nsComponent* pComponent, nsOnComponentFinishedAction2::Enum action)
{
  HandleFinishedActionImpl<nsOnComponentFinishedAction2>(pComponent, action);
}

void nsOnComponentFinishedAction2::HandleDeleteObjectMsg(nsMsgDeleteGameObject& ref_msg, nsEnum<nsOnComponentFinishedAction2>& ref_action)
{
  HandleDeleteObjectMsgImpl(ref_msg, ref_action);
}

NS_STATICLINK_FILE(Core, Core_World_Implementation_Declarations);
