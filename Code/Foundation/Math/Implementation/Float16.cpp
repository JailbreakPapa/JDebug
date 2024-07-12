#include <Foundation/FoundationPCH.h>

#include <Foundation/Math/Float16.h>
#include <Foundation/Math/Vec2.h>
#include <Foundation/Math/Vec3.h>
#include <Foundation/Math/Vec4.h>

nsFloat16::nsFloat16(float f)
{
  operator=(f);
}

void nsFloat16::operator=(float f)
{
  // source: http://www.ogre3d.org/docs/api/html/OgreBitwise_8h_source.html

  const nsUInt32 i = *reinterpret_cast<nsUInt32*>(&f);

  const nsUInt32 s = (i >> 16) & 0x00008000;
  const nsInt32 e = ((i >> 23) & 0x000000ff) - (127 - 15);
  nsUInt32 m = i & 0x007fffff;

  if (e <= 0)
  {
    if (e < -10)
    {
      m_uiData = 0;
      return;
    }
    m = (m | 0x00800000) >> (1 - e);

    m_uiData = static_cast<nsUInt16>(s | (m >> 13));
  }
  else if (e == 0xff - (127 - 15))
  {
    if (m == 0) // Inf
    {
      m_uiData = static_cast<nsUInt16>(s | 0x7c00);
    }
    else // NAN
    {
      m >>= 13;
      m_uiData = static_cast<nsUInt16>(s | 0x7c00 | m | (m == 0));
    }
  }
  else
  {
    if (e > 30) // Overflow
    {
      m_uiData = static_cast<nsUInt16>(s | 0x7c00);
      return;
    }

    m_uiData = static_cast<nsUInt16>(s | (e << 10) | (m >> 13));
  }
}

nsFloat16::operator float() const
{
  const nsUInt32 s = (m_uiData >> 15) & 0x00000001;
  nsUInt32 e = (m_uiData >> 10) & 0x0000001f;
  nsUInt32 m = m_uiData & 0x000003ff;

  nsUInt32 uiResult;

  if (e == 0)
  {
    if (m == 0) // Plus or minus zero
    {
      uiResult = s << 31;
      return *reinterpret_cast<float*>(&uiResult);
    }
    else // Denormalized number -- renormalize it
    {
      while (!(m & 0x00000400))
      {
        m <<= 1;
        e -= 1;
      }

      e += 1;
      m &= ~0x00000400;
    }
  }
  else if (e == 31)
  {
    if (m == 0) // Inf
    {
      uiResult = (s << 31) | 0x7f800000;
      return *reinterpret_cast<float*>(&uiResult);
    }
    else // NaN
    {
      uiResult = (s << 31) | 0x7f800000 | (m << 13);
      return *reinterpret_cast<float*>(&uiResult);
    }
  }

  e = e + (127 - 15);
  m = m << 13;

  uiResult = (s << 31) | (e << 23) | m;

  return *reinterpret_cast<float*>(&uiResult);
}

//////////////////////////////////////////////////////////////////////////

nsFloat16Vec2::nsFloat16Vec2(const nsVec2& vVec)
{
  operator=(vVec);
}

void nsFloat16Vec2::operator=(const nsVec2& vVec)
{
  x = vVec.x;
  y = vVec.y;
}

nsFloat16Vec2::operator nsVec2() const
{
  return nsVec2(x, y);
}

//////////////////////////////////////////////////////////////////////////

nsFloat16Vec3::nsFloat16Vec3(const nsVec3& vVec)
{
  operator=(vVec);
}

void nsFloat16Vec3::operator=(const nsVec3& vVec)
{
  x = vVec.x;
  y = vVec.y;
  z = vVec.z;
}

nsFloat16Vec3::operator nsVec3() const
{
  return nsVec3(x, y, z);
}

//////////////////////////////////////////////////////////////////////////

nsFloat16Vec4::nsFloat16Vec4(const nsVec4& vVec)
{
  operator=(vVec);
}

void nsFloat16Vec4::operator=(const nsVec4& vVec)
{
  x = vVec.x;
  y = vVec.y;
  z = vVec.z;
  w = vVec.w;
}

nsFloat16Vec4::operator nsVec4() const
{
  return nsVec4(x, y, z, w);
}
