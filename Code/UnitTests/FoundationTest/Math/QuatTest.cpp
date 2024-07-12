#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/Quat.h>

NS_CREATE_SIMPLE_TEST(Math, Quaternion)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Default Constructor")
  {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    if (nsMath::SupportsNaN<nsMat3T::ComponentType>())
    {
      // In debug the default constructor initializes everything with NaN.
      nsQuatT p;
      NS_TEST_BOOL(nsMath::IsNaN(p.x) && nsMath::IsNaN(p.y) && nsMath::IsNaN(p.z) && nsMath::IsNaN(p.w));
    }
#else
    // Placement new of the default constructor should not have any effect on the previous data.
    nsQuatT::ComponentType testBlock[4] = {
      (nsQuatT::ComponentType)1, (nsQuatT::ComponentType)2, (nsQuatT::ComponentType)3, (nsQuatT::ComponentType)4};
    nsQuatT* p = ::new ((void*)&testBlock[0]) nsQuatT;
    NS_TEST_BOOL(p->x == (nsMat3T::ComponentType)1 && p->y == (nsMat3T::ComponentType)2 && p->z == (nsMat3T::ComponentType)3 &&
                 p->w == (nsMat3T::ComponentType)4);
#endif
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor(x,y,z,w)")
  {
    nsQuatT q(1, 2, 3, 4);

    NS_TEST_VEC3(q.GetVectorPart(), nsVec3T(1, 2, 3), 0.0001f);
    NS_TEST_FLOAT(q.w, 4, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeIdentity")
  {
    nsQuatT q = nsQuatT::MakeIdentity();

    NS_TEST_VEC3(q.GetVectorPart(), nsVec3T(0, 0, 0), 0.0001f);
    NS_TEST_FLOAT(q.w, 1, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetIdentity")
  {
    nsQuatT q(1, 2, 3, 4);

    q.SetIdentity();

    NS_TEST_VEC3(q.GetVectorPart(), nsVec3T(0, 0, 0), 0.0001f);
    NS_TEST_FLOAT(q.w, 1, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetElements")
  {
    nsQuatT q(5, 6, 7, 8);

    q = nsQuat(1, 2, 3, 4);

    NS_TEST_VEC3(q.GetVectorPart(), nsVec3T(1, 2, 3), 0.0001f);
    NS_TEST_FLOAT(q.w, 4, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetFromAxisAndAngle / operator* (quat, vec)")
  {
    {
      nsQuatT q;
      q = nsQuat::MakeFromAxisAndAngle(nsVec3T(1, 0, 0), nsAngle::MakeFromDegree(90));

      NS_TEST_VEC3(q * nsVec3T(0, 1, 0), nsVec3T(0, 0, 1), 0.0001f);
    }

    {
      nsQuatT q;
      q = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(90));

      NS_TEST_VEC3(q * nsVec3T(1, 0, 0), nsVec3T(0, 0, -1), 0.0001f);
    }

    {
      nsQuatT q;
      q = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(90));

      NS_TEST_VEC3(q * nsVec3T(0, 1, 0), nsVec3T(-1, 0, 0), 0.0001f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetShortestRotation / IsEqualRotation")
  {
    nsQuatT q1, q2, q3;
    q1 = nsQuat::MakeShortestRotation(nsVec3T(0, 1, 0), nsVec3T(1, 0, 0));
    q2 = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, -1), nsAngle::MakeFromDegree(90));
    q3 = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(-90));

    NS_TEST_BOOL(q1.IsEqualRotation(q2, nsMath::LargeEpsilon<float>()));
    NS_TEST_BOOL(q1.IsEqualRotation(q3, nsMath::LargeEpsilon<float>()));

    NS_TEST_BOOL(nsQuatT::MakeIdentity().IsEqualRotation(nsQuatT::MakeIdentity(), nsMath::LargeEpsilon<float>()));
    NS_TEST_BOOL(nsQuatT::MakeIdentity().IsEqualRotation(nsQuatT(0, 0, 0, -1), nsMath::LargeEpsilon<float>()));

    nsQuatT q4{0, 0, 0, 1.00000012f};
    nsQuatT q5{0, 0, 0, 1.00000023f};
    NS_TEST_BOOL(q4.IsEqualRotation(q5, nsMath::LargeEpsilon<float>()));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetFromMat3")
  {
    nsMat3T m;
    m = nsMat3::MakeRotationZ(nsAngle::MakeFromDegree(-90));

    nsQuatT q1, q2, q3;
    q1 = nsQuat::MakeFromMat3(m);
    q2 = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, -1), nsAngle::MakeFromDegree(90));
    q3 = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(-90));

    NS_TEST_BOOL(q1.IsEqualRotation(q2, nsMath::LargeEpsilon<float>()));
    NS_TEST_BOOL(q1.IsEqualRotation(q3, nsMath::LargeEpsilon<float>()));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetSlerp")
  {
    nsQuatT q1, q2, q3, qr;
    q1 = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(45));
    q2 = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(0));
    q3 = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(90));

    qr = nsQuat::MakeSlerp(q2, q3, 0.5f);

    NS_TEST_BOOL(q1.IsEqualRotation(qr, 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetRotationAxisAndAngle")
  {
    nsQuatT q1, q2, q3;
    q1 = nsQuat::MakeShortestRotation(nsVec3T(0, 1, 0), nsVec3T(1, 0, 0));
    q2 = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, -1), nsAngle::MakeFromDegree(90));
    q3 = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(-90));

    nsVec3T axis;
    nsAngle angle;

    q1.GetRotationAxisAndAngle(axis, angle);
    NS_TEST_VEC3(axis, nsVec3T(0, 0, -1), 0.001f);
    NS_TEST_FLOAT(angle.GetDegree(), 90, nsMath::LargeEpsilon<nsMat3T::ComponentType>());

    q2.GetRotationAxisAndAngle(axis, angle);
    NS_TEST_VEC3(axis, nsVec3T(0, 0, -1), 0.001f);
    NS_TEST_FLOAT(angle.GetDegree(), 90, nsMath::LargeEpsilon<nsMat3T::ComponentType>());

    q3.GetRotationAxisAndAngle(axis, angle);
    NS_TEST_VEC3(axis, nsVec3T(0, 0, -1), 0.001f);
    NS_TEST_FLOAT(angle.GetDegree(), 90, nsMath::LargeEpsilon<nsMat3T::ComponentType>());

    nsQuatT::MakeIdentity().GetRotationAxisAndAngle(axis, angle);
    NS_TEST_VEC3(axis, nsVec3T(1, 0, 0), 0.001f);
    NS_TEST_FLOAT(angle.GetDegree(), 0, nsMath::LargeEpsilon<nsMat3T::ComponentType>());

    nsQuatT otherIdentity(0, 0, 0, -1);
    otherIdentity.GetRotationAxisAndAngle(axis, angle);
    NS_TEST_VEC3(axis, nsVec3T(1, 0, 0), 0.001f);
    NS_TEST_FLOAT(angle.GetDegree(), 360, nsMath::LargeEpsilon<nsMat3T::ComponentType>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetAsMat3")
  {
    nsQuatT q;
    q = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(90));

    nsMat3T mr;
    mr = nsMat3::MakeRotationZ(nsAngle::MakeFromDegree(90));

    nsMat3T m = q.GetAsMat3();

    NS_TEST_BOOL(mr.IsEqual(m, nsMath::DefaultEpsilon<nsMat3T::ComponentType>()));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetAsMat4")
  {
    nsQuatT q;
    q = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(90));

    nsMat4T mr;
    mr = nsMat4::MakeRotationZ(nsAngle::MakeFromDegree(90));

    nsMat4T m = q.GetAsMat4();

    NS_TEST_BOOL(mr.IsEqual(m, nsMath::DefaultEpsilon<nsMat3T::ComponentType>()));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsValid / Normalize")
  {
    nsQuatT q(1, 2, 3, 4);
    NS_TEST_BOOL(!q.IsValid(0.001f));

    q.Normalize();
    NS_TEST_BOOL(q.IsValid(0.001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetInverse / Invert")
  {
    nsQuatT q, q1;
    q = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(90));
    q1 = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(-90));

    nsQuatT q2 = q.GetInverse();
    NS_TEST_BOOL(q1.IsEqualRotation(q2, 0.0001f));

    nsQuatT q3 = q;
    q3.Invert();
    NS_TEST_BOOL(q1.IsEqualRotation(q3, 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Dot")
  {
    nsQuatT q, q1, q2;
    q = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(90));
    q1 = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(-90));
    q2 = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 1, 0), nsAngle::MakeFromDegree(45));

    NS_TEST_FLOAT(q.Dot(q), 1.0f, 0.0001f);
    NS_TEST_FLOAT(q.Dot(nsQuat::MakeIdentity()), cos(nsAngle::DegToRad(90.0f / 2)), 0.0001f);
    NS_TEST_FLOAT(q.Dot(q1), 0.0f, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator*(quat, quat)")
  {
    nsQuatT q1, q2, qr, q3;
    q1 = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(60));
    q2 = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(30));
    q3 = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(90));

    qr = q1 * q2;

    NS_TEST_BOOL(qr.IsEqualRotation(q3, 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator==/!=")
  {
    nsQuatT q1, q2;
    q1 = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(60));
    q2 = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(30));
    NS_TEST_BOOL(q1 != q2);

    q2 = nsQuat::MakeFromAxisAndAngle(nsVec3T(1, 0, 0), nsAngle::MakeFromDegree(60));
    NS_TEST_BOOL(q1 != q2);

    q2 = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(60));
    NS_TEST_BOOL(q1 == q2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsNaN")
  {
    if (nsMath::SupportsNaN<nsMathTestType>())
    {
      nsQuatT q;

      q.SetIdentity();
      NS_TEST_BOOL(!q.IsNaN());

      q.SetIdentity();
      q.w = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(q.IsNaN());

      q.SetIdentity();
      q.x = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(q.IsNaN());

      q.SetIdentity();
      q.y = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(q.IsNaN());

      q.SetIdentity();
      q.z = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(q.IsNaN());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "rotation direction")
  {
    nsMat3T m;
    m = nsMat3::MakeRotationZ(nsAngle::MakeFromDegree(90.0f));

    nsQuatT q;
    q = nsQuat::MakeFromAxisAndAngle(nsVec3T(0, 0, 1), nsAngle::MakeFromDegree(90.0f));

    nsVec3T xAxis(1, 0, 0);

    nsVec3T temp1 = m.TransformDirection(xAxis);
    nsVec3T temp2 = q.GetAsMat3().TransformDirection(xAxis);

    NS_TEST_BOOL(temp1.IsEqual(temp2, 0.01f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetAsEulerAngles / SetFromEulerAngles")
  {
    nsAngle ax, ay, az;

    for (nsUInt32 x = 0; x < 360; x += 15)
    {
      nsQuat q = nsQuat::MakeFromEulerAngles(nsAngle::MakeFromDegree(x), {}, {});

      nsMat3 m;
      m = nsMat3::MakeRotationX(nsAngle::MakeFromDegree(x));
      nsQuat qm;
      qm = nsQuat::MakeFromMat3(m);
      NS_TEST_BOOL(q.IsEqualRotation(qm, 0.01f));

      nsVec3 axis;
      nsAngle angle;
      q.GetRotationAxisAndAngle(axis, angle, 0.01f);

      NS_TEST_VEC3(axis, nsVec3::MakeAxisX(), 0.001f);
      NS_TEST_FLOAT(angle.GetDegree(), (float)x, 0.1f);

      q.GetAsEulerAngles(ax, ay, az);
      NS_TEST_BOOL(ax.IsEqualNormalized(nsAngle::MakeFromDegree(x), nsAngle::MakeFromDegree(0.1f)));
    }

    for (nsInt32 y = -90; y < 360; y += 15)
    {
      nsQuat q = nsQuat::MakeFromEulerAngles({}, nsAngle::MakeFromDegree(y), {});

      nsMat3 m;
      m = nsMat3::MakeRotationY(nsAngle::MakeFromDegree(y));
      nsQuat qm;
      qm = nsQuat::MakeFromMat3(m);
      NS_TEST_BOOL(q.IsEqualRotation(qm, 0.01f));

      nsVec3 axis;
      nsAngle angle;
      q.GetRotationAxisAndAngle(axis, angle, 0.01f);

      if (y < 0)
      {
        NS_TEST_VEC3(axis, -nsVec3::MakeAxisY(), 0.001f);
        NS_TEST_FLOAT(angle.GetDegree(), (float)-y, 0.1f);
      }
      else if (y > 0)
      {
        NS_TEST_VEC3(axis, nsVec3::MakeAxisY(), 0.001f);
        NS_TEST_FLOAT(angle.GetDegree(), (float)y, 0.1f);
      }

      // pitch is only defined in -90..90 range
      if (y >= -90 && y <= 90)
      {
        q.GetAsEulerAngles(ax, ay, az);
        NS_TEST_FLOAT(ay.GetDegree(), (float)y, 0.1f);
      }
    }

    for (nsUInt32 z = 15; z < 360; z += 15)
    {
      nsQuat q = nsQuat::MakeFromEulerAngles({}, {}, nsAngle::MakeFromDegree(z));

      nsMat3 m;
      m = nsMat3::MakeRotationZ(nsAngle::MakeFromDegree(z));
      nsQuat qm;
      qm = nsQuat::MakeFromMat3(m);
      NS_TEST_BOOL(q.IsEqualRotation(qm, 0.01f));

      nsVec3 axis;
      nsAngle angle;
      q.GetRotationAxisAndAngle(axis, angle, 0.01f);

      NS_TEST_VEC3(axis, nsVec3::MakeAxisZ(), 0.001f);
      NS_TEST_FLOAT(angle.GetDegree(), (float)z, 0.1f);

      q.GetAsEulerAngles(ax, ay, az);
      NS_TEST_BOOL(az.IsEqualNormalized(nsAngle::MakeFromDegree(z), nsAngle::MakeFromDegree(0.1f)));
    }

    for (nsUInt32 x = 0; x < 360; x += 15)
    {
      for (nsUInt32 y = 0; y < 360; y += 15)
      {
        for (nsUInt32 z = 0; z < 360; z += 30)
        {
          nsQuat q1 = nsQuat::MakeFromEulerAngles(nsAngle::MakeFromDegree(x), nsAngle::MakeFromDegree(y), nsAngle::MakeFromDegree(z));

          q1.GetAsEulerAngles(ax, ay, az);

          nsQuat q2 = nsQuat::MakeFromEulerAngles(ax, ay, az);

          NS_TEST_BOOL(q1.IsEqualRotation(q2, 0.1f));

          // Check that euler order is ZYX aka 3-2-1
          nsQuat q3;
          {
            nsQuat xRot, yRot, zRot;
            xRot = nsQuat::MakeFromAxisAndAngle(nsVec3::MakeAxisX(), nsAngle::MakeFromDegree(x));
            yRot = nsQuat::MakeFromAxisAndAngle(nsVec3::MakeAxisY(), nsAngle::MakeFromDegree(y));
            zRot = nsQuat::MakeFromAxisAndAngle(nsVec3::MakeAxisZ(), nsAngle::MakeFromDegree(z));

            q3 = zRot * yRot * xRot;
          }
          NS_TEST_BOOL(q1.IsEqualRotation(q3, 0.01f));
        }
      }
    }
  }
}
