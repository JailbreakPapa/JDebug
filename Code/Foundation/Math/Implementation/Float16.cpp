#include <Foundation/FoundationPCH.h>

#include <Foundation/Math/Float16.h>
#include <Foundation/Math/Vec2.h>
#include <Foundation/Math/Vec3.h>
#include <Foundation/Math/Vec4.h>

wdFloat16::wdFloat16(float f)
{
  operator=(f);
}

void wdFloat16::operator=(float f)
{
  // source: http://www.ogre3d.org/docs/api/html/OgreBitwise_8h_source.html

  const wdUInt32 i = *reinterpret_cast<wdUInt32*>(&f);

  const wdUInt32 s = (i >> 16) & 0x00008000;
  const wdInt32 e = ((i >> 23) & 0x000000ff) - (127 - 15);
  wdUInt32 m = i & 0x007fffff;

  if (e <= 0)
  {
    if (e < -10)
    {
      m_uiData = 0;
      return;
    }
    m = (m | 0x00800000) >> (1 - e);

    m_uiData = static_cast<wdUInt16>(s | (m >> 13));
  }
  else if (e == 0xff - (127 - 15))
  {
    if (m == 0) // Inf
    {
      m_uiData = static_cast<wdUInt16>(s | 0x7c00);
    }
    else // NAN
    {
      m >>= 13;
      m_uiData = static_cast<wdUInt16>(s | 0x7c00 | m | (m == 0));
    }
  }
  else
  {
    if (e > 30) // Overflow
    {
      m_uiData = static_cast<wdUInt16>(s | 0x7c00);
      return;
    }

    m_uiData = static_cast<wdUInt16>(s | (e << 10) | (m >> 13));
  }
}

wdFloat16::operator float() const
{
  const wdUInt32 s = (m_uiData >> 15) & 0x00000001;
  wdUInt32 e = (m_uiData >> 10) & 0x0000001f;
  wdUInt32 m = m_uiData & 0x000003ff;

  wdUInt32 uiResult;

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

wdFloat16Vec2::wdFloat16Vec2(const wdVec2& vVec)
{
  operator=(vVec);
}

void wdFloat16Vec2::operator=(const wdVec2& vVec)
{
  x = vVec.x;
  y = vVec.y;
}

wdFloat16Vec2::operator wdVec2() const
{
  return wdVec2(x, y);
}

//////////////////////////////////////////////////////////////////////////

wdFloat16Vec3::wdFloat16Vec3(const wdVec3& vVec)
{
  operator=(vVec);
}

void wdFloat16Vec3::operator=(const wdVec3& vVec)
{
  x = vVec.x;
  y = vVec.y;
  z = vVec.z;
}

wdFloat16Vec3::operator wdVec3() const
{
  return wdVec3(x, y, z);
}

//////////////////////////////////////////////////////////////////////////

wdFloat16Vec4::wdFloat16Vec4(const wdVec4& vVec)
{
  operator=(vVec);
}

void wdFloat16Vec4::operator=(const wdVec4& vVec)
{
  x = vVec.x;
  y = vVec.y;
  z = vVec.z;
  w = vVec.w;
}

wdFloat16Vec4::operator wdVec4() const
{
  return wdVec4(x, y, z, w);
}


WD_STATICLINK_FILE(Foundation, Foundation_Math_Implementation_Float16);
