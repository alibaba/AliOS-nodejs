function doWorker() {
    var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js",
            "test/mjsunit-worker/thread-worker/newWorker/worker.js");

    worker.onmessage = function(env) {
        assertEquals(env.data, "OK");
        worker.terminate();
    }
    worker.postMessage("hello");
}

for (var i = 0; i < 5; i++) {
    doWorker();
}
