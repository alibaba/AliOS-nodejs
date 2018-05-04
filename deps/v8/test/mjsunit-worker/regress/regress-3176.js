// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --opt

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-3176.js");
	}
}
function foo(a) {
  var sum = 0;
  for (var i = 0; i < 10; i++) {
    sum += a[i];

    if (i > 6) {
      sum -= a[i - 4];
      sum -= a[i - 5];
    }
  }
  return sum;
}

var a = new Int32Array(10);

foo(a);
foo(a);
%OptimizeFunctionOnNextCall(foo);
foo(a);
%OptimizeFunctionOnNextCall(foo);
foo(a);
assertOptimized(foo);
