// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-686102.js");
	}
}
var a = [];
Object.freeze(a);
function foo() {
  return a.length;
}
assertEquals(0, foo());
assertEquals(0, foo());
%OptimizeFunctionOnNextCall(foo);
assertEquals(0, foo());
