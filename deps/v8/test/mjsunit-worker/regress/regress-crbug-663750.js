// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-663750.js");
	}
}
var v = 0;
function foo(a) {
  v = a;
}
this.x = 0;
delete x;

foo();
foo();
%OptimizeFunctionOnNextCall(foo);
foo();
assertEquals(undefined, v);

Object.freeze(this);

foo(4);
foo(5);
%OptimizeFunctionOnNextCall(foo);
foo(6);
assertEquals(undefined, v);
