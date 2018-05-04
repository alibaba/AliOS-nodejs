// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/regress/regress-crbug-448730.js");
	}
}
function bar() {}
bar({ a: new Proxy({}, {}) });
function foo(x) { x.a.b == ""; }
var x = {a: {b: "" }};
foo(x);
foo(x);
%OptimizeFunctionOnNextCall(foo);
foo(x);
