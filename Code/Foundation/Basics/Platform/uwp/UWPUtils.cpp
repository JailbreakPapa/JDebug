#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)

#  include <Foundation/Basics/Platform/uwp/UWPUtils.h>
#  include <Foundation/Types/Uuid.h>
#  include <Windows.Foundation.numerics.h>

nsMat4 nsUwpUtils::ConvertMat4(const ABI::Windows::Foundation::Numerics::Matrix4x4& in)
{
  return nsMat4(in.M11, in.M21, in.M31, in.M41, in.M12, in.M22, in.M32, in.M42, in.M13, in.M23, in.M33, in.M43, in.M14, in.M24, in.M34, in.M44);
}

nsVec3 nsUwpUtils::ConvertVec3(const ABI::Windows::Foundation::Numerics::Vector3& in)
{
  return nsVec3(in.X, in.Y, in.Z);
}

void nsUwpUtils::ConvertVec3(const nsVec3& in, ABI::Windows::Foundation::Numerics::Vector3& out)
{
  out.X = in.x;
  out.Y = in.y;
  out.Z = in.z;
}

nsQuat nsUwpUtils::ConvertQuat(const ABI::Windows::Foundation::Numerics::Quaternion& in)
{
  return nsQuat(in.X, in.Y, in.Z, in.W);
}

void nsUwpUtils::ConvertQuat(const nsQuat& in, ABI::Windows::Foundation::Numerics::Quaternion& out)
{
  out.X = in.x;
  out.Y = in.y;
  out.Z = in.z;
  out.W = in.w;
}

nsUuid nsUwpUtils::ConvertGuid(const GUID& in)
{
  return *reinterpret_cast<const nsUuid*>(&in);
}

void nsUwpUtils::ConvertGuid(const nsUuid& in, GUID& out)
{
  nsMemoryUtils::Copy(reinterpret_cast<nsUInt32*>(&out), reinterpret_cast<const nsUInt32*>(&in), 4);
}


#endif
