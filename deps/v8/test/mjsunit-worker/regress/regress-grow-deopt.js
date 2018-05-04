// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-grow-deopt.js");
	}
}
function f(a, v) {
  a[a.length] = v;
}

var a = [1.4];
f(a, 1);
f(a, 2);
%OptimizeFunctionOnNextCall(f);
f(a, {});
assertEquals(4, a.length);
