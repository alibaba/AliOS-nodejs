// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-504729.js");
	}
}
if (this.Worker) {
  Function.prototype.toString = "foo";
  function __f_7() {}
  assertThrows(function() { var __v_5 = new Worker(__f_7.toString()); });
}
