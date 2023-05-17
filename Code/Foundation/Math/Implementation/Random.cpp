#include <Foundation/FoundationPCH.h>

#include <Foundation/Math/Random.h>
#include <Foundation/Time/Timestamp.h>

wdRandom::wdRandom()
{
  for (wdUInt32 i = 0; i < WD_ARRAY_SIZE(m_uiState); ++i)
    m_uiState[i] = 0;

  m_uiIndex = 0xFFFFFFFF;
}

void wdRandom::Initialize(wdUInt64 uiSeed)
{
  // make sure the seed is never zero
  // otherwise the state will become zero and the RNG will produce only zeros
  uiSeed |= 0x0102030405060708;

  m_uiIndex = 0;

  for (wdUInt32 i = 0; i < WD_ARRAY_SIZE(m_uiState); i += 2)
  {
    m_uiState[i + 0] = uiSeed & 0xFFFFFFFF;
    m_uiState[i + 1] = (uiSeed >> 32) & 0xFFFFFFFF;
  }

  // skip the first values to ensure the random number generator is 'warmed up'
  for (wdUInt32 i = 0; i < 128; ++i)
  {
    UInt();
  }
}


void wdRandom::InitializeFromCurrentTime()
{
  wdTimestamp ts = wdTimestamp::CurrentTimestamp();
  Initialize(static_cast<wdUInt64>(ts.GetInt64(wdSIUnitOfTime::Nanosecond)));
}

void wdRandom::Save(wdStreamWriter& inout_stream) const
{
  inout_stream << m_uiIndex;

  inout_stream.WriteBytes(&m_uiState[0], sizeof(wdUInt32) * 16).IgnoreResult();
}


void wdRandom::Load(wdStreamReader& inout_stream)
{
  inout_stream >> m_uiIndex;

  inout_stream.ReadBytes(&m_uiState[0], sizeof(wdUInt32) * 16);
}

wdUInt32 wdRandom::UInt()
{
  WD_ASSERT_DEBUG(m_uiIndex < 16, "Random number generator has not been initialized");

  // Implementation for the random number generator was copied from here:
  // http://stackoverflow.com/questions/1046714/what-is-a-good-random-number-generator-for-a-game
  //
  // It is the WELL algorithm from this paper:
  // http://www.lomont.org/Math/Papers/2008/Lomont_PRNG_2008.pdf

  wdUInt32 a, b, c, d;
  a = m_uiState[m_uiIndex];
  c = m_uiState[(m_uiIndex + 13) & 15];
  b = (a ^ c) ^ (a << 16) ^ (c << 15);
  c = m_uiState[(m_uiIndex + 9) & 15];
  c ^= (c >> 11);
  a = m_uiState[m_uiIndex] = b ^ c;
  d = a ^ ((a << 5) & 0xDA442D24UL);
  m_uiIndex = (m_uiIndex + 15) & 15;
  a = m_uiState[m_uiIndex];
  m_uiState[m_uiIndex] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);
  return m_uiState[m_uiIndex];
}

wdUInt32 wdRandom::UIntInRange(wdUInt32 uiRange)
{
  WD_ASSERT_DEBUG(uiRange > 0, "Invalid range for random number");

  const wdUInt32 uiSteps = 0xFFFFFFFF / uiRange;
  const wdUInt32 uiMaxValue = uiRange * uiSteps;

  wdUInt32 result = 0;

  do
  {
    result = UInt();
  } while (result > uiMaxValue);

  return result % uiRange;
}

wdInt32 wdRandom::IntInRange(wdInt32 iMinValue, wdUInt32 uiRange)
{
  return iMinValue + (wdInt32)UIntInRange(uiRange);
}

wdInt32 wdRandom::IntMinMax(wdInt32 iMinValue, wdInt32 iMaxValue)
{
  WD_ASSERT_DEBUG(iMinValue <= iMaxValue, "Invalid min/max values");

  return IntInRange(iMinValue, iMaxValue - iMinValue + 1);
}

double wdRandom::DoubleInRange(double fMinValue, double fRange)
{
  return fMinValue + DoubleZeroToOneExclusive() * fRange;
}

double wdRandom::DoubleMinMax(double fMinValue, double fMaxValue)
{
  WD_ASSERT_DEBUG(fMinValue <= fMaxValue, "Invalid min/max values");

  return fMinValue + DoubleZeroToOneExclusive() * (fMaxValue - fMinValue); /// \todo Probably not correct
}

