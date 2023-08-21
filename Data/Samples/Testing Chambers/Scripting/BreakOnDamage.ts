import wd = require("TypeScript/wd")

export class BreakOnDamage extends wd.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    Health: number = 10;
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {

        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgDamage, "OnMsgDamage");
    }

    OnMsgDamage(msg: wd.MsgDamage): void {
        
        if (this.Health > 0) {

            this.Health -= msg.Damage;

            if (this.Health <= 0) {

                let spawnNode = this.GetOwner().FindChildByName("OnBreakSpawn");
                if (spawnNode != null) {

                    let spawnComp = spawnNode.TryGetComponentOfBaseType(wd.SpawnComponent);

                    if (spawnComp != null) {

                        let offset = wd.Vec3.CreateRandomPointInSphere();
                        offset.MulNumber(0.3);
                        spawnComp.TriggerManualSpawn(true, offset);
                    }
                }

                wd.World.DeleteObjectDelayed(this.GetOwner());
            }
        }
    }
}

