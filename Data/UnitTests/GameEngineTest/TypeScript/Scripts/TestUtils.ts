import ns = require("TypeScript/ns")
import NS_TEST = require("./TestFramework")
import prefab = require("./Prefab")

export class TestUtils extends ns.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {
        ns.TypescriptComponent.RegisterMessageHandler(ns.MsgGenericEvent, "OnMsgGenericEvent");
    }

    ExecuteTests(): void {

        // IsNumberEqual
        {
            NS_TEST.BOOL(ns.Utils.IsNumberEqual(13, 14, 0.9) == false);
            NS_TEST.BOOL(ns.Utils.IsNumberEqual(13, 14, 1.01) == true);
        }

        // IsNumberZero
        {
            NS_TEST.BOOL(ns.Utils.IsNumberZero(0.1, 0.09) == false);
            NS_TEST.BOOL(ns.Utils.IsNumberZero(0.1, 0.11) == true);

            NS_TEST.BOOL(ns.Utils.IsNumberZero(-0.1, 0.09) == false);
            NS_TEST.BOOL(ns.Utils.IsNumberZero(-0.1, 0.11) == true);
        }

        // StringToHash
        {
            NS_TEST.BOOL(ns.Utils.StringToHash("a") != ns.Utils.StringToHash("b"));
        }

        // Clamp
        {
            NS_TEST.INT(ns.Utils.Clamp(13, 8, 11), 11);
            NS_TEST.INT(ns.Utils.Clamp(6, 8, 11), 8);
            NS_TEST.INT(ns.Utils.Clamp(9, 8, 11), 9);
        }

        // Saturate
        {
            NS_TEST.FLOAT(ns.Utils.Saturate(-0.7), 0, 0.001);
            NS_TEST.FLOAT(ns.Utils.Saturate(0.3), 0.3, 0.001);
            NS_TEST.FLOAT(ns.Utils.Saturate(1.3), 1.0, 0.001);
        }

        // FindPrefabRootNode / FindPrefabRootScript / Exposed Script Parameters
        {
            let p1 = this.GetOwner().FindChildByName("Prefab1");
            let p2 = this.GetOwner().FindChildByName("Prefab2");

            NS_TEST.BOOL(p1 != null);
            NS_TEST.BOOL(p2 != null);

            {
                let p1r = ns.Utils.FindPrefabRootNode(p1);
                let p1s: prefab.Prefab = ns.Utils.FindPrefabRootScript(p1, "Prefab");

                NS_TEST.BOOL(p1r != null);
                NS_TEST.BOOL(p1r.GetName() == "root");

                NS_TEST.BOOL(p1s != null);
                NS_TEST.FLOAT(p1s.NumberVar, 11, 0.001);
                NS_TEST.BOOL(p1s.BoolVar);
                NS_TEST.BOOL(p1s.StringVar == "Hello");
                NS_TEST.BOOL(p1s.Vec3Var.IsEqual(new ns.Vec3(1, 2, 3)));

                let c = new ns.Color();
                c.SetGammaByteRGBA(227, 106, 6, 255);
                NS_TEST.BOOL(p1s.ColorVar.IsEqualRGBA(c));
            }

            {
                let p2r = ns.Utils.FindPrefabRootNode(p2);
                let p2s: prefab.Prefab = ns.Utils.FindPrefabRootScript(p2, "Prefab");

                NS_TEST.BOOL(p2r != null);
                NS_TEST.BOOL(p2r.GetName() == "root");

                NS_TEST.BOOL(p2s != null);
                NS_TEST.FLOAT(p2s.NumberVar, 2, 0.001);
                NS_TEST.BOOL(p2s.BoolVar == false);
                NS_TEST.BOOL(p2s.StringVar == "Bye");
                NS_TEST.BOOL(p2s.Vec3Var.IsEqual(new ns.Vec3(4, 5, 6)));

                let c = new ns.Color();
                c.SetGammaByteRGBA(6, 164, 227, 255);
                NS_TEST.BOOL(p2s.ColorVar.IsEqualRGBA(c));
            }
        }
    }

    OnMsgGenericEvent(msg: ns.MsgGenericEvent): void {

        if (msg.Message == "TestUtils") {

            this.ExecuteTests();
            msg.Message = "done";
        }
    }

}

