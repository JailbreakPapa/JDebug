import wd = require("TypeScript/wd")

export class MyMessage extends wd.Message {
    WD_DECLARE_MESSAGE_TYPE;

    text: string = "hello";
}

export class MyMessage2 extends wd.Message {
    WD_DECLARE_MESSAGE_TYPE;

    value: number = 0;
}

