import wd = require("TypeScript/wd")
import WD_TEST = require("./TestFramework")

export class TestLifetime extends wd.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {
        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgGenericEvent, "OnMsgGenericEvent");
    }

    step: number = 0;
    obj1: wd.GameObject = null;
    comp1: wd.MeshComponent = null;
    comp2: wd.MeshComponent = null;

    ExecuteTests(): boolean {

        if (this.step == 0) {

            let d = new wd.GameObjectDesc;
            d.Name = "Jonny";
            d.LocalPosition = new wd.Vec3(1, 2, 3);
            d.Dynamic = true;
            d.Parent = this.GetOwner();

            this.obj1 = wd.World.CreateObject(d);

            WD_TEST.BOOL(this.obj1.GetName() == d.Name);
            WD_TEST.BOOL(this.obj1.GetParent() == this.GetOwner());
            WD_TEST.BOOL(this.obj1.GetLocalPosition().IsEqual(d.LocalPosition));
            WD_TEST.BOOL(this.obj1.GetLocalRotation().IsEqualRotation(wd.Quat.IdentityQuaternion()));
            WD_TEST.BOOL(this.obj1.GetLocalScaling().IsEqual(wd.Vec3.OneVector()));
            WD_TEST.FLOAT(this.obj1.GetLocalUniformScaling(), 1.0, 0.0001);

            this.comp1 = wd.World.CreateComponent(this.obj1, wd.MeshComponent);
            this.comp1.Mesh = "{ 6d619c33-6611-432b-a924-27b1b9bfd8db }"; // Box
            this.comp1.Color = wd.Color.BlueViolet();

            this.comp2 = wd.World.CreateComponent(this.obj1, wd.MeshComponent);
            this.comp2.Mesh = "{ 618ee743-ed04-4fac-bf5f-572939db2f1d }"; // Sphere
            this.comp2.Color = wd.Color.PaleVioletRed();

            return true;
        }

        if (this.step == 1) {
            WD_TEST.BOOL(this.obj1 != null);
            WD_TEST.BOOL(this.obj1.IsValid());

            WD_TEST.BOOL(this.comp1.IsValid());
            WD_TEST.BOOL(this.comp2.IsValid());

            this.obj1.SetLocalUniformScaling(2.0);

            wd.World.DeleteComponent(this.comp2);

            WD_TEST.BOOL(!this.comp2.IsValid());

            return true;
        }

        if (this.step == 2) {
            WD_TEST.BOOL(this.obj1 != null);
            WD_TEST.BOOL(this.obj1.IsValid());

            WD_TEST.BOOL(this.comp1.IsValid());
            WD_TEST.BOOL(!this.comp2.IsValid());

            wd.World.DeleteObjectDelayed(this.obj1);

            // still valid this frame
            WD_TEST.BOOL(this.obj1.IsValid());
            WD_TEST.BOOL(this.comp1.IsValid());

            return true;
        }

        if (this.step == 3) {
            WD_TEST.BOOL(this.obj1 != null);
            WD_TEST.BOOL(!this.obj1.IsValid());
            WD_TEST.BOOL(!this.comp1.IsValid());
            WD_TEST.BOOL(!this.comp2.IsValid());
        }

        return false;
    }

    OnMsgGenericEvent(msg: wd.MsgGenericEvent): void {

        if (msg.Message == "TestLifetime") {

            if (this.ExecuteTests()) {
                msg.Message = "repeat";
            }
            else {
                msg.Message = "done";
            }

            this.step += 1;
        }
    }

}

