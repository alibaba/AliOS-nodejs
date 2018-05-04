// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-667689.js");
	}
}
function foo() {}
foo.__defineGetter__(undefined, function() {})

function bar() {}
function baz(x) { return x instanceof bar };
%OptimizeFunctionOnNextCall(baz);
baz();
Object.setPrototypeOf(bar, null);
bar[Symbol.hasInstance] = function() { return true };
assertTrue(baz());
