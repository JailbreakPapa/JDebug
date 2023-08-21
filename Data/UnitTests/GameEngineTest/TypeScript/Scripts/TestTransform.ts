import wd = require("TypeScript/wd")
import WD_TEST = require("./TestFramework")

export class TestTransform extends wd.TypescriptComponent {

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
            let t = new wd.Transform();

            WD_TEST.VEC3(t.position, wd.Vec3.ZeroVector());
            WD_TEST.QUAT(t.rotation, wd.Quat.IdentityQuaternion());
            WD_TEST.VEC3(t.scale, wd.Vec3.OneVector());
        }

        // Clone
        {
            let t = new wd.Transform();
            t.position.Set(1, 2, 3);
            t.rotation.SetFromAxisAndAngle(wd.Vec3.UnitAxisZ(), wd.Angle.DegreeToRadian(90));
            t.scale.Set(4, 5, 6);

            let c = t.Clone();
            WD_TEST.BOOL(t != c);
            WD_TEST.BOOL(t.position != c.position);
            WD_TEST.BOOL(t.rotation != c.rotation);
            WD_TEST.BOOL(t.scale != c.scale);

            WD_TEST.VEC3(t.position, c.position);
            WD_TEST.QUAT(t.rotation, c.rotation);
            WD_TEST.VEC3(t.scale, c.scale);
        }

        // SetTransform
        {
            let t = new wd.Transform();
            t.position.Set(1, 2, 3);
            t.rotation.SetFromAxisAndAngle(wd.Vec3.UnitAxisZ(), wd.Angle.DegreeToRadian(90));
            t.scale.Set(4, 5, 6);

            let c = new wd.Transform();
            c.SetTransform(t);

            WD_TEST.BOOL(t != c);
            WD_TEST.BOOL(t.position != c.position);
            WD_TEST.BOOL(t.rotation != c.rotation);
            WD_TEST.BOOL(t.scale != c.scale);

            WD_TEST.VEC3(t.position, c.position);
            WD_TEST.QUAT(t.rotation, c.rotation);
            WD_TEST.VEC3(t.scale, c.scale);
        }

        // SetIdentity
        {
            let t = new wd.Transform();
            t.position.Set(1, 2, 3);
            t.rotation.SetFromAxisAndAngle(wd.Vec3.UnitAxisZ(), wd.Angle.DegreeToRadian(90));
            t.scale.Set(4, 5, 6);

            t.SetIdentity();

            WD_TEST.VEC3(t.position, wd.Vec3.ZeroVector());
            WD_TEST.QUAT(t.rotation, wd.Quat.IdentityQuaternion());
            WD_TEST.VEC3(t.scale, wd.Vec3.OneVector());
        }

        // IdentityTransform
        {
            let t = wd.Transform.IdentityTransform();

            WD_TEST.VEC3(t.position, wd.Vec3.ZeroVector());
            WD_TEST.QUAT(t.rotation, wd.Quat.IdentityQuaternion());
            WD_TEST.VEC3(t.scale, wd.Vec3.OneVector());
        }

        // IsIdentical
        {
            let t = new wd.Transform();
            t.position.Set(1, 2, 3);
            t.rotation.SetFromAxisAndAngle(wd.Vec3.UnitAxisZ(), wd.Angle.DegreeToRadian(90));
            t.scale.Set(4, 5, 6);

            let c = new wd.Transform();
            c.SetTransform(t);

            WD_TEST.BOOL(t.IsIdentical(c));

            c.position.x += 0.0001;

            WD_TEST.BOOL(!t.IsIdentical(c));
        }

        // IsEqual
        {
            let t = new wd.Transform();
            t.position.Set(1, 2, 3);
            t.rotation.SetFromAxisAndAngle(wd.Vec3.UnitAxisZ(), wd.Angle.DegreeToRadian(90));
            t.scale.Set(4, 5, 6);

            let c = new wd.Transform();
            c.SetTransform(t);

            WD_TEST.BOOL(t.IsEqual(c));

            c.position.x += 0.0001;

            WD_TEST.BOOL(t.IsEqual(c, 0.001));
            WD_TEST.BOOL(!t.IsEqual(c, 0.00001));
        }

        // Translate
        {
            let t = new wd.Transform();
            t.Translate(new wd.Vec3(1, 2, 3));

            WD_TEST.VEC3(t.position, new wd.Vec3(1, 2, 3));
            WD_TEST.QUAT(t.rotation, wd.Quat.IdentityQuaternion());
            WD_TEST.VEC3(t.scale, wd.Vec3.OneVector());
        }

        // SetMulTransform / MulTransform
        {
            let tParent = new wd.Transform();
            tParent.position.Set(1, 2, 3);

            tParent.rotation.SetFromAxisAndAngle(new wd.Vec3(0, 1, 0), wd.Angle.DegreeToRadian(90));
            tParent.scale.SetAll(2);

            let tToChild = new wd.Transform();
            tToChild.position.Set(4, 5, 6);

            tToChild.rotation.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(90));
            tToChild.scale.SetAll(4);

            // this is exactly the same as SetGlobalTransform
            let tChild = new wd.Transform();
            tChild.SetMulTransform(tParent, tToChild);

            WD_TEST.BOOL(tChild.position.IsEqual(new wd.Vec3(13, 12, -5), 0.0001));

            let q1 = new wd.Quat();
            q1.SetConcatenatedRotations(tParent.rotation, tToChild.rotation);
            WD_TEST.BOOL(tChild.rotation.IsEqualRotation(q1, 0.0001));

            WD_TEST.VEC3(tChild.scale, new wd.Vec3(8, 8, 8));

            tChild = tParent.Clone();
            tChild.MulTransform(tToChild);

            WD_TEST.BOOL(tChild.position.IsEqual(new wd.Vec3(13, 12, -5), 0.0001));

            q1.SetConcatenatedRotations(tParent.rotation, tToChild.rotation);
            WD_TEST.QUAT(tChild.rotation, q1);
            WD_TEST.VEC3(tChild.scale, new wd.Vec3(8, 8, 8));

            let a = new wd.Vec3(7, 8, 9);
            let b = a.Clone();
            tToChild.TransformPosition(b);
            tParent.TransformPosition(b);

            let c = a.Clone();
            tChild.TransformPosition(c);

            WD_TEST.VEC3(b, c);
        }

        // Invert / GetInverse
        {
            let tParent = new wd.Transform();
            tParent.position.Set(1, 2, 3);

            tParent.rotation.SetFromAxisAndAngle(new wd.Vec3(0, 1, 0), wd.Angle.DegreeToRadian(90));
            tParent.scale.SetAll(2);

            let tToChild = new wd.Transform();
            tParent.position.Set(4, 5, 6);

            tToChild.rotation.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(90));
            tToChild.scale.SetAll(4);

            let tChild = new wd.Transform();
            tChild.SetMulTransform(tParent, tToChild);

            // invert twice -> get back original
            let t2 = tToChild.Clone();
            t2.Invert();
            WD_TEST.BOOL(!t2.IsEqual(tToChild, 0.0001));
            t2 = t2.GetInverse();
            WD_TEST.BOOL(t2.IsEqual(tToChild, 0.0001));

            let tInvToChild = tToChild.GetInverse();

            let tParentFromChild = new wd.Transform();
            tParentFromChild.SetMulTransform(tChild, tInvToChild);

            WD_TEST.BOOL(tParent.IsEqual(tParentFromChild, 0.0001));
        }

        // SetLocalTransform
        {
            let q = new wd.Quat();
            q.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(90));

            let tParent = new wd.Transform();
            tParent.position.Set(1, 2, 3);
            tParent.rotation.SetFromAxisAndAngle(new wd.Vec3(0, 1, 0), wd.Angle.DegreeToRadian(90));
            tParent.scale.SetAll(2);

            let tChild = new wd.Transform();
            tChild.position.Set(13, 12, -5);
            tChild.rotation.SetConcatenatedRotations(tParent.rotation, q);
            tChild.scale.SetAll(8);

            let tToChild = new wd.Transform();
            tToChild.SetLocalTransform(tParent, tChild);

            WD_TEST.VEC3(tToChild.position, new wd.Vec3(4, 5, 6));
            WD_TEST.QUAT(tToChild.rotation, q);
            WD_TEST.VEC3(tToChild.scale, new wd.Vec3(4, 4, 4));
        }

        // SetGlobalTransform
        {
            let tParent = new wd.Transform();
            tParent.position.Set(1, 2, 3);
            tParent.rotation.SetFromAxisAndAngle(new wd.Vec3(0, 1, 0), wd.Angle.DegreeToRadian(90));
            tParent.scale.SetAll(2);

            let tToChild = new wd.Transform();
            tToChild.position.Set(4, 5, 6);
            tToChild.rotation.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(90));
            tToChild.scale.SetAll(4);

            let tChild = new wd.Transform();
            tChild.SetGlobalTransform(tParent, tToChild);

            WD_TEST.VEC3(tChild.position, new wd.Vec3(13, 12, -5));

            let q = new wd.Quat();
            q.SetConcatenatedRotations(tParent.rotation, tToChild.rotation);
            WD_TEST.QUAT(tChild.rotation, q);
            WD_TEST.VEC3(tChild.scale, new wd.Vec3(8, 8, 8));
        }

        // TransformPosition / TransformDirection
        {
            let qRotX = new wd.Quat();
            let qRotY = new wd.Quat();

            qRotX.SetFromAxisAndAngle(new wd.Vec3(1, 0, 0), wd.Angle.DegreeToRadian(90));
            qRotY.SetFromAxisAndAngle(new wd.Vec3(0, 1, 0), wd.Angle.DegreeToRadian(90));

            let t = new wd.Transform();
            t.position.Set(1, 2, 3);
            t.rotation.SetConcatenatedRotations(qRotY, qRotX);
            t.scale.Set(2, -2, 4);

            let v = new wd.Vec3(4, 5, 6);
            t.TransformPosition(v);
            WD_TEST.VEC3(v, new wd.Vec3((5 * -2) + 1, (-6 * 4) + 2, (-4 * 2) + 3));

            v.Set(4, 5, 6);
            t.TransformDirection(v);
            WD_TEST.VEC3(v, new wd.Vec3((5 * -2), (-6 * 4), (-4 * 2)));
        }

        // ConcatenateRotations / ConcatenateRotationsReverse
        {
            let t = new wd.Transform();
            t.position.Set(1, 2, 3);
            t.rotation.SetFromAxisAndAngle(new wd.Vec3(0, 1, 0), wd.Angle.DegreeToRadian(90));
            t.scale.SetAll(2);

            let q = new wd.Quat();
            q.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(90));

            let t2 = t.Clone();
            let t4 = t.Clone();
            t2.ConcatenateRotations(q);
            t4.ConcatenateRotationsReverse(q);

            let t3 = t.Clone();
            t3.ConcatenateRotations(q);
            WD_TEST.BOOL(t2.IsEqual(t3));
            WD_TEST.BOOL(!t3.IsEqual(t4));

            let a = new wd.Vec3(7, 8, 9);
            let b = a.Clone();
            t2.TransformPosition(b);

            let c = a.Clone();
            q.RotateVec3(c);
            t.TransformPosition(c);

            WD_TEST.VEC3(b, c);
        }

        // GetAsMat4
        {
            let t = new wd.Transform();
            t.position.Set(1, 2, 3);
            t.rotation.SetFromAxisAndAngle(new wd.Vec3(0, 1, 0), wd.Angle.DegreeToRadian(34));
            t.scale.Set(2, -1, 5);

            let m = t.GetAsMat4();

            // reference
            {
                let q = new wd.Quat();
                q.SetFromAxisAndAngle(new wd.Vec3(0, 1, 0), wd.Angle.DegreeToRadian(34));

                let referenceTransform = new wd.Transform();
                referenceTransform.position.Set(1, 2, 3);
                referenceTransform.rotation.SetQuat(q);
                referenceTransform.scale.Set(2, -1, 5);

                let refM = referenceTransform.GetAsMat4();

                WD_TEST.BOOL(m.IsEqual(refM));
            }

            let p: wd.Vec3[] = [new wd.Vec3(- 4, 0, 0), new wd.Vec3(5, 0, 0), new wd.Vec3(0, -6, 0), new wd.Vec3(0, 7, 0),
            new wd.Vec3(0, 0, -8), new wd.Vec3(0, 0, 9), new wd.Vec3(1, -2, 3), new wd.Vec3(-4, 5, 7)];

            for (let i = 0; i < 8; ++i) {

                let pt = p[i].Clone();
                t.TransformPosition(pt);

                let pm = p[i].Clone();
                m.TransformPosition(pm);

                WD_TEST.VEC3(pt, pm);
            }
        }

        // SetFromMat4
        {
            let mRot = new wd.Mat3();
            mRot.SetRotationMatrix((new wd.Vec3(1, 2, 3)).GetNormalized(), wd.Angle.DegreeToRadian(42));

            let mTrans = new wd.Mat4();
            mTrans.SetTransformationMatrix(mRot, new wd.Vec3(1, 2, 3));

            let t = new wd.Transform();
            t.SetFromMat4(mTrans);
            WD_TEST.VEC3(t.position, new wd.Vec3(1, 2, 3), 0);
            WD_TEST.BOOL(t.rotation.GetAsMat3().IsEqual(mRot, 0.001));
        }
    }

    OnMsgGenericEvent(msg: wd.MsgGenericEvent): void {

        if (msg.Message == "TestTransform") {

            this.ExecuteTests();

            msg.Message = "done";
        }
    }

}

