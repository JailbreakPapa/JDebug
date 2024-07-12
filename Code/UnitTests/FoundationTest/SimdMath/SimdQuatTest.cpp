#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/SimdMath/SimdQuat.h>

NS_CREATE_SIMPLE_TEST(SimdMath, SimdQuat)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    // In debug the default constructor initializes everything with NaN.
    nsSimdQuat vDefCtor;
    NS_TEST_BOOL(vDefCtor.IsNaN());
#else

#  if NS_DISABLED(NS_COMPILER_GCC)
    // Placement new of the default constructor should not have any effect on the previous data.
    alignas(16) float testBlock[4] = {1, 2, 3, 4};
    nsSimdQuat* pDefCtor = ::new ((void*)&testBlock[0]) nsSimdQuat;
    NS_TEST_BOOL(pDefCtor->m_v.x() == 1.0f && pDefCtor->m_v.y() == 2.0f && pDefCtor->m_v.z() == 3.0f && pDefCtor->m_v.w() == 4.0f);
#  endif

#endif

    // Make sure the class didn't accidentally change in size.
#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
    NS_CHECK_AT_COMPILETIME(sizeof(nsSimdQuat) == 16);
    NS_CHECK_AT_COMPILETIME(NS_ALIGNMENT_OF(nsSimdQuat) == 16);
