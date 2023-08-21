import wd = require("TypeScript/wd")

declare function wdTestFailure(file: string, line: number, func: string, msg: string): void;

function TestFailed(msg: string) {
    let stack = (new Error).stack;
    let str = stack.split("\n")[3];
    let file = str.split(":")[0].split("(")[1];
    file = file.replace(".ts", ".js");
    let line = parseInt(str.split(":")[1]);
    let func = "";

    wdTestFailure(file, line, func, msg);
}

export function BOOL(condition: boolean) {

    if (!condition) {
        TestFailed("BOOL condition is false");
    }
}

export function INT(i1: number, i2: number) {

    if (i1 != i2) {
        TestFailed(i1 + " does not equal " + i2);
    }
}

export function FLOAT(f1: number, f2: number, epsilon: number = 0.001) {

    if (!wd.Utils.IsNumberEqual(f1, f2, epsilon)) {
        TestFailed(f1 + " does not equal " + f2);
    }
}

export function VEC2(v1: wd.Vec2, v2: wd.Vec2, epsilon: number = 0.001) {

    if (!v1.IsEqual(v2, epsilon)) {
        TestFailed("(" + v1.x + ", " + v1.y + ") does not equal (" + v2.x + ", " + v2.y + ")");
    }
}

export function VEC3(v1: wd.Vec3, v2: wd.Vec3, epsilon: number = 0.001) {

    if (!v1.IsEqual(v2, epsilon)) {
        TestFailed("(" + v1.x + ", " + v1.y + ", " + v1.z + ") does not equal (" + v2.x + ", " + v2.y + ", " + v2.z + ")");
    }
}

export function QUAT(q1: wd.Quat, q2: wd.Quat, epsilon: number = 0.001) {

    if (!q1.IsEqualRotation(q2, epsilon)) {
        TestFailed("(" + q1.x + ", " + q1.y + ", " + q1.z + ", " + q1.w + ") does not equal (" + q2.x + ", " + q2.y + ", " + q2.z + ", " + q2.w + ")");
    }
}

export function ARRAY(N: number, v1: number[], v2: number[], epsilon: number = 0.001): void {

    for (let i = 0; i < N; ++i) {

        if (!wd.Utils.IsNumberEqual(v1[i], v2[i], epsilon)) {

            TestFailed(v1[i] + " does not equal " + v2[i]);
        }
    }

}

export function COLOR(c1: wd.Color, c2: wd.Color, epsilon: number = 0.001) {

    if (!c1.IsEqualRGBA(c2, epsilon)) {
        TestFailed("(" + c1.r + ", " + c1.g + ", " + c1.b + ", "  + c1.a + ") does not equal (" + c2.r + ", " + c2.g + ", " + c2.b + ", "  + c2.a + ")");
    }
}