import wd = require("../../TypeScript/wd")

export class WallMine extends wd.TickedTypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    private distance = 0;

    constructor() {
        super()
    }

    OnSimulationStarted(): void {
        this.SetTickInterval(wd.Time.Milliseconds(40));
    }

    Tick(): void {

        let owner = this.GetOwner();
        let pos = owner.GetGlobalPosition();
        let dir = owner.GetGlobalDirForwards();

        let shapeId = -1;
        let staticactor = owner.TryGetComponentOfBaseType(wd.JoltStaticActorComponent);
        if (staticactor != null) {
            shapeId = staticactor.GetObjectFilterID();
        }

        let res = wd.Physics.Raycast(pos, dir, 10, 0, wd.Physics.ShapeType.Static | wd.Physics.ShapeType.AllInteractive , shapeId);

        if (res == null) {
            return;
        }

        if (res.distance < this.distance - 0.05) {
            // allow some slack

            this.Explode();
        }
        else if (res.distance > this.distance) {

            let glowLine = owner.FindChildByName("GlowLine", false);

            if (glowLine != null) {
                glowLine.SetLocalScaling(new wd.Vec3(res.distance, 1, 1));
                glowLine.SetLocalPosition(new wd.Vec3(res.distance * 0.5, 0, 0));
            }

            this.distance = res.distance;
        }
    }

    Explode(): void {

        let owner = this.GetOwner();
        let exp = owner.FindChildByName("Explosion");

        if (exp != null) {

            let spawnExpl = exp.TryGetComponentOfBaseType(wd.SpawnComponent);

            if (spawnExpl != null) {
                spawnExpl.TriggerManualSpawn(true, wd.Vec3.ZeroVector());
            }
        }

        wd.World.DeleteObjectDelayed(this.GetOwner());
    }

    // to use message handlers you must implement exactly this function
    static RegisterMessageHandlers() {
        // you can only call "RegisterMessageHandler" from within this function
        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgDamage, "OnMsgDamage");
    }

    OnMsgDamage(msg: wd.MsgDamage): void {
        // explode on any damage
        this.Explode();
    }
}

