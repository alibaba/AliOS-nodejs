onmessage = function(env) {
    print("worker get:" + env.data);
    if (env.data.indexOf("hello") !== -1) {
        postMessage("hello world");
    } else if (env.data === "This is a sample test") {
        postMessage("OK");
    } else {
        postMessage("Fail");
    }
}
