// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-4389-5.js");
	}
}
function foo(x) { Math.abs(x); }
foo(1);
foo(2);
%OptimizeFunctionOnNextCall(foo);
assertThrows(function() { foo(Symbol()) }, TypeError);
