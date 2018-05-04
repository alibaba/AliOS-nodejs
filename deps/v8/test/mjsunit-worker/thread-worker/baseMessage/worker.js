
onmessage = function(env) {
    var data = env.data;
    print("get message :" + data);
    if (data === "hello") {
        postMessage("hello world");
    } else if (data === "This is a sample test") {
        postMessage("OK");
    } else {
        postMessage("Fail");
    }
}
