import wd = require("TypeScript/wd")

export class Turret extends wd.TickedTypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    Health: number = 50;
    /* END AUTO-GENERATED: VARIABLES */

    target: wd.GameObject = null;
    gunSpawn: wd.SpawnComponent = null;
    gunSound: wd.FmodEventComponent = null;

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {

        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgDamage, "OnMsgDamage");
    }

    OnMsgDamage(msg: wd.MsgDamage) {

        if (this.Health <= 0)
            return;

        this.Health -= msg.Damage;

        if (this.Health > 0)
            return;

        let expObj = this.GetOwner().FindChildByName("Explosion", true);
        if (expObj == null)
            return;

        let expComp = expObj.TryGetComponentOfBaseType(wd.SpawnComponent);
        if (expComp == null)
            return;

        expComp.TriggerManualSpawn(true, wd.Vec3.ZeroVector());
    }

    OnSimulationStarted(): void {
        this.SetTickInterval(wd.Time.Milliseconds(50));

        let gun = this.GetOwner().FindChildByName("Gun", true);

        this.gunSpawn = gun.TryGetComponentOfBaseType(wd.SpawnComponent);
        this.gunSound = gun.TryGetComponentOfBaseType(wd.FmodEventComponent);
    }


    FoundObjectCallback = (go: wd.GameObject): boolean => {

        this.target = go;

        return false;
    }

    Tick(): void {

        if (this.Health <= 0)
            return;

        if (this.gunSpawn == null || !this.gunSpawn.IsValid())
            return;

        let owner = this.GetOwner();

        this.target = null;
        wd.World.FindObjectsInSphere("Player", owner.GetGlobalPosition(), 15, this.FoundObjectCallback);

        if (this.target == null)
            return;

        let dirToTarget = new wd.Vec3();
        dirToTarget.SetSub(this.target.GetGlobalPosition(), owner.GetGlobalPosition());

        let distance = dirToTarget.GetLength();

        let vis = wd.Physics.Raycast(owner.GetGlobalPosition(), dirToTarget, distance, 7, wd.Physics.ShapeType.Static);
        if (vis != null)
            return;

        let targetRotation = new wd.Quat();
        targetRotation.SetShortestRotation(wd.Vec3.UnitAxisX(), dirToTarget);

        let newRotation = new wd.Quat();
        newRotation.SetSlerp(owner.GetGlobalRotation(), targetRotation, 0.1);

        owner.SetGlobalRotation(newRotation);

        dirToTarget.Normalize();


        if (dirToTarget.Dot(owner.GetGlobalDirForwards()) > Math.cos(wd.Angle.DegreeToRadian(15))) {

            this.gunSpawn.ScheduleSpawn();
            this.gunSound.StartOneShot();
        }
    }
}

