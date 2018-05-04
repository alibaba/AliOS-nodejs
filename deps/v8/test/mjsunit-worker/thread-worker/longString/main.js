var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js",
        "test/mjsunit-worker/thread-worker/longString/worker.js");

var long_text = "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";

var result;
var exit = function() {
  assertEquals(result, "OK");
}

worker.onmessage = function(env) {
    if (env.data === "ERROR") {
        print("ERROR when worker receive data");
        result = env.data;
        print("Result:" + result);
        worker.terminate();
        exit();
    } else if (env.data === data.toString()) {
        result = "OK"
        print("Result:" + result);
        worker.terminate();
        exit();
    } else {
        print("ERROR on master");
        print("Receive data is " + env.data);
        print("+===========\n Real data is " + data.toString());
        worker.terminate();
        exit();
    }
}

var data = long_text;
worker.postMessage(data);
