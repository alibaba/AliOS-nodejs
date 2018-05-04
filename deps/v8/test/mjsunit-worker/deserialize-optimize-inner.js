// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --cache=code --no-lazy --opt

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//deserialize-optimize-inner.js");
	}
}
function f(x, y) { return x + y; }

assertEquals(1, f(0, 1));
assertEquals(5, f(2, 3));
%OptimizeFunctionOnNextCall(f);
assertEquals(9, f(4, 5));
assertOptimized(f);
