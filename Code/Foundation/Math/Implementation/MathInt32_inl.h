#pragma once

namespace nsMath
{
  constexpr NS_ALWAYS_INLINE nsInt32 RoundUp(nsInt32 value, nsUInt16 uiMultiple)
  {
    //
    return (value >= 0) ? ((value + uiMultiple - 1) / uiMultiple) * uiMultiple : (value / uiMultiple) * uiMultiple;
  }

  constexpr NS_ALWAYS_INLINE nsInt32 RoundDown(nsInt32 value, nsUInt16 uiMultiple)
  {
    //
    return (value <= 0) ? ((value - uiMultiple + 1) / uiMultiple) * uiMultiple : (value / uiMultiple) * uiMultiple;
  }

  constexpr NS_ALWAYS_INLINE nsUInt32 RoundUp(nsUInt32 value, nsUInt16 uiMultiple)
  {
    //
    return ((value + uiMultiple - 1) / uiMultiple) * uiMultiple;
  }

  constexpr NS_ALWAYS_INLINE nsUInt32 RoundDown(nsUInt32 value, nsUInt16 uiMultiple)
  {
    //
    return (value / uiMultiple) * uiMultiple;
  }

  constexpr NS_ALWAYS_INLINE bool IsOdd(nsInt32 i)
  {
    //
    return ((i & 1) != 0);
  }

  constexpr NS_ALWAYS_INLINE bool IsEven(nsInt32 i)
  {
    //
    return ((i & 1) == 0);
  }

  NS_ALWAYS_INLINE nsUInt32 Log2i(nsUInt32 uiVal)
  {
    return (uiVal != 0) ? FirstBitHigh(uiVal) : -1;
  }

  constexpr NS_ALWAYS_INLINE int Pow2(int i)
  {
    //
    return (1 << i);
  }

  inline int Pow(int iBase, int iExp)
  {
    int res = 1;
    while (iExp > 0)
    {
      res *= iBase;
      --iExp;
    }

    return res;
  }

} // namespace nsMath
