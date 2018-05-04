function createArrayBuffer(byteLength) {
    var ab = new ArrayBuffer(byteLength);
    var t = new Uint32Array(ab);
    for (var i = 0; i < byteLength / 4; ++i) {
        t[i] = i;
    }
    return ab;
}

var result;
var exit = function() {
  assertEquals(result, "OK");
}

w = new ThreadWorker("test/mjsunit-worker/mjsunit.js",
       "test/mjsunit-worker/thread-worker/objectMessage/worker.js");

w.onmessage = function(env) {
    var data = env.data;
    print("main get message:" + data);
    if (data === "Starting worker") {
        w.postMessage(undefined);
        w.postMessage(null);
        w.postMessage(true);
        w.postMessage(false);
        w.postMessage(100);
        w.postMessage("hi");
        w.postMessage([4, true, "bye"]);
        w.postMessage({a: 1, b: 2.5, c: "three"});
        // Clone ArrayBuffer
        var ab1 = createArrayBuffer(16);
        w.postMessage(ab1);
        assertEquals(16, ab1.byteLength); // ArrayBuffer should not be neutered.
    } else if (data === "DONE") {
        result = "OK";
        w.terminate();
        exit();
    }
}
