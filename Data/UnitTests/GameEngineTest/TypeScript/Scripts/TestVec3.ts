import wd = require("TypeScript/wd")
import WD_TEST = require("./TestFramework")

export class TestVec3 extends wd.TypescriptComponent {

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
        let d = new wd.Vec3();
        WD_TEST.FLOAT(d.x, 0, 0.001);
        WD_TEST.FLOAT(d.y, 0, 0.001);
        WD_TEST.FLOAT(d.z, 0, 0.001);

        // ZeroVector
        WD_TEST.VEC3(new wd.Vec3(), wd.Vec3.ZeroVector(), 0.0001);

        let v = new wd.Vec3(1, 2, 3);
        WD_TEST.FLOAT(v.x, 1, 0.001);
        WD_TEST.FLOAT(v.y, 2, 0.001);
        WD_TEST.FLOAT(v.z, 3, 0.001);

        // Clone
        WD_TEST.VEC3(v.Clone(), v, 0.001);

        // CloneAsVec2
        WD_TEST.VEC2(v.CloneAsVec2(), new wd.Vec2(1, 2), 0.001);

        // OneVector
        WD_TEST.VEC3(new wd.Vec3(1, 1, 1), wd.Vec3.OneVector(), 0.0001);

        // UnitAxisX
        WD_TEST.VEC3(new wd.Vec3(1, 0, 0), wd.Vec3.UnitAxisX(), 0.0001);

        // UnitAxisY
        WD_TEST.VEC3(new wd.Vec3(0, 1, 0), wd.Vec3.UnitAxisY(), 0.0001);

        // UnitAxisZ
        WD_TEST.VEC3(new wd.Vec3(0, 0, 1), wd.Vec3.UnitAxisZ(), 0.0001);

        // Set
        v.Set(4, 5, 6);
        WD_TEST.FLOAT(v.x, 4, 0.001);
        WD_TEST.FLOAT(v.y, 5, 0.001);
        WD_TEST.FLOAT(v.z, 6, 0.001);

        // SetVec3
        let v2 = new wd.Vec3();
        v2.SetVec3(v);
        WD_TEST.VEC3(v, v2, 0.0001);

        // SetAll
        v2.SetAll(7);
        WD_TEST.FLOAT(v2.x, 7, 0.001);
        WD_TEST.FLOAT(v2.y, 7, 0.001);
        WD_TEST.FLOAT(v2.z, 7, 0.001);

        // SetZero
        v2.SetZero();
        WD_TEST.FLOAT(v2.x, 0, 0.001);
        WD_TEST.FLOAT(v2.y, 0, 0.001);
        WD_TEST.FLOAT(v2.z, 0, 0.001);

        // GetLengthSquared
        WD_TEST.FLOAT(v2.GetLengthSquared(), 0, 0.001);
        v2.SetAll(1);

        WD_TEST.FLOAT(v2.GetLengthSquared(), 3, 0.001);

        // GetLength
        WD_TEST.FLOAT(v2.GetLength(), Math.sqrt(3), 0.001);

        // GetLengthAndNormalize
        let l = v2.GetLengthAndNormalize();
        WD_TEST.FLOAT(l, Math.sqrt(3), 0.001);
        WD_TEST.FLOAT(v2.GetLength(), 1, 0.001);

        // IsNormalized
        WD_TEST.BOOL(!v.IsNormalized());

        // Normalize
        v.Normalize();

        WD_TEST.FLOAT(v.GetLength(), 1, 0.001);
        WD_TEST.BOOL(v.IsNormalized());

        // GetNormalized
        v.Set(3, 0, 0);
        WD_TEST.VEC3(v.GetNormalized(), wd.Vec3.UnitAxisX(), 0.0001);

        // NormalizeIfNotZero
        WD_TEST.BOOL(v.NormalizeIfNotZero(wd.Vec3.UnitAxisZ(), 0.001));
        WD_TEST.VEC3(v, wd.Vec3.UnitAxisX(), 0.0001);

        // IsZero
        WD_TEST.BOOL(!v.IsZero());

        // SetZero
        v.SetZero();
        WD_TEST.BOOL(v.IsZero());

