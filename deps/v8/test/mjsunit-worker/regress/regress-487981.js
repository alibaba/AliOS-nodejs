// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags:  --allow-natives-syntax --stress-compaction

// To reliably reproduce the crash use --verify-heap --random-seed=-133185440

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-487981.js");
	}
}
function __f_2(o) {
  return o.field.b.x;
}

try {
  %OptimizeFunctionOnNextCall(__f_2);
  __v_1 = __f_2();
} catch(e) { }

function __f_3() { __f_3(/./.test()); };

try {
__f_3();
} catch(e) { }