double wdRandom::DoubleVariance(double fValue, double fVariance)
{
  /// \todo Test whether this is actually correct

  const double dev = DoubleZeroToOneInclusive();
  const double offset = fValue * fVariance * dev;
  return DoubleMinMax(fValue - offset, fValue + offset);
}

double wdRandom::DoubleVarianceAroundZero(double fAbsMaxValue)
{
  /// \todo Test whether this is actually correct

  const double dev = DoubleZeroToOneInclusive();
  const double offset = fAbsMaxValue * dev;
  return DoubleMinMax(-offset, +offset);
}

static double Gauss(double x, double fSigma)
{
  // taken from https://en.wikipedia.org/wiki/Normal_distribution
  // mue is 0 because we want the curve to center around the origin

  // float G = (1.0f / (sqrt (2.0f * pi) * fSigma)) * exp ((-(x * x) / (2.0f * fSigma * fSigma)));

  const double sqrt2pi = 2.506628274631000502415765284811;

  const double G = (1.0 / (sqrt2pi * fSigma)) * wdMath::Exp((-(x * x) / (2.0 * fSigma * fSigma)));

  return G;
}

void wdRandomGauss::Initialize(wdUInt64 uiRandomSeed, wdUInt32 uiMaxValue, float fVariance)
{
  WD_ASSERT_DEV(uiMaxValue >= 2, "Invalid value");

  m_Generator.Initialize(uiRandomSeed);

  SetupTable(uiMaxValue, wdMath::Sqrt(fVariance));
}


void wdRandomGauss::SetupTable(wdUInt32 uiMaxValue, float fSigma)
{
  // create half a bell curve with a fixed sigma

  const double UsefulRange = 5.0;

  m_fSigma = fSigma;
  m_GaussAreaSum.SetCountUninitialized(uiMaxValue);

  const double fBase2 = Gauss(UsefulRange, fSigma); // we clamp to zero at uiMaxValue, so we need the Gauss value there to subtract it from all other values

  m_fAreaSum = 0;

  for (wdUInt32 i = 0; i < uiMaxValue; ++i)
  {
    const double g = Gauss((UsefulRange / (uiMaxValue - 1)) * i, fSigma) - fBase2;
    m_fAreaSum += g;
    m_GaussAreaSum[i] = (float)m_fAreaSum;
  }
}

wdUInt32 wdRandomGauss::UnsignedValue()
{
  const double fRand = m_Generator.DoubleInRange(0, m_fAreaSum);

  const wdUInt32 uiMax = m_GaussAreaSum.GetCount();
  for (wdUInt32 i = 0; i < uiMax; ++i)
  {
    if (fRand < m_GaussAreaSum[i])
      return i;
  }

  return uiMax - 1;
}

wdInt32 wdRandomGauss::SignedValue()
{
  const double fRand = m_Generator.DoubleInRange(-m_fAreaSum, m_fAreaSum * 2.0);
  const wdUInt32 uiMax = m_GaussAreaSum.GetCount();

  if (fRand >= 0.0)
  {
    for (wdUInt32 i = 0; i < uiMax; ++i)
    {
      if (fRand < m_GaussAreaSum[i])
        return i;
    }

    return uiMax - 1;
  }
  else
  {
    const double fRandAbs = (-fRand);

    for (wdUInt32 i = 0; i < uiMax - 1; ++i)
    {
      if (fRandAbs < m_GaussAreaSum[i])
        return -(wdInt32)i - 1;
    }

    return -(wdInt32)(uiMax - 1);
  }
}

void wdRandomGauss::Save(wdStreamWriter& inout_stream) const
{
  inout_stream << m_GaussAreaSum.GetCount();
  inout_stream << m_fSigma;
  m_Generator.Save(inout_stream);
}

void wdRandomGauss::Load(wdStreamReader& inout_stream)
{
  wdUInt32 uiMax = 0;
  inout_stream >> uiMax;

  float fVariance = 0.0f;
  inout_stream >> fVariance;

  SetupTable(uiMax, fVariance);

  m_Generator.Load(inout_stream);
}



WD_STATICLINK_FILE(Foundation, Foundation_Math_Implementation_Random);