        WD_TEST.BOOL(!v.NormalizeIfNotZero(wd.Vec3.UnitAxisZ(), 0.001));
        WD_TEST.VEC3(v, wd.Vec3.UnitAxisZ(), 0.0001);

        // GetNegated
        v.Set(1, 2, 3);
        WD_TEST.VEC3(v.GetNegated(), new wd.Vec3(-1, -2, -3), 0.0001);
        WD_TEST.VEC3(v, new wd.Vec3(1, 2, 3), 0.0001);

        // Negate
        v.Negate();
        WD_TEST.VEC3(v, new wd.Vec3(-1, -2, -3), 0.0001);

        // AddVec3
        v.Set(2, 3, 4);
        v2.Set(5, 6, 7);
        v.AddVec3(v2);
        WD_TEST.VEC3(v, new wd.Vec3(7, 9, 11), 0.0001);

        // SubVec3
        v.SubVec3(v2);
        WD_TEST.VEC3(v, new wd.Vec3(2, 3, 4), 0.0001);

        // MulVec3
        v.MulVec3(v);
        WD_TEST.VEC3(v, new wd.Vec3(4, 9, 16), 0.0001);

        // DivVec3
        v.DivVec3(new wd.Vec3(2, 3, 4));
        WD_TEST.VEC3(v, new wd.Vec3(2, 3, 4), 0.0001);

        // MulNumber
        v.MulNumber(2);
        WD_TEST.VEC3(v, new wd.Vec3(4, 6, 8), 0.0001);

        // DivNumber
        v.DivNumber(2);
        WD_TEST.VEC3(v, new wd.Vec3(2, 3, 4), 0.0001);

        // IsIdentical
        WD_TEST.BOOL(v.IsIdentical(v));
        WD_TEST.BOOL(!v.IsIdentical(v2));

        // IsEqual
        WD_TEST.BOOL(v.IsEqual(new wd.Vec3(2, 3, 4), 0.0001));
        WD_TEST.BOOL(!v.IsEqual(new wd.Vec3(2, 3.5, 4), 0.0001));

        // Dot
        v.Set(2, 3, 4);
        v2.Set(3, 4, 5);
        WD_TEST.FLOAT(v.Dot(v2), 38, 0.001);

        // CrossRH
        v.Set(1, 0, 0);
        v2.Set(0, 1, 0);
        WD_TEST.VEC3(v.CrossRH(v2), new wd.Vec3(0, 0, 1), 0.001);

        // SetCrossRH
        let v3 = new wd.Vec3();
        v3.SetCrossRH(v, v2);
        WD_TEST.VEC3(v3, new wd.Vec3(0, 0, 1), 0.001);

        // GetCompMin
        v.Set(2, 4, 6);
        v2.Set(1, 5, 7);
        WD_TEST.VEC3(v.GetCompMin(v2), new wd.Vec3(1, 4, 6), 0.001);

        // GetCompMax
        v.Set(2, 4, 6);
        v2.Set(1, 5, 7);
        WD_TEST.VEC3(v.GetCompMax(v2), new wd.Vec3(2, 5, 7), 0.001);

        // GetCompClamp
        WD_TEST.VEC3(v.GetCompClamp(new wd.Vec3(3, 4, 5), new wd.Vec3(4, 5, 6)), new wd.Vec3(3, 4, 6), 0.001);

        // GetCompMul
        WD_TEST.VEC3(v.GetCompMul(new wd.Vec3(2, 3, 4)), new wd.Vec3(4, 12, 24), 0.001);

        // GetCompDiv
        WD_TEST.VEC3(v.GetCompDiv(new wd.Vec3(2, 4, 6)), wd.Vec3.OneVector(), 0.001);

        // GetAbs
        v.Set(-1, -2, -3);
        WD_TEST.VEC3(v.GetAbs(), new wd.Vec3(1, 2, 3), 0.001);

        // SetAbs
        v2.SetAbs(v);
        WD_TEST.VEC3(v2, new wd.Vec3(1, 2, 3), 0.001);

