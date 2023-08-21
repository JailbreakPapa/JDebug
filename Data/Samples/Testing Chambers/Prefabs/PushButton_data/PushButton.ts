import wd = require("TypeScript/wd")

export class PushButton extends wd.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    ButtonName: string = "";
    /* END AUTO-GENERATED: VARIABLES */

    slider: wd.TransformComponent = null;

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {
        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgGenericEvent, "OnMsgGenericEvent");
    }

    OnSimulationStarted(): void {

        let owner = this.GetOwner();
        let button = owner.FindChildByName("Button");
        this.slider = button.TryGetComponentOfBaseType(wd.TransformComponent);
    }

    OnMsgGenericEvent(msg: wd.MsgGenericEvent): void {

        if (msg.Message == "Use") {

            if (this.slider == null || this.slider.Running)
                return;

            this.slider.SetDirectionForwards(true);
            this.slider.Running = true;

            let newMsg = new wd.MsgGenericEvent();
            newMsg.Message = this.ButtonName;

            this.BroadcastEvent(newMsg);
        }
    }
}

