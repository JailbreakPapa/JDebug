#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/Transform.h>

NS_CREATE_SIMPLE_TEST(Math, Transform)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructors")
  {
    nsTransformT t0;

    {
      nsTransformT t(nsVec3T(1, 2, 3));
      NS_TEST_VEC3(t.m_vPosition, nsVec3T(1, 2, 3), 0);
    }

    {
      nsQuat qRot;
      qRot = nsQuat::MakeFromAxisAndAngle(nsVec3T(1, 2, 3).GetNormalized(), nsAngle::MakeFromDegree(42.0f));

      nsTransformT t(nsVec3T(4, 5, 6), qRot);
      NS_TEST_VEC3(t.m_vPosition, nsVec3T(4, 5, 6), 0);
      NS_TEST_BOOL(t.m_qRotation == qRot);
    }

    {
      nsMat3 mRot = nsMat3::MakeAxisRotation(nsVec3T(1, 2, 3).GetNormalized(), nsAngle::MakeFromDegree(42.0f));

      nsQuat q;
      q = nsQuat::MakeFromMat3(mRot);

      nsTransformT t(nsVec3T(4, 5, 6), q);
      NS_TEST_VEC3(t.m_vPosition, nsVec3T(4, 5, 6), 0);
      NS_TEST_BOOL(t.m_qRotation.GetAsMat3().IsEqual(mRot, 0.0001f));
    }

    {
      nsQuat qRot;
      qRot.SetIdentity();

      nsTransformT t(nsVec3T(4, 5, 6), qRot, nsVec3T(2, 3, 4));
      NS_TEST_VEC3(t.m_vPosition, nsVec3T(4, 5, 6), 0);
      NS_TEST_BOOL(t.m_qRotation.GetAsMat3().IsEqual(nsMat3::MakeFromValues(1, 0, 0, 0, 1, 0, 0, 0, 1), 0.001f));
      NS_TEST_VEC3(t.m_vScale, nsVec3T(2, 3, 4), 0);
    }

    {
      nsMat3T mRot = nsMat3::MakeAxisRotation(nsVec3T(1, 2, 3).GetNormalized(), nsAngle::MakeFromDegree(42.0f));
      nsMat4T mTrans;
      mTrans.SetTransformationMatrix(mRot, nsVec3T(1, 2, 3));

      nsTransformT t = nsTransform::MakeFromMat4(mTrans);
      NS_TEST_VEC3(t.m_vPosition, nsVec3T(1, 2, 3), 0);
      NS_TEST_BOOL(t.m_qRotation.GetAsMat3().IsEqual(mRot, 0.001f));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetIdentity")
  {
    nsTransformT t;
    t.SetIdentity();

    NS_TEST_VEC3(t.m_vPosition, nsVec3T(0), 0);
    NS_TEST_BOOL(t.m_qRotation == nsQuat::MakeIdentity());
    NS_TEST_BOOL(t.m_vScale == nsVec3T(1.0f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetAsMat4")
  {
    nsQuat qRot;
    qRot.SetIdentity();

    nsTransformT t(nsVec3T(4, 5, 6), qRot, nsVec3T(2, 3, 4));
    NS_TEST_BOOL(t.GetAsMat4() == nsMat4T::MakeFromValues(2, 0, 0, 4, 0, 3, 0, 5, 0, 0, 4, 6, 0, 0, 0, 1));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator + / -")
  {
    nsTransformT t0, t1;
    t0.SetIdentity();
    t1.SetIdentity();

    t1 = t0 + nsVec3T(2, 3, 4);
    NS_TEST_VEC3(t1.m_vPosition, nsVec3T(2, 3, 4), 0.0001f);

    t1 = t1 - nsVec3T(4, 2, 1);
    NS_TEST_VEC3(t1.m_vPosition, nsVec3T(-2, 1, 3), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator * (quat)")
  {
    nsQuat qRotX, qRotY;
    qRotX = nsQuat::MakeFromAxisAndAngle(nsVec3T(1, 0, 0), nsAngle::MakeFromRadian(1.57079637f));
    qRotY = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 1, 0), nsAngle::MakeFromRadian(1.57079637f));

    nsTransformT t0, t1;
    t0.SetIdentity();
    t1.SetIdentity();

    t1 = qRotX * t0;
    NS_TEST_VEC3(t1.m_vPosition, nsVec3T(0, 0, 0), 0.0001f);

    nsQuat q;
    q = nsQuat::MakeFromMat3(nsMat3::MakeFromValues(1, 0, 0, 0, 0, -1, 0, 1, 0));
    NS_TEST_BOOL(t1.m_qRotation.IsEqualRotation(q, 0.0001f));

    t1 = qRotY * t1;
    NS_TEST_VEC3(t1.m_vPosition, nsVec3T(0, 0, 0), 0.0001f);
    q = nsQuat::MakeFromMat3(nsMat3::MakeFromValues(0, 1, 0, 0, 0, -1, -1, 0, 0));
    NS_TEST_BOOL(t1.m_qRotation.IsEqualRotation(q, 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator * (vec3)")
  {
    nsQuat qRotX, qRotY;
    qRotX = nsQuat::MakeFromAxisAndAngle(nsVec3T(1, 0, 0), nsAngle::MakeFromRadian(1.57079637f));
    qRotY = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 1, 0), nsAngle::MakeFromRadian(1.57079637f));

    nsTransformT t;
    t.SetIdentity();

    t = qRotX * t;

    NS_TEST_BOOL(t.m_qRotation.IsEqualRotation(qRotX, 0.0001f));
    NS_TEST_VEC3(t.m_vPosition, nsVec3T(0, 0, 0), 0.0001f);
    NS_TEST_VEC3(t.m_vScale, nsVec3T(1, 1, 1), 0.0001f);

    t = t + nsVec3T(1, 2, 3);

    NS_TEST_BOOL(t.m_qRotation.IsEqualRotation(qRotX, 0.0001f));
    NS_TEST_VEC3(t.m_vPosition, nsVec3T(1, 2, 3), 0.0001f);
    NS_TEST_VEC3(t.m_vScale, nsVec3T(1, 1, 1), 0.0001f);

    t = qRotY * t;

    NS_TEST_BOOL(t.m_qRotation.IsEqualRotation(qRotY * qRotX, 0.0001f));
    NS_TEST_VEC3(t.m_vPosition, nsVec3T(1, 2, 3), 0.0001f);
    NS_TEST_VEC3(t.m_vScale, nsVec3T(1, 1, 1), 0.0001f);

    nsQuat q;
    q = nsQuat::MakeFromMat3(nsMat3::MakeFromValues(0, 1, 0, 0, 0, -1, -1, 0, 0));
    NS_TEST_BOOL(t.m_qRotation.IsEqualRotation(q, 0.0001f));

    nsVec3T v;
    v = t * nsVec3T(4, 5, 6);

    NS_TEST_VEC3(v, nsVec3T(5 + 1, -6 + 2, -4 + 3), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsIdentical")
  {
    nsTransformT t(nsVec3T(1, 2, 3));
    t.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(90));

    NS_TEST_BOOL(t.IsIdentical(t));

    nsTransformT t2(nsVec3T(1, 2, 4));
    t2.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(90));

    NS_TEST_BOOL(!t.IsIdentical(t2));

    nsTransformT t3(nsVec3T(1, 2, 3));
    t3.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(91));

    NS_TEST_BOOL(!t.IsIdentical(t3));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator == / !=")
  {
    nsTransformT t(nsVec3T(1, 2, 3));
    t.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(90));

    NS_TEST_BOOL(t == t);

    nsTransformT t2(nsVec3T(1, 2, 4));
    t2.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(90));

    NS_TEST_BOOL(t != t2);

    nsTransformT t3(nsVec3T(1, 2, 3));
    t3.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(91));

    NS_TEST_BOOL(t != t3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqual")
  {
    nsTransformT t(nsVec3T(1, 2, 3));
    t.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(90));

    NS_TEST_BOOL(t.IsEqual(t, 0.0001f));

    nsTransformT t2(nsVec3T(1, 2, 3.0002f));
    t2.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(90));

    NS_TEST_BOOL(t.IsEqual(t2, 0.001f));
    NS_TEST_BOOL(!t.IsEqual(t2, 0.0001f));

    nsTransformT t3(nsVec3T(1, 2, 3));
    t3.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(90.01f));

    NS_TEST_BOOL(t.IsEqual(t3, 0.01f));
    NS_TEST_BOOL(!t.IsEqual(t3, 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator*(nsTransformT, nsTransformT)")
  {
    nsTransformT tParent(nsVec3T(1, 2, 3));
    tParent.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 1, 0), nsAngle::MakeFromRadian(1.57079637f));
    tParent.m_vScale.Set(2);

    nsTransformT tToChild(nsVec3T(4, 5, 6));
    tToChild.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromRadian(1.57079637f));
    tToChild.m_vScale.Set(4);

    // this is exactly the same as SetGlobalTransform
    nsTransformT tChild;
    tChild = tParent * tToChild;

    NS_TEST_VEC3(tChild.m_vPosition, nsVec3T(13, 12, -5), 0.003f);
    NS_TEST_BOOL(tChild.m_qRotation.GetAsMat3().IsEqual(nsMat3::MakeFromValues(0, 0, 1, 1, 0, 0, 0, 1, 0), 0.0001f));
    NS_TEST_VEC3(tChild.m_vScale, nsVec3T(8, 8, 8), 0.0001f);

    // verify that it works exactly like a 4x4 matrix
    const nsMat4 mParent = tParent.GetAsMat4();
    const nsMat4 mToChild = tToChild.GetAsMat4();
    const nsMat4 mChild = mParent * mToChild;

    NS_TEST_BOOL(mChild.IsEqual(tChild.GetAsMat4(), 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator*(nsTransformT, nsMat4)")
  {
    nsTransformT tParent(nsVec3T(1, 2, 3));
    tParent.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(90));
    tParent.m_vScale.Set(2);

    nsTransformT tToChild(nsVec3T(4, 5, 6));
    tToChild.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(90));
    tToChild.m_vScale.Set(4);

    // this is exactly the same as SetGlobalTransform
    nsTransformT tChild;
    tChild = tParent * tToChild;

    NS_TEST_VEC3(tChild.m_vPosition, nsVec3T(13, 12, -5), 0.0001f);
    NS_TEST_BOOL(tChild.m_qRotation.GetAsMat3().IsEqual(nsMat3::MakeFromValues(0, 0, 1, 1, 0, 0, 0, 1, 0), 0.0001f));
    NS_TEST_VEC3(tChild.m_vScale, nsVec3T(8, 8, 8), 0.0001f);

    // verify that it works exactly like a 4x4 matrix
    const nsMat4 mParent = tParent.GetAsMat4();
    const nsMat4 mToChild = tToChild.GetAsMat4();
    const nsMat4 mChild = mParent * mToChild;

    NS_TEST_BOOL(mChild.IsEqual(tChild.GetAsMat4(), 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator*(nsMat4, nsTransformT)")
  {
    nsTransformT tParent(nsVec3T(1, 2, 3));
    tParent.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(90));
    tParent.m_vScale.Set(2);

    nsTransformT tToChild(nsVec3T(4, 5, 6));
    tToChild.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(90));
    tToChild.m_vScale.Set(4);

    // this is exactly the same as SetGlobalTransform
    nsTransformT tChild;
    tChild = tParent * tToChild;

    NS_TEST_VEC3(tChild.m_vPosition, nsVec3T(13, 12, -5), 0.0001f);
    NS_TEST_BOOL(tChild.m_qRotation.GetAsMat3().IsEqual(nsMat3::MakeFromValues(0, 0, 1, 1, 0, 0, 0, 1, 0), 0.0001f));
    NS_TEST_VEC3(tChild.m_vScale, nsVec3T(8, 8, 8), 0.0001f);

    // verify that it works exactly like a 4x4 matrix
    const nsMat4 mParent = tParent.GetAsMat4();
    const nsMat4 mToChild = tToChild.GetAsMat4();
    const nsMat4 mChild = mParent * mToChild;

    NS_TEST_BOOL(mChild.IsEqual(tChild.GetAsMat4(), 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Invert / GetInverse")
  {
    nsTransformT tParent(nsVec3T(1, 2, 3));
    tParent.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(90));
    tParent.m_vScale.Set(2);

    nsTransformT tToChild(nsVec3T(4, 5, 6));
    tToChild.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(90));
    tToChild.m_vScale.Set(4);

    nsTransformT tChild;
    tChild = nsTransform::MakeGlobalTransform(tParent, tToChild);

    // negate twice -> get back original
    tToChild.Invert();
    tToChild.Invert();

    nsTransformT tInvToChild = tToChild.GetInverse();

    nsTransformT tParentFromChild;
    tParentFromChild = nsTransform::MakeGlobalTransform(tChild, tInvToChild);

    NS_TEST_BOOL(tParent.IsEqual(tParentFromChild, 0.0001f));
  }

  //////////////////////////////////////////////////////////////////////////
  // Tests copied and ported over from nsSimdTransform
  //////////////////////////////////////////////////////////////////////////

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    nsTransform t0;

    {
      nsQuat qRot;
      qRot = nsQuat::MakeFromAxisAndAngle(nsVec3(1, 2, 3).GetNormalized(), nsAngle::MakeFromDegree(42.0f));

      nsVec3 pos(4, 5, 6);
      nsVec3 scale(7, 8, 9);

      nsTransform t(pos);
      NS_TEST_BOOL((t.m_vPosition == pos));
      NS_TEST_BOOL(t.m_qRotation == nsQuat::MakeIdentity());
      NS_TEST_BOOL((t.m_vScale == nsVec3(1)));

      t = nsTransform(pos, qRot);
      NS_TEST_BOOL((t.m_vPosition == pos));
      NS_TEST_BOOL(t.m_qRotation == qRot);
      NS_TEST_BOOL((t.m_vScale == nsVec3(1)));

      t = nsTransform(pos, qRot, scale);
      NS_TEST_BOOL((t.m_vPosition == pos));
      NS_TEST_BOOL(t.m_qRotation == qRot);
      NS_TEST_BOOL((t.m_vScale == scale));

      t = nsTransform(nsVec3::MakeZero(), qRot);
      NS_TEST_BOOL(t.m_vPosition.IsZero());
      NS_TEST_BOOL(t.m_qRotation == qRot);
      NS_TEST_BOOL((t.m_vScale == nsVec3(1)));
    }

    {
      nsTransform t;
      t.SetIdentity();

      NS_TEST_BOOL(t.m_vPosition.IsZero());
      NS_TEST_BOOL(t.m_qRotation == nsQuat::MakeIdentity());
      NS_TEST_BOOL((t.m_vScale == nsVec3(1)));

      NS_TEST_BOOL(t == nsTransform::MakeIdentity());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Inverse")
  {
    nsTransform tParent(nsVec3(1, 2, 3));
    tParent.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 1, 0), nsAngle::MakeFromDegree(90));
    tParent.m_vScale = nsVec3(2);

    nsTransform tToChild(nsVec3(4, 5, 6));
    tToChild.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 0, 1), nsAngle::MakeFromDegree(90));
    tToChild.m_vScale = nsVec3(4);

    nsTransform tChild;
    tChild = tParent * tToChild;

    // invert twice -> get back original
    nsTransform t2 = tToChild;
    t2.Invert();
    t2.Invert();
    NS_TEST_BOOL(t2.IsEqual(tToChild, 0.0001f));

    nsTransform tInvToChild = tToChild.GetInverse();

    nsTransform tParentFromChild;
    tParentFromChild = tChild * tInvToChild;

    NS_TEST_BOOL(tParent.IsEqual(tParentFromChild, 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetLocalTransform")
  {
    nsQuat q;
    q = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 0, 1), nsAngle::MakeFromDegree(90));

    nsTransform tParent(nsVec3(1, 2, 3));
    tParent.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 1, 0), nsAngle::MakeFromDegree(90));
    tParent.m_vScale = nsVec3(2);

    nsTransform tChild;
    tChild.m_vPosition = nsVec3(13, 12, -5);
    tChild.m_qRotation = tParent.m_qRotation * q;
    tChild.m_vScale = nsVec3(8);

    nsTransform tToChild;
    tToChild = nsTransform::MakeLocalTransform(tParent, tChild);

    NS_TEST_BOOL(tToChild.m_vPosition.IsEqual(nsVec3(4, 5, 6), 0.0001f));
    NS_TEST_BOOL(tToChild.m_qRotation.IsEqualRotation(q, 0.0001f));
    NS_TEST_BOOL((tToChild.m_vScale == nsVec3(4)));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetGlobalTransform")
  {
    nsTransform tParent(nsVec3(1, 2, 3));
    tParent.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 1, 0), nsAngle::MakeFromDegree(90));
    tParent.m_vScale = nsVec3(2);

    nsTransform tToChild(nsVec3(4, 5, 6));
    tToChild.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 0, 1), nsAngle::MakeFromDegree(90));
    tToChild.m_vScale = nsVec3(4);

    nsTransform tChild;
    tChild = nsTransform::MakeGlobalTransform(tParent, tToChild);

    NS_TEST_BOOL(tChild.m_vPosition.IsEqual(nsVec3(13, 12, -5), 0.0001f));
    NS_TEST_BOOL(tChild.m_qRotation.IsEqualRotation(tParent.m_qRotation * tToChild.m_qRotation, 0.0001f));
    NS_TEST_BOOL((tChild.m_vScale == nsVec3(8)));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetAsMat4")
  {
    nsTransform t(nsVec3(1, 2, 3));
    t.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 1, 0), nsAngle::MakeFromDegree(34));
    t.m_vScale = nsVec3(2, -1, 5);

    nsMat4 m = t.GetAsMat4();

    nsMat4 refM;
    refM.SetZero();
    {
      nsQuat q;
      q = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 1, 0), nsAngle::MakeFromDegree(34));

      nsTransform referenceTransform(nsVec3(1, 2, 3), q, nsVec3(2, -1, 5));
      nsMat4 tmp = referenceTransform.GetAsMat4();
      refM = nsMat4::MakeFromColumnMajorArray(tmp.m_fElementsCM);
    }
    NS_TEST_BOOL(m.IsEqual(refM, 0.00001f));

    nsVec3 p[8] = {
      nsVec3(-4, 0, 0), nsVec3(5, 0, 0), nsVec3(0, -6, 0), nsVec3(0, 7, 0), nsVec3(0, 0, -8), nsVec3(0, 0, 9), nsVec3(1, -2, 3), nsVec3(-4, 5, 7)};

    for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(p); ++i)
    {
      nsVec3 pt = t.TransformPosition(p[i]);
      nsVec3 pm = m.TransformPosition(p[i]);

      NS_TEST_BOOL(pt.IsEqual(pm, 0.00001f));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TransformPos / Dir / operator*")
  {
    nsQuat qRotX, qRotY;
    qRotX = nsQuat::MakeFromAxisAndAngle(nsVec3(1, 0, 0), nsAngle::MakeFromDegree(90.0f));
    qRotY = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 1, 0), nsAngle::MakeFromDegree(90.0f));

    nsTransform t(nsVec3(1, 2, 3), qRotY * qRotX, nsVec3(2, -2, 4));

    nsVec3 v;
    v = t.TransformPosition(nsVec3(4, 5, 6));
    NS_TEST_BOOL(v.IsEqual(nsVec3((5 * -2) + 1, (-6 * 4) + 2, (-4 * 2) + 3), 0.0001f));

    v = t.TransformDirection(nsVec3(4, 5, 6));
    NS_TEST_BOOL(v.IsEqual(nsVec3((5 * -2), (-6 * 4), (-4 * 2)), 0.0001f));

    v = t * nsVec3(4, 5, 6);
    NS_TEST_BOOL(v.IsEqual(nsVec3((5 * -2) + 1, (-6 * 4) + 2, (-4 * 2) + 3), 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Operators")
  {
    {
      nsTransform tParent(nsVec3(1, 2, 3));
      tParent.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 1, 0), nsAngle::MakeFromDegree(90));
      tParent.m_vScale = nsVec3(2);

      nsTransform tToChild(nsVec3(4, 5, 6));
      tToChild.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 0, 1), nsAngle::MakeFromDegree(90));
      tToChild.m_vScale = nsVec3(4);

      // this is exactly the same as SetGlobalTransform
      nsTransform tChild;
      tChild = tParent * tToChild;

      NS_TEST_BOOL(tChild.m_vPosition.IsEqual(nsVec3(13, 12, -5), 0.0001f));
      NS_TEST_BOOL(tChild.m_qRotation.IsEqualRotation(tParent.m_qRotation * tToChild.m_qRotation, 0.0001f));
      NS_TEST_BOOL((tChild.m_vScale == nsVec3(8)));

      tChild = tParent;
      tChild = tChild * tToChild;

      NS_TEST_BOOL(tChild.m_vPosition.IsEqual(nsVec3(13, 12, -5), 0.0001f));
      NS_TEST_BOOL(tChild.m_qRotation.IsEqualRotation(tParent.m_qRotation * tToChild.m_qRotation, 0.0001f));
      NS_TEST_BOOL((tChild.m_vScale == nsVec3(8)));

      nsVec3 a(7, 8, 9);
      nsVec3 b;
      b = tToChild.TransformPosition(a);
      b = tParent.TransformPosition(b);

      nsVec3 c;
      c = tChild.TransformPosition(a);

      NS_TEST_BOOL(b.IsEqual(c, 0.0001f));

      // verify that it works exactly like a 4x4 matrix
      const nsMat4 mParent = tParent.GetAsMat4();
      const nsMat4 mToChild = tToChild.GetAsMat4();
      const nsMat4 mChild = mParent * mToChild;

      NS_TEST_BOOL(mChild.IsEqual(tChild.GetAsMat4(), 0.0001f));
    }

    {
      nsTransform t(nsVec3(1, 2, 3));
      t.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 1, 0), nsAngle::MakeFromDegree(90));
      t.m_vScale = nsVec3(2);

      nsQuat q;
      q = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 0, 1), nsAngle::MakeFromDegree(90));

      nsTransform t2 = t * q;
      nsTransform t4 = q * t;

      nsTransform t3 = t;
      t3 = t3 * q;
      NS_TEST_BOOL(t2 == t3);
      NS_TEST_BOOL(t3 != t4);

      nsVec3 a(7, 8, 9);
      nsVec3 b;
      b = t2.TransformPosition(a);

      nsVec3 c = q * a;
      c = t.TransformPosition(c);

      NS_TEST_BOOL(b.IsEqual(c, 0.0001f));
    }

    {
      nsTransform t(nsVec3(1, 2, 3));
      t.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 1, 0), nsAngle::MakeFromDegree(90));
      t.m_vScale = nsVec3(2);

      nsVec3 p(4, 5, 6);

      nsTransform t2 = t + p;
      nsTransform t3 = t;
      t3 += p;
      NS_TEST_BOOL(t2 == t3);

      nsVec3 a(7, 8, 9);
      nsVec3 b;
      b = t2.TransformPosition(a);

      nsVec3 c = t.TransformPosition(a) + p;

      NS_TEST_BOOL(b.IsEqual(c, 0.0001f));
    }

    {
      nsTransform t(nsVec3(1, 2, 3));
      t.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 1, 0), nsAngle::MakeFromDegree(90));
      t.m_vScale = nsVec3(2);

      nsVec3 p(4, 5, 6);

      nsTransform t2 = t - p;
      nsTransform t3 = t;
      t3 -= p;
      NS_TEST_BOOL(t2 == t3);

      nsVec3 a(7, 8, 9);
      nsVec3 b;
      b = t2.TransformPosition(a);

      nsVec3 c = t.TransformPosition(a) - p;

      NS_TEST_BOOL(b.IsEqual(c, 0.0001f));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Comparison")
  {
    nsTransform t(nsVec3(1, 2, 3));
    t.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 1, 0), nsAngle::MakeFromDegree(90));

    NS_TEST_BOOL(t == t);

    nsTransform t2(nsVec3(1, 2, 4));
    t2.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 1, 0), nsAngle::MakeFromDegree(90));

    NS_TEST_BOOL(t != t2);

    nsTransform t3(nsVec3(1, 2, 3));
    t3.m_qRotation = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 1, 0), nsAngle::MakeFromDegree(91));

    NS_TEST_BOOL(t != t3);
  }
}
