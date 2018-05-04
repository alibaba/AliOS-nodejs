// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/eager-deopt-simple.js");
	}
}
function g(a, b, c) {
  return a + b + c;
}

function f() {
  return g(1, (%_DeoptimizeNow(), 2), 3);
}

f();
f();
%OptimizeFunctionOnNextCall(f);
assertEquals(6, f());
