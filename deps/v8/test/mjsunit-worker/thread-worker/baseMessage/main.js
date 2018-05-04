"use strict";

var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js",
        "test/mjsunit-worker/thread-worker/baseMessage/worker.js");

worker.onmessage = function(env) {
    var data = env.data;
    if (data === "hello world") {
        worker.postMessage("This is a sample test");
    } else {
        assertEquals(data, "OK");
        worker.terminate();
        // exit();
    }
}

worker.postMessage("hello");
