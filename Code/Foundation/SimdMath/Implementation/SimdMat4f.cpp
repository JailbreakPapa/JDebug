#include <Foundation/FoundationPCH.h>

#include <Foundation/Math/Mat4.h>
#include <Foundation/SimdMath/SimdMat4f.h>

///\todo optimize

wdResult wdSimdMat4f::Invert(const wdSimdFloat& fEpsilon)
{
  wdMat4 tmp;
  GetAsArray(tmp.m_fElementsCM, wdMatrixLayout::ColumnMajor);

  if (tmp.Invert(fEpsilon).Failed())
    return WD_FAILURE;

  SetFromArray(tmp.m_fElementsCM, wdMatrixLayout::ColumnMajor);

  return WD_SUCCESS;
}



WD_STATICLINK_FILE(Foundation, Foundation_SimdMath_Implementation_SimdMat4f);
