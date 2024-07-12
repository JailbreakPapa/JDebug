nsInt32 nsImageFilterWeights::GetFirstSourceSampleIndex(nsUInt32 uiDstSampleIndex) const
{
  nsSimdFloat dstSampleInSourceSpace = (nsSimdFloat(uiDstSampleIndex) + nsSimdFloat(0.5f)) * m_fDestToSourceScale;

  return nsInt32(nsMath::Floor(dstSampleInSourceSpace - m_fWidthInSourceSpace));
}

inline nsArrayPtr<const float> nsImageFilterWeights::ViewWeights() const
{
  return m_Weights;
}
