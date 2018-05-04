// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --no-inline-new

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/ignition/regress-597565-double-to-object-transition.js");
	}
}
function __f_2(b, value) {
  b[1] = value;
}
function __f_9() {
 var arr = [1.5, 0, 0];
  // Call with a double, so the expected element type is double.
  __f_2(1.5);
  // Call with an object, which triggers transition from FAST_double
  // to Object for the elements type.
  __f_2(arr);
}
__f_9();
