// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-699282.js");
	}
}
var v = 1;
function foo() { return Math.floor(-v / 125); }
assertEquals(-1, foo());
%OptimizeFunctionOnNextCall(foo);
assertEquals(-1, foo());
