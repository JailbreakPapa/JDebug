import ns = require("TypeScript/ns")
import NS_TEST = require("./TestFramework")

export class TestWorld extends ns.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {
        ns.TypescriptComponent.RegisterMessageHandler(ns.MsgGenericEvent, "OnMsgGenericEvent");
    }

    foundObjs: ns.GameObject[] = [];

    FoundObjectCallback = (go: ns.GameObject): boolean => {

        this.foundObjs.push(go);
        return true;
    }

    ExecuteTests(): boolean {

        // FindObjectsInSphere
        {
            this.foundObjs = [];
            ns.World.FindObjectsInSphere("Category1", new ns.Vec3(5, 0, 0), 3, this.FoundObjectCallback);
            NS_TEST.INT(this.foundObjs.length, 2);

            this.foundObjs = [];
            ns.World.FindObjectsInSphere("Category2", new ns.Vec3(5, 0, 0), 3, this.FoundObjectCallback);
            NS_TEST.INT(this.foundObjs.length, 1);
        }

        // FindObjectsInBox
        {
            this.foundObjs = [];
            ns.World.FindObjectsInBox("Category1", new ns.Vec3(-10, 0, -5), new ns.Vec3(0, 10, 5), this.FoundObjectCallback);
            NS_TEST.INT(this.foundObjs.length, 3);

            this.foundObjs = [];
            ns.World.FindObjectsInBox("Category2", new ns.Vec3(-10, 0, -5), new ns.Vec3(0, 10, 5), this.FoundObjectCallback);
            NS_TEST.INT(this.foundObjs.length, 2);
        }

        return false;
    }

    OnMsgGenericEvent(msg: ns.MsgGenericEvent): void {

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

