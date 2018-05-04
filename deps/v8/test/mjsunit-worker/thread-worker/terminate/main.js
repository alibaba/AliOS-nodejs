var result;
var exit = function() {
  assertEquals(result, "OK");
}

function doTerminate(i) {
    var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js",
        "test/mjsunit-worker/thread-worker/terminate/worker.js");

    worker.onmessage = function(env) {
        if (env.data === "hello world") {
            worker.postMessage("This is a sample test");
        } else if (env.data === "OK") {
            result = "OK";
            print("Result:" + result);
            worker.terminate();
            exit();
        } else {
            result = "Fail";
            print("Result:" + result);
            worker.terminate();
            exit();
        }
    }
    worker.postMessage("hello:" + i);
    // worker.terminate();
}

for (var i = 0; i < 50; i++) {
    doTerminate(i);
}
