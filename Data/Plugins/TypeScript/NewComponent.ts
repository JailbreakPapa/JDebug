import wd = require("<PATH-TO-EZ-TS>")

//export class NewComponent extends wd.TypescriptComponent {
export class NewComponent extends wd.TickedTypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {

        // you can only call "RegisterMessageHandler" from within this function
        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgSetColor, "OnMsgSetColor");
    }

    // Initialize(): void { }
    // Deinitialize(): void { }
    // OnActivated(): void { }
    // OnDeactivated(): void { }

    OnSimulationStarted(): void {
        this.SetTickInterval(wd.Time.Milliseconds(100));
    }

    OnMsgSetColor(msg: wd.MsgSetColor): void {
        wd.Log.Info("MsgSetColor: " + msg.Color.r + ", " + msg.Color.g + ", " + msg.Color.b + ", " + msg.Color.a);
    }

    Tick(): void {
        // if a regular tick is not needed, remove this and derive directly from wd.TypescriptComponent
        wd.Log.Info("NewComponent.Tick()")
    }
}

