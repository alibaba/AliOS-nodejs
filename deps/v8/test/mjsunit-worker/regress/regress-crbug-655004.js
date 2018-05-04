// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-655004.js");
	}
}
function foo(a) {
  a.x = 0;
  if (a.x === 0) a[1] = 0.1;
  a.x = {};
}
foo(new Array(1));
foo(new Array(1));
%OptimizeFunctionOnNextCall(foo);
foo(new Array(1));
