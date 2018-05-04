// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-656037.js");
	}
}
function foo(a) {
  return a.push(true);
}

var a = [];
assertEquals(1, foo(a));
assertEquals(2, foo(a));
%OptimizeFunctionOnNextCall(foo);
assertEquals(3, foo(a));
