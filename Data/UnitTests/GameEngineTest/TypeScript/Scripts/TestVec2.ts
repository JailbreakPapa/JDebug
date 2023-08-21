import wd = require("TypeScript/wd")
import WD_TEST = require("./TestFramework")

export class TestVec2 extends wd.TypescriptComponent {

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
        let d = new wd.Vec2();
        WD_TEST.FLOAT(d.x, 0, 0.001);
        WD_TEST.FLOAT(d.y, 0, 0.001);

        // ZeroVector
        WD_TEST.VEC2(new wd.Vec2(), wd.Vec2.ZeroVector(), 0.0001);

        let v = new wd.Vec2(1, 2);
        WD_TEST.FLOAT(v.x, 1, 0.001);
        WD_TEST.FLOAT(v.y, 2, 0.001);

        // Clone
        WD_TEST.VEC2(v.Clone(), v, 0.001);

        // TODO: CloneAsVec3
        //WD_TEST.VEC3(v.CloneAsVec3(3), new wd.Vec3(1, 2, 3), 0.001);

        // OneVector
        WD_TEST.VEC2(new wd.Vec2(1, 1), wd.Vec2.OneVector(), 0.0001);

        // UnitAxisX
        WD_TEST.VEC2(new wd.Vec2(1, 0), wd.Vec2.UnitAxisX(), 0.0001);

        // UnitAxisY
        WD_TEST.VEC2(new wd.Vec2(0, 1), wd.Vec2.UnitAxisY(), 0.0001);

        // Set
        v.Set(4, 5);
        WD_TEST.FLOAT(v.x, 4, 0.001);
        WD_TEST.FLOAT(v.y, 5, 0.001);

        // SetVec2
        let v2 = new wd.Vec2();
        v2.SetVec2(v);
        WD_TEST.VEC2(v, v2, 0.0001);

        // SetAll
        v2.SetAll(7);
        WD_TEST.FLOAT(v2.x, 7, 0.001);
        WD_TEST.FLOAT(v2.y, 7, 0.001);

        // SetZero
        v2.SetZero();
        WD_TEST.FLOAT(v2.x, 0, 0.001);
        WD_TEST.FLOAT(v2.y, 0, 0.001);

        // GetLengthSquared
        WD_TEST.FLOAT(v2.GetLengthSquared(), 0, 0.001);
        v2.SetAll(1);

        WD_TEST.FLOAT(v2.GetLengthSquared(), 2, 0.001);

        // GetLength
        WD_TEST.FLOAT(v2.GetLength(), Math.sqrt(2), 0.001);

        // GetLengthAndNormalize
        let l = v2.GetLengthAndNormalize();
        WD_TEST.FLOAT(l, Math.sqrt(2), 0.001);
        WD_TEST.FLOAT(v2.GetLength(), 1, 0.001);

        // IsNormalized
        WD_TEST.BOOL(!v.IsNormalized());

        // Normalize
        v.Normalize();

        WD_TEST.FLOAT(v.GetLength(), 1, 0.001);
        WD_TEST.BOOL(v.IsNormalized());


        // GetNormalized
        v.Set(3, 0);
        WD_TEST.VEC2(v.GetNormalized(), wd.Vec2.UnitAxisX(), 0.0001);

        // NormalizeIfNotZero
        WD_TEST.BOOL(v.NormalizeIfNotZero(wd.Vec2.UnitAxisY(), 0.001));
        WD_TEST.VEC2(v, wd.Vec2.UnitAxisX(), 0.0001);

        // IsZero
        WD_TEST.BOOL(!v.IsZero());

        // SetZero
        v.SetZero();
        WD_TEST.BOOL(v.IsZero());

        WD_TEST.BOOL(!v.NormalizeIfNotZero(wd.Vec2.UnitAxisY(), 0.001));
        WD_TEST.VEC2(v, wd.Vec2.UnitAxisY(), 0.0001);

        // GetNegated
        v.Set(1, 2);
        WD_TEST.VEC2(v.GetNegated(), new wd.Vec2(-1, -2), 0.0001);
        WD_TEST.VEC2(v, new wd.Vec2(1, 2), 0.0001);

        // Negate
        v.Negate();
        WD_TEST.VEC2(v, new wd.Vec2(-1, -2), 0.0001);

        // AddVec2
        v.Set(2, 3);
        v2.Set(5, 6);
        v.AddVec2(v2);
        WD_TEST.VEC2(v, new wd.Vec2(7, 9), 0.0001);

        // SubVec2
        v.SubVec2(v2);
        WD_TEST.VEC2(v, new wd.Vec2(2, 3), 0.0001);

        // MulVec2
        v.MulVec2(v);
        WD_TEST.VEC2(v, new wd.Vec2(4, 9), 0.0001);

