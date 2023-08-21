import wd = require("TypeScript/wd")

export class PhysicsLevelScript extends wd.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {

        // you can only call "RegisterMessageHandler" from within this function
        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgTriggerTriggered, "OnMsgTriggerTriggered");
    }

    OnSimulationStarted(): void {
    }

    OnMsgTriggerTriggered(msg: wd.MsgTriggerTriggered): void {

        if (msg.Message == "ActivatePaddleWheel") {

            if (msg.TriggerState == wd.TriggerState.Activated) {

                let spawn1 = wd.World.TryGetObjectWithGlobalKey("PaddleWheelSpawn1");
                if (spawn1 != null) {
                    spawn1.SetActiveFlag(true);
                }

            }
            else if (msg.TriggerState == wd.TriggerState.Deactivated) {

                let spawn1 = wd.World.TryGetObjectWithGlobalKey("PaddleWheelSpawn1");
                if (spawn1 != null) {
                    spawn1.SetActiveFlag(false);
                }

            }
        }

        if (msg.Message == "ActivateSwing") {

            if (msg.TriggerState == wd.TriggerState.Activated) {

                let spawn1 = wd.World.TryGetObjectWithGlobalKey("SwingSpawn1");
                if (spawn1 != null) {
                    spawn1.SetActiveFlag(true);
                }

            }
            else if (msg.TriggerState == wd.TriggerState.Deactivated) {

                let spawn1 = wd.World.TryGetObjectWithGlobalKey("SwingSpawn1");
                if (spawn1 != null) {
                    spawn1.SetActiveFlag(false);
                }

            }
        }
    }
}

