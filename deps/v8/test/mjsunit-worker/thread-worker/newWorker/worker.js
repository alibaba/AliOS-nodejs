onmessage = function(env) {
    print(env.data);
    if (env.data === "hello") {
        postMessage("OK");
    }
}
