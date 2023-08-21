import wd = require("TypeScript/wd")

export class ShootingStar2 extends wd.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    ragdoll:boolean = false;

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {

        // you can only call "RegisterMessageHandler" from within this function
        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgDamage, "OnMsgDamage");
    }

    OnMsgDamage(msg: wd.MsgDamage): void {

        if (!this.ragdoll) {
            this.ragdoll = true;

            var col = this.GetOwner().TryGetComponentOfBaseType(wd.JoltBoneColliderComponent);
            
            if (col != null) {
                // if present, deactivate the bone collider component, it isn't needed anymore
                col.SetActiveFlag(false);
            }
            
            var rdc = this.GetOwner().TryGetComponentOfBaseType(wd.JoltRagdollComponent);
            
            if (rdc != null) {
                rdc.Start = wd.JoltRagdollStart.WaitForPose;

                // we want the ragdoll to get a kick, so send an impulse message
                var imp = new wd.MsgPhysicsAddImpulse();
                imp.Impulse = msg.ImpactDirection.Clone();
                imp.Impulse.MulNumber(Math.min(msg.Damage, 5) * 10);
                imp.GlobalPosition = msg.GlobalPosition.Clone();
                rdc.SendMessage(imp);

                // wd.Log.Info("Impulse: " + imp.Impulse.x + ", " + imp.Impulse.y + ", " + imp.Impulse.z)
            }
        }
    }
}

