// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-694416.js");
	}
}
var good = 23;
var boom = 42;

function foo(name) {
  return this[name];
}

assertEquals(23, foo('good'));
assertEquals(23, foo('good'));
%OptimizeFunctionOnNextCall(foo);
assertEquals(42, foo('boom'));
