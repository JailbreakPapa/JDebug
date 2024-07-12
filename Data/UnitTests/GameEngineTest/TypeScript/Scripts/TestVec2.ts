import ns = require("TypeScript/ns")
import NS_TEST = require("./TestFramework")

export class TestVec2 extends ns.TypescriptComponent {

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
        let d = new ns.Vec2();
        NS_TEST.FLOAT(d.x, 0, 0.001);
        NS_TEST.FLOAT(d.y, 0, 0.001);

        // ZeroVector
        NS_TEST.VEC2(new ns.Vec2(), ns.Vec2.ZeroVector(), 0.0001);

        let v = new ns.Vec2(1, 2);
        NS_TEST.FLOAT(v.x, 1, 0.001);
        NS_TEST.FLOAT(v.y, 2, 0.001);

        // Clone
        NS_TEST.VEC2(v.Clone(), v, 0.001);

        // TODO: CloneAsVec3
        //NS_TEST.VEC3(v.CloneAsVec3(3), new ns.Vec3(1, 2, 3), 0.001);

        // OneVector
        NS_TEST.VEC2(new ns.Vec2(1, 1), ns.Vec2.OneVector(), 0.0001);

        // UnitAxisX
        NS_TEST.VEC2(new ns.Vec2(1, 0), ns.Vec2.UnitAxisX(), 0.0001);

        // UnitAxisY
        NS_TEST.VEC2(new ns.Vec2(0, 1), ns.Vec2.UnitAxisY(), 0.0001);

        // Set
        v.Set(4, 5);
        NS_TEST.FLOAT(v.x, 4, 0.001);
        NS_TEST.FLOAT(v.y, 5, 0.001);

        // SetVec2
        let v2 = new ns.Vec2();
        v2.SetVec2(v);
        NS_TEST.VEC2(v, v2, 0.0001);

        // SetAll
        v2.SetAll(7);
        NS_TEST.FLOAT(v2.x, 7, 0.001);
        NS_TEST.FLOAT(v2.y, 7, 0.001);

        // SetZero
        v2.SetZero();
        NS_TEST.FLOAT(v2.x, 0, 0.001);
        NS_TEST.FLOAT(v2.y, 0, 0.001);

        // GetLengthSquared
        NS_TEST.FLOAT(v2.GetLengthSquared(), 0, 0.001);
        v2.SetAll(1);

        NS_TEST.FLOAT(v2.GetLengthSquared(), 2, 0.001);

        // GetLength
        NS_TEST.FLOAT(v2.GetLength(), Math.sqrt(2), 0.001);

        // GetLengthAndNormalize
        let l = v2.GetLengthAndNormalize();
        NS_TEST.FLOAT(l, Math.sqrt(2), 0.001);
        NS_TEST.FLOAT(v2.GetLength(), 1, 0.001);

        // IsNormalized
        NS_TEST.BOOL(!v.IsNormalized());

        // Normalize
        v.Normalize();

        NS_TEST.FLOAT(v.GetLength(), 1, 0.001);
        NS_TEST.BOOL(v.IsNormalized());


        // GetNormalized
        v.Set(3, 0);
        NS_TEST.VEC2(v.GetNormalized(), ns.Vec2.UnitAxisX(), 0.0001);

        // NormalizeIfNotZero
        NS_TEST.BOOL(v.NormalizeIfNotZero(ns.Vec2.UnitAxisY(), 0.001));
        NS_TEST.VEC2(v, ns.Vec2.UnitAxisX(), 0.0001);

        // IsZero
        NS_TEST.BOOL(!v.IsZero());

        // SetZero
        v.SetZero();
        NS_TEST.BOOL(v.IsZero());

        NS_TEST.BOOL(!v.NormalizeIfNotZero(ns.Vec2.UnitAxisY(), 0.001));
        NS_TEST.VEC2(v, ns.Vec2.UnitAxisY(), 0.0001);

        // GetNegated
        v.Set(1, 2);
        NS_TEST.VEC2(v.GetNegated(), new ns.Vec2(-1, -2), 0.0001);
        NS_TEST.VEC2(v, new ns.Vec2(1, 2), 0.0001);

        // Negate
        v.Negate();
        NS_TEST.VEC2(v, new ns.Vec2(-1, -2), 0.0001);

        // AddVec2
        v.Set(2, 3);
        v2.Set(5, 6);
        v.AddVec2(v2);
        NS_TEST.VEC2(v, new ns.Vec2(7, 9), 0.0001);

        // SubVec2
        v.SubVec2(v2);
        NS_TEST.VEC2(v, new ns.Vec2(2, 3), 0.0001);

        // MulVec2
        v.MulVec2(v);
        NS_TEST.VEC2(v, new ns.Vec2(4, 9), 0.0001);

