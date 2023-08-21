import wd = require("TypeScript/wd")
import WD_TEST = require("./TestFramework")
import prefab = require("./Prefab")

export class TestUtils extends wd.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {
        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgGenericEvent, "OnMsgGenericEvent");
    }

    ExecuteTests(): void {

        // IsNumberEqual
        {
            WD_TEST.BOOL(wd.Utils.IsNumberEqual(13, 14, 0.9) == false);
            WD_TEST.BOOL(wd.Utils.IsNumberEqual(13, 14, 1.01) == true);
        }

        // IsNumberZero
        {
            WD_TEST.BOOL(wd.Utils.IsNumberZero(0.1, 0.09) == false);
            WD_TEST.BOOL(wd.Utils.IsNumberZero(0.1, 0.11) == true);

            WD_TEST.BOOL(wd.Utils.IsNumberZero(-0.1, 0.09) == false);
            WD_TEST.BOOL(wd.Utils.IsNumberZero(-0.1, 0.11) == true);
        }

        // StringToHash
        {
            WD_TEST.BOOL(wd.Utils.StringToHash("a") != wd.Utils.StringToHash("b"));
        }

        // Clamp
        {
            WD_TEST.INT(wd.Utils.Clamp(13, 8, 11), 11);
            WD_TEST.INT(wd.Utils.Clamp(6, 8, 11), 8);
            WD_TEST.INT(wd.Utils.Clamp(9, 8, 11), 9);
        }

        // Saturate
        {
            WD_TEST.FLOAT(wd.Utils.Saturate(-0.7), 0, 0.001);
            WD_TEST.FLOAT(wd.Utils.Saturate(0.3), 0.3, 0.001);
            WD_TEST.FLOAT(wd.Utils.Saturate(1.3), 1.0, 0.001);
        }

        // FindPrefabRootNode / FindPrefabRootScript / Exposed Script Parameters
        {
            let p1 = this.GetOwner().FindChildByName("Prefab1");
            let p2 = this.GetOwner().FindChildByName("Prefab2");

            WD_TEST.BOOL(p1 != null);
            WD_TEST.BOOL(p2 != null);

            {
                let p1r = wd.Utils.FindPrefabRootNode(p1);
                let p1s: prefab.Prefab = wd.Utils.FindPrefabRootScript(p1, "Prefab");

                WD_TEST.BOOL(p1r != null);
                WD_TEST.BOOL(p1r.GetName() == "root");

                WD_TEST.BOOL(p1s != null);
                WD_TEST.FLOAT(p1s.NumberVar, 11, 0.001);
                WD_TEST.BOOL(p1s.BoolVar);
                WD_TEST.BOOL(p1s.StringVar == "Hello");
                WD_TEST.BOOL(p1s.Vec3Var.IsEqual(new wd.Vec3(1, 2, 3)));

                let c = new wd.Color();
                c.SetGammaByteRGBA(227, 106, 6, 255);
                WD_TEST.BOOL(p1s.ColorVar.IsEqualRGBA(c));
            }

            {
                let p2r = wd.Utils.FindPrefabRootNode(p2);
                let p2s: prefab.Prefab = wd.Utils.FindPrefabRootScript(p2, "Prefab");

                WD_TEST.BOOL(p2r != null);
                WD_TEST.BOOL(p2r.GetName() == "root");

                WD_TEST.BOOL(p2s != null);
                WD_TEST.FLOAT(p2s.NumberVar, 2, 0.001);
                WD_TEST.BOOL(p2s.BoolVar == false);
                WD_TEST.BOOL(p2s.StringVar == "Bye");
                WD_TEST.BOOL(p2s.Vec3Var.IsEqual(new wd.Vec3(4, 5, 6)));

                let c = new wd.Color();
                c.SetGammaByteRGBA(6, 164, 227, 255);
                WD_TEST.BOOL(p2s.ColorVar.IsEqualRGBA(c));
            }
        }
    }

    OnMsgGenericEvent(msg: wd.MsgGenericEvent): void {

        if (msg.Message == "TestUtils") {

            this.ExecuteTests();
            msg.Message = "done";
        }
    }

}

