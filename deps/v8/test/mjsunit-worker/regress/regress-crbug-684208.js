// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-684208.js");
	}
}
function foo() {
  var a = [1, 2.3, /*hole*/, 4.2];
  %_DeoptimizeNow();
  return a[2];
}
assertSame(undefined, foo());
assertSame(undefined, foo());
%OptimizeFunctionOnNextCall(foo)
assertSame(undefined, foo());
