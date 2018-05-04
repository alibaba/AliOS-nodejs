// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-503578.js");
	}
}
if (this.Worker) {
  function __f_0(byteLength) {
    var __v_1 = new ArrayBuffer(byteLength);
    var __v_5 = new Uint32Array(__v_1);
    return __v_5;
  }
  var __v_6 = new Worker('onmessage = function() {}');
  var __v_3 = __f_0(16);
  __v_6.postMessage(__v_3);
}
