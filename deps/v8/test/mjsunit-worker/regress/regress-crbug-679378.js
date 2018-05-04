// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-679378.js");
	}
}
var x = {};
x.__defineGetter__('0', () => 0);
x.a = {v: 1.51};

var y = {};
y.a = {u:"OK"};

function foo(o) { return o.a.u; }
foo(y);
foo(y);
foo(x);
%OptimizeFunctionOnNextCall(foo);
%DebugPrint(foo(x));
