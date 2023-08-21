import wd = require("TypeScript/wd")
import WD_TEST = require("./TestFramework")

export class TestQuat extends wd.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {
        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgGenericEvent, "OnMsgGenericEvent");
    }

    ExecuteTests(): void {

        // constructor
        {
            let q = new wd.Quat();
            WD_TEST.FLOAT(q.x, 0, 0.001);
            WD_TEST.FLOAT(q.y, 0, 0.001);
            WD_TEST.FLOAT(q.z, 0, 0.001);
            WD_TEST.FLOAT(q.w, 1, 0.001);
        }

        // Clone / Normalize
        {
            let q = new wd.Quat(1, 2, 3, 4);
            WD_TEST.BOOL(q.IsIdentical(new wd.Quat(1, 2, 3, 4)));

            q.Normalize();
            WD_TEST.QUAT(q.Clone(), q, 0.001);
        }

        // SetIdentity
        {
            let q = new wd.Quat(1, 2, 3, 4);
            q.SetIdentity();
            WD_TEST.QUAT(q, wd.Quat.IdentityQuaternion(), 0.001);
        }


        // IdentityQuaternion
        {
            let q = wd.Quat.IdentityQuaternion();

            WD_TEST.FLOAT(q.x, 0, 0.001);
            WD_TEST.FLOAT(q.y, 0, 0.001);
            WD_TEST.FLOAT(q.z, 0, 0.001);
            WD_TEST.FLOAT(q.w, 1, 0.001);
        }

        // SetFromAxisAndAngle / RotateVec3
        {
            {
                let q = new wd.Quat();
                q.SetFromAxisAndAngle(new wd.Vec3(1, 0, 0), wd.Angle.DegreeToRadian(90));

                let v = new wd.Vec3(0, 1, 0);
                q.RotateVec3(v);
                WD_TEST.VEC3(v, new wd.Vec3(0, 0, 1), 0.0001);

                let v2 = new wd.Vec3(0, 1, 0);
                q.InvRotateVec3(v2);
                WD_TEST.VEC3(v2, new wd.Vec3(0, 0, -1), 0.0001);
            }

            {
                let q = new wd.Quat();
                q.SetFromAxisAndAngle(new wd.Vec3(0, 1, 0), wd.Angle.DegreeToRadian(90));

                let v = new wd.Vec3(1, 0, 0);
                q.RotateVec3(v);
                WD_TEST.VEC3(v, new wd.Vec3(0, 0, -1), 0.0001);

                let v2 = new wd.Vec3(1, 0, 0);
                q.InvRotateVec3(v2);
                WD_TEST.VEC3(v2, new wd.Vec3(0, 0, 1), 0.0001);
            }

            {
                let q = new wd.Quat();
                q.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(90));

                let v = new wd.Vec3(0, 1, 0);
                q.RotateVec3(v);
                WD_TEST.VEC3(v, new wd.Vec3(-1, 0, 0), 0.0001);

                let v2 = new wd.Vec3(0, 1, 0);
                q.InvRotateVec3(v2);
                WD_TEST.VEC3(v2, new wd.Vec3(1, 0, 0), 0.0001);
            }
        }

        // SetQuat
        {
            let q = new wd.Quat(1, 2, 3, 4);
            let q2 = new wd.Quat();
            q2.SetQuat(q);

            WD_TEST.FLOAT(q2.x, 1, 0.001);
            WD_TEST.FLOAT(q2.y, 2, 0.001);
            WD_TEST.FLOAT(q2.z, 3, 0.001);
            WD_TEST.FLOAT(q2.w, 4, 0.001);
        }

        // SetFromMat3
        {
            let m = new wd.Mat3();
            m.SetRotationMatrixZ(wd.Angle.DegreeToRadian(-90));

            let q1 = new wd.Quat();
            let q2 = new wd.Quat();
            let q3 = new wd.Quat();

            q1.SetFromMat3(m);
            q2.SetFromAxisAndAngle(new wd.Vec3(0, 0, -1), wd.Angle.DegreeToRadian(90));
            q3.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(-90));

            WD_TEST.BOOL(q1.IsEqualRotation(q2, 0.001));
            WD_TEST.BOOL(q1.IsEqualRotation(q3, 0.001));
        }

        // SetSlerp
        {
            let q1 = new wd.Quat();
            let q2 = new wd.Quat();
            let q3 = new wd.Quat();
            let qr = new wd.Quat();

            q1.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(45));
            q2.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(0));
            q3.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(90));

            qr.SetSlerp(q2, q3, 0.5);

            WD_TEST.QUAT(q1, qr, 0.0001);
        }

        // GetRotationAxisAndAngle
        {
            let q1 = new wd.Quat();
            let q2 = new wd.Quat();
            let q3 = new wd.Quat();

            q1.SetShortestRotation(new wd.Vec3(0, 1, 0), new wd.Vec3(1, 0, 0));
            q2.SetFromAxisAndAngle(new wd.Vec3(0, 0, -1), wd.Angle.DegreeToRadian(90));
            q3.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(-90));

            let res = q1.GetRotationAxisAndAngle();
            WD_TEST.VEC3(res.axis, new wd.Vec3(0, 0, -1), 0.001);
            WD_TEST.FLOAT(wd.Angle.RadianToDegree(res.angleInRadian), 90, 0.001);

            res = q2.GetRotationAxisAndAngle();
            WD_TEST.VEC3(res.axis, new wd.Vec3(0, 0, -1), 0.001);
            WD_TEST.FLOAT(wd.Angle.RadianToDegree(res.angleInRadian), 90, 0.001);

            res = q3.GetRotationAxisAndAngle();
            WD_TEST.VEC3(res.axis, new wd.Vec3(0, 0, -1), 0.001);
            WD_TEST.FLOAT(wd.Angle.RadianToDegree(res.angleInRadian), 90, 0.001);

            res = wd.Quat.IdentityQuaternion().GetRotationAxisAndAngle();
            WD_TEST.VEC3(res.axis, new wd.Vec3(1, 0, 0), 0.001);
            WD_TEST.FLOAT(wd.Angle.RadianToDegree(res.angleInRadian), 0, 0.001);

            let otherIdentity = new wd.Quat(0, 0, 0, -1);
            res = otherIdentity.GetRotationAxisAndAngle();
            WD_TEST.VEC3(res.axis, new wd.Vec3(1, 0, 0), 0.001);
            WD_TEST.FLOAT(wd.Angle.RadianToDegree(res.angleInRadian), 360, 0.001);
        }

        // GetAsMat3
        {
            let q = new wd.Quat();
            q.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(90));

            let mr = new wd.Mat3();
            mr.SetRotationMatrixZ(wd.Angle.DegreeToRadian(90));

            let m = q.GetAsMat3();

            WD_TEST.BOOL(mr.IsEqual(m, 0.001));
        }

        // GetAsMat4
        {
            let q = new wd.Quat();
            q.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(90));

            let mr = new wd.Mat4();
            mr.SetRotationMatrixZ(wd.Angle.DegreeToRadian(90));

            let m = q.GetAsMat4();

            WD_TEST.BOOL(mr.IsEqual(m, 0.001));
        }

        // SetShortestRotation / IsEqualRotation
        {
            let q1 = new wd.Quat();
            let q2 = new wd.Quat();
            let q3 = new wd.Quat();

            q1.SetShortestRotation(new wd.Vec3(0, 1, 0), new wd.Vec3(1, 0, 0));
            q2.SetFromAxisAndAngle(new wd.Vec3(0, 0, -1), wd.Angle.DegreeToRadian(90));
            q3.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(-90));

            WD_TEST.BOOL(q1.IsEqualRotation(q2, 0.001));
            WD_TEST.BOOL(q1.IsEqualRotation(q3, 0.001));

            WD_TEST.BOOL(wd.Quat.IdentityQuaternion().IsEqualRotation(wd.Quat.IdentityQuaternion(), 0.001));
            WD_TEST.BOOL(wd.Quat.IdentityQuaternion().IsEqualRotation(new wd.Quat(0, 0, 0, -1), 0.001));
        }

        // SetConcatenatedRotations / ConcatenateRotations
        {
            let q1 = new wd.Quat();
            let q2 = new wd.Quat();
            let q3 = new wd.Quat();
            let qr = new wd.Quat();

            q1.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(60));
            q2.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(30));
            q3.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(90));

            qr.SetConcatenatedRotations(q1, q2);

            WD_TEST.BOOL(qr.IsEqualRotation(q3, 0.0001));

            let qr2 = q1.Clone();
            qr2.ConcatenateRotations(q2);

            WD_TEST.QUAT(qr, qr2, 0.001);
        }

        // IsIdentical
        {
            let q1 = new wd.Quat();
            let q2 = new wd.Quat();

            q1.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(60));
            q2.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(30));
            WD_TEST.BOOL(!q1.IsIdentical(q2));

            q2.SetFromAxisAndAngle(new wd.Vec3(1, 0, 0), wd.Angle.DegreeToRadian(60));
            WD_TEST.BOOL(!q1.IsIdentical(q2));

            q2.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(60));
            WD_TEST.BOOL(q1.IsIdentical(q2));
        }

        // Negate / GetNegated
        {
            let q = new wd.Quat();
            q.SetFromAxisAndAngle(new wd.Vec3(1, 0, 0), wd.Angle.DegreeToRadian(90));

            let v = new wd.Vec3(0, 1, 0);
            q.RotateVec3(v);
            WD_TEST.VEC3(v, new wd.Vec3(0, 0, 1), 0.0001);

            let n1 = q.GetNegated();
            let n2 = q.Clone();
            n2.Negate();

            WD_TEST.QUAT(n1, n2, 0.001);

            let v2 = new wd.Vec3(0, 1, 0);
            n1.RotateVec3(v2);
            WD_TEST.VEC3(v2, new wd.Vec3(0, 0, -1), 0.0001);
        }

        // SetFromEulerAngles / GetAsEulerAngles
        {
            let q = new wd.Quat();
            q.SetFromEulerAngles(wd.Angle.DegreeToRadian(90), 0, 0);

            let euler = q.GetAsEulerAngles();
            WD_TEST.FLOAT(euler.roll, wd.Angle.DegreeToRadian(90), 0.001);
            WD_TEST.FLOAT(euler.pitch, wd.Angle.DegreeToRadian(0), 0.001);
            WD_TEST.FLOAT(euler.yaw, wd.Angle.DegreeToRadian(0), 0.001);

            q.SetFromEulerAngles(0, wd.Angle.DegreeToRadian(90), 0);
            euler = q.GetAsEulerAngles();
            WD_TEST.FLOAT(euler.pitch, wd.Angle.DegreeToRadian(90), 0.001);

            // due to compilation differences, this the result for this computation can be very different (but equivalent)
            WD_TEST.BOOL((wd.Utils.IsNumberEqual(euler.roll, wd.Angle.DegreeToRadian(180), 0.001) &&
                          wd.Utils.IsNumberEqual(euler.yaw, wd.Angle.DegreeToRadian(180), 0.001)) ||
                          (wd.Utils.IsNumberEqual(euler.roll, wd.Angle.DegreeToRadian(0), 0.001) &&
                          wd.Utils.IsNumberEqual(euler.yaw, wd.Angle.DegreeToRadian(0), 0.001)));


            q.SetFromEulerAngles(0, 0, wd.Angle.DegreeToRadian(90));
            euler = q.GetAsEulerAngles();
            WD_TEST.FLOAT(euler.roll, wd.Angle.DegreeToRadian(0), 0.001);
            WD_TEST.FLOAT(euler.pitch, wd.Angle.DegreeToRadian(0), 0.001);
            WD_TEST.FLOAT(euler.yaw, wd.Angle.DegreeToRadian(90), 0.001);
        }
    }


    OnMsgGenericEvent(msg: wd.MsgGenericEvent): void {

        if (msg.Message == "TestQuat") {

            this.ExecuteTests();

            msg.Message = "done";
        }
    }

}

