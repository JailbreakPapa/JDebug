import wd = require("TypeScript/wd")
import WD_TEST = require("./TestFramework")

export class TestDebug extends wd.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {
        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgGenericEvent, "OnMsgGenericEvent");
    }

    ExecuteTests(): void {


        wd.Debug.RegisterCVar_Boolean("test.bool", true, "bool");
        WD_TEST.BOOL(wd.Debug.ReadCVar_Boolean("test.bool") == true);
        wd.Debug.WriteCVar_Boolean("test.bool", false);
        WD_TEST.BOOL(wd.Debug.ReadCVar_Boolean("test.bool") == false);

        wd.Debug.RegisterCVar_Int("test.int", 12, "int");
        WD_TEST.BOOL(wd.Debug.ReadCVar_Int("test.int") == 12);
        wd.Debug.WriteCVar_Int("test.int", -12);
        WD_TEST.BOOL(wd.Debug.ReadCVar_Int("test.int") == -12);

        wd.Debug.RegisterCVar_Float("test.float", 19, "float");
        WD_TEST.BOOL(wd.Debug.ReadCVar_Float("test.float") == 19);
        wd.Debug.WriteCVar_Float("test.float", -19);
        WD_TEST.BOOL(wd.Debug.ReadCVar_Float("test.float") == -19);

        wd.Debug.RegisterCVar_String("test.string", "hello", "string");
        WD_TEST.BOOL(wd.Debug.ReadCVar_String("test.string") == "hello");
        wd.Debug.WriteCVar_String("test.string", "world");
        WD_TEST.BOOL(wd.Debug.ReadCVar_String("test.string") == "world");
    }

    OnMsgGenericEvent(msg: wd.MsgGenericEvent): void {

        if (msg.Message == "TestDebug") {

            this.ExecuteTests();

            msg.Message = "done";
        }
    }

}

