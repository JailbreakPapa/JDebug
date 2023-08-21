import wd = require("TypeScript/wd")

export class MsgSwitchMonitor extends wd.Message {

    WD_DECLARE_MESSAGE_TYPE;

    renderTarget: string;
    screenMaterial: string;
}

export class Monitor extends wd.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {

        wd.TypescriptComponent.RegisterMessageHandler(MsgSwitchMonitor, "OnMsgSwitchMonitor");
    }

    OnMsgSwitchMonitor(msg: MsgSwitchMonitor): void {

        let owner = this.GetOwner();
        let display = owner.FindChildByName("Display");

        let mat = new wd.MsgSetMeshMaterial();
        mat.MaterialSlot = 0;
        mat.Material = msg.screenMaterial;

        display.SendMessage(mat);

        let activator = display.TryGetComponentOfBaseType(wd.RenderTargetActivatorComponent);
        activator.RenderTarget = msg.renderTarget;
    }
}