        // CalculateNormal
        WD_TEST.BOOL(v.CalculateNormal(new wd.Vec3(-1, 0, 1), new wd.Vec3(1, 0, 1), new wd.Vec3(0, 0, -1)));
        WD_TEST.VEC3(v, new wd.Vec3(0, 1, 0), 0.001);

        WD_TEST.BOOL(v.CalculateNormal(new wd.Vec3(-1, 0, -1), new wd.Vec3(1, 0, -1), new wd.Vec3(0, 0, 1)));
        WD_TEST.VEC3(v, new wd.Vec3(0, -1, 0), 0.001);

        WD_TEST.BOOL(v.CalculateNormal(new wd.Vec3(-1, 0, 1), new wd.Vec3(1, 0, 1), new wd.Vec3(1, 0, 1)) == false);

        // MakeOrthogonalTo
        v.Set(1, 1, 0);
        v.MakeOrthogonalTo(new wd.Vec3(1, 0, 0));
        WD_TEST.VEC3(v, new wd.Vec3(0, 1, 0), 0.001);

        v.Set(1, 1, 0);
        v.MakeOrthogonalTo(new wd.Vec3(0, 1, 0));
        WD_TEST.VEC3(v, new wd.Vec3(1, 0, 0), 0.001);

        // GetOrthogonalVector
        for (let i = 1; i < 360; i += 3.0) {
            v.Set(i, i * 3, i * 7);
            WD_TEST.FLOAT(v.GetOrthogonalVector().Dot(v), 0.0, 0.001);
        }

        // GetReflectedVector
        v.Set(1, 1, 0);
        v2 = v.GetReflectedVector(new wd.Vec3(0, -1, 0));
        WD_TEST.VEC3(v2, new wd.Vec3(1, -1, 0), 0.0001);

        // SetAdd
        v.SetAdd(new wd.Vec3(1, 2, 3), new wd.Vec3(4, 5, 6));
        WD_TEST.VEC3(v, new wd.Vec3(5, 7, 9), 0.0001);

        // SetSub
        v.SetSub(new wd.Vec3(4, 5, 6), new wd.Vec3(1, 2, 3));
        WD_TEST.VEC3(v, new wd.Vec3(3, 3, 3), 0.0001);

        // SetMul
        v.SetMul(new wd.Vec3(1, 2, 3), 2);
        WD_TEST.VEC3(v, new wd.Vec3(2, 4, 6), 0.0001);

        // SetDiv
        v.SetDiv(new wd.Vec3(2, 4, 6), 2);
        WD_TEST.VEC3(v, new wd.Vec3(1, 2, 3), 0.0001);

        // SetLength
        v.Set(0, 2, 0);
        WD_TEST.BOOL(v.SetLength(5, 0.001));
        WD_TEST.VEC3(v, new wd.Vec3(0, 5, 0), 0.0001);

        // CreateRandomPointInSphere
        {
            let avg = new wd.Vec3();

            const uiNumSamples = 1000;
            for (let i = 0; i < uiNumSamples; ++i) {
                v = wd.Vec3.CreateRandomPointInSphere();

                WD_TEST.BOOL(v.GetLength() <= 1.0);
                WD_TEST.BOOL(!v.IsZero());

                avg.AddVec3(v);
            }

            avg.DivNumber(uiNumSamples);

            // the average point cloud center should be within at least 10% of the sphere's center
            // otherwise the points aren't equally distributed
            WD_TEST.BOOL(avg.IsZero(0.1));
        }

        // CreateRandomDirection
        {
            let avg = new wd.Vec3();

            const uiNumSamples = 1000;
            for (let i = 0; i < uiNumSamples; ++i) {
                v = wd.Vec3.CreateRandomDirection();

                WD_TEST.BOOL(v.IsNormalized());

                avg.AddVec3(v);
            }

            avg.DivNumber(uiNumSamples);

            // the average point cloud center should be within at least 10% of the sphere's center
            // otherwise the points aren't equally distributed
            WD_TEST.BOOL(avg.IsZero(0.1));
        }
    }

    OnMsgGenericEvent(msg: wd.MsgGenericEvent): void {

        if (msg.Message == "TestVec3") {

            this.ExecuteTests();

            msg.Message = "done";
        }
    }

}

