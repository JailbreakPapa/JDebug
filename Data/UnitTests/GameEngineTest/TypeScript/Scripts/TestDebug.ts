import ns = require("TypeScript/ns")
import NS_TEST = require("./TestFramework")

export class TestDebug extends ns.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {
        ns.TypescriptComponent.RegisterMessageHandler(ns.MsgGenericEvent, "OnMsgGenericEvent");
    }

    ExecuteTests(): void {

        
        ns.Debug.RegisterCVar_Boolean("test.bool", true, "bool");
        NS_TEST.BOOL(ns.Debug.ReadCVar_Boolean("test.bool") == true);
        ns.Debug.WriteCVar_Boolean("test.bool", false);
        NS_TEST.BOOL(ns.Debug.ReadCVar_Boolean("test.bool") == false);
        
        ns.Debug.RegisterCVar_Int("test.int", 12, "int");
        NS_TEST.BOOL(ns.Debug.ReadCVar_Int("test.int") == 12);
        ns.Debug.WriteCVar_Int("test.int", -12);
        NS_TEST.BOOL(ns.Debug.ReadCVar_Int("test.int") == -12);
        
        ns.Debug.RegisterCVar_Float("test.float", 19, "float");
        NS_TEST.BOOL(ns.Debug.ReadCVar_Float("test.float") == 19);
        ns.Debug.WriteCVar_Float("test.float", -19);
        NS_TEST.BOOL(ns.Debug.ReadCVar_Float("test.float") == -19);

        ns.Debug.RegisterCVar_String("test.string", "hello", "string");
        NS_TEST.BOOL(ns.Debug.ReadCVar_String("test.string") == "hello");
        ns.Debug.WriteCVar_String("test.string", "world");
        NS_TEST.BOOL(ns.Debug.ReadCVar_String("test.string") == "world");
    }

    OnMsgGenericEvent(msg: ns.MsgGenericEvent): void {

        if (msg.Message == "TestDebug") {

            this.ExecuteTests();

            msg.Message = "done";
        }
    }

}

