// Flags: --harmony-sharedarraybuffer
var sab = new SharedArrayBuffer(1024);
var array = new Uint32Array(sab);

array[0] = 10000;
array[1] = 20000;
array[2] = 0;


function doWorkerPostMessageBenchmark() {
      this.worker_p = new ThreadWorker("./worker_locker_p.js");
      this.worker_c = new ThreadWorker("./worker_locker_c.js");
     // the sharedarraybuffer should not be put in the transferable list, but this node version has a bug which handle sab as ArrayBuffer.so ,this bug
     // make us put the sab into transferable list now.
     this.worker_p.postMessage({heap:sab});
     this.worker_c.postMessage({heap:sab});
     print("Master end...");
}

doWorkerPostMessageBenchmark();

print("run master end.....");
