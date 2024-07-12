#pragma once

#include <Foundation/Math/FixedPoint.h>

/*
namespace nsMath
{
#define FIXEDPOINT_OVERLOADS(Bits)                                                                                                       \
  template <>                                                                                                                            \
  NS_ALWAYS_INLINE nsFixedPoint<Bits> BasicType<nsFixedPoint<Bits>>::MaxValue() { return (nsFixedPoint<Bits>)((1 << (31 - Bits)) - 1); } \
  template <>                                                                                                                            \
  NS_ALWAYS_INLINE nsFixedPoint<Bits> BasicType<nsFixedPoint<Bits>>::SmallEpsilon() { return (nsFixedPoint<Bits>)0.0001; }               \
  template <>                                                                                                                            \
  NS_ALWAYS_INLINE nsFixedPoint<Bits> BasicType<nsFixedPoint<Bits>>::DefaultEpsilon() { return (nsFixedPoint<Bits>)0.001; }              \
  template <>                                                                                                                            \
  NS_ALWAYS_INLINE nsFixedPoint<Bits> BasicType<nsFixedPoint<Bits>>::LargeEpsilon() { return (nsFixedPoint<Bits>)0.01; }                 \
  template <>                                                                                                                            \
  NS_ALWAYS_INLINE nsFixedPoint<Bits> BasicType<nsFixedPoint<Bits>>::HugeEpsilon() { return (nsFixedPoint<Bits>)0.1; }

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

  template <nsUInt8 DecimalBits>
  NS_FORCE_INLINE nsFixedPoint<DecimalBits> Floor(nsFixedPoint<DecimalBits> f)
  {
    NS_REPORT_FAILURE("This function is not really implemented yet.");

    return (nsFixedPoint<DecimalBits>)floor(f.ToDouble());
  }

  template <nsUInt8 DecimalBits>
  NS_FORCE_INLINE nsFixedPoint<DecimalBits> Ceil(nsFixedPoint<DecimalBits> f)
  {
    NS_REPORT_FAILURE("This function is not really implemented yet.");

    return (nsFixedPoint<DecimalBits>)ceil(f.ToDouble());
  }

  template <nsUInt8 DecimalBits>
  inline nsFixedPoint<DecimalBits> Floor(nsFixedPoint<DecimalBits> f, nsFixedPoint<DecimalBits> fMultiple)
  {
    NS_REPORT_FAILURE("This function is not really implemented yet.");

    nsFixedPoint<DecimalBits> fDivides = f / fMultiple;
    nsFixedPoint<DecimalBits> fFactor = Floor(fDivides);
    return fFactor * fMultiple;
  }

  template <nsUInt8 DecimalBits>
  inline nsFixedPoint<DecimalBits> Ceil(nsFixedPoint<DecimalBits> f, nsFixedPoint<DecimalBits> fMultiple)
  {
    NS_REPORT_FAILURE("This function is not really implemented yet.");

    nsFixedPoint<DecimalBits> fDivides = f / fMultiple;
    nsFixedPoint<DecimalBits> fFactor = Ceil(fDivides);
    return fFactor * fMultiple;
  }

  template <nsUInt8 DecimalBits>
  NS_FORCE_INLINE nsFixedPoint<DecimalBits> Exp(nsFixedPoint<DecimalBits> f)
  {
    NS_REPORT_FAILURE("This function is not really implemented yet.");

    return (nsFixedPoint<DecimalBits>)exp(f.ToDouble());
  }

  template <nsUInt8 DecimalBits>
  NS_FORCE_INLINE nsFixedPoint<DecimalBits> Ln(nsFixedPoint<DecimalBits> f)
  {
    NS_REPORT_FAILURE("This function is not really implemented yet.");

    return (nsFixedPoint<DecimalBits>)log(f.ToDouble());
  }

  template <nsUInt8 DecimalBits>
  NS_FORCE_INLINE nsFixedPoint<DecimalBits> Log2(nsFixedPoint<DecimalBits> f)
  {
    NS_REPORT_FAILURE("This function is not really implemented yet.");

    return (nsFixedPoint<DecimalBits>)(log10(f.ToDouble()) / log10(2.0));
  }

  template <nsUInt8 DecimalBits>
  NS_FORCE_INLINE nsFixedPoint<DecimalBits> Log10(nsFixedPoint<DecimalBits> f)
  {
    NS_REPORT_FAILURE("This function is not really implemented yet.");

    return (nsFixedPoint<DecimalBits>)log10(f.ToDouble());
  }

  template <nsUInt8 DecimalBits>
  NS_FORCE_INLINE nsFixedPoint<DecimalBits> Log(nsFixedPoint<DecimalBits> fBase, nsFixedPoint<DecimalBits> f)
  {
    NS_REPORT_FAILURE("This function is not really implemented yet.");

    return (nsFixedPoint<DecimalBits>)(log10(f.ToDouble()) / log10(fBase.ToDouble()));
  }

  template <nsUInt8 DecimalBits>
  NS_FORCE_INLINE nsFixedPoint<DecimalBits> Pow2(nsFixedPoint<DecimalBits> f)
  {
    NS_REPORT_FAILURE("This function is not really implemented yet.");

    return (nsFixedPoint<DecimalBits>)pow(2.0, f.ToDouble());
  }

  template <nsUInt8 DecimalBits>
  NS_FORCE_INLINE nsFixedPoint<DecimalBits> Pow(nsFixedPoint<DecimalBits> base, nsFixedPoint<DecimalBits> exp)
  {
    NS_REPORT_FAILURE("This function is not really implemented yet.");

    return (nsFixedPoint<DecimalBits>)pow(base.ToDouble(), exp.ToDouble());
  }

  template <nsUInt8 DecimalBits>
  NS_FORCE_INLINE nsFixedPoint<DecimalBits> Root(nsFixedPoint<DecimalBits> f, nsFixedPoint<DecimalBits> NthRoot)
  {
    NS_REPORT_FAILURE("This function is not really implemented yet.");

    return (nsFixedPoint<DecimalBits>)pow(f.ToDouble(), 1.0 / NthRoot.ToDouble());
  }

  template <nsUInt8 DecimalBits>
  nsFixedPoint<DecimalBits> Sqrt(nsFixedPoint<DecimalBits> a)
  {
    return (nsFixedPoint<DecimalBits>)sqrt(a.ToDouble());
    //if (a <= nsFixedPoint<DecimalBits>(0))
    //  return nsFixedPoint<DecimalBits>(0);

    //nsFixedPoint<DecimalBits> x = a / 2;

    //for (nsUInt32 i = 0; i < 8; ++i)
    //{
    //  nsFixedPoint<DecimalBits> ax = a / x;
    //  nsFixedPoint<DecimalBits> xpax = x + ax;
    //  x = xpax / 2;
    //}

    //return x;
  }

  template <nsUInt8 DecimalBits>
  NS_FORCE_INLINE nsFixedPoint<DecimalBits> Mod(nsFixedPoint<DecimalBits> f, nsFixedPoint<DecimalBits> div)
  {
    NS_REPORT_FAILURE("This function is not really implemented yet.");

    return (nsFixedPoint<DecimalBits>)fmod(f.ToDouble(), div);
  }
}
*/
