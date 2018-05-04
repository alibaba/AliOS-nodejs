// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-407946.js");
	}
}
function f(n) { return [0].indexOf((n - n) + 0); }

assertEquals(0, f(.1));
assertEquals(0, f(.1));
%OptimizeFunctionOnNextCall(f);
assertEquals(0, f(.1));