#endif
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IdentityQuaternion")
  {
    nsSimdQuat q = nsSimdQuat::MakeIdentity();

    NS_TEST_BOOL(q.m_v.x() == 0.0f && q.m_v.y() == 0.0f && q.m_v.z() == 0.0f && q.m_v.w() == 1.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetIdentity")
  {
    nsSimdQuat q(nsSimdVec4f(1, 2, 3, 4));

    q = nsSimdQuat::MakeIdentity();

    NS_TEST_BOOL(q.m_v.x() == 0.0f && q.m_v.y() == 0.0f && q.m_v.z() == 0.0f && q.m_v.w() == 1.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetFromAxisAndAngle / operator* (quat, vec)")
  {
    {
      nsSimdQuat q = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(1, 0, 0), nsAngle::MakeFromDegree(90));

      NS_TEST_BOOL((q * nsSimdVec4f(0, 1, 0)).IsEqual(nsSimdVec4f(0, 0, 1), 0.0001f).AllSet());
    }

    {
      nsSimdQuat q = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 1, 0), nsAngle::MakeFromDegree(90));

      NS_TEST_BOOL((q * nsSimdVec4f(1, 0, 0)).IsEqual(nsSimdVec4f(0, 0, -1), 0.0001f).AllSet());
    }

    {
      nsSimdQuat q = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(90));

      NS_TEST_BOOL((q * nsSimdVec4f(0, 1, 0)).IsEqual(nsSimdVec4f(-1, 0, 0), 0.0001f).AllSet());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetShortestRotation / IsEqualRotation")
  {
    nsSimdQuat q1, q2, q3;
    q1 = nsSimdQuat::MakeShortestRotation(nsSimdVec4f(0, 1, 0), nsSimdVec4f(1, 0, 0));
    q2 = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, -1), nsAngle::MakeFromDegree(90));
    q3 = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(-90));

    NS_TEST_BOOL(q1.IsEqualRotation(q2, nsMath::LargeEpsilon<float>()));
    NS_TEST_BOOL(q1.IsEqualRotation(q3, nsMath::LargeEpsilon<float>()));

    NS_TEST_BOOL(nsSimdQuat::MakeIdentity().IsEqualRotation(nsSimdQuat::MakeIdentity(), nsMath::LargeEpsilon<float>()));
    NS_TEST_BOOL(nsSimdQuat::MakeIdentity().IsEqualRotation(nsSimdQuat(nsSimdVec4f(0, 0, 0, -1)), nsMath::LargeEpsilon<float>()));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetSlerp")
  {
    nsSimdQuat q1, q2, q3, qr;
    q1 = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(45));
    q2 = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(0));
    q3 = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(90));

    qr = nsSimdQuat::MakeSlerp(q2, q3, 0.5f);

    NS_TEST_BOOL(q1.IsEqualRotation(qr, 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetRotationAxisAndAngle")
  {
    nsSimdQuat q1, q2, q3;
    q1 = nsSimdQuat::MakeShortestRotation(nsSimdVec4f(0, 1, 0), nsSimdVec4f(1, 0, 0));
    q2 = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, -1), nsAngle::MakeFromDegree(90));
    q3 = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(-90));

    nsSimdVec4f axis;
    nsSimdFloat angle;

    NS_TEST_BOOL(q1.GetRotationAxisAndAngle(axis, angle) == NS_SUCCESS);
    NS_TEST_BOOL(axis.IsEqual(nsSimdVec4f(0, 0, -1), 0.001f).AllSet<3>());
    NS_TEST_FLOAT(nsAngle::RadToDeg((float)angle), 90, nsMath::LargeEpsilon<float>());

    NS_TEST_BOOL(q2.GetRotationAxisAndAngle(axis, angle) == NS_SUCCESS);
    NS_TEST_BOOL(axis.IsEqual(nsSimdVec4f(0, 0, -1), 0.001f).AllSet<3>());
    NS_TEST_FLOAT(nsAngle::RadToDeg((float)angle), 90, nsMath::LargeEpsilon<float>());

    NS_TEST_BOOL(q3.GetRotationAxisAndAngle(axis, angle) == NS_SUCCESS);
    NS_TEST_BOOL(axis.IsEqual(nsSimdVec4f(0, 0, -1), 0.001f).AllSet<3>());
    NS_TEST_FLOAT(nsAngle::RadToDeg((float)angle), 90, nsMath::LargeEpsilon<float>());

    NS_TEST_BOOL(nsSimdQuat::MakeIdentity().GetRotationAxisAndAngle(axis, angle) == NS_SUCCESS);
    NS_TEST_BOOL(axis.IsEqual(nsSimdVec4f(1, 0, 0), 0.001f).AllSet<3>());
    NS_TEST_FLOAT(nsAngle::RadToDeg((float)angle), 0, nsMath::LargeEpsilon<float>());

    nsSimdQuat otherIdentity(nsSimdVec4f(0, 0, 0, -1));
    NS_TEST_BOOL(otherIdentity.GetRotationAxisAndAngle(axis, angle) == NS_SUCCESS);
    NS_TEST_BOOL(axis.IsEqual(nsSimdVec4f(1, 0, 0), 0.001f).AllSet<3>());
    NS_TEST_FLOAT(nsAngle::RadToDeg((float)angle), 360, nsMath::LargeEpsilon<float>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsValid / Normalize")
  {
    nsSimdQuat q(nsSimdVec4f(1, 2, 3, 4));
    NS_TEST_BOOL(!q.IsValid(0.001f));

    q.Normalize();
    NS_TEST_BOOL(q.IsValid(0.001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator-")
  {
    nsSimdQuat q, q1;
    q = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(90));
    q1 = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(-90));

    nsSimdQuat q2 = -q;
    NS_TEST_BOOL(q1.IsEqualRotation(q2, 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator*(quat, quat)")
  {
    nsSimdQuat q1, q2, qr, q3;
    q1 = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(60));
    q2 = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(30));
    q3 = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(90));

    qr = q1 * q2;

    NS_TEST_BOOL(qr.IsEqualRotation(q3, 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator==/!=")
  {
    nsSimdQuat q1, q2;
    q1 = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(60));
    q2 = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(30));
    NS_TEST_BOOL(q1 != q2);

    q2 = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(1, 0, 0), nsAngle::MakeFromDegree(60));
    NS_TEST_BOOL(q1 != q2);

    q2 = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(60));
    NS_TEST_BOOL(q1 == q2);
  }
}
