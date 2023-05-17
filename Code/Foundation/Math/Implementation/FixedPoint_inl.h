#pragma once

#include <Foundation/Math/Math.h>

template <wdUInt8 DecimalBits>
const wdFixedPoint<DecimalBits>& wdFixedPoint<DecimalBits>::operator=(wdInt32 iVal)
{
  m_iValue = iVal << DecimalBits;
  return *this;
}

template <wdUInt8 DecimalBits>
const wdFixedPoint<DecimalBits>& wdFixedPoint<DecimalBits>::operator=(float fVal)
{
  m_iValue = (wdInt32)wdMath::Round(fVal * (1 << DecimalBits));
  return *this;
}

template <wdUInt8 DecimalBits>
const wdFixedPoint<DecimalBits>& wdFixedPoint<DecimalBits>::operator=(double fVal)
{
  m_iValue = (wdInt32)wdMath::Round(fVal * (1 << DecimalBits));
  return *this;
}

template <wdUInt8 DecimalBits>
wdInt32 wdFixedPoint<DecimalBits>::ToInt() const
{
  return (wdInt32)(m_iValue >> DecimalBits);
}

template <wdUInt8 DecimalBits>
float wdFixedPoint<DecimalBits>::ToFloat() const
{
  return (float)((double)m_iValue / (double)(1 << DecimalBits));
}

template <wdUInt8 DecimalBits>
double wdFixedPoint<DecimalBits>::ToDouble() const
{
  return ((double)m_iValue / (double)(1 << DecimalBits));
}

template <wdUInt8 DecimalBits>
void wdFixedPoint<DecimalBits>::operator*=(const wdFixedPoint<DecimalBits>& rhs)
{
  // lhs and rhs are in N:M format (N Bits for the Integer part, M Bits for the fractional part)
  // after multiplication, it will be in 2N:2M format

  const wdInt64 TempLHS = m_iValue;
  const wdInt64 TempRHS = rhs.m_iValue;

  wdInt64 TempRes = TempLHS * TempRHS;

  // the lower DecimalBits Bits are nearly of no concern (we throw them away anyway), except for the upper most Bit
  // that is Bit '(DecimalBits - 1)' and its Bitmask is therefore '(1 << (DecimalBits - 1))'
  // If that Bit is set, then the lowest DecimalBits represent a value of more than '0.5' (of their range)
  // so '(TempRes & (1 << (DecimalBits - 1))) ' is either 0 or 1 depending on whether the lower DecimalBits Bits represent a value larger than 0.5 or
  // not we shift that Bit one to the left and add it to the original value and thus 'round up' the result
  TempRes += ((TempRes & (1 << (DecimalBits - 1))) << 1);

  TempRes >>= DecimalBits; // result format: 2N:M

  // the upper N Bits are thrown away during conversion from 64 Bit to 32 Bit
  m_iValue = (wdInt32)TempRes;
}

template <wdUInt8 DecimalBits>
void wdFixedPoint<DecimalBits>::operator/=(const wdFixedPoint<DecimalBits>& rhs)
{
  wdInt64 TempLHS = m_iValue;
  const wdInt64 TempRHS = rhs.m_iValue;

  TempLHS <<= 31;

  wdInt64 TempRes = TempLHS / TempRHS;

  // same rounding concept as in multiplication
  TempRes += ((TempRes & (1 << (31 - DecimalBits - 1))) << 1);

  TempRes >>= (31 - DecimalBits);

  // here we throw away the upper 32 Bits again (not needed anymore)
  m_iValue = (wdInt32)TempRes;
}


template <wdUInt8 DecimalBits>
wdFixedPoint<DecimalBits> operator+(const wdFixedPoint<DecimalBits>& lhs, const wdFixedPoint<DecimalBits>& rhs)
{
  wdFixedPoint<DecimalBits> res = lhs;
  res += rhs;
  return res;
}

template <wdUInt8 DecimalBits>
wdFixedPoint<DecimalBits> operator-(const wdFixedPoint<DecimalBits>& lhs, const wdFixedPoint<DecimalBits>& rhs)
{
  wdFixedPoint<DecimalBits> res = lhs;
  res -= rhs;
  return res;
}

template <wdUInt8 DecimalBits>
wdFixedPoint<DecimalBits> operator*(const wdFixedPoint<DecimalBits>& lhs, const wdFixedPoint<DecimalBits>& rhs)
{
  wdFixedPoint<DecimalBits> res = lhs;
  res *= rhs;
  return res;
}

template <wdUInt8 DecimalBits>
wdFixedPoint<DecimalBits> operator/(const wdFixedPoint<DecimalBits>& lhs, const wdFixedPoint<DecimalBits>& rhs)
{
  wdFixedPoint<DecimalBits> res = lhs;
  res /= rhs;
  return res;
}


template <wdUInt8 DecimalBits>
wdFixedPoint<DecimalBits> operator*(const wdFixedPoint<DecimalBits>& lhs, wdInt32 rhs)
{
  wdFixedPoint<DecimalBits> ret = lhs;
  ret *= rhs;
  return ret;
}

template <wdUInt8 DecimalBits>
wdFixedPoint<DecimalBits> operator*(wdInt32 lhs, const wdFixedPoint<DecimalBits>& rhs)
{
  wdFixedPoint<DecimalBits> ret = rhs;
  ret *= lhs;
  return ret;
}

template <wdUInt8 DecimalBits>
wdFixedPoint<DecimalBits> operator/(const wdFixedPoint<DecimalBits>& lhs, wdInt32 rhs)
{
  wdFixedPoint<DecimalBits> ret = lhs;
  ret /= rhs;
  return ret;
}
