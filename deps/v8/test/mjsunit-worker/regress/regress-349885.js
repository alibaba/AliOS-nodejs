// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

// The bug 349885

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-349885.js");
	}
}
function foo(a) {
  a[292755462] = new Object();
}
foo(new Array(5));
foo(new Array(5));
%OptimizeFunctionOnNextCall(foo);
foo(new Array(10));
