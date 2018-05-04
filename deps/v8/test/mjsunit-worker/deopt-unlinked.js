// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --opt --no-always-opt --no-stress-fullcodegen

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//deopt-unlinked.js");
	}
}
function foo() {}

assertEquals(0, %GetOptimizationCount(foo));
assertEquals(0, %GetDeoptCount(foo));

foo();
foo();
%OptimizeFunctionOnNextCall(foo);
foo();

assertOptimized(foo);
assertEquals(1, %GetOptimizationCount(foo));
assertEquals(0, %GetDeoptCount(foo));

// Unlink the function.
%DeoptimizeFunction(foo);

assertUnoptimized(foo);
assertEquals(1, %GetOptimizationCount(foo));
assertEquals(1, %GetDeoptCount(foo));

foo();

assertUnoptimized(foo);
assertEquals(1, %GetOptimizationCount(foo));
assertEquals(1, %GetDeoptCount(foo));
