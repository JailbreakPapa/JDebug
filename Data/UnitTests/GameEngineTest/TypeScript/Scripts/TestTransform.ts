import ns = require("TypeScript/ns")
import NS_TEST = require("./TestFramework")

export class TestTransform extends ns.TypescriptComponent {

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
            let t = new ns.Transform();

            NS_TEST.VEC3(t.position, ns.Vec3.ZeroVector());
            NS_TEST.QUAT(t.rotation, ns.Quat.IdentityQuaternion());
            NS_TEST.VEC3(t.scale, ns.Vec3.OneVector());
        }

        // Clone
        {
            let t = new ns.Transform();
            t.position.Set(1, 2, 3);
            t.rotation.SetFromAxisAndAngle(ns.Vec3.UnitAxisZ(), ns.Angle.DegreeToRadian(90));
            t.scale.Set(4, 5, 6);

            let c = t.Clone();
            NS_TEST.BOOL(t != c);
            NS_TEST.BOOL(t.position != c.position);
            NS_TEST.BOOL(t.rotation != c.rotation);
            NS_TEST.BOOL(t.scale != c.scale);

            NS_TEST.VEC3(t.position, c.position);
            NS_TEST.QUAT(t.rotation, c.rotation);
            NS_TEST.VEC3(t.scale, c.scale);
        }

        // SetTransform
        {
            let t = new ns.Transform();
            t.position.Set(1, 2, 3);
            t.rotation.SetFromAxisAndAngle(ns.Vec3.UnitAxisZ(), ns.Angle.DegreeToRadian(90));
            t.scale.Set(4, 5, 6);

            let c = new ns.Transform();
            c.SetTransform(t);

            NS_TEST.BOOL(t != c);
            NS_TEST.BOOL(t.position != c.position);
            NS_TEST.BOOL(t.rotation != c.rotation);
            NS_TEST.BOOL(t.scale != c.scale);

            NS_TEST.VEC3(t.position, c.position);
            NS_TEST.QUAT(t.rotation, c.rotation);
            NS_TEST.VEC3(t.scale, c.scale);
        }

        // SetIdentity
        {
            let t = new ns.Transform();
            t.position.Set(1, 2, 3);
            t.rotation.SetFromAxisAndAngle(ns.Vec3.UnitAxisZ(), ns.Angle.DegreeToRadian(90));
            t.scale.Set(4, 5, 6);

            t.SetIdentity();

            NS_TEST.VEC3(t.position, ns.Vec3.ZeroVector());
            NS_TEST.QUAT(t.rotation, ns.Quat.IdentityQuaternion());
            NS_TEST.VEC3(t.scale, ns.Vec3.OneVector());
        }

        // IdentityTransform
        {
            let t = ns.Transform.IdentityTransform();

            NS_TEST.VEC3(t.position, ns.Vec3.ZeroVector());
            NS_TEST.QUAT(t.rotation, ns.Quat.IdentityQuaternion());
            NS_TEST.VEC3(t.scale, ns.Vec3.OneVector());
        }

        // IsIdentical
        {
            let t = new ns.Transform();
            t.position.Set(1, 2, 3);
            t.rotation.SetFromAxisAndAngle(ns.Vec3.UnitAxisZ(), ns.Angle.DegreeToRadian(90));
            t.scale.Set(4, 5, 6);

            let c = new ns.Transform();
            c.SetTransform(t);

            NS_TEST.BOOL(t.IsIdentical(c));

            c.position.x += 0.0001;

            NS_TEST.BOOL(!t.IsIdentical(c));
        }

        // IsEqual
        {
            let t = new ns.Transform();
            t.position.Set(1, 2, 3);
            t.rotation.SetFromAxisAndAngle(ns.Vec3.UnitAxisZ(), ns.Angle.DegreeToRadian(90));
            t.scale.Set(4, 5, 6);

            let c = new ns.Transform();
            c.SetTransform(t);

            NS_TEST.BOOL(t.IsEqual(c));

            c.position.x += 0.0001;

            NS_TEST.BOOL(t.IsEqual(c, 0.001));
            NS_TEST.BOOL(!t.IsEqual(c, 0.00001));
        }

        // Translate
        {
            let t = new ns.Transform();
            t.Translate(new ns.Vec3(1, 2, 3));

            NS_TEST.VEC3(t.position, new ns.Vec3(1, 2, 3));
            NS_TEST.QUAT(t.rotation, ns.Quat.IdentityQuaternion());
            NS_TEST.VEC3(t.scale, ns.Vec3.OneVector());
        }

        // SetMulTransform / MulTransform
        {
            let tParent = new ns.Transform();
            tParent.position.Set(1, 2, 3);

            tParent.rotation.SetFromAxisAndAngle(new ns.Vec3(0, 1, 0), ns.Angle.DegreeToRadian(90));
            tParent.scale.SetAll(2);

            let tToChild = new ns.Transform();
            tToChild.position.Set(4, 5, 6);

            tToChild.rotation.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(90));
            tToChild.scale.SetAll(4);

            // this is exactly the same as SetGlobalTransform
            let tChild = new ns.Transform();
            tChild.SetMulTransform(tParent, tToChild);

            NS_TEST.BOOL(tChild.position.IsEqual(new ns.Vec3(13, 12, -5), 0.0001));

            let q1 = new ns.Quat();
            q1.SetConcatenatedRotations(tParent.rotation, tToChild.rotation);
            NS_TEST.BOOL(tChild.rotation.IsEqualRotation(q1, 0.0001));

            NS_TEST.VEC3(tChild.scale, new ns.Vec3(8, 8, 8));

            tChild = tParent.Clone();
            tChild.MulTransform(tToChild);

            NS_TEST.BOOL(tChild.position.IsEqual(new ns.Vec3(13, 12, -5), 0.0001));

            q1.SetConcatenatedRotations(tParent.rotation, tToChild.rotation);
            NS_TEST.QUAT(tChild.rotation, q1);
            NS_TEST.VEC3(tChild.scale, new ns.Vec3(8, 8, 8));

            let a = new ns.Vec3(7, 8, 9);
            let b = a.Clone();
            tToChild.TransformPosition(b);
            tParent.TransformPosition(b);

            let c = a.Clone();
            tChild.TransformPosition(c);

            NS_TEST.VEC3(b, c);
        }

        // Invert / GetInverse
        {
            let tParent = new ns.Transform();
            tParent.position.Set(1, 2, 3);

            tParent.rotation.SetFromAxisAndAngle(new ns.Vec3(0, 1, 0), ns.Angle.DegreeToRadian(90));
            tParent.scale.SetAll(2);

            let tToChild = new ns.Transform();
            tParent.position.Set(4, 5, 6);

            tToChild.rotation.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(90));
            tToChild.scale.SetAll(4);

            let tChild = new ns.Transform();
            tChild.SetMulTransform(tParent, tToChild);

            // invert twice -> get back original
            let t2 = tToChild.Clone();
            t2.Invert();
            NS_TEST.BOOL(!t2.IsEqual(tToChild, 0.0001));
            t2 = t2.GetInverse();
            NS_TEST.BOOL(t2.IsEqual(tToChild, 0.0001));

            let tInvToChild = tToChild.GetInverse();

            let tParentFromChild = new ns.Transform();
            tParentFromChild.SetMulTransform(tChild, tInvToChild);

            NS_TEST.BOOL(tParent.IsEqual(tParentFromChild, 0.0001));
        }

        // SetLocalTransform
        {
            let q = new ns.Quat();
            q.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(90));

            let tParent = new ns.Transform();
            tParent.position.Set(1, 2, 3);
            tParent.rotation.SetFromAxisAndAngle(new ns.Vec3(0, 1, 0), ns.Angle.DegreeToRadian(90));
            tParent.scale.SetAll(2);

            let tChild = new ns.Transform();
            tChild.position.Set(13, 12, -5);
            tChild.rotation.SetConcatenatedRotations(tParent.rotation, q);
            tChild.scale.SetAll(8);

            let tToChild = new ns.Transform();
            tToChild.SetLocalTransform(tParent, tChild);

            NS_TEST.VEC3(tToChild.position, new ns.Vec3(4, 5, 6));
            NS_TEST.QUAT(tToChild.rotation, q);
            NS_TEST.VEC3(tToChild.scale, new ns.Vec3(4, 4, 4));
        }

        // SetGlobalTransform
        {
            let tParent = new ns.Transform();
            tParent.position.Set(1, 2, 3);
            tParent.rotation.SetFromAxisAndAngle(new ns.Vec3(0, 1, 0), ns.Angle.DegreeToRadian(90));
            tParent.scale.SetAll(2);

            let tToChild = new ns.Transform();
            tToChild.position.Set(4, 5, 6);
            tToChild.rotation.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(90));
            tToChild.scale.SetAll(4);

            let tChild = new ns.Transform();
            tChild.SetGlobalTransform(tParent, tToChild);

            NS_TEST.VEC3(tChild.position, new ns.Vec3(13, 12, -5));

            let q = new ns.Quat();
            q.SetConcatenatedRotations(tParent.rotation, tToChild.rotation);
            NS_TEST.QUAT(tChild.rotation, q);
            NS_TEST.VEC3(tChild.scale, new ns.Vec3(8, 8, 8));
        }

        // TransformPosition / TransformDirection
        {
            let qRotX = new ns.Quat();
            let qRotY = new ns.Quat();

            qRotX.SetFromAxisAndAngle(new ns.Vec3(1, 0, 0), ns.Angle.DegreeToRadian(90));
            qRotY.SetFromAxisAndAngle(new ns.Vec3(0, 1, 0), ns.Angle.DegreeToRadian(90));

            let t = new ns.Transform();
            t.position.Set(1, 2, 3);
            t.rotation.SetConcatenatedRotations(qRotY, qRotX);
            t.scale.Set(2, -2, 4);

            let v = new ns.Vec3(4, 5, 6);
            t.TransformPosition(v);
            NS_TEST.VEC3(v, new ns.Vec3((5 * -2) + 1, (-6 * 4) + 2, (-4 * 2) + 3));

            v.Set(4, 5, 6);
            t.TransformDirection(v);
            NS_TEST.VEC3(v, new ns.Vec3((5 * -2), (-6 * 4), (-4 * 2)));
        }

        // ConcatenateRotations / ConcatenateRotationsReverse
        {
            let t = new ns.Transform();
            t.position.Set(1, 2, 3);
            t.rotation.SetFromAxisAndAngle(new ns.Vec3(0, 1, 0), ns.Angle.DegreeToRadian(90));
            t.scale.SetAll(2);

            let q = new ns.Quat();
            q.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(90));

            let t2 = t.Clone();
            let t4 = t.Clone();
            t2.ConcatenateRotations(q);
            t4.ConcatenateRotationsReverse(q);

            let t3 = t.Clone();
            t3.ConcatenateRotations(q);
            NS_TEST.BOOL(t2.IsEqual(t3));
            NS_TEST.BOOL(!t3.IsEqual(t4));

            let a = new ns.Vec3(7, 8, 9);
            let b = a.Clone();
            t2.TransformPosition(b);

            let c = a.Clone();
            q.RotateVec3(c);
            t.TransformPosition(c);

            NS_TEST.VEC3(b, c);
        }

        // GetAsMat4
        {
            let t = new ns.Transform();
            t.position.Set(1, 2, 3);
            t.rotation.SetFromAxisAndAngle(new ns.Vec3(0, 1, 0), ns.Angle.DegreeToRadian(34));
            t.scale.Set(2, -1, 5);

            let m = t.GetAsMat4();

            // reference
            {
                let q = new ns.Quat();
                q.SetFromAxisAndAngle(new ns.Vec3(0, 1, 0), ns.Angle.DegreeToRadian(34));

                let referenceTransform = new ns.Transform();
                referenceTransform.position.Set(1, 2, 3);
                referenceTransform.rotation.SetQuat(q);
                referenceTransform.scale.Set(2, -1, 5);

                let refM = referenceTransform.GetAsMat4();

                NS_TEST.BOOL(m.IsEqual(refM));
            }

            let p: ns.Vec3[] = [new ns.Vec3(- 4, 0, 0), new ns.Vec3(5, 0, 0), new ns.Vec3(0, -6, 0), new ns.Vec3(0, 7, 0),
            new ns.Vec3(0, 0, -8), new ns.Vec3(0, 0, 9), new ns.Vec3(1, -2, 3), new ns.Vec3(-4, 5, 7)];

            for (let i = 0; i < 8; ++i) {

                let pt = p[i].Clone();
                t.TransformPosition(pt);

                let pm = p[i].Clone();
                m.TransformPosition(pm);

                NS_TEST.VEC3(pt, pm);
            }
        }

        // SetFromMat4
        {
            let mRot = new ns.Mat3();
            mRot.SetRotationMatrix((new ns.Vec3(1, 2, 3)).GetNormalized(), ns.Angle.DegreeToRadian(42));

            let mTrans = new ns.Mat4();
            mTrans.SetTransformationMatrix(mRot, new ns.Vec3(1, 2, 3));

            let t = new ns.Transform();
            t.SetFromMat4(mTrans);
            NS_TEST.VEC3(t.position, new ns.Vec3(1, 2, 3), 0);
            NS_TEST.BOOL(t.rotation.GetAsMat3().IsEqual(mRot, 0.001));
        }
    }

    OnMsgGenericEvent(msg: ns.MsgGenericEvent): void {

        if (msg.Message == "TestTransform") {

            this.ExecuteTests();

            msg.Message = "done";
        }
    }

}

