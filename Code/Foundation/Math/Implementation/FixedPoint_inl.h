#pragma once

#include <Foundation/Math/Math.h>

template <nsUInt8 DecimalBits>
const nsFixedPoint<DecimalBits>& nsFixedPoint<DecimalBits>::operator=(nsInt32 iVal)
{
  m_iValue = iVal << DecimalBits;
  return *this;
}

template <nsUInt8 DecimalBits>
const nsFixedPoint<DecimalBits>& nsFixedPoint<DecimalBits>::operator=(float fVal)
{
  m_iValue = (nsInt32)nsMath::Round(fVal * (1 << DecimalBits));
  return *this;
}

template <nsUInt8 DecimalBits>
const nsFixedPoint<DecimalBits>& nsFixedPoint<DecimalBits>::operator=(double fVal)
{
  m_iValue = (nsInt32)nsMath::Round(fVal * (1 << DecimalBits));
  return *this;
}

template <nsUInt8 DecimalBits>
nsInt32 nsFixedPoint<DecimalBits>::ToInt() const
{
  return (nsInt32)(m_iValue >> DecimalBits);
}

template <nsUInt8 DecimalBits>
float nsFixedPoint<DecimalBits>::ToFloat() const
{
  return (float)((double)m_iValue / (double)(1 << DecimalBits));
}

template <nsUInt8 DecimalBits>
double nsFixedPoint<DecimalBits>::ToDouble() const
{
  return ((double)m_iValue / (double)(1 << DecimalBits));
}

template <nsUInt8 DecimalBits>
void nsFixedPoint<DecimalBits>::operator*=(const nsFixedPoint<DecimalBits>& rhs)
{
  // lhs and rhs are in N:M format (N Bits for the Integer part, M Bits for the fractional part)
  // after multiplication, it will be in 2N:2M format

  const nsInt64 TempLHS = m_iValue;
  const nsInt64 TempRHS = rhs.m_iValue;

  nsInt64 TempRes = TempLHS * TempRHS;

  // the lower DecimalBits Bits are nearly of no concern (we throw them away anyway), except for the upper most Bit
  // that is Bit '(DecimalBits - 1)' and its Bitmask is therefore '(1 << (DecimalBits - 1))'
  // If that Bit is set, then the lowest DecimalBits represent a value of more than '0.5' (of their range)
  // so '(TempRes & (1 << (DecimalBits - 1))) ' is either 0 or 1 depending on whether the lower DecimalBits Bits represent a value larger than 0.5 or
  // not we shift that Bit one to the left and add it to the original value and thus 'round up' the result
  TempRes += ((TempRes & (1 << (DecimalBits - 1))) << 1);

  TempRes >>= DecimalBits; // result format: 2N:M

  // the upper N Bits are thrown away during conversion from 64 Bit to 32 Bit
  m_iValue = (nsInt32)TempRes;
}

template <nsUInt8 DecimalBits>
void nsFixedPoint<DecimalBits>::operator/=(const nsFixedPoint<DecimalBits>& rhs)
{
  nsInt64 TempLHS = m_iValue;
  const nsInt64 TempRHS = rhs.m_iValue;

  TempLHS <<= 31;

  nsInt64 TempRes = TempLHS / TempRHS;

  // same rounding concept as in multiplication
  TempRes += ((TempRes & (1 << (31 - DecimalBits - 1))) << 1);

  TempRes >>= (31 - DecimalBits);

  // here we throw away the upper 32 Bits again (not needed anymore)
  m_iValue = (nsInt32)TempRes;
}


template <nsUInt8 DecimalBits>
nsFixedPoint<DecimalBits> operator+(const nsFixedPoint<DecimalBits>& lhs, const nsFixedPoint<DecimalBits>& rhs)
{
  nsFixedPoint<DecimalBits> res = lhs;
  res += rhs;
  return res;
}

template <nsUInt8 DecimalBits>
nsFixedPoint<DecimalBits> operator-(const nsFixedPoint<DecimalBits>& lhs, const nsFixedPoint<DecimalBits>& rhs)
{
  nsFixedPoint<DecimalBits> res = lhs;
  res -= rhs;
  return res;
}

template <nsUInt8 DecimalBits>
nsFixedPoint<DecimalBits> operator*(const nsFixedPoint<DecimalBits>& lhs, const nsFixedPoint<DecimalBits>& rhs)
{
  nsFixedPoint<DecimalBits> res = lhs;
  res *= rhs;
  return res;
}

template <nsUInt8 DecimalBits>
nsFixedPoint<DecimalBits> operator/(const nsFixedPoint<DecimalBits>& lhs, const nsFixedPoint<DecimalBits>& rhs)
{
  nsFixedPoint<DecimalBits> res = lhs;
  res /= rhs;
  return res;
}


template <nsUInt8 DecimalBits>
nsFixedPoint<DecimalBits> operator*(const nsFixedPoint<DecimalBits>& lhs, nsInt32 rhs)
{
  nsFixedPoint<DecimalBits> ret = lhs;
  ret *= rhs;
  return ret;
}

template <nsUInt8 DecimalBits>
nsFixedPoint<DecimalBits> operator*(nsInt32 lhs, const nsFixedPoint<DecimalBits>& rhs)
{
  nsFixedPoint<DecimalBits> ret = rhs;
  ret *= lhs;
  return ret;
}

template <nsUInt8 DecimalBits>
nsFixedPoint<DecimalBits> operator/(const nsFixedPoint<DecimalBits>& lhs, nsInt32 rhs)
{
  nsFixedPoint<DecimalBits> ret = lhs;
  ret /= rhs;
  return ret;
}
