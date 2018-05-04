// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-631318-4.js");
	}
}
function foo(x) { return x <= x; }
foo(1);
foo(2);

function bar(x) { foo(x); }
%OptimizeFunctionOnNextCall(bar);

assertThrows(() => bar(Symbol.toPrimitive));
