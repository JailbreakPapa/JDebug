import wd = require("TypeScript/wd")

export class JointBroke extends wd.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    OnBreakMsg: string = "Joint Broke !";
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {

        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgPhysicsJointBroke, "OnMsgPhysicsJointBroke");
    }

    OnMsgPhysicsJointBroke(msg: wd.MsgPhysicsJointBroke): void {
        wd.Log.Info(this.OnBreakMsg);
    }
}

