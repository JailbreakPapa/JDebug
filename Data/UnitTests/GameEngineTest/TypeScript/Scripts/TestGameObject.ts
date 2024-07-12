import ns = require("TypeScript/ns")
import NS_TEST = require("./TestFramework")

export class TestGameObject extends ns.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {
        ns.TypescriptComponent.RegisterMessageHandler(ns.MsgGenericEvent, "OnMsgGenericEvent");
    }

    ExecuteTests(): void {

        let owner = this.GetOwner();
        let child1 = owner.FindChildByName("Child1");
        let child2 = owner.FindChildByPath("Child2");


        // IsValid
        {
            NS_TEST.BOOL(owner.IsValid());
            NS_TEST.BOOL(child1.IsValid());
            NS_TEST.BOOL(child2.IsValid());
        }

        // GetName / SetName
        {
            NS_TEST.BOOL(owner.GetName() == "GameObject");
            owner.SetName("TestGameObject");
            NS_TEST.BOOL(owner.GetName() == "TestGameObject");
        }

        // Active Flag / Active State
        {
            NS_TEST.BOOL(child1.GetActiveFlag());
            NS_TEST.BOOL(child1.IsActive());
            NS_TEST.BOOL(child2.GetActiveFlag());
            NS_TEST.BOOL(child2.IsActive());

            child2.SetActiveFlag(false);

            NS_TEST.BOOL(child1.GetActiveFlag());
            NS_TEST.BOOL(child1.IsActive());
            NS_TEST.BOOL(!child2.GetActiveFlag());
            NS_TEST.BOOL(!child2.IsActive());

            child2.SetActiveFlag(true);

            NS_TEST.BOOL(child1.GetActiveFlag());
            NS_TEST.BOOL(child1.IsActive());
            NS_TEST.BOOL(child2.GetActiveFlag());
            NS_TEST.BOOL(child2.IsActive());
        }

        // Local Position
        {
            NS_TEST.VEC3(child1.GetLocalPosition(), new ns.Vec3(1, 2, 3));
            NS_TEST.VEC3(child2.GetLocalPosition(), new ns.Vec3(4, 5, 6));

            child1.SetLocalPosition(new ns.Vec3(11, 22, 33));
            NS_TEST.VEC3(child1.GetLocalPosition(), new ns.Vec3(11, 22, 33));
        }

        // Local Rotation
        {
            NS_TEST.QUAT(child1.GetLocalRotation(), ns.Quat.IdentityQuaternion());

            let nr = new ns.Quat();
            nr.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(45));

            child1.SetLocalRotation(nr);
            NS_TEST.QUAT(child1.GetLocalRotation(), nr);
        }

        // Local Scaling
        {
            NS_TEST.VEC3(child2.GetLocalScaling(), new ns.Vec3(2, 3, 4));
            NS_TEST.FLOAT(child2.GetLocalUniformScaling(), 5);

            child2.SetLocalScaling(new ns.Vec3(22, 33, 44));
            child2.SetLocalUniformScaling(55);

            NS_TEST.VEC3(child2.GetLocalScaling(), new ns.Vec3(22, 33, 44));
            NS_TEST.FLOAT(child2.GetLocalUniformScaling(), 55);
        }

        // Global Position
        {
            NS_TEST.VEC3(child1.GetGlobalPosition(), new ns.Vec3(1, 2, 3));
            NS_TEST.VEC3(child2.GetGlobalPosition(), new ns.Vec3(4, 5, 6));

            child1.SetGlobalPosition(new ns.Vec3(11, 22, 33));
            NS_TEST.VEC3(child1.GetGlobalPosition(), new ns.Vec3(11, 22, 33));

        }

        // Global Rotation
        {
            NS_TEST.QUAT(child1.GetGlobalRotation(), ns.Quat.IdentityQuaternion());

            let nr = new ns.Quat();
            nr.SetFromAxisAndAngle(new ns.Vec3(0, 1, 0), ns.Angle.DegreeToRadian(30));

            child1.SetGlobalRotation(nr);
            NS_TEST.QUAT(child1.GetGlobalRotation(), nr);
        }

        // Global Scaling
        {
            NS_TEST.VEC3(child2.GetGlobalScaling(), new ns.Vec3(2 * 5, 3 * 5, 4 * 5));

            child2.SetGlobalScaling(new ns.Vec3(1, 2, 3));

            NS_TEST.VEC3(child2.GetGlobalScaling(), new ns.Vec3(1, 2, 3));
            NS_TEST.FLOAT(child2.GetLocalUniformScaling(), 1);
        }

        // Global Dirs
        {
            child1.SetGlobalRotation(ns.Quat.IdentityQuaternion());

            NS_TEST.VEC3(child1.GetGlobalDirForwards(), new ns.Vec3(1, 0, 0));
            NS_TEST.VEC3(child1.GetGlobalDirRight(), new ns.Vec3(0, 1, 0));
            NS_TEST.VEC3(child1.GetGlobalDirUp(), new ns.Vec3(0, 0, 1));

            let r = new ns.Quat();
            r.SetFromAxisAndAngle(new ns.Vec3(0, 0, 1), ns.Angle.DegreeToRadian(90));

            child1.SetGlobalRotation(r);

            NS_TEST.VEC3(child1.GetGlobalDirForwards(), new ns.Vec3(0, 1, 0));
            NS_TEST.VEC3(child1.GetGlobalDirRight(), new ns.Vec3(-1, 0, 0));
            NS_TEST.VEC3(child1.GetGlobalDirUp(), new ns.Vec3(0, 0, 1));
        }

        // Velocity
        {
            NS_TEST.VEC3(child1.GetLinearVelocity(), new ns.Vec3(300, 600, 900));
        }

        // Team ID
        {
            NS_TEST.FLOAT(child1.GetTeamID(), 0);
            child1.SetTeamID(11);
            NS_TEST.FLOAT(child1.GetTeamID(), 11);
        }

        // FindChildByName
        {
            let c = owner.FindChildByName("Child1_Child1", false);
            NS_TEST.BOOL(c == null);

            c = owner.FindChildByName("Child1_Child1", true);
            NS_TEST.BOOL(c != null);
            NS_TEST.BOOL(c.IsValid());
            NS_TEST.BOOL(c.GetName() == "Child1_Child1");
        }

        // FindChildByName
        {
            let c = owner.FindChildByPath("Child2_Child1");
            NS_TEST.BOOL(c == null);

            c = owner.FindChildByPath("Child2/Child2_Child1");
            NS_TEST.BOOL(c != null);
            NS_TEST.BOOL(c.IsValid());
            NS_TEST.BOOL(c.GetName() == "Child2_Child1");
        }

        // SearchForChildByNameSequence
        {
            let c = owner.SearchForChildByNameSequence("Child1_Child1/A");
            NS_TEST.BOOL(c != null && c.IsValid());
            NS_TEST.FLOAT(c.GetLocalUniformScaling(), 2);

            c = owner.SearchForChildWithComponentByNameSequence("Child2/A", ns.PointLightComponent);
            NS_TEST.BOOL(c != null && c.IsValid());
            NS_TEST.FLOAT(c.GetLocalUniformScaling(), 3);
        }

        // TryGetComponentOfBaseType
        {
            let sl = child1.TryGetComponentOfBaseType(ns.SpotLightComponent);
            NS_TEST.BOOL(sl != null && sl.IsValid());

            let pl = child1.TryGetComponentOfBaseTypeName<ns.SpotLightComponent>("nsPointLightComponent");
            NS_TEST.BOOL(pl != null && pl.IsValid());
        }

        // Tags
        {
            NS_TEST.BOOL(owner.HasAllTags("AutoColMesh"));
            NS_TEST.BOOL(owner.HasAllTags("CastShadow"));
            NS_TEST.BOOL(owner.HasAllTags("AutoColMesh", "CastShadow"));
            NS_TEST.BOOL(owner.HasAnyTags("AutoColMesh", "NOTAG"));
            NS_TEST.BOOL(owner.HasAnyTags("CastShadow", "NOTAG"));
            NS_TEST.BOOL(owner.HasAnyTags("AutoColMesh", "CastShadow"));

            owner.RemoveTags("CastShadow", "AutoColMesh");
            NS_TEST.BOOL(!owner.HasAnyTags("AutoColMesh", "CastShadow"));

            owner.AddTags("CastShadow", "TAG1");
            NS_TEST.BOOL(owner.HasAnyTags("AutoColMesh", "CastShadow"));
            NS_TEST.BOOL(!owner.HasAllTags("AutoColMesh", "CastShadow"));

            owner.SetTags("TAG");
            NS_TEST.BOOL(owner.HasAnyTags("TAG"));
            NS_TEST.BOOL(!owner.HasAnyTags("AutoColMesh", "CastShadow", "TAG1"));
        }

        // Global Key
        {
            let obj = ns.World.TryGetObjectWithGlobalKey("Tests");
            NS_TEST.BOOL(obj != null);
            NS_TEST.BOOL(obj.GetName() == "All Tests");

            this.GetOwner().SetGlobalKey("TestGameObjects");
            NS_TEST.BOOL(this.GetOwner().GetGlobalKey() == "TestGameObjects");
            let tgo = ns.World.TryGetObjectWithGlobalKey("TestGameObjects");
            NS_TEST.BOOL(tgo == this.GetOwner());
            this.GetOwner().SetGlobalKey("");
            let tgo2 = ns.World.TryGetObjectWithGlobalKey("TestGameObjects");
            NS_TEST.BOOL(tgo2 == null);
        }

        // GetChildren
        {
            NS_TEST.INT(this.GetOwner().GetChildCount(), 3);

            let children = this.GetOwner().GetChildren();
            NS_TEST.INT(this.GetOwner().GetChildCount(), children.length);

            this.GetOwner().DetachChild(children[0]);
            NS_TEST.INT(this.GetOwner().GetChildCount(), 2);
            NS_TEST.BOOL(children[0].GetParent() == null);

            children[0].SetParent(this.GetOwner());
            NS_TEST.INT(this.GetOwner().GetChildCount(), 3);
            NS_TEST.BOOL(children[0].GetParent() == this.GetOwner());

            this.GetOwner().DetachChild(children[2]);
            NS_TEST.BOOL(children[2].GetParent() == null);
            this.GetOwner().AddChild(children[2]);
            NS_TEST.BOOL(children[2].GetParent() == this.GetOwner());
        }
    }

    OnMsgGenericEvent(msg: ns.MsgGenericEvent): void {

        if (msg.Message == "TestGameObject") {

            this.ExecuteTests();

            msg.Message = "done";
        }
    }

}

