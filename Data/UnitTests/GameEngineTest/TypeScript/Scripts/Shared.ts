import ns = require("TypeScript/ns")

export class MyMessage extends ns.Message {
    NS_DECLARE_MESSAGE_TYPE;

    text: string = "hello";
}

export class MyMessage2 extends ns.Message {
    NS_DECLARE_MESSAGE_TYPE;

    value: number = 0;
}

