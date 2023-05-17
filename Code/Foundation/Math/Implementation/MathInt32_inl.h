#pragma once

namespace wdMath
{
  constexpr WD_ALWAYS_INLINE wdInt32 RoundUp(wdInt32 value, wdUInt16 uiMultiple)
  {
    //
    return (value >= 0) ? ((value + uiMultiple - 1) / uiMultiple) * uiMultiple : (value / uiMultiple) * uiMultiple;
  }

  constexpr WD_ALWAYS_INLINE wdInt32 RoundDown(wdInt32 value, wdUInt16 uiMultiple)
  {
    //
    return (value <= 0) ? ((value - uiMultiple + 1) / uiMultiple) * uiMultiple : (value / uiMultiple) * uiMultiple;
  }

  constexpr WD_ALWAYS_INLINE wdUInt32 RoundUp(wdUInt32 value, wdUInt16 uiMultiple)
  {
    //
    return ((value + uiMultiple - 1) / uiMultiple) * uiMultiple;
  }

  constexpr WD_ALWAYS_INLINE wdUInt32 RoundDown(wdUInt32 value, wdUInt16 uiMultiple)
  {
    //
    return (value / uiMultiple) * uiMultiple;
  }

  constexpr WD_ALWAYS_INLINE bool IsOdd(wdInt32 i)
  {
    //
    return ((i & 1) != 0);
  }

  constexpr WD_ALWAYS_INLINE bool IsEven(wdInt32 i)
  {
    //
    return ((i & 1) == 0);
  }

  WD_ALWAYS_INLINE wdUInt32 Log2i(wdUInt32 uiVal)
  {
    return (uiVal != 0) ? FirstBitHigh(uiVal) : -1;
  }

  constexpr WD_ALWAYS_INLINE int Pow2(int i)
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

} // namespace wdMath
