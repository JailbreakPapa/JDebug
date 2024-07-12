#include <Texture/TexturePCH.h>

#include <Texture/Image/ImageFilter.h>

nsSimdFloat nsImageFilter::GetWidth() const
{
  return m_fWidth;
}

nsImageFilter::nsImageFilter(float width)
  : m_fWidth(width)
{
}

nsImageFilterBox::nsImageFilterBox(float fWidth)
  : nsImageFilter(fWidth)
{
}

nsSimdFloat nsImageFilterBox::SamplePoint(const nsSimdFloat& x) const
{
  nsSimdFloat absX = x.Abs();

  if (absX <= GetWidth())
  {
    return 1.0f;
  }
  else
  {
    return 0.0f;
  }
}

nsImageFilterTriangle::nsImageFilterTriangle(float fWidth)
  : nsImageFilter(fWidth)
{
}

nsSimdFloat nsImageFilterTriangle::SamplePoint(const nsSimdFloat& x) const
{
  nsSimdFloat absX = x.Abs();

  nsSimdFloat width = GetWidth();

  if (absX <= width)
  {
    return width - absX;
  }
  else
  {
    return 0.0f;
  }
}

static nsSimdFloat sinc(const nsSimdFloat& x)
{
  nsSimdFloat absX = x.Abs();

  // Use Taylor expansion for small values to avoid division
  if (absX < 0.0001f)
  {
    // sin(x) / x = (x - x^3/6 + x^5/120 - ...) / x = 1 - x^2/6 + x^4/120 - ...
    return nsSimdFloat(1.0f) - x * x * nsSimdFloat(1.0f / 6.0f);
  }
  else
  {
    return nsMath::Sin(nsAngle::MakeFromRadian(x)) / x;
  }
}

static nsSimdFloat modifiedBessel0(const nsSimdFloat& x)
{
  // Implementation as I0(x) = sum((1/4 * x * x) ^ k / (k!)^2, k, 0, inf), see
  // http://mathworld.wolfram.com/ModifiedBesselFunctionoftheFirstKind.html

  nsSimdFloat sum = 1.0f;

  nsSimdFloat xSquared = x * x * nsSimdFloat(0.25f);

  nsSimdFloat currentTerm = xSquared;

  for (nsUInt32 i = 2; currentTerm > 0.001f; ++i)
  {
    sum += currentTerm;
    currentTerm *= xSquared / nsSimdFloat(i * i);
  }

  return sum;
}

nsImageFilterSincWithKaiserWindow::nsImageFilterSincWithKaiserWindow(float fWidth, float fBeta)
  : nsImageFilter(fWidth)
  , m_fBeta(fBeta)
  , m_fInvBesselBeta(1.0f / modifiedBessel0(m_fBeta))
{
}

nsSimdFloat nsImageFilterSincWithKaiserWindow::SamplePoint(const nsSimdFloat& x) const
{
  nsSimdFloat scaledX = x / GetWidth();

  nsSimdFloat xSq = 1.0f - scaledX * scaledX;

  if (xSq <= 0.0f)
  {
    return 0.0f;
  }
  else
  {
    return sinc(x * nsSimdFloat(nsMath::Pi<float>())) * modifiedBessel0(m_fBeta * xSq.GetSqrt()) * m_fInvBesselBeta;
  }
}

nsImageFilterWeights::nsImageFilterWeights(const nsImageFilter& filter, nsUInt32 uiSrcSamples, nsUInt32 uiDstSamples)
{
  // Filter weights repeat after the common phase
  nsUInt32 commonPhase = nsMath::GreatestCommonDivisor(uiSrcSamples, uiDstSamples);

  uiSrcSamples /= commonPhase;
  uiDstSamples /= commonPhase;

  m_uiDstSamplesReduced = uiDstSamples;

  m_fSourceToDestScale = float(uiDstSamples) / float(uiSrcSamples);
  m_fDestToSourceScale = float(uiSrcSamples) / float(uiDstSamples);

  nsSimdFloat filterScale, invFilterScale;

  if (uiDstSamples > uiSrcSamples)
  {
    // When upsampling, reconstruct the source by applying the filter in source space and resampling
    filterScale = 1.0f;
    invFilterScale = 1.0f;
  }
  else
  {
    // When downsampling, widen the filter in order to narrow its frequency spectrum, which effectively combines reconstruction + low-pass
    // filter
    filterScale = m_fDestToSourceScale;
    invFilterScale = m_fSourceToDestScale;
  }

  m_fWidthInSourceSpace = filter.GetWidth() * filterScale;

  m_uiNumWeights = nsUInt32(nsMath::Ceil(m_fWidthInSourceSpace * nsSimdFloat(2.0f))) + 1;

  m_Weights.SetCountUninitialized(uiDstSamples * m_uiNumWeights);

  for (nsUInt32 dstSample = 0; dstSample < uiDstSamples; ++dstSample)
  {
    nsSimdFloat dstSampleInSourceSpace = (nsSimdFloat(dstSample) + nsSimdFloat(0.5f)) * m_fDestToSourceScale;

    nsInt32 firstSourceIdx = GetFirstSourceSampleIndex(dstSample);

    nsSimdFloat totalWeight = 0.0f;

    for (nsUInt32 weightIdx = 0; weightIdx < m_uiNumWeights; ++weightIdx)
    {
      nsSimdFloat sourceSample = nsSimdFloat(firstSourceIdx + nsInt32(weightIdx)) + nsSimdFloat(0.5f);

      nsSimdFloat weight = filter.SamplePoint((dstSampleInSourceSpace - sourceSample) * invFilterScale);
      totalWeight += weight;
      m_Weights[dstSample * m_uiNumWeights + weightIdx] = weight;
    }

    // Normalize weights
    nsSimdFloat invWeight = 1.0f / totalWeight;

    for (nsUInt32 weightIdx = 0; weightIdx < m_uiNumWeights; ++weightIdx)
    {
      m_Weights[dstSample * m_uiNumWeights + weightIdx] *= invWeight;
    }
  }
}

nsUInt32 nsImageFilterWeights::GetNumWeights() const
{
  return m_uiNumWeights;
}

nsSimdFloat nsImageFilterWeights::GetWeight(nsUInt32 uiDstSampleIndex, nsUInt32 uiWeightIndex) const
{
  NS_ASSERT_DEBUG(uiWeightIndex < m_uiNumWeights, "Invalid weight index {} (should be < {})", uiWeightIndex, m_uiNumWeights);

  return nsSimdFloat(m_Weights[(uiDstSampleIndex % m_uiDstSamplesReduced) * m_uiNumWeights + uiWeightIndex]);
}
