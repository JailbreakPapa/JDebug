import ns = require("TypeScript/ns")
import NS_TEST = require("./TestFramework")

export class TestQuat extends ns.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {
        ns.TypescriptComponent.RegisterMessageHandler(ns.MsgGenericEvent, "OnMsgGenericEvent");
    }

    ExecuteTests(): void {

        // constructor
        {
            let q = new ns.Quat();
            NS_TEST.FLOAT(q.x, 0, 0.001);
            NS_TEST.FLOAT(q.y, 0, 0.001);
            NS_TEST.FLOAT(q.z, 0, 0.001);
            NS_TEST.FLOAT(q.w, 1, 0.001);
        }

        // Clone / Normalize
        {
            let q = new ns.Quat(1, 2, 3, 4);
            NS_TEST.BOOL(q.IsIdentical(new ns.Quat(1, 2, 3, 4)));

            q.Normalize();
            NS_TEST.QUAT(q.Clone(), q, 0.001);
        }

        // SetIdentity
        {
            let q = new ns.Quat(1, 2, 3, 4);
            q.SetIdentity();
            NS_TEST.QUAT(q, ns.Quat.IdentityQuaternion(), 0.001);
        }


        // IdentityQuaternion
        {
            let q = ns.Quat.IdentityQuaternion();

            NS_TEST.FLOAT(q.x, 0, 0.001);
            NS_TEST.FLOAT(q.y, 0, 0.001);
            NS_TEST.FLOAT(q.z, 0, 0.001);
            NS_TEST.FLOAT(q.w, 1, 0.001);
        }

        // SetFromAxisAndAngle / RotateVec3
        {
            {
                let q = new ns.Quat();
                q.SetFromAxisAndAngle(new ns.Vec3(1, 0, 0), ns.Angle.DegreeToRadian(90));

                let v = new ns.Vec3(0, 1, 0);
                q.RotateVec3(v);
                NS_TEST.VEC3(v, new ns.Vec3(0, 0, 1), 0.0001);

                let v2 = new ns.Vec3(0, 1, 0);
                q.InvRotateVec3(v2);
                NS_TEST.VEC3(v2, new ns.Vec3(0, 0, -1), 0.0001);
            }

            {
                let q = new ns.Quat();
                q.SetFromAxisAndAngle(new ns.Vec3(0, 1, 0), ns.Angle.DegreeToRadian(90));

                let v = new ns.Vec3(1, 0, 0);
                q.RotateVec3(v);
                NS_TEST.VEC3(v, new ns.Vec3(0, 0, -1), 0.0001);

                let v2 = new ns.Vec3(1, 0, 0);
                q.InvRotateVec3(v2);
                NS_TEST.VEC3(v2, new ns.Vec3(0, 0, 1), 0.0001);
            }

            {
                let q = new ns.Quat();
                q.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(90));

                let v = new ns.Vec3(0, 1, 0);
                q.RotateVec3(v);
                NS_TEST.VEC3(v, new ns.Vec3(-1, 0, 0), 0.0001);

                let v2 = new ns.Vec3(0, 1, 0);
                q.InvRotateVec3(v2);
                NS_TEST.VEC3(v2, new ns.Vec3(1, 0, 0), 0.0001);
            }
        }

        // SetQuat
        {
            let q = new ns.Quat(1, 2, 3, 4);
            let q2 = new ns.Quat();
            q2.SetQuat(q);

            NS_TEST.FLOAT(q2.x, 1, 0.001);
            NS_TEST.FLOAT(q2.y, 2, 0.001);
            NS_TEST.FLOAT(q2.z, 3, 0.001);
            NS_TEST.FLOAT(q2.w, 4, 0.001);
        }

        // SetFromMat3
        {
            let m = new ns.Mat3();
            m.SetRotationMatrixZ(ns.Angle.DegreeToRadian(-90));

            let q1 = new ns.Quat();
            let q2 = new ns.Quat();
            let q3 = new ns.Quat();

            q1.SetFromMat3(m);
            q2.SetFromAxisAndAngle(new ns.Vec3(0, 0, -1), ns.Angle.DegreeToRadian(90));
            q3.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(-90));

            NS_TEST.BOOL(q1.IsEqualRotation(q2, 0.001));
            NS_TEST.BOOL(q1.IsEqualRotation(q3, 0.001));
        }

        // SetSlerp
        {
            let q1 = new ns.Quat();
            let q2 = new ns.Quat();
            let q3 = new ns.Quat();
            let qr = new ns.Quat();

            q1.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(45));
            q2.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(0));
            q3.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(90));

            qr.SetSlerp(q2, q3, 0.5);

            NS_TEST.QUAT(q1, qr, 0.0001);
        }

        // GetRotationAxisAndAngle
        {
            let q1 = new ns.Quat();
            let q2 = new ns.Quat();
            let q3 = new ns.Quat();

            q1.SetShortestRotation(new ns.Vec3(0, 1, 0), new ns.Vec3(1, 0, 0));
            q2.SetFromAxisAndAngle(new ns.Vec3(0, 0, -1), ns.Angle.DegreeToRadian(90));
            q3.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(-90));

            let res = q1.GetRotationAxisAndAngle();
            NS_TEST.VEC3(res.axis, new ns.Vec3(0, 0, -1), 0.001);
            NS_TEST.FLOAT(ns.Angle.RadianToDegree(res.angleInRadian), 90, 0.001);

            res = q2.GetRotationAxisAndAngle();
            NS_TEST.VEC3(res.axis, new ns.Vec3(0, 0, -1), 0.001);
            NS_TEST.FLOAT(ns.Angle.RadianToDegree(res.angleInRadian), 90, 0.001);

            res = q3.GetRotationAxisAndAngle();
            NS_TEST.VEC3(res.axis, new ns.Vec3(0, 0, -1), 0.001);
            NS_TEST.FLOAT(ns.Angle.RadianToDegree(res.angleInRadian), 90, 0.001);

            res = ns.Quat.IdentityQuaternion().GetRotationAxisAndAngle();
            NS_TEST.VEC3(res.axis, new ns.Vec3(1, 0, 0), 0.001);
            NS_TEST.FLOAT(ns.Angle.RadianToDegree(res.angleInRadian), 0, 0.001);

            let otherIdentity = new ns.Quat(0, 0, 0, -1);
            res = otherIdentity.GetRotationAxisAndAngle();
            NS_TEST.VEC3(res.axis, new ns.Vec3(1, 0, 0), 0.001);
            NS_TEST.FLOAT(ns.Angle.RadianToDegree(res.angleInRadian), 360, 0.001);
        }

        // GetAsMat3
        {
            let q = new ns.Quat();
            q.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(90));

            let mr = new ns.Mat3();
            mr.SetRotationMatrixZ(ns.Angle.DegreeToRadian(90));

            let m = q.GetAsMat3();

            NS_TEST.BOOL(mr.IsEqual(m, 0.001));
        }

        // GetAsMat4
        {
            let q = new ns.Quat();
            q.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(90));

            let mr = new ns.Mat4();
            mr.SetRotationMatrixZ(ns.Angle.DegreeToRadian(90));

            let m = q.GetAsMat4();

            NS_TEST.BOOL(mr.IsEqual(m, 0.001));
        }

        // SetShortestRotation / IsEqualRotation
        {
            let q1 = new ns.Quat();
            let q2 = new ns.Quat();
            let q3 = new ns.Quat();

            q1.SetShortestRotation(new ns.Vec3(0, 1, 0), new ns.Vec3(1, 0, 0));
            q2.SetFromAxisAndAngle(new ns.Vec3(0, 0, -1), ns.Angle.DegreeToRadian(90));
            q3.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(-90));

            NS_TEST.BOOL(q1.IsEqualRotation(q2, 0.001));
            NS_TEST.BOOL(q1.IsEqualRotation(q3, 0.001));

            NS_TEST.BOOL(ns.Quat.IdentityQuaternion().IsEqualRotation(ns.Quat.IdentityQuaternion(), 0.001));
            NS_TEST.BOOL(ns.Quat.IdentityQuaternion().IsEqualRotation(new ns.Quat(0, 0, 0, -1), 0.001));
        }

        // SetConcatenatedRotations / ConcatenateRotations
        {
            let q1 = new ns.Quat();
            let q2 = new ns.Quat();
            let q3 = new ns.Quat();
            let qr = new ns.Quat();

            q1.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(60));
            q2.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(30));
            q3.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(90));

            qr.SetConcatenatedRotations(q1, q2);

            NS_TEST.BOOL(qr.IsEqualRotation(q3, 0.0001));

            let qr2 = q1.Clone();
            qr2.ConcatenateRotations(q2);

            NS_TEST.QUAT(qr, qr2, 0.001);
        }

        // IsIdentical
        {
            let q1 = new ns.Quat();
            let q2 = new ns.Quat();

            q1.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(60));
            q2.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(30));
            NS_TEST.BOOL(!q1.IsIdentical(q2));

            q2.SetFromAxisAndAngle(new ns.Vec3(1, 0, 0), ns.Angle.DegreeToRadian(60));
            NS_TEST.BOOL(!q1.IsIdentical(q2));

            q2.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(60));
            NS_TEST.BOOL(q1.IsIdentical(q2));
        }

        // Negate / GetNegated
        {
            let q = new ns.Quat();
            q.SetFromAxisAndAngle(new ns.Vec3(1, 0, 0), ns.Angle.DegreeToRadian(90));

            let v = new ns.Vec3(0, 1, 0);
            q.RotateVec3(v);
            NS_TEST.VEC3(v, new ns.Vec3(0, 0, 1), 0.0001);

            let n1 = q.GetNegated();
            let n2 = q.Clone();
            n2.Negate();

            NS_TEST.QUAT(n1, n2, 0.001);

            let v2 = new ns.Vec3(0, 1, 0);
            n1.RotateVec3(v2);
            NS_TEST.VEC3(v2, new ns.Vec3(0, 0, -1), 0.0001);
        }

        // SetFromEulerAngles / GetAsEulerAngles
        {
            let q = new ns.Quat();
            q.SetFromEulerAngles(ns.Angle.DegreeToRadian(90), 0, 0);

            let euler = q.GetAsEulerAngles();
            NS_TEST.FLOAT(euler.roll, ns.Angle.DegreeToRadian(90), 0.001);
            NS_TEST.FLOAT(euler.pitch, ns.Angle.DegreeToRadian(0), 0.001);
            NS_TEST.FLOAT(euler.yaw, ns.Angle.DegreeToRadian(0), 0.001);

            q.SetFromEulerAngles(0, ns.Angle.DegreeToRadian(90), 0);
            euler = q.GetAsEulerAngles();
            NS_TEST.FLOAT(euler.pitch, ns.Angle.DegreeToRadian(90), 0.001);

            // due to compilation differences, this the result for this computation can be very different (but equivalent)
            NS_TEST.BOOL((ns.Utils.IsNumberEqual(euler.roll, ns.Angle.DegreeToRadian(180), 0.001) && 
                          ns.Utils.IsNumberEqual(euler.yaw, ns.Angle.DegreeToRadian(180), 0.001)) ||
                          (ns.Utils.IsNumberEqual(euler.roll, ns.Angle.DegreeToRadian(0), 0.001) && 
                          ns.Utils.IsNumberEqual(euler.yaw, ns.Angle.DegreeToRadian(0), 0.001)));


            q.SetFromEulerAngles(0, 0, ns.Angle.DegreeToRadian(90));
            euler = q.GetAsEulerAngles();
            NS_TEST.FLOAT(euler.roll, ns.Angle.DegreeToRadian(0), 0.001);
            NS_TEST.FLOAT(euler.pitch, ns.Angle.DegreeToRadian(0), 0.001);
            NS_TEST.FLOAT(euler.yaw, ns.Angle.DegreeToRadian(90), 0.001);
        }
    }


    OnMsgGenericEvent(msg: ns.MsgGenericEvent): void {

        if (msg.Message == "TestQuat") {

            this.ExecuteTests();

            msg.Message = "done";
        }
    }

}

