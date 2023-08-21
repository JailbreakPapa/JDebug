import wd = require("../../TypeScript/wd")

export class HealthPickup extends wd.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    pfxTop: wd.ParticleComponent = null;
    pfxPickup: wd.ParticleComponent = null;

    OnSimulationStarted(): void {
        this.pfxTop = this.GetOwner().FindChildByName("Particle", true).TryGetComponentOfBaseType(wd.ParticleComponent);
        this.pfxPickup = this.GetOwner().TryGetComponentOfBaseType(wd.ParticleComponent);
    }

    static RegisterMessageHandlers() {

        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgGenericEvent, "OnMsgGenericEvent");
        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgTriggerTriggered, "OnMsgTriggerTriggered");
    }

    OnMsgGenericEvent(msg: wd.MsgGenericEvent): void {

        if (msg.Message == "Animation Cue 1") {
            this.pfxTop.StartEffect();
        }
    }    

    OnMsgTriggerTriggered(msg: wd.MsgTriggerTriggered): void {

        if (msg.TriggerState == wd.TriggerState.Activated && msg.Message == "Pickup") {

            this.pfxPickup.StartEffect();

            let del = new wd.MsgDeleteGameObject();
            this.GetOwner().PostMessage(del, 0.1);
        }
    }    
}

