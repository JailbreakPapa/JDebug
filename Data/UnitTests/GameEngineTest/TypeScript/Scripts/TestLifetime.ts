import ns = require("TypeScript/ns")
import NS_TEST = require("./TestFramework")

export class TestLifetime extends ns.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {
        ns.TypescriptComponent.RegisterMessageHandler(ns.MsgGenericEvent, "OnMsgGenericEvent");
    }

    step: number = 0;
    obj1: ns.GameObject = null;
    comp1: ns.MeshComponent = null;
    comp2: ns.MeshComponent = null;

    ExecuteTests(): boolean {

        if (this.step == 0) {

            let d = new ns.GameObjectDesc;
            d.Name = "Jonny";
            d.LocalPosition = new ns.Vec3(1, 2, 3);
            d.Dynamic = true;
            d.Parent = this.GetOwner();

            this.obj1 = ns.World.CreateObject(d);

            NS_TEST.BOOL(this.obj1.GetName() == d.Name);
            NS_TEST.BOOL(this.obj1.GetParent() == this.GetOwner());
            NS_TEST.BOOL(this.obj1.GetLocalPosition().IsEqual(d.LocalPosition));
            NS_TEST.BOOL(this.obj1.GetLocalRotation().IsEqualRotation(ns.Quat.IdentityQuaternion()));
            NS_TEST.BOOL(this.obj1.GetLocalScaling().IsEqual(ns.Vec3.OneVector()));
            NS_TEST.FLOAT(this.obj1.GetLocalUniformScaling(), 1.0, 0.0001);

            this.comp1 = ns.World.CreateComponent(this.obj1, ns.MeshComponent);
            this.comp1.Mesh = "{ 6d619c33-6611-432b-a924-27b1b9bfd8db }"; // Box
            this.comp1.Color = ns.Color.BlueViolet();

            this.comp2 = ns.World.CreateComponent(this.obj1, ns.MeshComponent);
            this.comp2.Mesh = "{ 618ee743-ed04-4fac-bf5f-572939db2f1d }"; // Sphere
            this.comp2.Color = ns.Color.PaleVioletRed();

            return true;
        }

        if (this.step == 1) {
            NS_TEST.BOOL(this.obj1 != null);
            NS_TEST.BOOL(this.obj1.IsValid());

            NS_TEST.BOOL(this.comp1.IsValid());
            NS_TEST.BOOL(this.comp2.IsValid());

            this.obj1.SetLocalUniformScaling(2.0);
            
            ns.World.DeleteComponent(this.comp2);
            
            NS_TEST.BOOL(!this.comp2.IsValid());

            return true;
        }

        if (this.step == 2) {
            NS_TEST.BOOL(this.obj1 != null);
            NS_TEST.BOOL(this.obj1.IsValid());

            NS_TEST.BOOL(this.comp1.IsValid());
            NS_TEST.BOOL(!this.comp2.IsValid());

            ns.World.DeleteObjectDelayed(this.obj1);

            // still valid this frame
            NS_TEST.BOOL(this.obj1.IsValid());
            NS_TEST.BOOL(this.comp1.IsValid());

            return true;
        }

        if (this.step == 3) {
            NS_TEST.BOOL(this.obj1 != null);
            NS_TEST.BOOL(!this.obj1.IsValid());
            NS_TEST.BOOL(!this.comp1.IsValid());
            NS_TEST.BOOL(!this.comp2.IsValid());
        }

        return false;
    }

    OnMsgGenericEvent(msg: ns.MsgGenericEvent): void {

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

