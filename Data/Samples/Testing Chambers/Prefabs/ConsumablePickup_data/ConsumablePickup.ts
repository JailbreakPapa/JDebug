import wd = require("TypeScript/wd")
import _gm = require("Scripting/GameMessages")

export class ConsumablePickup extends wd.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    ConsumableType: number = 0;
    Amount: number = 0;
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {

        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgTriggerTriggered, "OnMsgTriggerTriggered");
    }

    OnMsgTriggerTriggered(msg: wd.MsgTriggerTriggered): void {

        if (msg.TriggerState == wd.TriggerState.Activated && msg.Message == "Pickup") {

            // TODO: need GO handles in messages to identify who entered the trigger
            let player = wd.World.TryGetObjectWithGlobalKey("Player");
            if (player == null)
                return;

            let hm = new _gm.MsgAddConsumable();
            hm.consumableType = this.ConsumableType;
            hm.amount = this.Amount;

            player.SendMessage(hm, true);

            if (hm.return_consumed == false)
                return;

            let sound = this.GetOwner().TryGetComponentOfBaseType(wd.FmodEventComponent);
            sound.StartOneShot();

            let del = new wd.MsgDeleteGameObject();
            this.GetOwner().PostMessage(del, 0.1);
        }
    }
}

