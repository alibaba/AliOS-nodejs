postMessage("Starting worker");

// Set a global variable; should not be visible outside of the worker's
// context.
var c = 0;

onmessage = function(env) {
    var m = env.data;
    switch (c++) {
    case 0:
        if (m !== undefined) {
            throw new Error("undefined");
        }
        break;
    case 1:
        if (m !== null) {
            throw new Error("null");
        }
        break;
    case 2:
        if (m !== true) {
            throw new Error("true");
        }
        break;
    case 3:
        if (m !== false) {
            throw new Error("false");
        }
        break;
    case 4:
        if (m !== 100) {
            throw new Error("Number");
        }
        break;
    case 5:
        if (m !== "hi") {
            throw new Error("String");
        }
        break;
    case 6:
        if (JSON.stringify(m) !== "[4,true,\"bye\"]") {
            throw new Error("Array");
        }
        break;
    case 7:
        if (JSON.stringify(m) !== "{\"a\":1,\"b\":2.5,\"c\":\"three\"}") {
            console.error(JSON.stringify(m));
            throw new Error("Object");
        }
        break;
    case 8:
        var ab = m;
        var t = new Uint32Array(ab);
        if (ab.byteLength !== 16) {
            throw new Error("ArrayBuffer clone byteLength");
        }
        for (var i = 0; i < 4; ++i) {
            if (t[i] !== i) {
                throw new Error("ArrayBuffer clone value " + i);
            }
        }
        break;
    case 9:
        ab = m;
        t = new Uint32Array(ab);
        if (ab.byteLength !== 32) {
            throw new Error("ArrayBuffer transfer byteLength");
        }
        for (i = 0; i < 8; ++i) {
            if (t[i] !== i) {
                throw new Error("ArrayBuffer transfer value " + i);
            }
        }
        break;
    };

    if (c === 8) {
        postMessage("DONE");
    }
}
