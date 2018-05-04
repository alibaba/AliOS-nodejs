// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/deopt-bool.js");
	}
}
function foo(a, b) {
  var passed = a == 3;
  if (passed) {
    if (passed) {
      passed = b == 4;
    }
  }
  %DeoptimizeFunction(foo);
  return passed;
}

assertTrue(foo(3, 4));
assertTrue(foo(3, 4));
assertFalse(foo(3.1, 4));
assertFalse(foo(3, 4.1));

%OptimizeFunctionOnNextCall(foo);

assertTrue(foo(3, 4));
assertTrue(foo(3, 4));
assertFalse(foo(3.1, 4));
assertFalse(foo(3, 4.1));
