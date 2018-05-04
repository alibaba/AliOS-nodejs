// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-685050.js");
	}
}
function bar(a) {
  a[0] = 0;
  a[1] = 0;
}

var a = new Int32Array(2);
bar([1, 2, 3]);
function foo() {
  bar([1, 2, 3]);
  bar(a);
}
%OptimizeFunctionOnNextCall(foo);
foo();
