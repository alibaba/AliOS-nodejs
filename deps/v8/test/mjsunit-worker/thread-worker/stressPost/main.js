var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js",
        "test/mjsunit-worker/thread-worker/stressPost/worker.js");


var result;
var exit = function() {
  assertEquals(result, "OK");
}

var count = 0;

worker.onmessage = function(env) {
    if (env.data === "hello") {
        count = count + 1;
        return;
    } else if (env.data === "end") {
        if (count !== 1000) {
            print("Real is " + count);
            print("Excepte is : 1000");
            print("Result:Fail");
        } else {
            print("Result:OK");
        }
    } else if (env.data === "OK") {
        result = "OK";
        print("Result:" + result);
    } else {
        result = "Fail";
        print("Result:" + result);
    }

    worker.terminate();
    exit();
}

for (var i = 0; i < 1000; i++) {
    worker.postMessage("hello");
}

worker.postMessage("end");
