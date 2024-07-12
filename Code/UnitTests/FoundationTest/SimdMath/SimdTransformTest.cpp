#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/Transform.h>
#include <Foundation/SimdMath/SimdConversion.h>
#include <Foundation/SimdMath/SimdTransform.h>

NS_CREATE_SIMPLE_TEST(SimdMath, SimdTransform)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    nsSimdTransform t0;

    {
      nsSimdQuat qRot = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(1, 2, 3).GetNormalized<3>(), nsAngle::MakeFromDegree(42.0f));

      nsSimdVec4f pos(4, 5, 6);
      nsSimdVec4f scale(7, 8, 9);

      nsSimdTransform t(pos);
      NS_TEST_BOOL((t.m_Position == pos).AllSet<3>());
      NS_TEST_BOOL(t.m_Rotation == nsSimdQuat::MakeIdentity());
      NS_TEST_BOOL((t.m_Scale == nsSimdVec4f(1)).AllSet<3>());

      t = nsSimdTransform(pos, qRot);
      NS_TEST_BOOL((t.m_Position == pos).AllSet<3>());
      NS_TEST_BOOL(t.m_Rotation == qRot);
      NS_TEST_BOOL((t.m_Scale == nsSimdVec4f(1)).AllSet<3>());

      t = nsSimdTransform(pos, qRot, scale);
      NS_TEST_BOOL((t.m_Position == pos).AllSet<3>());
      NS_TEST_BOOL(t.m_Rotation == qRot);
      NS_TEST_BOOL((t.m_Scale == scale).AllSet<3>());

      t = nsSimdTransform(qRot);
      NS_TEST_BOOL(t.m_Position.IsZero<3>());
      NS_TEST_BOOL(t.m_Rotation == qRot);
      NS_TEST_BOOL((t.m_Scale == nsSimdVec4f(1)).AllSet<3>());
    }

    {
      nsSimdQuat qRot;
      qRot = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(1, 2, 3).GetNormalized<3>(), nsAngle::MakeFromDegree(42.0f));

      nsSimdVec4f pos(4, 5, 6);
      nsSimdVec4f scale(7, 8, 9);

      nsSimdTransform t = nsSimdTransform::Make(pos);
      NS_TEST_BOOL((t.m_Position == pos).AllSet<3>());
      NS_TEST_BOOL(t.m_Rotation == nsSimdQuat::MakeIdentity());
      NS_TEST_BOOL((t.m_Scale == nsSimdVec4f(1)).AllSet<3>());

      t = nsSimdTransform::Make(pos, qRot);
      NS_TEST_BOOL((t.m_Position == pos).AllSet<3>());
      NS_TEST_BOOL(t.m_Rotation == qRot);
      NS_TEST_BOOL((t.m_Scale == nsSimdVec4f(1)).AllSet<3>());

      t = nsSimdTransform::Make(pos, qRot, scale);
      NS_TEST_BOOL((t.m_Position == pos).AllSet<3>());
      NS_TEST_BOOL(t.m_Rotation == qRot);
      NS_TEST_BOOL((t.m_Scale == scale).AllSet<3>());
    }

    {
      nsSimdTransform t = nsSimdTransform::MakeIdentity();

      NS_TEST_BOOL(t.m_Position.IsZero<3>());
      NS_TEST_BOOL(t.m_Rotation == nsSimdQuat::MakeIdentity());
      NS_TEST_BOOL((t.m_Scale == nsSimdVec4f(1)).AllSet<3>());

      NS_TEST_BOOL(t == nsSimdTransform::MakeIdentity());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Inverse")
  {
    nsSimdTransform tParent(nsSimdVec4f(1, 2, 3));
    tParent.m_Rotation = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 1, 0), nsAngle::MakeFromDegree(90));
    tParent.m_Scale = nsSimdVec4f(2);

    nsSimdTransform tToChild(nsSimdVec4f(4, 5, 6));
    tToChild.m_Rotation = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(90));
    tToChild.m_Scale = nsSimdVec4f(4);

    nsSimdTransform tChild;
    tChild = tParent * tToChild;

    // invert twice -> get back original
    nsSimdTransform t2 = tToChild;
    t2.Invert();
    t2.Invert();
    NS_TEST_BOOL(t2.IsEqual(tToChild, 0.0001f));

    nsSimdTransform tInvToChild = tToChild.GetInverse();

    nsSimdTransform tParentFromChild;
    tParentFromChild = tChild * tInvToChild;

    NS_TEST_BOOL(tParent.IsEqual(tParentFromChild, 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetLocalTransform")
  {
    nsSimdQuat q;
    q = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(90));

    nsSimdTransform tParent(nsSimdVec4f(1, 2, 3));
    tParent.m_Rotation = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 1, 0), nsAngle::MakeFromDegree(90));
    tParent.m_Scale = nsSimdVec4f(2);

    nsSimdTransform tChild;
    tChild.m_Position = nsSimdVec4f(13, 12, -5);
    tChild.m_Rotation = tParent.m_Rotation * q;
    tChild.m_Scale = nsSimdVec4f(8);

    nsSimdTransform tToChild = nsSimdTransform::MakeLocalTransform(tParent, tChild);

    NS_TEST_BOOL(tToChild.m_Position.IsEqual(nsSimdVec4f(4, 5, 6), 0.0001f).AllSet<3>());
    NS_TEST_BOOL(tToChild.m_Rotation.IsEqualRotation(q, 0.0001f));
    NS_TEST_BOOL((tToChild.m_Scale == nsSimdVec4f(4)).AllSet<3>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetGlobalTransform")
  {
    nsSimdTransform tParent(nsSimdVec4f(1, 2, 3));
    tParent.m_Rotation = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 1, 0), nsAngle::MakeFromDegree(90));
    tParent.m_Scale = nsSimdVec4f(2);

    nsSimdTransform tToChild(nsSimdVec4f(4, 5, 6));
    tToChild.m_Rotation = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(90));
    tToChild.m_Scale = nsSimdVec4f(4);

    nsSimdTransform tChild = nsSimdTransform::MakeGlobalTransform(tParent, tToChild);

    NS_TEST_BOOL(tChild.m_Position.IsEqual(nsSimdVec4f(13, 12, -5), 0.0001f).AllSet<3>());
    NS_TEST_BOOL(tChild.m_Rotation.IsEqualRotation(tParent.m_Rotation * tToChild.m_Rotation, 0.0001f));
    NS_TEST_BOOL((tChild.m_Scale == nsSimdVec4f(8)).AllSet<3>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetAsMat4")
  {
    nsSimdTransform t(nsSimdVec4f(1, 2, 3));
    t.m_Rotation = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 1, 0), nsAngle::MakeFromDegree(34));
    t.m_Scale = nsSimdVec4f(2, -1, 5);

    nsSimdMat4f m = t.GetAsMat4();

    // reference
    nsSimdMat4f refM;
    {
      nsQuat q = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 1, 0), nsAngle::MakeFromDegree(34));

      nsTransform referenceTransform(nsVec3(1, 2, 3), q, nsVec3(2, -1, 5));
      nsMat4 tmp = referenceTransform.GetAsMat4();
      refM = nsSimdMat4f::MakeFromColumnMajorArray(tmp.m_fElementsCM);
    }
    NS_TEST_BOOL(m.IsEqual(refM, 0.00001f));

    nsSimdVec4f p[8] = {nsSimdVec4f(-4, 0, 0), nsSimdVec4f(5, 0, 0), nsSimdVec4f(0, -6, 0), nsSimdVec4f(0, 7, 0), nsSimdVec4f(0, 0, -8),
      nsSimdVec4f(0, 0, 9), nsSimdVec4f(1, -2, 3), nsSimdVec4f(-4, 5, 7)};

    for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(p); ++i)
    {
      nsSimdVec4f pt = t.TransformPosition(p[i]);
      nsSimdVec4f pm = m.TransformPosition(p[i]);

      NS_TEST_BOOL(pt.IsEqual(pm, 0.00001f).AllSet<3>());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TransformPos / Dir / operator*")
  {
    nsSimdQuat qRotX, qRotY;
    qRotX = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(1, 0, 0), nsAngle::MakeFromDegree(90.0f));
    qRotY = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 1, 0), nsAngle::MakeFromDegree(90.0f));

    nsSimdTransform t(nsSimdVec4f(1, 2, 3, 10), qRotY * qRotX, nsSimdVec4f(2, -2, 4, 11));

    nsSimdVec4f v;
    v = t.TransformPosition(nsSimdVec4f(4, 5, 6, 12));
    NS_TEST_BOOL(v.IsEqual(nsSimdVec4f((5 * -2) + 1, (-6 * 4) + 2, (-4 * 2) + 3), 0.0001f).AllSet<3>());

    v = t.TransformDirection(nsSimdVec4f(4, 5, 6, 13));
    NS_TEST_BOOL(v.IsEqual(nsSimdVec4f((5 * -2), (-6 * 4), (-4 * 2)), 0.0001f).AllSet<3>());

    v = t * nsSimdVec4f(4, 5, 6, 12);
    NS_TEST_BOOL(v.IsEqual(nsSimdVec4f((5 * -2) + 1, (-6 * 4) + 2, (-4 * 2) + 3), 0.0001f).AllSet<3>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Operators")
  {
    {
      nsSimdTransform tParent(nsSimdVec4f(1, 2, 3));
      tParent.m_Rotation = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 1, 0), nsAngle::MakeFromDegree(90));
      tParent.m_Scale = nsSimdVec4f(2);

      nsSimdTransform tToChild(nsSimdVec4f(4, 5, 6));
      tToChild.m_Rotation = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(90));
      tToChild.m_Scale = nsSimdVec4f(4);

      // this is exactly the same as SetGlobalTransform
      nsSimdTransform tChild;
      tChild = tParent * tToChild;

      NS_TEST_BOOL(tChild.m_Position.IsEqual(nsSimdVec4f(13, 12, -5), 0.0001f).AllSet<3>());
      NS_TEST_BOOL(tChild.m_Rotation.IsEqualRotation(tParent.m_Rotation * tToChild.m_Rotation, 0.0001f));
      NS_TEST_BOOL((tChild.m_Scale == nsSimdVec4f(8)).AllSet<3>());

      tChild = tParent;
      tChild *= tToChild;

      NS_TEST_BOOL(tChild.m_Position.IsEqual(nsSimdVec4f(13, 12, -5), 0.0001f).AllSet<3>());
      NS_TEST_BOOL(tChild.m_Rotation.IsEqualRotation(tParent.m_Rotation * tToChild.m_Rotation, 0.0001f));
      NS_TEST_BOOL((tChild.m_Scale == nsSimdVec4f(8)).AllSet<3>());

      nsSimdVec4f a(7, 8, 9);
      nsSimdVec4f b;
      b = tToChild.TransformPosition(a);
      b = tParent.TransformPosition(b);

      nsSimdVec4f c;
      c = tChild.TransformPosition(a);

      NS_TEST_BOOL(b.IsEqual(c, 0.0001f).AllSet());

      // verify that it works exactly like a 4x4 matrix
      /*const nsMat4 mParent = tParent.GetAsMat4();
      const nsMat4 mToChild = tToChild.GetAsMat4();
      const nsMat4 mChild = mParent * mToChild;

      NS_TEST_BOOL(mChild.IsEqual(tChild.GetAsMat4(), 0.0001f));*/
    }

    {
      nsSimdTransform t(nsSimdVec4f(1, 2, 3));
      t.m_Rotation = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 1, 0), nsAngle::MakeFromDegree(90));
      t.m_Scale = nsSimdVec4f(2);

      nsSimdQuat q = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(90));

      nsSimdTransform t2 = t * q;
      nsSimdTransform t4 = q * t;

      nsSimdTransform t3 = t;
      t3 *= q;
      NS_TEST_BOOL(t2 == t3);
      NS_TEST_BOOL(t3 != t4);

      nsSimdVec4f a(7, 8, 9);
      nsSimdVec4f b;
      b = t2.TransformPosition(a);

      nsSimdVec4f c = q * a;
      c = t.TransformPosition(c);

      NS_TEST_BOOL(b.IsEqual(c, 0.0001f).AllSet());
    }

    {
      nsSimdTransform t(nsSimdVec4f(1, 2, 3));
      t.m_Rotation = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 1, 0), nsAngle::MakeFromDegree(90));
      t.m_Scale = nsSimdVec4f(2);

      nsSimdVec4f p(4, 5, 6);

      nsSimdTransform t2 = t + p;
      nsSimdTransform t3 = t;
      t3 += p;
      NS_TEST_BOOL(t2 == t3);

      nsSimdVec4f a(7, 8, 9);
      nsSimdVec4f b;
      b = t2.TransformPosition(a);

      nsSimdVec4f c = t.TransformPosition(a) + p;

      NS_TEST_BOOL(b.IsEqual(c, 0.0001f).AllSet());
    }

    {
      nsSimdTransform t(nsSimdVec4f(1, 2, 3));
      t.m_Rotation = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 1, 0), nsAngle::MakeFromDegree(90));
      t.m_Scale = nsSimdVec4f(2);

      nsSimdVec4f p(4, 5, 6);

      nsSimdTransform t2 = t - p;
      nsSimdTransform t3 = t;
      t3 -= p;
      NS_TEST_BOOL(t2 == t3);

      nsSimdVec4f a(7, 8, 9);
      nsSimdVec4f b;
      b = t2.TransformPosition(a);

      nsSimdVec4f c = t.TransformPosition(a) - p;

      NS_TEST_BOOL(b.IsEqual(c, 0.0001f).AllSet());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Comparison")
  {
    nsSimdTransform t(nsSimdVec4f(1, 2, 3));
    t.m_Rotation = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 1, 0), nsAngle::MakeFromDegree(90));

    NS_TEST_BOOL(t == t);

    nsSimdTransform t2(nsSimdVec4f(1, 2, 4));
    t2.m_Rotation = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 1, 0), nsAngle::MakeFromDegree(90));

    NS_TEST_BOOL(t != t2);

    nsSimdTransform t3(nsSimdVec4f(1, 2, 3));
    t3.m_Rotation = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 1, 0), nsAngle::MakeFromDegree(91));

    NS_TEST_BOOL(t != t3);
  }
}
