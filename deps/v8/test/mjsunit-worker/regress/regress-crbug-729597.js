// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --verify-heap

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-729597.js");
	}
}
function __f_3(f) {
  arguments.__defineGetter__('length', f);
  return arguments;
}
function __f_4() { return "boom"; }

__v_4 = [];
__v_13 = "";

for (var i = 0; i < 12800; ++i) {
  __v_13 +=  __v_4.__proto__ = __f_3(__f_4);
}
