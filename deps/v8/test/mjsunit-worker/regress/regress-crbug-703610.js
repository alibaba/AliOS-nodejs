// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-703610.js");
	}
}
function fun() {};
fun.prototype = 42;
new fun();
function f() {
  return fun.prototype;
}
assertEquals(42, f());
assertEquals(42, f());
%OptimizeFunctionOnNextCall(f);
assertEquals(42, f());
