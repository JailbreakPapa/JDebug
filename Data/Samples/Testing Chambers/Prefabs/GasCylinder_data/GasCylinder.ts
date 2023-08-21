import wd = require("TypeScript/wd")

export class GasCylinder extends wd.TickedTypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    static RegisterMessageHandlers() {

        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgDamage, "OnMsgDamage");
    }

    private capHealth = 5;
    private bodyHealth = 50;

    constructor() {
        super()
    }

    OnSimulationStarted(): void {
        this.SetTickInterval(wd.Time.Milliseconds(100));
    }

    Tick(): void {

        if (this.capHealth <= 0) {

            let owner = this.GetOwner();
            let cap = owner.FindChildByName("Cap");

            let forceMsg = new wd.MsgPhysicsAddForce();

            forceMsg.GlobalPosition = cap.GetGlobalPosition();
            forceMsg.Force = cap.GetGlobalDirUp();

            let randomDir = wd.Vec3.CreateRandomDirection();
            randomDir.MulNumber(0.6);

            forceMsg.Force.AddVec3(randomDir);
            forceMsg.Force.MulNumber(-400);

            owner.SendMessage(forceMsg);
        }
    }

    OnMsgDamage(msg: wd.MsgDamage) {

        //wd.Log.Info("Damaged: " + msg.HitObjectName + " - " + msg.Damage)

        this.bodyHealth -= msg.Damage;

        if (this.bodyHealth <= 0) {
            this.Explode();
            return;
        }

        if (msg.HitObjectName == "Cap") {

            if (this.capHealth > 0) {

                this.capHealth -= msg.Damage;

                if (this.capHealth <= 0) {

                    this.SetTickInterval(wd.Time.Milliseconds(0));

                    let leakObj = this.GetOwner().FindChildByName("LeakEffect");
                    if (leakObj != null) {

                        let leakFX = leakObj.TryGetComponentOfBaseType(wd.ParticleComponent);

                        if (leakFX != null) {
                            leakFX.StartEffect();
                        }

                        let leakSound = leakObj.TryGetComponentOfBaseType(wd.FmodEventComponent);

                        if (leakSound != null) {
                            leakSound.Restart();
                        }
                    }

                    // trigger code path below
                    msg.HitObjectName = "Tick";
                }
            }
        }

        if (msg.HitObjectName == "Tick") {

            let tickDmg = new wd.MsgDamage();
            tickDmg.Damage = 1;
            tickDmg.HitObjectName = "Tick";
            this.PostMessage(tickDmg, wd.Time.Milliseconds(100));

        }
    }

    Explode(): void {

        let owner = this.GetOwner();
        let exp = owner.FindChildByName("Explosion");

        if (exp != null) {

            let spawnExpl = exp.TryGetComponentOfBaseType(wd.SpawnComponent);

            if (spawnExpl != null) {
                spawnExpl.TriggerManualSpawn(false, wd.Vec3.ZeroVector());
            }
        }

        wd.World.DeleteObjectDelayed(this.GetOwner());
    }
}

