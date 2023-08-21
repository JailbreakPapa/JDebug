import wd = require("TypeScript/wd")
import WD_TEST = require("./TestFramework")

export class TestWorld extends wd.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {
        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgGenericEvent, "OnMsgGenericEvent");
    }

    foundObjs: wd.GameObject[] = [];

    FoundObjectCallback = (go: wd.GameObject): boolean => {

        this.foundObjs.push(go);
        return true;
    }

    ExecuteTests(): boolean {

        // FindObjectsInSphere
        {
            this.foundObjs = [];
            wd.World.FindObjectsInSphere("Category1", new wd.Vec3(5, 0, 0), 3, this.FoundObjectCallback);
            WD_TEST.INT(this.foundObjs.length, 2);

            this.foundObjs = [];
            wd.World.FindObjectsInSphere("Category2", new wd.Vec3(5, 0, 0), 3, this.FoundObjectCallback);
            WD_TEST.INT(this.foundObjs.length, 1);
        }

        // FindObjectsInBox
        {
            this.foundObjs = [];
            wd.World.FindObjectsInBox("Category1", new wd.Vec3(-10, 0, -5), new wd.Vec3(0, 10, 5), this.FoundObjectCallback);
            WD_TEST.INT(this.foundObjs.length, 3);

            this.foundObjs = [];
            wd.World.FindObjectsInBox("Category2", new wd.Vec3(-10, 0, -5), new wd.Vec3(0, 10, 5), this.FoundObjectCallback);
            WD_TEST.INT(this.foundObjs.length, 2);
        }

        return false;
    }

    OnMsgGenericEvent(msg: wd.MsgGenericEvent): void {

        if (msg.Message == "TestWorld") {

            if (this.ExecuteTests()) {
                msg.Message = "repeat";
            }
            else {
                msg.Message = "done";
            }

        }
    }

}

