#include <Foundation/FoundationPCH.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)

#  include <Foundation/Basics/Platform/uwp/UWPUtils.h>
#  include <Foundation/Types/Uuid.h>
#  include <Windows.Foundation.numerics.h>

wdMat4 wdUwpUtils::ConvertMat4(const ABI::Windows::Foundation::Numerics::Matrix4x4& in)
{
  return wdMat4(in.M11, in.M21, in.M31, in.M41, in.M12, in.M22, in.M32, in.M42, in.M13, in.M23, in.M33, in.M43, in.M14, in.M24, in.M34, in.M44);
}

wdVec3 wdUwpUtils::ConvertVec3(const ABI::Windows::Foundation::Numerics::Vector3& in)
{
  return wdVec3(in.X, in.Y, in.Z);
}

void wdUwpUtils::ConvertVec3(const wdVec3& in, ABI::Windows::Foundation::Numerics::Vector3& out)
{
  out.X = in.x;
  out.Y = in.y;
  out.Z = in.z;
}

wdQuat wdUwpUtils::ConvertQuat(const ABI::Windows::Foundation::Numerics::Quaternion& in)
{
  return wdQuat(in.X, in.Y, in.Z, in.W);
}

void wdUwpUtils::ConvertQuat(const wdQuat& in, ABI::Windows::Foundation::Numerics::Quaternion& out)
{
  out.X = in.v.x;
  out.Y = in.v.y;
  out.Z = in.v.z;
  out.W = in.w;
}

wdUuid wdUwpUtils::ConvertGuid(const GUID& in)
{
  return *reinterpret_cast<const wdUuid*>(&in);
}

void wdUwpUtils::ConvertGuid(const wdUuid& in, GUID& out)
{
  wdMemoryUtils::Copy(reinterpret_cast<wdUInt32*>(&out), reinterpret_cast<const wdUInt32*>(&in), 4);
}


#endif



WD_STATICLINK_FILE(Foundation, Foundation_Basics_Platform_uwp_UWPUtils);
