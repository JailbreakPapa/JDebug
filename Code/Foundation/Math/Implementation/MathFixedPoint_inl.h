#pragma once

#include <Foundation/Math/FixedPoint.h>

/*
namespace wdMath
{
#define FIXEDPOINT_OVERLOADS(Bits)                                                                                                       \
  template <>                                                                                                                            \
  WD_ALWAYS_INLINE wdFixedPoint<Bits> BasicType<wdFixedPoint<Bits>>::MaxValue() { return (wdFixedPoint<Bits>)((1 << (31 - Bits)) - 1); } \
  template <>                                                                                                                            \
  WD_ALWAYS_INLINE wdFixedPoint<Bits> BasicType<wdFixedPoint<Bits>>::SmallEpsilon() { return (wdFixedPoint<Bits>)0.0001; }               \
  template <>                                                                                                                            \
  WD_ALWAYS_INLINE wdFixedPoint<Bits> BasicType<wdFixedPoint<Bits>>::DefaultEpsilon() { return (wdFixedPoint<Bits>)0.001; }              \
  template <>                                                                                                                            \
  WD_ALWAYS_INLINE wdFixedPoint<Bits> BasicType<wdFixedPoint<Bits>>::LargeEpsilon() { return (wdFixedPoint<Bits>)0.01; }                 \
  template <>                                                                                                                            \
  WD_ALWAYS_INLINE wdFixedPoint<Bits> BasicType<wdFixedPoint<Bits>>::HugeEpsilon() { return (wdFixedPoint<Bits>)0.1; }

  FIXEDPOINT_OVERLOADS(1);
  FIXEDPOINT_OVERLOADS(2);
  FIXEDPOINT_OVERLOADS(3);
  FIXEDPOINT_OVERLOADS(4);
  FIXEDPOINT_OVERLOADS(5);
  FIXEDPOINT_OVERLOADS(6);
  FIXEDPOINT_OVERLOADS(7);
  FIXEDPOINT_OVERLOADS(8);
  FIXEDPOINT_OVERLOADS(9);
  FIXEDPOINT_OVERLOADS(10);
  FIXEDPOINT_OVERLOADS(11);
  FIXEDPOINT_OVERLOADS(12);
  FIXEDPOINT_OVERLOADS(13);
  FIXEDPOINT_OVERLOADS(14);
  FIXEDPOINT_OVERLOADS(15);
  FIXEDPOINT_OVERLOADS(16);
  FIXEDPOINT_OVERLOADS(17);
  FIXEDPOINT_OVERLOADS(18);
  FIXEDPOINT_OVERLOADS(19);
  FIXEDPOINT_OVERLOADS(20);
  FIXEDPOINT_OVERLOADS(21);
  FIXEDPOINT_OVERLOADS(22);
  FIXEDPOINT_OVERLOADS(23);
  FIXEDPOINT_OVERLOADS(24);
  FIXEDPOINT_OVERLOADS(25);
  FIXEDPOINT_OVERLOADS(26);
  FIXEDPOINT_OVERLOADS(27);
  FIXEDPOINT_OVERLOADS(28);
  FIXEDPOINT_OVERLOADS(29);
  FIXEDPOINT_OVERLOADS(30);
  //FIXEDPOINT_OVERLOADS(31);

  template <wdUInt8 DecimalBits>
  WD_FORCE_INLINE wdFixedPoint<DecimalBits> Floor(wdFixedPoint<DecimalBits> f)
  {
    WD_REPORT_FAILURE("This function is not really implemented yet.");

    return (wdFixedPoint<DecimalBits>)floor(f.ToDouble());
  }

  template <wdUInt8 DecimalBits>
  WD_FORCE_INLINE wdFixedPoint<DecimalBits> Ceil(wdFixedPoint<DecimalBits> f)
  {
    WD_REPORT_FAILURE("This function is not really implemented yet.");

    return (wdFixedPoint<DecimalBits>)ceil(f.ToDouble());
  }

  template <wdUInt8 DecimalBits>
  inline wdFixedPoint<DecimalBits> Floor(wdFixedPoint<DecimalBits> f, wdFixedPoint<DecimalBits> fMultiple)
  {
    WD_REPORT_FAILURE("This function is not really implemented yet.");

    wdFixedPoint<DecimalBits> fDivides = f / fMultiple;
    wdFixedPoint<DecimalBits> fFactor = Floor(fDivides);
    return fFactor * fMultiple;
  }

  template <wdUInt8 DecimalBits>
  inline wdFixedPoint<DecimalBits> Ceil(wdFixedPoint<DecimalBits> f, wdFixedPoint<DecimalBits> fMultiple)
  {
    WD_REPORT_FAILURE("This function is not really implemented yet.");

    wdFixedPoint<DecimalBits> fDivides = f / fMultiple;
    wdFixedPoint<DecimalBits> fFactor = Ceil(fDivides);
    return fFactor * fMultiple;
  }

  template <wdUInt8 DecimalBits>
  WD_FORCE_INLINE wdFixedPoint<DecimalBits> Exp(wdFixedPoint<DecimalBits> f)
  {
    WD_REPORT_FAILURE("This function is not really implemented yet.");

    return (wdFixedPoint<DecimalBits>)exp(f.ToDouble());
  }

  template <wdUInt8 DecimalBits>
  WD_FORCE_INLINE wdFixedPoint<DecimalBits> Ln(wdFixedPoint<DecimalBits> f)
  {
    WD_REPORT_FAILURE("This function is not really implemented yet.");

    return (wdFixedPoint<DecimalBits>)log(f.ToDouble());
  }

  template <wdUInt8 DecimalBits>
  WD_FORCE_INLINE wdFixedPoint<DecimalBits> Log2(wdFixedPoint<DecimalBits> f)
  {
    WD_REPORT_FAILURE("This function is not really implemented yet.");

    return (wdFixedPoint<DecimalBits>)(log10(f.ToDouble()) / log10(2.0));
  }

  template <wdUInt8 DecimalBits>
  WD_FORCE_INLINE wdFixedPoint<DecimalBits> Log10(wdFixedPoint<DecimalBits> f)
  {
    WD_REPORT_FAILURE("This function is not really implemented yet.");

    return (wdFixedPoint<DecimalBits>)log10(f.ToDouble());
  }

  template <wdUInt8 DecimalBits>
  WD_FORCE_INLINE wdFixedPoint<DecimalBits> Log(wdFixedPoint<DecimalBits> fBase, wdFixedPoint<DecimalBits> f)
  {
    WD_REPORT_FAILURE("This function is not really implemented yet.");

    return (wdFixedPoint<DecimalBits>)(log10(f.ToDouble()) / log10(fBase.ToDouble()));
  }

  template <wdUInt8 DecimalBits>
  WD_FORCE_INLINE wdFixedPoint<DecimalBits> Pow2(wdFixedPoint<DecimalBits> f)
  {
    WD_REPORT_FAILURE("This function is not really implemented yet.");

    return (wdFixedPoint<DecimalBits>)pow(2.0, f.ToDouble());
  }

  template <wdUInt8 DecimalBits>
  WD_FORCE_INLINE wdFixedPoint<DecimalBits> Pow(wdFixedPoint<DecimalBits> base, wdFixedPoint<DecimalBits> exp)
  {
    WD_REPORT_FAILURE("This function is not really implemented yet.");

    return (wdFixedPoint<DecimalBits>)pow(base.ToDouble(), exp.ToDouble());
  }

  template <wdUInt8 DecimalBits>
  WD_FORCE_INLINE wdFixedPoint<DecimalBits> Root(wdFixedPoint<DecimalBits> f, wdFixedPoint<DecimalBits> NthRoot)
  {
    WD_REPORT_FAILURE("This function is not really implemented yet.");

    return (wdFixedPoint<DecimalBits>)pow(f.ToDouble(), 1.0 / NthRoot.ToDouble());
  }

  template <wdUInt8 DecimalBits>
  wdFixedPoint<DecimalBits> Sqrt(wdFixedPoint<DecimalBits> a)
  {
    return (wdFixedPoint<DecimalBits>)sqrt(a.ToDouble());
    //if (a <= wdFixedPoint<DecimalBits>(0))
    //  return wdFixedPoint<DecimalBits>(0);

    //wdFixedPoint<DecimalBits> x = a / 2;

    //for (wdUInt32 i = 0; i < 8; ++i)
    //{
    //  wdFixedPoint<DecimalBits> ax = a / x;
    //  wdFixedPoint<DecimalBits> xpax = x + ax;
    //  x = xpax / 2;
    //}

    //return x;
  }

  template <wdUInt8 DecimalBits>
  WD_FORCE_INLINE wdFixedPoint<DecimalBits> Mod(wdFixedPoint<DecimalBits> f, wdFixedPoint<DecimalBits> div)
  {
    WD_REPORT_FAILURE("This function is not really implemented yet.");

    return (wdFixedPoint<DecimalBits>)fmod(f.ToDouble(), div);
  }
}
*/
