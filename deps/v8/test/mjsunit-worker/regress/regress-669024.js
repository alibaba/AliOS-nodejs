// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-669024.js");
	}
}
function h(y) { return y.u; }

function g() { return h.apply(0, arguments); }

function f(x) {
  var o = { u : x };
  return g(o);
}

f(42);
f(0.1);

%OptimizeFunctionOnNextCall(f);

assertEquals(undefined, f(undefined));
