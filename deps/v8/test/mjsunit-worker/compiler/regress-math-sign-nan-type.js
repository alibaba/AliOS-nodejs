// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-math-sign-nan-type.js");
	}
}
function f(a) {
  return Math.sign(+a) < 2;
}

f(NaN);
f(NaN);
%OptimizeFunctionOnNextCall(f);
assertFalse(f(NaN));
