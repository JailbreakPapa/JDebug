#pragma once

#include <RendererCore/RendererCoreDLL.h>

#include <Foundation/Math/Mat3.h>
#include <Foundation/Math/Transform.h>

/// \brief A wrapper class that converts a nsMat3 into the correct data layout for shaders.
class nsShaderMat3
{
public:
  NS_DECLARE_POD_TYPE();

  NS_ALWAYS_INLINE nsShaderMat3() = default;

  NS_ALWAYS_INLINE nsShaderMat3(const nsMat3& m) { *this = m; }

  NS_FORCE_INLINE void operator=(const nsMat3& m)
  {
    for (nsUInt32 c = 0; c < 3; ++c)
    {
      m_Data[c * 4 + 0] = m.Element(c, 0);
      m_Data[c * 4 + 1] = m.Element(c, 1);
      m_Data[c * 4 + 2] = m.Element(c, 2);
      m_Data[c * 4 + 3] = 0.0f;
    }
  }

private:
  float m_Data[12];
};

/// \brief A wrapper class that converts a nsTransform into the correct data layout for shaders.
class nsShaderTransform
{
public:
  NS_DECLARE_POD_TYPE();

  NS_ALWAYS_INLINE nsShaderTransform() = default;

  inline void operator=(const nsTransform& t) { *this = t.GetAsMat4(); }

  inline void operator=(const nsMat4& t)
  {
    float data[16];
    t.GetAsArray(data, nsMatrixLayout::RowMajor);

    for (nsUInt32 i = 0; i < 12; ++i)
    {
      m_Data[i] = data[i];
    }
  }

  inline void operator=(const nsMat3& t)
  {
    float data[9];
    t.GetAsArray(data, nsMatrixLayout::RowMajor);

    m_Data[0] = data[0];
    m_Data[1] = data[1];
    m_Data[2] = data[2];
    m_Data[3] = 0;

    m_Data[4] = data[3];
    m_Data[5] = data[4];
    m_Data[6] = data[5];
    m_Data[7] = 0;

    m_Data[8] = data[6];
    m_Data[9] = data[7];
    m_Data[10] = data[8];
    m_Data[11] = 0;
  }

  inline nsMat4 GetAsMat4() const
  {
    nsMat4 res;
    res.SetRow(0, reinterpret_cast<const nsVec4&>(m_Data[0]));
    res.SetRow(1, reinterpret_cast<const nsVec4&>(m_Data[4]));
    res.SetRow(2, reinterpret_cast<const nsVec4&>(m_Data[8]));
    res.SetRow(3, nsVec4(0, 0, 0, 1));

    return res;
  }

  inline nsVec3 GetTranslationVector() const
  {
    return nsVec3(m_Data[3], m_Data[7], m_Data[11]);
  }

private:
  float m_Data[12];
};

/// \brief A wrapper class that converts a bool into the correct data layout for shaders.
class nsShaderBool
{
public:
  NS_DECLARE_POD_TYPE();

  NS_ALWAYS_INLINE nsShaderBool() = default;

  NS_ALWAYS_INLINE nsShaderBool(bool b) { m_uiData = b ? 0xFFFFFFFF : 0; }

  NS_ALWAYS_INLINE void operator=(bool b) { m_uiData = b ? 0xFFFFFFFF : 0; }

private:
  nsUInt32 m_uiData;
};
