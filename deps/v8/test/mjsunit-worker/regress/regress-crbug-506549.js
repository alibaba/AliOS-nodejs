// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-506549.js");
	}
}
if (this.Worker) {
  var __v_5 = {};
  __v_5.__defineGetter__('byteLength', function() {foo();});
  var __v_8 = new Worker('onmessage = function() {};');
  assertThrows(function() { __v_8.postMessage(__v_5); });
}
