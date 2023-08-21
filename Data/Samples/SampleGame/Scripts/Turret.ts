import wd = require("TypeScript/wd")

export class Turret extends wd.TickedTypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    Range: number = 3;
    /* END AUTO-GENERATED: VARIABLES */

    allTargets: wd.GameObject[] = [];
    lastDamageTime: number;

    constructor() {
        super()
    }

    OnSimulationStarted(): void {

        // update this component every frame
        this.SetTickInterval(wd.Time.Zero());
        this.lastDamageTime = wd.Time.GetGameTime();
    }

    FoundTargetCallback = (go: wd.GameObject): boolean => {
        this.allTargets.push(go);
        return true;
    }

    Tick(): void {

        let owner = this.GetOwner();

        // find all objects with the 'TurretTarget' marker that are close by
        this.allTargets = [];
        wd.World.FindObjectsInSphere("TurretTarget", owner.GetGlobalPosition(), this.Range, this.FoundTargetCallback);

        this.DrawLinesToTargets();

        if (wd.Time.GetGameTime() - this.lastDamageTime > wd.Time.Milliseconds(40)) {
            
            this.lastDamageTime = wd.Time.GetGameTime();

            this.DamageAllTargets(4);
        }
    }

    DrawLinesToTargets(): void {

        const startPos = this.GetOwner().GetGlobalPosition();

        let lines: wd.Debug.Line[] = [];

        for (let i = 0; i < this.allTargets.length; ++i) {

            const target = this.allTargets[i];
            const endPos = target.GetGlobalPosition();

            let line = new wd.Debug.Line();
            line.startX = startPos.x;
            line.startY = startPos.y;
            line.startZ = startPos.z;
            line.endX = endPos.x;
            line.endY = endPos.y;
            line.endZ = endPos.z;

            lines.push(line);
        }

        wd.Debug.DrawLines(lines, wd.Color.OrangeRed());
    }

    DamageAllTargets(damage: number): void {

        let dmgMsg = new wd.MsgDamage();
        dmgMsg.Damage = damage;

        for (let i = 0; i < this.allTargets.length; ++i) {

            this.allTargets[i].SendMessage(dmgMsg);
        }
    }
}

