import wd = require("TypeScript/wd")

export class JointMotorFlip extends wd.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    Seconds: number = 10;
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {

        // you can only call "RegisterMessageHandler" from within this function
        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgComponentInternalTrigger, "OnMsgComponentInternalTrigger");
    }

    OnSimulationStarted(): void {

        let msg = new wd.MsgComponentInternalTrigger();
        msg.Message = "FlipMotor";

        this.PostMessage(msg, this.Seconds);

    }

    OnMsgComponentInternalTrigger(msg: wd.MsgComponentInternalTrigger): void {

        let joint = this.GetOwner().TryGetComponentOfBaseType(wd.JoltHingeConstraintComponent);

        if (joint != null) {

            joint.DriveTargetValue = -joint.DriveTargetValue;
        }

        this.PostMessage(msg, this.Seconds);
    }
}

