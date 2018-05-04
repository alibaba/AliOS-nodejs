// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-344186.js");
	}
}
var dummy = new Int32Array(100);
var array = new Int32Array(128);
function fun(base) {
  array[base - 95] = 1;
  array[base - 99] = 2;
  array[base + 4] = 3;
}
fun(100);
%OptimizeFunctionOnNextCall(fun);
fun(0);

for (var i = 0; i < dummy.length; i++) {
  assertEquals(0, dummy[i]);
}
