import wd = require("TypeScript/wd")
import WD_TEST = require("./TestFramework")

export class TestComponent extends wd.TypescriptComponent {

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

        let mesh = owner.TryGetComponentOfBaseType(wd.MeshComponent);
        let text = owner.TryGetComponentOfBaseType(wd.DebugTextComponent);

        // IsValid
        {
            WD_TEST.BOOL(mesh != null && mesh.IsValid());
            WD_TEST.BOOL(text != null && text.IsValid());
        }

        // GetOWner
        {
            WD_TEST.BOOL(mesh.GetOwner() == owner);
            WD_TEST.BOOL(text.GetOwner() == owner);
        }

        // Active Flag / Active State
        {
            WD_TEST.BOOL(mesh.IsActive());
            WD_TEST.BOOL(mesh.IsActiveAndInitialized());
            WD_TEST.BOOL(mesh.IsActiveAndSimulating());

            WD_TEST.BOOL(!text.GetActiveFlag());
            WD_TEST.BOOL(!text.IsActive());
            WD_TEST.BOOL(!text.IsActiveAndInitialized());

            text.SetActiveFlag(true);
            WD_TEST.BOOL(text.GetActiveFlag());
            WD_TEST.BOOL(text.IsActive());
            WD_TEST.BOOL(text.IsActiveAndInitialized());

            mesh.SetActiveFlag(false);
            WD_TEST.BOOL(!mesh.GetActiveFlag());
            WD_TEST.BOOL(!mesh.IsActive());
            WD_TEST.BOOL(!mesh.IsActiveAndInitialized());
            WD_TEST.BOOL(!mesh.IsActiveAndSimulating());
        }

        // GetUniqueID
        {
            // wdInvalidIndex
            WD_TEST.INT(mesh.GetUniqueID(), 4294967295);
            WD_TEST.INT(text.GetUniqueID(), 4294967295);
        }

        // TryGetScriptComponent
        {
            let sc = this.GetOwner().TryGetScriptComponent("TestComponent");

            WD_TEST.BOOL(sc == this);
        }

        // interact with C++ components
        {
            let c = wd.World.CreateComponent(this.GetOwner(), wd.MoveToComponent);

            // execute function
            c.SetTargetPosition(new wd.Vec3(1, 2, 3));

            // get/set properties
            c.TranslationSpeed = 23;
            WD_TEST.FLOAT(c.TranslationSpeed, 23);

            c.TranslationSpeed = 17;
            WD_TEST.FLOAT(c.TranslationSpeed, 17);
        }
    }

    OnMsgGenericEvent(msg: wd.MsgGenericEvent): void {

        if (msg.Message == "TestComponent") {

            this.ExecuteTests();

            msg.Message = "done";
        }
    }

}

