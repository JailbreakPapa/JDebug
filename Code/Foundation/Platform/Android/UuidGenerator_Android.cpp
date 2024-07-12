#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_ANDROID)

#  include <Foundation/Types/Uuid.h>

#  include <Foundation/Basics/Platform/Android/AndroidJni.h>
#  include <Foundation/Basics/Platform/Android/AndroidUtils.h>
#  include <android_native_app_glue.h>

nsUuid nsUuid::MakeUuid()
{
  nsJniAttachment attachment;

  nsJniClass uuidClass("java/util/UUID");
  NS_ASSERT_DEBUG(!uuidClass.IsNull(), "UUID class not found.");
  nsJniObject javaUuid = uuidClass.CallStatic<nsJniObject>("randomUUID");
  jlong mostSignificant = javaUuid.Call<jlong>("getMostSignificantBits");
  jlong leastSignificant = javaUuid.Call<jlong>("getLeastSignificantBits");

  return nsUuid(leastSignificant, mostSignificant);

  // #TODO maybe faster to read /proc/sys/kernel/random/uuid, but that can't be done via nsOSFile
  //  see https://stackoverflow.com/questions/11888055/include-uuid-h-into-android-ndk-project
}

#endif
