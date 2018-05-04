// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

// Test push double as tagged.
if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-force-constant-representation.js");
	}
}
var a = [{}];
function f(a) {
  a.push(Infinity);
}

f(a);
f(a);
f(a);
%OptimizeFunctionOnNextCall(f);
f(a);
assertEquals([{}, Infinity, Infinity, Infinity, Infinity], a);
