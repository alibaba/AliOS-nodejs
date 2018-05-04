var count = 0;
onmessage = function(env) {
    if (env.data === "hello") {
        count = count + 1;
    } else if (env.data === "end") {
        if (count !== 1000) {
            print("Real is : " + count);
            print("Excepte is : 1000");
            print("Result: Fail");
            postMessage("Fail");
        } else {
            postMessage("OK");
        }
    }
}
