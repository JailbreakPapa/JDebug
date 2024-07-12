import ns = require("TypeScript/ns")
import NS_TEST = require("./TestFramework")
import shared = require("./Shared")
import helper = require("./HelperComponent")

export class TestMessaging extends ns.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {
        ns.TypescriptComponent.RegisterMessageHandler(ns.MsgGenericEvent, "OnMsgGenericEvent");
        ns.TypescriptComponent.RegisterMessageHandler(shared.MyMessage, "OnMyMessage");
        ns.TypescriptComponent.RegisterMessageHandler(shared.MyMessage2, "OnMyMessage2");
    }

    step: number = 0;
    msgCount: number = 0;
    gotEvent: boolean = false;

    ExecuteTests(): boolean {

        if (this.step == 0) {
            let m = new shared.MyMessage();

            m.text = "hello 1";
            this.SendMessage(m, true);
            NS_TEST.BOOL(m.text == "Got: hello 1");

            m.text = "hello 2";
            this.SendMessage(m, false);
            NS_TEST.BOOL(m.text == "Got: hello 2");

            m.text = "hello 3";
            this.GetOwner().SendMessage(m, true);
            NS_TEST.BOOL(m.text == "Got: hello 3");

            m.text = "hello 4";
            this.GetOwner().GetParent().SendMessageRecursive(m, true);
            NS_TEST.BOOL(m.text == "Got: hello 4");

            return true;
        }

        if (this.step == 1) {
            let m = new shared.MyMessage2;
            m.value = 1;

            this.PostMessage(m);

            NS_TEST.INT(this.msgCount, 0);

            return true;
        }

        if (this.step == 2) {

            NS_TEST.INT(this.msgCount, 1);

            let m = new shared.MyMessage2;
            m.value = 1;

            this.GetOwner().PostMessage(m);

            NS_TEST.INT(this.msgCount, 1);

            return true;
        }

        if (this.step == 3) {

            NS_TEST.INT(this.msgCount, 2);

            let m = new shared.MyMessage2;
            m.value = 1;

            this.GetOwner().GetParent().PostMessage(m);

            NS_TEST.INT(this.msgCount, 2);

            return true;
        }

        if (this.step == 4) {

            NS_TEST.INT(this.msgCount, 2);

            let m = new shared.MyMessage2;
            m.value = 1;

            this.GetOwner().GetParent().PostMessageRecursive(m);

            NS_TEST.INT(this.msgCount, 2);

            return true;
        }

        if (this.step == 5) {

            NS_TEST.INT(this.msgCount, 3);

            let children = this.GetOwner().GetChildren();
            NS_TEST.INT(children.length, 1);

            let hc: helper.HelperComponent = children[0].TryGetScriptComponent("HelperComponent");
            NS_TEST.BOOL(hc != null);

            NS_TEST.BOOL(!this.gotEvent);

            let te = new ns.MsgGenericEvent;
            te.Message = "Event1";

            hc.SendMessage(te);

            NS_TEST.BOOL(this.gotEvent);
            this.gotEvent = false;

            hc.RaiseEvent("e1");
            NS_TEST.BOOL(this.gotEvent);

            return true;
        }

        if (this.step == 6) {

            NS_TEST.INT(this.msgCount, 3);

            return true;
        }

        return false;
    }

    OnMyMessage(msg: shared.MyMessage) {

        msg.text = "Got: " + msg.text;
    }

    OnMyMessage2(msg: shared.MyMessage2) {
        this.msgCount += msg.value;
    }

    OnMsgGenericEvent(msg: ns.MsgGenericEvent): void {

        if (msg.Message == "TestMessaging") {


            if (this.ExecuteTests()) {
                msg.Message = "repeat";
            }
            else {

                NS_TEST.INT(this.msgCount, 3);

                msg.Message = "done";
            }

            this.step += 1;
        }

        if (msg.Message == "e1") {
            this.gotEvent = true;
        }
    }
}

