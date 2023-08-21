import wd = require("TypeScript/wd")
import WD_TEST = require("./TestFramework")

export class TestGameObject extends wd.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {
        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgGenericEvent, "OnMsgGenericEvent");
    }

    ExecuteTests(): void {

        let owner = this.GetOwner();
        let child1 = owner.FindChildByName("Child1");
        let child2 = owner.FindChildByPath("Child2");


        // IsValid
        {
            WD_TEST.BOOL(owner.IsValid());
            WD_TEST.BOOL(child1.IsValid());
            WD_TEST.BOOL(child2.IsValid());
        }

        // GetName / SetName
        {
            WD_TEST.BOOL(owner.GetName() == "GameObject");
            owner.SetName("TestGameObject");
            WD_TEST.BOOL(owner.GetName() == "TestGameObject");
        }

        // Active Flag / Active State
        {
            WD_TEST.BOOL(child1.GetActiveFlag());
            WD_TEST.BOOL(child1.IsActive());
            WD_TEST.BOOL(child2.GetActiveFlag());
            WD_TEST.BOOL(child2.IsActive());

            child2.SetActiveFlag(false);

            WD_TEST.BOOL(child1.GetActiveFlag());
            WD_TEST.BOOL(child1.IsActive());
            WD_TEST.BOOL(!child2.GetActiveFlag());
            WD_TEST.BOOL(!child2.IsActive());

            child2.SetActiveFlag(true);

            WD_TEST.BOOL(child1.GetActiveFlag());
            WD_TEST.BOOL(child1.IsActive());
            WD_TEST.BOOL(child2.GetActiveFlag());
            WD_TEST.BOOL(child2.IsActive());
        }

        // Local Position
        {
            WD_TEST.VEC3(child1.GetLocalPosition(), new wd.Vec3(1, 2, 3));
            WD_TEST.VEC3(child2.GetLocalPosition(), new wd.Vec3(4, 5, 6));

            child1.SetLocalPosition(new wd.Vec3(11, 22, 33));
            WD_TEST.VEC3(child1.GetLocalPosition(), new wd.Vec3(11, 22, 33));
        }

        // Local Rotation
        {
            WD_TEST.QUAT(child1.GetLocalRotation(), wd.Quat.IdentityQuaternion());

            let nr = new wd.Quat();
            nr.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(45));

            child1.SetLocalRotation(nr);
            WD_TEST.QUAT(child1.GetLocalRotation(), nr);
        }

        // Local Scaling
        {
            WD_TEST.VEC3(child2.GetLocalScaling(), new wd.Vec3(2, 3, 4));
            WD_TEST.FLOAT(child2.GetLocalUniformScaling(), 5);

            child2.SetLocalScaling(new wd.Vec3(22, 33, 44));
            child2.SetLocalUniformScaling(55);

            WD_TEST.VEC3(child2.GetLocalScaling(), new wd.Vec3(22, 33, 44));
            WD_TEST.FLOAT(child2.GetLocalUniformScaling(), 55);
        }

        // Global Position
        {
            WD_TEST.VEC3(child1.GetGlobalPosition(), new wd.Vec3(1, 2, 3));
            WD_TEST.VEC3(child2.GetGlobalPosition(), new wd.Vec3(4, 5, 6));

            child1.SetGlobalPosition(new wd.Vec3(11, 22, 33));
            WD_TEST.VEC3(child1.GetGlobalPosition(), new wd.Vec3(11, 22, 33));

        }

        // Global Rotation
        {
            WD_TEST.QUAT(child1.GetGlobalRotation(), wd.Quat.IdentityQuaternion());

            let nr = new wd.Quat();
            nr.SetFromAxisAndAngle(new wd.Vec3(0, 1, 0), wd.Angle.DegreeToRadian(30));

            child1.SetGlobalRotation(nr);
            WD_TEST.QUAT(child1.GetGlobalRotation(), nr);
        }

        // Global Scaling
        {
            WD_TEST.VEC3(child2.GetGlobalScaling(), new wd.Vec3(2 * 5, 3 * 5, 4 * 5));

            child2.SetGlobalScaling(new wd.Vec3(1, 2, 3));

            WD_TEST.VEC3(child2.GetGlobalScaling(), new wd.Vec3(1, 2, 3));
            WD_TEST.FLOAT(child2.GetLocalUniformScaling(), 1);
        }

        // Global Dirs
        {
            child1.SetGlobalRotation(wd.Quat.IdentityQuaternion());

            WD_TEST.VEC3(child1.GetGlobalDirForwards(), new wd.Vec3(1, 0, 0));
            WD_TEST.VEC3(child1.GetGlobalDirRight(), new wd.Vec3(0, 1, 0));
            WD_TEST.VEC3(child1.GetGlobalDirUp(), new wd.Vec3(0, 0, 1));

            let r = new wd.Quat();
            r.SetFromAxisAndAngle(new wd.Vec3(0, 0, 1), wd.Angle.DegreeToRadian(90));

            child1.SetGlobalRotation(r);

            WD_TEST.VEC3(child1.GetGlobalDirForwards(), new wd.Vec3(0, 1, 0));
            WD_TEST.VEC3(child1.GetGlobalDirRight(), new wd.Vec3(-1, 0, 0));
            WD_TEST.VEC3(child1.GetGlobalDirUp(), new wd.Vec3(0, 0, 1));
        }

        // Velocity
        {
            WD_TEST.VEC3(child1.GetVelocity(), wd.Vec3.ZeroVector());

            child1.SetVelocity(new wd.Vec3(1, 2, 3));
            WD_TEST.VEC3(child1.GetVelocity(), new wd.Vec3(1, 2, 3));
        }

        // Team ID
        {
            WD_TEST.FLOAT(child1.GetTeamID(), 0);
            child1.SetTeamID(11);
            WD_TEST.FLOAT(child1.GetTeamID(), 11);
        }

        // FindChildByName
        {
            let c = owner.FindChildByName("Child1_Child1", false);
            WD_TEST.BOOL(c == null);

            c = owner.FindChildByName("Child1_Child1", true);
            WD_TEST.BOOL(c != null);
            WD_TEST.BOOL(c.IsValid());
            WD_TEST.BOOL(c.GetName() == "Child1_Child1");
        }

        // FindChildByName
        {
            let c = owner.FindChildByPath("Child2_Child1");
            WD_TEST.BOOL(c == null);

            c = owner.FindChildByPath("Child2/Child2_Child1");
            WD_TEST.BOOL(c != null);
            WD_TEST.BOOL(c.IsValid());
            WD_TEST.BOOL(c.GetName() == "Child2_Child1");
        }

        // SearchForChildByNameSequence
        {
            let c = owner.SearchForChildByNameSequence("Child1_Child1/A");
            WD_TEST.BOOL(c != null && c.IsValid());
            WD_TEST.FLOAT(c.GetLocalUniformScaling(), 2);

            c = owner.SearchForChildWithComponentByNameSequence("Child2/A", wd.PointLightComponent);
            WD_TEST.BOOL(c != null && c.IsValid());
            WD_TEST.FLOAT(c.GetLocalUniformScaling(), 3);
        }

        // TryGetComponentOfBaseType
        {
            let sl = child1.TryGetComponentOfBaseType(wd.SpotLightComponent);
            WD_TEST.BOOL(sl != null && sl.IsValid());

            let pl = child1.TryGetComponentOfBaseTypeName<wd.SpotLightComponent>("wdPointLightComponent");
            WD_TEST.BOOL(pl != null && pl.IsValid());
        }

        // Tags
        {
            WD_TEST.BOOL(owner.HasAllTags("AutoColMesh"));
            WD_TEST.BOOL(owner.HasAllTags("CastShadow"));
            WD_TEST.BOOL(owner.HasAllTags("AutoColMesh", "CastShadow"));
            WD_TEST.BOOL(owner.HasAnyTags("AutoColMesh", "NOTAG"));
            WD_TEST.BOOL(owner.HasAnyTags("CastShadow", "NOTAG"));
            WD_TEST.BOOL(owner.HasAnyTags("AutoColMesh", "CastShadow"));

            owner.RemoveTags("CastShadow", "AutoColMesh");
            WD_TEST.BOOL(!owner.HasAnyTags("AutoColMesh", "CastShadow"));

            owner.AddTags("CastShadow", "TAG1");
            WD_TEST.BOOL(owner.HasAnyTags("AutoColMesh", "CastShadow"));
            WD_TEST.BOOL(!owner.HasAllTags("AutoColMesh", "CastShadow"));

            owner.SetTags("TAG");
            WD_TEST.BOOL(owner.HasAnyTags("TAG"));
            WD_TEST.BOOL(!owner.HasAnyTags("AutoColMesh", "CastShadow", "TAG1"));
        }

        // Global Key
        {
            let obj = wd.World.TryGetObjectWithGlobalKey("Tests");
            WD_TEST.BOOL(obj != null);
            WD_TEST.BOOL(obj.GetName() == "All Tests");

            this.GetOwner().SetGlobalKey("TestGameObjects");
            WD_TEST.BOOL(this.GetOwner().GetGlobalKey() == "TestGameObjects");
            let tgo = wd.World.TryGetObjectWithGlobalKey("TestGameObjects");
            WD_TEST.BOOL(tgo == this.GetOwner());
            this.GetOwner().SetGlobalKey("");
            let tgo2 = wd.World.TryGetObjectWithGlobalKey("TestGameObjects");
            WD_TEST.BOOL(tgo2 == null);
        }

        // GetChildren
        {
            WD_TEST.INT(this.GetOwner().GetChildCount(), 3);

            let children = this.GetOwner().GetChildren();
            WD_TEST.INT(this.GetOwner().GetChildCount(), children.length);

            this.GetOwner().DetachChild(children[0]);
            WD_TEST.INT(this.GetOwner().GetChildCount(), 2);
            WD_TEST.BOOL(children[0].GetParent() == null);

            children[0].SetParent(this.GetOwner());
            WD_TEST.INT(this.GetOwner().GetChildCount(), 3);
            WD_TEST.BOOL(children[0].GetParent() == this.GetOwner());

            this.GetOwner().DetachChild(children[2]);
            WD_TEST.BOOL(children[2].GetParent() == null);
            this.GetOwner().AddChild(children[2]);
            WD_TEST.BOOL(children[2].GetParent() == this.GetOwner());
        }
    }

    OnMsgGenericEvent(msg: wd.MsgGenericEvent): void {

        if (msg.Message == "TestGameObject") {

            this.ExecuteTests();

            msg.Message = "done";
        }
    }

}

