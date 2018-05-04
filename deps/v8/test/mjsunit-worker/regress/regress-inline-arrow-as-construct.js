// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

// This tests that inlining a constructor call to a function which cannot be
// used as a constructor (e.g. arrow function) still throws correctly.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-inline-arrow-as-construct.js");
	}
}
var g = () => {}

function f() {
  return new g();
}

assertThrows(f);
assertThrows(f);
%OptimizeFunctionOnNextCall(f);
assertThrows(f);
