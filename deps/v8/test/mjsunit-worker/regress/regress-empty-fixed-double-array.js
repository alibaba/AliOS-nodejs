// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --enable-slow-asserts

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-empty-fixed-double-array.js");
	}
}
function f(a, x) {
  a.shift();
  a[0] = x;
}

f([1], 1.1);
f([1], 1.1);
%OptimizeFunctionOnNextCall(f);
f([1], 1.1);