        // DivVec2
        v.DivVec2(new ns.Vec2(2, 3));
        NS_TEST.VEC2(v, new ns.Vec2(2, 3), 0.0001);

        // MulNumber
        v.MulNumber(2);
        NS_TEST.VEC2(v, new ns.Vec2(4, 6), 0.0001);

        // DivNumber
        v.DivNumber(2);
        NS_TEST.VEC2(v, new ns.Vec2(2, 3), 0.0001);

        // IsIdentical
        NS_TEST.BOOL(v.IsIdentical(v));
        NS_TEST.BOOL(!v.IsIdentical(v2));

        // IsEqual
        NS_TEST.BOOL(v.IsEqual(new ns.Vec2(2, 3), 0.0001));
        NS_TEST.BOOL(!v.IsEqual(new ns.Vec2(2, 3.5), 0.0001));

        // Dot
        v.Set(2, 3);
        v2.Set(3, 4);
        NS_TEST.FLOAT(v.Dot(v2), 18, 0.001);

        // GetCompMin
        v.Set(2, 4);
        v2.Set(1, 5);
        NS_TEST.VEC2(v.GetCompMin(v2), new ns.Vec2(1, 4), 0.001);

        // GetCompMax
        v.Set(2, 4);
        v2.Set(1, 5);
        NS_TEST.VEC2(v.GetCompMax(v2), new ns.Vec2(2, 5), 0.001);

        // GetCompClamp
        NS_TEST.VEC2(v.GetCompClamp(new ns.Vec2(3, 4), new ns.Vec2(4, 5)), new ns.Vec2(3, 4), 0.001);

        // GetCompMul
        NS_TEST.VEC2(v.GetCompMul(new ns.Vec2(2, 3)), new ns.Vec2(4, 12), 0.001);

        // GetCompDiv
        NS_TEST.VEC2(v.GetCompDiv(new ns.Vec2(2, 4)), ns.Vec2.OneVector(), 0.001);

        // GetAbs
        v.Set(-1, -2);
        NS_TEST.VEC2(v.GetAbs(), new ns.Vec2(1, 2), 0.001);

        // SetAbs
        v2.SetAbs(v);
        NS_TEST.VEC2(v2, new ns.Vec2(1, 2), 0.001);

        // GetReflectedVector
        v.Set(1, 1);
        v2 = v.GetReflectedVector(new ns.Vec2(0, -1));
        NS_TEST.VEC2(v2, new ns.Vec2(1, -1), 0.0001);

        // SetAdd
        v.SetAdd(new ns.Vec2(1, 2), new ns.Vec2(4, 5));
        NS_TEST.VEC2(v, new ns.Vec2(5, 7), 0.0001);

        // SetSub
        v.SetSub(new ns.Vec2(4, 5), new ns.Vec2(1, 2));
        NS_TEST.VEC2(v, new ns.Vec2(3, 3), 0.0001);

        // SetMul
        v.SetMul(new ns.Vec2(1, 2), 2);
        NS_TEST.VEC2(v, new ns.Vec2(2, 4), 0.0001);

        // SetDiv
        v.SetDiv(new ns.Vec2(2, 4), 2);
        NS_TEST.VEC2(v, new ns.Vec2(1, 2), 0.0001);

        // CreateRandomPointInCircle
        {
            let avg = new ns.Vec2();

            const uiNumSamples = 1000;
            for (let i = 0; i < uiNumSamples; ++i) {
                v = ns.Vec2.CreateRandomPointInCircle();

                NS_TEST.BOOL(v.GetLength() <= 1.0);
                NS_TEST.BOOL(!v.IsZero());

                avg.AddVec2(v);
            }

            avg.DivNumber(uiNumSamples);

            // the average point cloud center should be within at least 10% of the sphere's center
            // otherwise the points aren't equally distributed
            NS_TEST.BOOL(avg.IsZero(0.1));
        }

        // CreateRandomDirection
        {
            let avg = new ns.Vec2();

            const uiNumSamples = 1000;
            for (let i = 0; i < uiNumSamples; ++i) {
                v = ns.Vec2.CreateRandomDirection();

                NS_TEST.BOOL(v.IsNormalized());

                avg.AddVec2(v);
            }

            avg.DivNumber(uiNumSamples);

            // the average point cloud center should be within at least 10% of the sphere's center
            // otherwise the points aren't equally distributed
            NS_TEST.BOOL(avg.IsZero(0.1));
        }
    }

    OnMsgGenericEvent(msg: ns.MsgGenericEvent): void {

        if (msg.Message == "TestVec2") {

            this.ExecuteTests();

            msg.Message = "done";
        }
    }

}

