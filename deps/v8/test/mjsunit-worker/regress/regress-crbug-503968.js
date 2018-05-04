// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-503968.js");
	}
}
if (this.Worker) {
  function __f_0() { this.s = new Object(); }
  function __f_1() {
    this.l = [new __f_0, new __f_0];
  }
  __v_6 = new __f_1;
  var __v_9 = new Worker('');
  __v_9.postMessage(__v_6);
}
