import ns = require("TypeScript/ns")
import NS_TEST = require("./TestFramework")

export class TestComponent extends ns.TypescriptComponent {

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

        let mesh = owner.TryGetComponentOfBaseType(ns.MeshComponent);
        let text = owner.TryGetComponentOfBaseType(ns.DebugTextComponent);

        // IsValid
        {
            NS_TEST.BOOL(mesh != null && mesh.IsValid());
            NS_TEST.BOOL(text != null && text.IsValid());
        }

        // GetOWner
        {
            NS_TEST.BOOL(mesh.GetOwner() == owner);
            NS_TEST.BOOL(text.GetOwner() == owner);
        }

        // Active Flag / Active State
        {
            NS_TEST.BOOL(mesh.IsActive());
            NS_TEST.BOOL(mesh.IsActiveAndInitialized());
            NS_TEST.BOOL(mesh.IsActiveAndSimulating());
            
            NS_TEST.BOOL(!text.GetActiveFlag());
            NS_TEST.BOOL(!text.IsActive());
            NS_TEST.BOOL(!text.IsActiveAndInitialized());
            
            text.SetActiveFlag(true);
            NS_TEST.BOOL(text.GetActiveFlag());
            NS_TEST.BOOL(text.IsActive());
            NS_TEST.BOOL(text.IsActiveAndInitialized());
            
            mesh.SetActiveFlag(false);
            NS_TEST.BOOL(!mesh.GetActiveFlag());
            NS_TEST.BOOL(!mesh.IsActive());
            NS_TEST.BOOL(!mesh.IsActiveAndInitialized());
            NS_TEST.BOOL(!mesh.IsActiveAndSimulating());
        }

        // GetUniqueID
        {
            // nsInvalidIndex
            NS_TEST.INT(mesh.GetUniqueID(), 4294967295);
            NS_TEST.INT(text.GetUniqueID(), 4294967295);
        }

        // TryGetScriptComponent
        {
            let sc = this.GetOwner().TryGetScriptComponent("TestComponent");

            NS_TEST.BOOL(sc == this);
        }

        // interact with C++ components
        {
            let c = ns.World.CreateComponent(this.GetOwner(), ns.MoveToComponent);

            // execute function
            c.SetTargetPosition(new ns.Vec3(1, 2, 3));

            // get/set properties
            c.TranslationSpeed = 23;
            NS_TEST.FLOAT(c.TranslationSpeed, 23);
            
            c.TranslationSpeed = 17;
            NS_TEST.FLOAT(c.TranslationSpeed, 17);
        }
    }

    OnMsgGenericEvent(msg: ns.MsgGenericEvent): void {

        if (msg.Message == "TestComponent") {

            this.ExecuteTests();

            msg.Message = "done";
        }
    }

}

