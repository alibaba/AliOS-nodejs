// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --turbo-escape

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-escape-analysis-indirect.js");
	}
}
function f(apply) {
  var value = 23;
  apply(function bogeyman() { value = 42 });
  return value;
}
function apply(fun) { fun() }
assertEquals(42, f(apply));
assertEquals(42, f(apply));
%NeverOptimizeFunction(apply);
%OptimizeFunctionOnNextCall(f);
assertEquals(42, f(apply));
