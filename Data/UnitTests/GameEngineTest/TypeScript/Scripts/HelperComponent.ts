import ns = require("TypeScript/ns")
import NS_TEST = require("./TestFramework")
import shared = require("./Shared")

export class HelperComponent extends ns.TickedTypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {

        ns.TypescriptComponent.RegisterMessageHandler(ns.MsgGenericEvent, "OnMsgGenericEvent");
    }

    // Initialize(): void { }
    // Deinitialize(): void { }
    // OnActivated(): void { }
    // OnDeactivated(): void { }

    OnSimulationStarted(): void {
        this.SetTickInterval(ns.Time.Milliseconds(0));
    }

    RaiseEvent(text: string): void {
        let e = new ns.MsgGenericEvent;
        e.Message = text;
        this.BroadcastEvent(e);
    }

    OnMsgGenericEvent(msg: ns.MsgGenericEvent): void {

        if (msg.Message == "Event1") {

            this.RaiseEvent("e1");
        }

        // should not reach itself
        NS_TEST.BOOL(msg.Message != "e1");
    }

    Tick(): void {
    }
}

