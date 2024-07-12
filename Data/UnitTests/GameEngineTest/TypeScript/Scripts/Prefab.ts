import ns = require("TypeScript/ns")

export class Prefab extends ns.TypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    NumberVar: number = 11;
    BoolVar: boolean = true;
    StringVar: string = "Hello";
    Vec3Var: ns.Vec3 = new ns.Vec3(1, 2, 3);
    ColorVar: ns.Color = new ns.Color(0.768151, 0.142913, 0.001891, 1);
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {

    }
}

