// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-518747.js");
	}
}
if (this.Worker) {
  Worker.prototype = 12;
  var __v_6 = new Worker('');
  __v_6.postMessage([]);
}
