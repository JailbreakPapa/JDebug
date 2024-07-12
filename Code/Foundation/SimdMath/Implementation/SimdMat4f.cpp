#include <Foundation/FoundationPCH.h>

#include <Foundation/Math/Mat4.h>
#include <Foundation/SimdMath/SimdMat4f.h>

///\todo optimize

nsResult nsSimdMat4f::Invert(const nsSimdFloat& fEpsilon)
{
  nsMat4 tmp;
  GetAsArray(tmp.m_fElementsCM, nsMatrixLayout::ColumnMajor);

  if (tmp.Invert(fEpsilon).Failed())
    return NS_FAILURE;

  *this = nsSimdMat4f::MakeFromColumnMajorArray(tmp.m_fElementsCM);

  return NS_SUCCESS;
}
