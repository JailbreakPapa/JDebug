import ns = require("TypeScript/ns")
import NS_TEST = require("./TestFramework")

export class TestColor extends ns.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {
        ns.TypescriptComponent.RegisterMessageHandler(ns.MsgGenericEvent, "OnMsgGenericEvent");
    }

    ExecuteTests(): void {

        const op1 = new ns.Color(-4.0, 0.2, -7.0, -0.0);
        const op2 = new ns.Color(2.0, 0.3, 0.0, 1.0);
        const compArray = [new ns.Color(1.0, 0.0, 0.0, 0.0), new ns.Color(0.0, 1.0, 0.0, 0.0), new ns.Color(0.0, 0.0, 1.0, 0.0), new ns.Color(0.0, 0.0, 0.0, 1.0)];

        // default constructor
        {
            let c = new ns.Color();

            NS_TEST.FLOAT(c.r, 0);
            NS_TEST.FLOAT(c.g, 0);
            NS_TEST.FLOAT(c.b, 0);
            NS_TEST.FLOAT(c.a, 1);
        }

        // constructor
        {
            let c = new ns.Color(1, 2, 3, 4);

            NS_TEST.FLOAT(c.r, 1);
            NS_TEST.FLOAT(c.g, 2);
            NS_TEST.FLOAT(c.b, 3);
            NS_TEST.FLOAT(c.a, 4);
        }

        // Clone
        {
            let c0 = new ns.Color(1, 2, 3, 4);
            let c = c0.Clone();
            c0.SetLinearRGBA(2, 3, 4, 5);

            NS_TEST.FLOAT(c.r, 1);
            NS_TEST.FLOAT(c.g, 2);
            NS_TEST.FLOAT(c.b, 3);
            NS_TEST.FLOAT(c.a, 4);
        }

        // SetColor
        {
            let c0 = new ns.Color(1, 2, 3, 4);
            let c = new ns.Color();
            c.SetColor(c0);

            NS_TEST.FLOAT(c.r, 1);
            NS_TEST.FLOAT(c.g, 2);
            NS_TEST.FLOAT(c.b, 3);
            NS_TEST.FLOAT(c.a, 4);
        }

        // ZeroColor
        {
            let c = ns.Color.ZeroColor();
            NS_TEST.FLOAT(c.r, 0);
            NS_TEST.FLOAT(c.g, 0);
            NS_TEST.FLOAT(c.b, 0);
            NS_TEST.FLOAT(c.a, 0);
        }

        // SetZero
        {
            let c = new ns.Color(1, 2, 3, 4);
            c.SetZero();

            NS_TEST.FLOAT(c.r, 0);
            NS_TEST.FLOAT(c.g, 0);
            NS_TEST.FLOAT(c.b, 0);
            NS_TEST.FLOAT(c.a, 0);
        }

        // ColorByteToFloat
        {
            NS_TEST.FLOAT(ns.Color.ColorByteToFloat(0), 0.0, 0.000001);
            NS_TEST.FLOAT(ns.Color.ColorByteToFloat(128), 0.501960784, 0.000001);
            NS_TEST.FLOAT(ns.Color.ColorByteToFloat(255), 1.0, 0.000001);
        }

        // ColorFloatToByte
        {
            NS_TEST.INT(ns.Color.ColorFloatToByte(-1.0), 0);
            NS_TEST.INT(ns.Color.ColorFloatToByte(0.0), 0);
            NS_TEST.INT(ns.Color.ColorFloatToByte(0.4), 102);
            NS_TEST.INT(ns.Color.ColorFloatToByte(1.0), 255);
            NS_TEST.INT(ns.Color.ColorFloatToByte(1.5), 255);
        }

        // SetLinearRGB / SetLinearRGBA
        {
            let c1 = new ns.Color(0, 0, 0, 0);

            c1.SetLinearRGBA(1, 2, 3, 4);

            NS_TEST.COLOR(c1, new ns.Color(1, 2, 3, 4));

            c1.SetLinearRGB(5, 6, 7);

            NS_TEST.COLOR(c1, new ns.Color(5, 6, 7, 4));
        }

        // IsIdenticalRGB
        {
            let c1 = new ns.Color(0, 0, 0, 0);
            let c2 = new ns.Color(0, 0, 0, 1);

            NS_TEST.BOOL(c1.IsIdenticalRGB(c2));
            NS_TEST.BOOL(!c1.IsIdenticalRGBA(c2));
        }

        // IsIdenticalRGBA
        {
            let tmp1 = new ns.Color();
            let tmp2 = new ns.Color();

            NS_TEST.BOOL(op1.IsIdenticalRGBA(op1));
            for (let i = 0; i < 4; ++i) {
                tmp1.SetColor(compArray[i]);
                tmp1.MulNumber(0.001);
                tmp1.AddColor(op1);

                tmp2.SetColor(compArray[i]);
                tmp2.MulNumber(-0.001);
                tmp2.AddColor(op1);

                NS_TEST.BOOL(!op1.IsIdenticalRGBA(tmp1));
                NS_TEST.BOOL(!op1.IsIdenticalRGBA(tmp2));
            }
        }

        // IsEqualRGB
        {
            let c1 = new ns.Color(0, 0, 0, 0);
            let c2 = new ns.Color(0, 0, 0.2, 1);

            NS_TEST.BOOL(!c1.IsEqualRGB(c2, 0.1));
            NS_TEST.BOOL(c1.IsEqualRGB(c2, 0.3));
            NS_TEST.BOOL(!c1.IsEqualRGBA(c2, 0.3));
        }

        // IsEqualRGBA
        {
            let tmp1 = new ns.Color();
            let tmp2 = new ns.Color();

            NS_TEST.BOOL(op1.IsEqualRGBA(op1, 0.0));
            for (let i = 0; i < 4; ++i) {
                tmp1.SetColor(compArray[i]);
                tmp1.MulNumber(0.001);
                tmp1.AddColor(op1);

                tmp2.SetColor(compArray[i]);
                tmp2.MulNumber(-0.001);
                tmp2.AddColor(op1);

                NS_TEST.BOOL(op1.IsEqualRGBA(tmp1, 2 * 0.001));
                NS_TEST.BOOL(op1.IsEqualRGBA(tmp2, 2 * 0.001));
                NS_TEST.BOOL(op1.IsEqualRGBA(tmp2, 2 * 0.001));
                NS_TEST.BOOL(op1.IsEqualRGBA(tmp2, 2 * 0.001));
            }
        }

        // AddColor
        {
            let plusAssign = op1.Clone();
            plusAssign.AddColor(op2);
            NS_TEST.BOOL(plusAssign.IsEqualRGBA(new ns.Color(-2.0, 0.5, -7.0, 1.0), 0.0001));
        }

        // SubColor
        {
            let minusAssign = op1.Clone();
            minusAssign.SubColor(op2);
            NS_TEST.BOOL(minusAssign.IsEqualRGBA(new ns.Color(-6.0, -0.1, -7.0, -1.0), 0.0001));
        }

        // MulNumber
        {
            let mulFloat = op1.Clone();
            mulFloat.MulNumber(2.0);
            NS_TEST.BOOL(mulFloat.IsEqualRGBA(new ns.Color(-8.0, 0.4, -14.0, -0.0), 0.0001));
            mulFloat.MulNumber(0.0);
            NS_TEST.BOOL(mulFloat.IsEqualRGBA(new ns.Color(0.0, 0.0, 0.0, 0.0), 0.0001));
        }

        // DivNumber
        {
            let vDivFloat = op1.Clone();
            vDivFloat.DivNumber(2.0);
            NS_TEST.BOOL(vDivFloat.IsEqualRGBA(new ns.Color(-2.0, 0.1, -3.5, -0.0), 0.0001));
        }

        // SetAdd
        {
            let plus = new ns.Color();
            plus.SetAdd(op1, op2);
            NS_TEST.BOOL(plus.IsEqualRGBA(new ns.Color(-2.0, 0.5, -7.0, 1.0), 0.0001));
        }

        // SetSub
        {
            let minus = new ns.Color();
            minus.SetSub(op1, op2);
            NS_TEST.BOOL(minus.IsEqualRGBA(new ns.Color(-6.0, -0.1, -7.0, -1.0), 0.0001));
        }

        // SetMulNumber
        {
            let mulVec4Float = new ns.Color();
            mulVec4Float.SetMulNumber(op1, 2);
            NS_TEST.BOOL(mulVec4Float.IsEqualRGBA(new ns.Color(-8.0, 0.4, -14.0, -0.0), 0.0001));
            mulVec4Float.SetMulNumber(op1, 0);
            NS_TEST.BOOL(mulVec4Float.IsEqualRGBA(new ns.Color(0.0, 0.0, 0.0, 0.0), 0.0001));
        }

        // SetDivNumber
        {
            let vDivVec4Float = new ns.Color();
            vDivVec4Float.SetDivNumber(op1, 2);
            NS_TEST.BOOL(vDivVec4Float.IsEqualRGBA(new ns.Color(-2.0, 0.1, -3.5, -0.0), 0.0001));
        }

        // SetGammaByteRGB
        {
            let c = new ns.Color();
            c.SetGammaByteRGB(50, 100, 150);

            NS_TEST.FLOAT(c.r, 0.031);
            NS_TEST.FLOAT(c.g, 0.127);
            NS_TEST.FLOAT(c.b, 0.304);
            NS_TEST.FLOAT(c.a, 1.0);
        }

        // SetGammaByteRGBA
        {
            let c = new ns.Color();
            c.SetGammaByteRGBA(50, 100, 150, 127.5);

            NS_TEST.FLOAT(c.r, 0.031);
            NS_TEST.FLOAT(c.g, 0.127);
            NS_TEST.FLOAT(c.b, 0.304);
            NS_TEST.FLOAT(c.a, 0.5);
        }

        // ScaleRGB
        {
            let c = new ns.Color(1, 2, 3, 0.5);
            c.ScaleRGB(2);

            NS_TEST.FLOAT(c.r, 2);
            NS_TEST.FLOAT(c.g, 4);
            NS_TEST.FLOAT(c.b, 6);
            NS_TEST.FLOAT(c.a, 0.5);
        }

        // MulColor
        {
            let c = new ns.Color(2, 3, 4, 6);
            let m = new ns.Color(5, 3, 2, 0.5);

            c.MulColor(m);

            NS_TEST.FLOAT(c.r, 10);
            NS_TEST.FLOAT(c.g, 9);
            NS_TEST.FLOAT(c.b, 8);
            NS_TEST.FLOAT(c.a, 3);
        }

        // SetMul
        {
            let n = new ns.Color(2, 3, 4, 6);
            let m = new ns.Color(5, 3, 2, 0.5);

            let c = new ns.Color();
            c.SetMul(n, m);

            NS_TEST.FLOAT(c.r, 10);
            NS_TEST.FLOAT(c.g, 9);
            NS_TEST.FLOAT(c.b, 8);
            NS_TEST.FLOAT(c.a, 3);
        }

        // WithAlpha
        {
            let o = new ns.Color(2, 3, 4, 6);

            let c = o.WithAlpha(0.5);

            NS_TEST.FLOAT(c.r, 2);
            NS_TEST.FLOAT(c.g, 3);
            NS_TEST.FLOAT(c.b, 4);
            NS_TEST.FLOAT(c.a, 0.5);
        }

        // CalcAverageRGB
        {
            let c = new ns.Color(1, 1, 1, 2);
            NS_TEST.FLOAT(c.CalcAverageRGB(), 1.0);
        }

        // IsNormalized
        {
            let c = new ns.Color(1, 1, 1, 1);
            NS_TEST.BOOL(c.IsNormalized());

            c.a = 2.0;
            NS_TEST.BOOL(!c.IsNormalized());
        }

        // GetLuminance
        {
            NS_TEST.FLOAT(ns.Color.Black().GetLuminance(), 0.0);
            NS_TEST.FLOAT(ns.Color.White().GetLuminance(), 1.0);

            NS_TEST.FLOAT(new ns.Color(0.5, 0.5, 0.5).GetLuminance(), 0.2126 * 0.5 + 0.7152 * 0.5 + 0.0722 * 0.5);
        }

        // GetInvertedColor
        {
            let c1 = new ns.Color(0.1, 0.3, 0.7, 0.9);

            let c2 = c1.GetInvertedColor();

            NS_TEST.BOOL(c2.IsEqualRGBA(new ns.Color(0.9, 0.7, 0.3, 0.1), 0.01));
        }

        // ComputeHdrExposureValue
        {
            let c = new ns.Color();
            NS_TEST.FLOAT(c.ComputeHdrExposureValue(), 0);

            c.SetLinearRGB(2, 3, 4);
            NS_TEST.FLOAT(c.ComputeHdrExposureValue(), 2);
        }

        // ApplyHdrExposureValue
        {
            let c = new ns.Color(1, 1, 1, 1);
            c.ApplyHdrExposureValue(2);

            NS_TEST.FLOAT(c.ComputeHdrExposureValue(), 2);
        }

        // GetAsGammaByteRGBA
        {
            let c = new ns.Color();
            c.SetGammaByteRGBA(50, 100, 150, 200);

            let g = c.GetAsGammaByteRGBA();

            NS_TEST.FLOAT(g.byteR, 50);
            NS_TEST.FLOAT(g.byteG, 100);
            NS_TEST.FLOAT(g.byteB, 150);
            NS_TEST.FLOAT(g.byteA, 200);
        }
    }

    OnMsgGenericEvent(msg: ns.MsgGenericEvent): void {

        if (msg.Message == "TestColor") {

            this.ExecuteTests();

            msg.Message = "done";
        }
    }

}

