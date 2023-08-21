import wd = require("TypeScript/wd")

export class TargetSphere extends wd.TickedTypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {

        // you can only call "RegisterMessageHandler" from within this function
        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgDamage, "OnMsgDamage");
        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgInputActionTriggered, "OnMsgInputActionTriggered");
    }

    curDamage = 0;
    fireFX: wd.ParticleComponent = null;

    OnSimulationStarted(): void {
        this.SetTickInterval(wd.Time.Milliseconds(100));

        this.fireFX = this.GetOwner().TryGetComponentOfBaseType(wd.ParticleComponent);
    }

    OnMsgDamage(msg: wd.MsgDamage): void {

        this.curDamage += msg.Damage;
    }

    OnMsgInputActionTriggered(msg: wd.MsgInputActionTriggered) {
     
        if (msg.TriggerState == wd.TriggerState.Activated) {
            if (msg.InputAction == "Heal") {
                this.curDamage = 0;
            }
        }
    }

    Tick(): void {

        this.curDamage = wd.Utils.Clamp(this.curDamage - 1.0, 0, 1000);
        const dmg = this.curDamage / 100.0;

        let msgCol = new wd.MsgSetColor();
        msgCol.Color.SetLinearRGBA(dmg, dmg * 0.05, dmg * 0.05);

        this.GetOwner().SendMessageRecursive(msgCol);

        if (this.fireFX != null && this.fireFX.IsValid()) {

            if (dmg > 1.0) {
                if (!this.fireFX.IsEffectActive()) {
                    this.fireFX.StartEffect();
                }
            }
            else if (dmg < 0.8) {
                this.fireFX.StopEffect();
            }
        }
    }
}