        // DivVec2
        v.DivVec2(new wd.Vec2(2, 3));
        WD_TEST.VEC2(v, new wd.Vec2(2, 3), 0.0001);

        // MulNumber
        v.MulNumber(2);
        WD_TEST.VEC2(v, new wd.Vec2(4, 6), 0.0001);

        // DivNumber
        v.DivNumber(2);
        WD_TEST.VEC2(v, new wd.Vec2(2, 3), 0.0001);

        // IsIdentical
        WD_TEST.BOOL(v.IsIdentical(v));
        WD_TEST.BOOL(!v.IsIdentical(v2));

        // IsEqual
        WD_TEST.BOOL(v.IsEqual(new wd.Vec2(2, 3), 0.0001));
        WD_TEST.BOOL(!v.IsEqual(new wd.Vec2(2, 3.5), 0.0001));

        // Dot
        v.Set(2, 3);
        v2.Set(3, 4);
        WD_TEST.FLOAT(v.Dot(v2), 18, 0.001);

        // GetCompMin
        v.Set(2, 4);
        v2.Set(1, 5);
        WD_TEST.VEC2(v.GetCompMin(v2), new wd.Vec2(1, 4), 0.001);

        // GetCompMax
        v.Set(2, 4);
        v2.Set(1, 5);
        WD_TEST.VEC2(v.GetCompMax(v2), new wd.Vec2(2, 5), 0.001);

        // GetCompClamp
        WD_TEST.VEC2(v.GetCompClamp(new wd.Vec2(3, 4), new wd.Vec2(4, 5)), new wd.Vec2(3, 4), 0.001);

        // GetCompMul
        WD_TEST.VEC2(v.GetCompMul(new wd.Vec2(2, 3)), new wd.Vec2(4, 12), 0.001);

        // GetCompDiv
        WD_TEST.VEC2(v.GetCompDiv(new wd.Vec2(2, 4)), wd.Vec2.OneVector(), 0.001);

        // GetAbs
        v.Set(-1, -2);
        WD_TEST.VEC2(v.GetAbs(), new wd.Vec2(1, 2), 0.001);

        // SetAbs
        v2.SetAbs(v);
        WD_TEST.VEC2(v2, new wd.Vec2(1, 2), 0.001);

        // GetReflectedVector
        v.Set(1, 1);
        v2 = v.GetReflectedVector(new wd.Vec2(0, -1));
        WD_TEST.VEC2(v2, new wd.Vec2(1, -1), 0.0001);

        // SetAdd
        v.SetAdd(new wd.Vec2(1, 2), new wd.Vec2(4, 5));
        WD_TEST.VEC2(v, new wd.Vec2(5, 7), 0.0001);

        // SetSub
        v.SetSub(new wd.Vec2(4, 5), new wd.Vec2(1, 2));
        WD_TEST.VEC2(v, new wd.Vec2(3, 3), 0.0001);

        // SetMul
        v.SetMul(new wd.Vec2(1, 2), 2);
        WD_TEST.VEC2(v, new wd.Vec2(2, 4), 0.0001);

        // SetDiv
        v.SetDiv(new wd.Vec2(2, 4), 2);
        WD_TEST.VEC2(v, new wd.Vec2(1, 2), 0.0001);

        // CreateRandomPointInCircle
        {
            let avg = new wd.Vec2();

            const uiNumSamples = 1000;
            for (let i = 0; i < uiNumSamples; ++i) {
                v = wd.Vec2.CreateRandomPointInCircle();

                WD_TEST.BOOL(v.GetLength() <= 1.0);
                WD_TEST.BOOL(!v.IsZero());

                avg.AddVec2(v);
            }

            avg.DivNumber(uiNumSamples);

            // the average point cloud center should be within at least 10% of the sphere's center
            // otherwise the points aren't equally distributed
            WD_TEST.BOOL(avg.IsZero(0.1));
        }

        // CreateRandomDirection
        {
            let avg = new wd.Vec2();

            const uiNumSamples = 1000;
            for (let i = 0; i < uiNumSamples; ++i) {
                v = wd.Vec2.CreateRandomDirection();

                WD_TEST.BOOL(v.IsNormalized());

                avg.AddVec2(v);
            }

            avg.DivNumber(uiNumSamples);

            // the average point cloud center should be within at least 10% of the sphere's center
            // otherwise the points aren't equally distributed
            WD_TEST.BOOL(avg.IsZero(0.1));
        }
    }

    OnMsgGenericEvent(msg: wd.MsgGenericEvent): void {

        if (msg.Message == "TestVec2") {

            this.ExecuteTests();

            msg.Message = "done";
        }
    }

}

