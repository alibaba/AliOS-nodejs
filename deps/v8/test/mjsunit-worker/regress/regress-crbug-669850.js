// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-669850.js");
	}
}
eval('function f(a) { return [' + new Array(1 << 17) + ',a] }');
assertEquals(23, f(23)[1 << 17]);
assertEquals(42, f(42)[1 << 17]);
%OptimizeFunctionOnNextCall(f);
assertEquals(65, f(65)[1 << 17]);
