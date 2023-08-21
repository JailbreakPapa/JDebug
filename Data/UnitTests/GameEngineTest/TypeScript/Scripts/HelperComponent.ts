import wd = require("TypeScript/wd")
import WD_TEST = require("./TestFramework")
import shared = require("./Shared")

export class HelperComponent extends wd.TickedTypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {

        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgGenericEvent, "OnMsgGenericEvent");
    }

    // Initialize(): void { }
    // Deinitialize(): void { }
    // OnActivated(): void { }
    // OnDeactivated(): void { }

    OnSimulationStarted(): void {
        this.SetTickInterval(wd.Time.Milliseconds(0));
    }

    RaiseEvent(text: string): void {
        let e = new wd.MsgGenericEvent;
        e.Message = text;
        this.BroadcastEvent(e);
    }

    OnMsgGenericEvent(msg: wd.MsgGenericEvent): void {

        if (msg.Message == "Event1") {

            this.RaiseEvent("e1");
        }

        // should not reach itself
        WD_TEST.BOOL(msg.Message != "e1");
    }

    Tick(): void {
    }
}

