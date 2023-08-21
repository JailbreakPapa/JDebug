import wd = require("TypeScript/wd")

export class Prefab extends wd.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    NumberVar: number = 11;
    BoolVar: boolean = true;
    StringVar: string = "Hello";
    Vec3Var: wd.Vec3 = new wd.Vec3(1, 2, 3);
    ColorVar: wd.Color = new wd.Color(0.768151, 0.142913, 0.001891, 1);
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {

    }
}

