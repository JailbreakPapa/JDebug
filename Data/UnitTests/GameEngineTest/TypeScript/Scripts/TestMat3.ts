import ns = require("TypeScript/ns")
import NS_TEST = require("./TestFramework")

function mul(m: ns.Mat3, v: ns.Vec3): ns.Vec3 {
    let r = v.Clone();
    m.TransformDirection(r);
    return r;
}

export class TestMat3 extends ns.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {
        ns.TypescriptComponent.RegisterMessageHandler(ns.MsgGenericEvent, "OnMsgGenericEvent");
    }

    ExecuteTests(): void {

        // Constructor (default)
        {
            let m = new ns.Mat3();

            NS_TEST.FLOAT(m.GetElement(0, 0), 1, 0.001);
            NS_TEST.FLOAT(m.GetElement(1, 0), 0, 0.001);
            NS_TEST.FLOAT(m.GetElement(2, 0), 0, 0.001);
            NS_TEST.FLOAT(m.GetElement(0, 1), 0, 0.001);
            NS_TEST.FLOAT(m.GetElement(1, 1), 1, 0.001);
            NS_TEST.FLOAT(m.GetElement(2, 1), 0, 0.001);
            NS_TEST.FLOAT(m.GetElement(0, 2), 0, 0.001);
            NS_TEST.FLOAT(m.GetElement(1, 2), 0, 0.001);
            NS_TEST.FLOAT(m.GetElement(2, 2), 1, 0.001);

            NS_TEST.BOOL(m.IsIdentity());
        }

        // Constructor (Elements)
        {
            let m = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);

            NS_TEST.FLOAT(m.GetElement(0, 0), 1, 0.001);
            NS_TEST.FLOAT(m.GetElement(1, 0), 2, 0.001);
            NS_TEST.FLOAT(m.GetElement(2, 0), 3, 0.001);
            NS_TEST.FLOAT(m.GetElement(0, 1), 4, 0.001);
            NS_TEST.FLOAT(m.GetElement(1, 1), 5, 0.001);
            NS_TEST.FLOAT(m.GetElement(2, 1), 6, 0.001);
            NS_TEST.FLOAT(m.GetElement(0, 2), 7, 0.001);
            NS_TEST.FLOAT(m.GetElement(1, 2), 8, 0.001);
            NS_TEST.FLOAT(m.GetElement(2, 2), 9, 0.001);
        }

        // Clone
        {
            let m0 = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);
            let m = m0.Clone();

            NS_TEST.FLOAT(m.GetElement(0, 0), 1, 0.001);
            NS_TEST.FLOAT(m.GetElement(1, 0), 2, 0.001);
            NS_TEST.FLOAT(m.GetElement(2, 0), 3, 0.001);
            NS_TEST.FLOAT(m.GetElement(0, 1), 4, 0.001);
            NS_TEST.FLOAT(m.GetElement(1, 1), 5, 0.001);
            NS_TEST.FLOAT(m.GetElement(2, 1), 6, 0.001);
            NS_TEST.FLOAT(m.GetElement(0, 2), 7, 0.001);
            NS_TEST.FLOAT(m.GetElement(1, 2), 8, 0.001);
            NS_TEST.FLOAT(m.GetElement(2, 2), 9, 0.001);
        }

        // SetMat3
        {
            let m0 = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);
            let m = new ns.Mat3();
            m.SetMat3(m0);

            NS_TEST.FLOAT(m.GetElement(0, 0), 1, 0.001);
            NS_TEST.FLOAT(m.GetElement(1, 0), 2, 0.001);
            NS_TEST.FLOAT(m.GetElement(2, 0), 3, 0.001);
            NS_TEST.FLOAT(m.GetElement(0, 1), 4, 0.001);
            NS_TEST.FLOAT(m.GetElement(1, 1), 5, 0.001);
            NS_TEST.FLOAT(m.GetElement(2, 1), 6, 0.001);
            NS_TEST.FLOAT(m.GetElement(0, 2), 7, 0.001);
            NS_TEST.FLOAT(m.GetElement(1, 2), 8, 0.001);
            NS_TEST.FLOAT(m.GetElement(2, 2), 9, 0.001);
        }

        // SetElement
        {
            let m = ns.Mat3.ZeroMatrix();

            m.SetElement(0, 0, 1);
            m.SetElement(1, 0, 2);
            m.SetElement(2, 0, 3);
            m.SetElement(0, 1, 4);
            m.SetElement(1, 1, 5);
            m.SetElement(2, 1, 6);
            m.SetElement(0, 2, 7);
            m.SetElement(1, 2, 8);
            m.SetElement(2, 2, 9);

            let m0 = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);
            NS_TEST.BOOL(m.IsIdentical(m0));
        }

        // SetElements
        {
            let m = ns.Mat3.ZeroMatrix();
            m.SetElements(1, 2, 3, 4, 5, 6, 7, 8, 9);

            let m0 = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);
            NS_TEST.BOOL(m.IsIdentical(m0));
        }

        // SetFromArray
        {
            const data = [1, 2, 3, 4, 5, 6, 7, 8, 9];

            {
                let m = new ns.Mat3();
                m.SetFromArray(data, true);

                NS_TEST.BOOL(m.m_ElementsCM[0] == 1.0 && m.m_ElementsCM[1] == 2.0 && m.m_ElementsCM[2] == 3.0 && m.m_ElementsCM[3] == 4.0 &&
                    m.m_ElementsCM[4] == 5.0 && m.m_ElementsCM[5] == 6.0 && m.m_ElementsCM[6] == 7.0 && m.m_ElementsCM[7] == 8.0 &&
                    m.m_ElementsCM[8] == 9.0);
            }

            {
                let m = new ns.Mat3();
                m.SetFromArray(data, false);

                NS_TEST.BOOL(m.m_ElementsCM[0] == 1.0 && m.m_ElementsCM[1] == 4.0 && m.m_ElementsCM[2] == 7.0 && m.m_ElementsCM[3] == 2.0 &&
                    m.m_ElementsCM[4] == 5.0 && m.m_ElementsCM[5] == 8.0 && m.m_ElementsCM[6] == 3.0 && m.m_ElementsCM[7] == 6.0 &&
                    m.m_ElementsCM[8] == 9.0);
            }
        }

        // GetAsArray
        {
            let m = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);

            let data = m.GetAsArray(true);
            NS_TEST.FLOAT(data[0], 1, 0.0001);
            NS_TEST.FLOAT(data[1], 4, 0.0001);
            NS_TEST.FLOAT(data[2], 7, 0.0001);
            NS_TEST.FLOAT(data[3], 2, 0.0001);
            NS_TEST.FLOAT(data[4], 5, 0.0001);
            NS_TEST.FLOAT(data[5], 8, 0.0001);
            NS_TEST.FLOAT(data[6], 3, 0.0001);
            NS_TEST.FLOAT(data[7], 6, 0.0001);
            NS_TEST.FLOAT(data[8], 9, 0.0001);

            data = m.GetAsArray(false);
            NS_TEST.FLOAT(data[0], 1, 0.0001);
            NS_TEST.FLOAT(data[1], 2, 0.0001);
            NS_TEST.FLOAT(data[2], 3, 0.0001);
            NS_TEST.FLOAT(data[3], 4, 0.0001);
            NS_TEST.FLOAT(data[4], 5, 0.0001);
            NS_TEST.FLOAT(data[5], 6, 0.0001);
            NS_TEST.FLOAT(data[6], 7, 0.0001);
            NS_TEST.FLOAT(data[7], 8, 0.0001);
            NS_TEST.FLOAT(data[8], 9, 0.0001);
        }

        // SetZero
        {
            let m = new ns.Mat3();
            m.SetZero();

            for (let i = 0; i < 9; ++i)
                NS_TEST.FLOAT(m.m_ElementsCM[i], 0.0, 0.0);
        }

        // SetIdentity
        {
            let m = new ns.Mat3();
            m.SetIdentity();

            NS_TEST.FLOAT(m.GetElement(0, 0), 1, 0);
            NS_TEST.FLOAT(m.GetElement(1, 0), 0, 0);
            NS_TEST.FLOAT(m.GetElement(2, 0), 0, 0);
            NS_TEST.FLOAT(m.GetElement(0, 1), 0, 0);
            NS_TEST.FLOAT(m.GetElement(1, 1), 1, 0);
            NS_TEST.FLOAT(m.GetElement(2, 1), 0, 0);
            NS_TEST.FLOAT(m.GetElement(0, 2), 0, 0);
            NS_TEST.FLOAT(m.GetElement(1, 2), 0, 0);
            NS_TEST.FLOAT(m.GetElement(2, 2), 1, 0);
        }

        // SetScalingMatrix
        {
            let m = new ns.Mat3();
            m.SetScalingMatrix(new ns.Vec3(2, 3, 4));

            NS_TEST.FLOAT(m.GetElement(0, 0), 2, 0);
            NS_TEST.FLOAT(m.GetElement(1, 0), 0, 0);
            NS_TEST.FLOAT(m.GetElement(2, 0), 0, 0);
            NS_TEST.FLOAT(m.GetElement(0, 1), 0, 0);
            NS_TEST.FLOAT(m.GetElement(1, 1), 3, 0);
            NS_TEST.FLOAT(m.GetElement(2, 1), 0, 0);
            NS_TEST.FLOAT(m.GetElement(0, 2), 0, 0);
            NS_TEST.FLOAT(m.GetElement(1, 2), 0, 0);
            NS_TEST.FLOAT(m.GetElement(2, 2), 4, 0);
        }

        // SetRotationMatrixX
        {
            let m = new ns.Mat3();

            m.SetRotationMatrixX(ns.Angle.DegreeToRadian(90));

            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(1, -3, 2), 0.0001));

            m.SetRotationMatrixX(ns.Angle.DegreeToRadian(180));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(1, -2, -3), 0.0001));

            m.SetRotationMatrixX(ns.Angle.DegreeToRadian(270));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(1, 3, -2), 0.0001));

            m.SetRotationMatrixX(ns.Angle.DegreeToRadian(360));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(1, 2, 3), 0.0001));
        }

        // SetRotationMatrixY
        {
            let m = new ns.Mat3();

            m.SetRotationMatrixY(ns.Angle.DegreeToRadian(90));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(3, 2, -1), 0.0001));

            m.SetRotationMatrixY(ns.Angle.DegreeToRadian(180));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(-1, 2, -3), 0.0001));

            m.SetRotationMatrixY(ns.Angle.DegreeToRadian(270));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(-3, 2, 1), 0.0001));

            m.SetRotationMatrixY(ns.Angle.DegreeToRadian(360));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(1, 2, 3), 0.0001));
        }

        // SetRotationMatrixZ
        {
            let m = new ns.Mat3();

            m.SetRotationMatrixZ(ns.Angle.DegreeToRadian(90));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(-2, 1, 3), 0.0001));

            m.SetRotationMatrixZ(ns.Angle.DegreeToRadian(180));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(-1, -2, 3), 0.0001));

            m.SetRotationMatrixZ(ns.Angle.DegreeToRadian(270));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(2, -1, 3), 0.0001));

            m.SetRotationMatrixZ(ns.Angle.DegreeToRadian(360));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(1, 2, 3), 0.0001));
        }

        // SetRotationMatrix
        {
            let m = new ns.Mat3();

            m.SetRotationMatrix(new ns.Vec3(1, 0, 0), ns.Angle.DegreeToRadian(90));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(1, -3, 2), 0.001));

            m.SetRotationMatrix(new ns.Vec3(1, 0, 0), ns.Angle.DegreeToRadian(180));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(1, -2, -3), 0.001));

            m.SetRotationMatrix(new ns.Vec3(1, 0, 0), ns.Angle.DegreeToRadian(270));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(1, 3, -2), 0.001));

            m.SetRotationMatrix(new ns.Vec3(0, 1, 0), ns.Angle.DegreeToRadian(90));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(3, 2, -1), 0.001));

            m.SetRotationMatrix(new ns.Vec3(0, 1, 0), ns.Angle.DegreeToRadian(180));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(-1, 2, -3), 0.001));

            m.SetRotationMatrix(new ns.Vec3(0, 1, 0), ns.Angle.DegreeToRadian(270));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(-3, 2, 1), 0.001));

            m.SetRotationMatrix(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(90));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(-2, 1, 3), 0.001));

            m.SetRotationMatrix(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(180));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(-1, -2, 3), 0.001));

            m.SetRotationMatrix(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(270));
            NS_TEST.BOOL(mul(m, new ns.Vec3(1, 2, 3)).IsEqual(new ns.Vec3(2, -1, 3), 0.001));
        }

        // IdentityMatrix
        {
            let m = ns.Mat3.IdentityMatrix();

            NS_TEST.FLOAT(m.GetElement(0, 0), 1, 0);
            NS_TEST.FLOAT(m.GetElement(1, 0), 0, 0);
            NS_TEST.FLOAT(m.GetElement(2, 0), 0, 0);
            NS_TEST.FLOAT(m.GetElement(0, 1), 0, 0);
            NS_TEST.FLOAT(m.GetElement(1, 1), 1, 0);
            NS_TEST.FLOAT(m.GetElement(2, 1), 0, 0);
            NS_TEST.FLOAT(m.GetElement(0, 2), 0, 0);
            NS_TEST.FLOAT(m.GetElement(1, 2), 0, 0);
            NS_TEST.FLOAT(m.GetElement(2, 2), 1, 0);
        }

        // ZeroMatrix
        {
            let m = ns.Mat3.ZeroMatrix();

            NS_TEST.FLOAT(m.GetElement(0, 0), 0, 0);
            NS_TEST.FLOAT(m.GetElement(1, 0), 0, 0);
            NS_TEST.FLOAT(m.GetElement(2, 0), 0, 0);
            NS_TEST.FLOAT(m.GetElement(0, 1), 0, 0);
            NS_TEST.FLOAT(m.GetElement(1, 1), 0, 0);
            NS_TEST.FLOAT(m.GetElement(2, 1), 0, 0);
            NS_TEST.FLOAT(m.GetElement(0, 2), 0, 0);
            NS_TEST.FLOAT(m.GetElement(1, 2), 0, 0);
            NS_TEST.FLOAT(m.GetElement(2, 2), 0, 0);
        }

        // Transpose
        {
            let m = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);

            m.Transpose();

            NS_TEST.FLOAT(m.GetElement(0, 0), 1, 0);
            NS_TEST.FLOAT(m.GetElement(1, 0), 4, 0);
            NS_TEST.FLOAT(m.GetElement(2, 0), 7, 0);
            NS_TEST.FLOAT(m.GetElement(0, 1), 2, 0);
            NS_TEST.FLOAT(m.GetElement(1, 1), 5, 0);
            NS_TEST.FLOAT(m.GetElement(2, 1), 8, 0);
            NS_TEST.FLOAT(m.GetElement(0, 2), 3, 0);
            NS_TEST.FLOAT(m.GetElement(1, 2), 6, 0);
            NS_TEST.FLOAT(m.GetElement(2, 2), 9, 0);
        }

        // GetTranspose
        {
            let m0 = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);

            let m = m0.GetTranspose();

            NS_TEST.FLOAT(m.GetElement(0, 0), 1);
            NS_TEST.FLOAT(m.GetElement(1, 0), 4);
            NS_TEST.FLOAT(m.GetElement(2, 0), 7);
            NS_TEST.FLOAT(m.GetElement(0, 1), 2);
            NS_TEST.FLOAT(m.GetElement(1, 1), 5);
            NS_TEST.FLOAT(m.GetElement(2, 1), 8);
            NS_TEST.FLOAT(m.GetElement(0, 2), 3);
            NS_TEST.FLOAT(m.GetElement(1, 2), 6);
            NS_TEST.FLOAT(m.GetElement(2, 2), 9);
        }

        // Invert
        {
            for (let x = 1.0; x < 360.0; x += 40.0) {
                for (let y = 2.0; y < 360.0; y += 37.0) {
                    for (let z = 3.0; z < 360.0; z += 53.0) {
                        let m = new ns.Mat3();

                        m.SetRotationMatrix(new ns.Vec3(x, y, z).GetNormalized(), ns.Angle.DegreeToRadian(19.0));
                        let inv = m.Clone();
                        NS_TEST.BOOL(inv.Invert());

                        let v = mul(m, new ns.Vec3(1, 1, 1));
                        let vinv = mul(inv, v);

                        NS_TEST.VEC3(vinv, new ns.Vec3(1, 1, 1), 0.001);
                    }
                }
            }
        }

        // GetInverse
        {
            for (let x = 1.0; x < 360.0; x += 39.0) {
                for (let y = 2.0; y < 360.0; y += 29.0) {
                    for (let z = 3.0; z < 360.0; z += 51.0) {
                        let m = new ns.Mat3();

                        m.SetRotationMatrix(new ns.Vec3(x, y, z).GetNormalized(), ns.Angle.DegreeToRadian(83.0));
                        let inv = m.GetInverse();

                        let v = mul(m, new ns.Vec3(1, 1, 1));
                        let vinv = mul(inv, v);

                        NS_TEST.VEC3(vinv, new ns.Vec3(1, 1, 1), 0.001);
                    }
                }
            }
        }

        // IsZero
        {
            let m = new ns.Mat3();

            m.SetIdentity();
            NS_TEST.BOOL(!m.IsZero());

            m.SetZero();
            NS_TEST.BOOL(m.IsZero());
        }

        // IsIdentity
        {
            let m = new ns.Mat3();

            m.SetIdentity();
            NS_TEST.BOOL(m.IsIdentity());

            m.SetZero();
            NS_TEST.BOOL(!m.IsIdentity());
        }

        // GetRow
        {
            let m = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);

            NS_TEST.ARRAY(3, m.GetRow(0), [1, 2, 3], 0.0);
            NS_TEST.ARRAY(3, m.GetRow(1), [4, 5, 6], 0.0);
            NS_TEST.ARRAY(3, m.GetRow(2), [7, 8, 9], 0.0);
        }

        // SetRow
        {
            let m = new ns.Mat3();
            m.SetZero();

            m.SetRow(0, 1, 2, 3);
            NS_TEST.ARRAY(3, m.GetRow(0), [1, 2, 3], 0.0);

            m.SetRow(1, 5, 6, 7);
            NS_TEST.ARRAY(3, m.GetRow(1), [5, 6, 7], 0.0);

            m.SetRow(2, 9, 10, 11);
            NS_TEST.ARRAY(3, m.GetRow(2), [9, 10, 11], 0.0);

            m.SetRow(3, 13, 14, 15);
            NS_TEST.ARRAY(3, m.GetRow(3), [13, 14, 15], 0.0);
        }

        // GetColumn
        {
            let m = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);

            NS_TEST.ARRAY(3, m.GetColumn(0), [1, 4, 7], 0.0);
            NS_TEST.ARRAY(3, m.GetColumn(1), [2, 5, 8], 0.0);
            NS_TEST.ARRAY(3, m.GetColumn(2), [3, 6, 9], 0.0);
        }

        // SetColumn
        {
            let m = new ns.Mat3();
            m.SetZero();

            m.SetColumn(0, 1, 2, 3);
            NS_TEST.ARRAY(3, m.GetColumn(0), [1, 2, 3], 0.0);

            m.SetColumn(1, 5, 6, 7);
            NS_TEST.ARRAY(3, m.GetColumn(1), [5, 6, 7], 0.0);

            m.SetColumn(2, 9, 10, 11);
            NS_TEST.ARRAY(3, m.GetColumn(2), [9, 10, 11], 0.0);

            m.SetColumn(3, 13, 14, 15);
            NS_TEST.ARRAY(3, m.GetColumn(3), [13, 14, 15], 0.0);
        }

        // GetDiagonal
        {
            let m = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);

            NS_TEST.ARRAY(3, m.GetDiagonal(), [1, 5, 9], 0.0);
        }

        // SetDiagonal
        {
            let m = new ns.Mat3();
            m.SetZero();

            m.SetDiagonal(1, 2, 3);
            NS_TEST.ARRAY(3, m.GetColumn(0), [1, 0, 0], 0.0);
            NS_TEST.ARRAY(3, m.GetColumn(1), [0, 2, 0], 0.0);
            NS_TEST.ARRAY(3, m.GetColumn(2), [0, 0, 3], 0.0);
        }

        // GetScalingFactors
        {
            let m = new ns.Mat3(1, 2, 3, 5, 6, 7, 9, 10, 11);

            let s = m.GetScalingFactors();
            NS_TEST.VEC3(s,
                new ns.Vec3(Math.sqrt((1 * 1 + 5 * 5 + 9 * 9)), Math.sqrt((2 * 2 + 6 * 6 + 10 * 10)),
                    Math.sqrt((3 * 3 + 7 * 7 + 11 * 11))),
                0.0001);
        }

        // SetScalingFactors
        {
            let m = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);

            NS_TEST.BOOL(m.SetScalingFactors(1, 2, 3));

            let s = m.GetScalingFactors();
            NS_TEST.VEC3(s, new ns.Vec3(1, 2, 3), 0.0001);
        }

        // TransformDirection
        {
            let m = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);

            let r = new ns.Vec3(1, 2, 3);
            m.TransformDirection(r);

            NS_TEST.VEC3(r, new ns.Vec3(1 * 1 + 2 * 2 + 3 * 3, 1 * 4 + 2 * 5 + 3 * 6, 1 * 7 + 2 * 8 + 3 * 9), 0.0001);
        }

        // IsIdentical
        {
            let m = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);

            let m2 = m.Clone();

            NS_TEST.BOOL(m.IsIdentical(m2));

            m2.m_ElementsCM[0] += 0.001;
            NS_TEST.BOOL(!m.IsIdentical(m2));
        }

        // IsEqual
        {
            let m = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);

            let m2 = m.Clone();

            NS_TEST.BOOL(m.IsEqual(m2, 0.0001));

            m2.m_ElementsCM[0] += 0.001;
            NS_TEST.BOOL(m.IsEqual(m2, 0.001));
            NS_TEST.BOOL(!m.IsEqual(m2, 0.0001));
        }

        // SetMulMat3
        {
            let m1 = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);

            let m2 = new ns.Mat3(-1, -2, -3, -4, -5, -6, -7, -8, -9);

            let r = new ns.Mat3();
            r.SetMulMat3(m1, m2);

            NS_TEST.ARRAY(3, r.GetColumn(0),
                [-1 * 1 + -4 * 2 + -7 * 3, -1 * 4 + -4 * 5 + -7 * 6, -1 * 7 + -4 * 8 + -7 * 9],
                0.001);
            NS_TEST.ARRAY(3, r.GetColumn(1),
                [-2 * 1 + -5 * 2 + -8 * 3, -2 * 4 + -5 * 5 + -8 * 6, -2 * 7 + -5 * 8 + -8 * 9],
                0.001);
            NS_TEST.ARRAY(3, r.GetColumn(2),
                [-3 * 1 + -6 * 2 + -9 * 3, -3 * 4 + -6 * 5 + -9 * 6, -3 * 7 + -6 * 8 + -9 * 9],
                0.001);
        }

        // MulNumber
        {
            let m0 = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);

            let m = m0.Clone();
            m.MulNumber(2);

            NS_TEST.ARRAY(3, m.GetRow(0), [2, 4, 6], 0.0001);
            NS_TEST.ARRAY(3, m.GetRow(1), [8, 10, 12], 0.0001);
            NS_TEST.ARRAY(3, m.GetRow(2), [14, 16, 18], 0.0001);
        }

        // DivNumber
        {
            let m0 = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);

            m0.MulNumber(4);

            let m = m0.Clone();
            m.DivNumber(2);

            NS_TEST.ARRAY(3, m.GetRow(0), [2, 4, 6], 0.0001);
            NS_TEST.ARRAY(3, m.GetRow(1), [8, 10, 12], 0.0001);
            NS_TEST.ARRAY(3, m.GetRow(2), [14, 16, 18], 0.0001);
        }

        // AddMat3 / SubMat3
        {
            let m0 = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);

            let m1 = new ns.Mat3(-1, -2, -3, -4, -5, -6, -7, -8, -9);

            let r1 = m0.Clone();
            r1.AddMat3(m1);

            let r2 = m0.Clone();
            r2.SubMat3(m1);

            let c2 = m0.Clone();
            c2.MulNumber(2);

            NS_TEST.BOOL(r1.IsZero());
            NS_TEST.BOOL(r2.IsEqual(c2, 0.0001));
        }

        // IsIdentical
        {
            let m = new ns.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);

            let m2 = m.Clone();

            NS_TEST.BOOL(m.IsIdentical(m2));

            m2.m_ElementsCM[0] += 0.001;

            NS_TEST.BOOL(!m.IsIdentical(m2));
        }
    }

    OnMsgGenericEvent(msg: ns.MsgGenericEvent): void {

        if (msg.Message == "TestMat3") {

            this.ExecuteTests();

            msg.Message = "done";
        }
    }
}

