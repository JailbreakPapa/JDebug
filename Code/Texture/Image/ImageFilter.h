#pragma once

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/SimdMath/SimdFloat.h>
#include <Texture/TextureDLL.h>

/// \brief Represents a function used for filtering an image.
class NS_TEXTURE_DLL nsImageFilter
{
public:
  /// \brief Samples the filter function at a single point. Note that the distribution isn't necessarily normalized.
  virtual nsSimdFloat SamplePoint(const nsSimdFloat& x) const = 0;

  /// \brief Returns the width of the filter; outside of the interval [-width, width], the filter function is always zero.
  nsSimdFloat GetWidth() const;

protected:
  nsImageFilter(float width);

private:
  nsSimdFloat m_fWidth;
};

/// \brief Box filter
class NS_TEXTURE_DLL nsImageFilterBox : public nsImageFilter
{
public:
  nsImageFilterBox(float fWidth = 0.5f);

  virtual nsSimdFloat SamplePoint(const nsSimdFloat& x) const override;
};

/// \brief Triangle filter
class NS_TEXTURE_DLL nsImageFilterTriangle : public nsImageFilter
{
public:
  nsImageFilterTriangle(float fWidth = 1.0f);

  virtual nsSimdFloat SamplePoint(const nsSimdFloat& x) const override;
};

/// \brief Kaiser-windowed sinc filter
class NS_TEXTURE_DLL nsImageFilterSincWithKaiserWindow : public nsImageFilter
{
public:
  /// \brief Construct a sinc filter with a Kaiser window of the given window width and beta parameter.
  /// Note that the beta parameter (equaling alpha * pi in the mathematical definition of the Kaiser window) is often incorrectly alpha by other
  /// filtering tools.
  nsImageFilterSincWithKaiserWindow(float fWindowWidth = 3.0f, float fBeta = 4.0f);

  virtual nsSimdFloat SamplePoint(const nsSimdFloat& x) const override;

private:
  nsSimdFloat m_fBeta;
  nsSimdFloat m_fInvBesselBeta;
};

/// \brief Pre-computes the required filter weights for rescaling a sequence of image samples.
class NS_TEXTURE_DLL nsImageFilterWeights
{
public:
  /// \brief Pre-compute the weights for the given filter for scaling between the given number of samples.
  nsImageFilterWeights(const nsImageFilter& filter, nsUInt32 uiSrcSamples, nsUInt32 uiDstSamples);

  /// \brief Returns the number of weights.
  nsUInt32 GetNumWeights() const;

  /// \brief Returns the weight used for the source sample GetFirstSourceSampleIndex(dstSampleIndex) + weightIndex
  nsSimdFloat GetWeight(nsUInt32 uiDstSampleIndex, nsUInt32 uiWeightIndex) const;

  /// \brief Returns the index of the first source sample that needs to be weighted to evaluate the destination sample
  inline nsInt32 GetFirstSourceSampleIndex(nsUInt32 uiDstSampleIndex) const;

  nsArrayPtr<const float> ViewWeights() const;

private:
  nsHybridArray<float, 16> m_Weights;
  nsSimdFloat m_fWidthInSourceSpace;
  nsSimdFloat m_fSourceToDestScale;
  nsSimdFloat m_fDestToSourceScale;
  nsUInt32 m_uiNumWeights;
  nsUInt32 m_uiDstSamplesReduced;
};

#include <Texture/Image/Implementation/ImageFilter_inl.h>
