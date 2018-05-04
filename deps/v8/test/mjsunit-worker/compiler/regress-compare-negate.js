// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --opt

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-compare-negate.js");
	}
}
function CompareNegate(a,b) {
 a = a|0;
 b = b|0;
 var sub = 0 - b;
 return a < (sub|0);
}

var x = CompareNegate(1,0x80000000);
%OptimizeFunctionOnNextCall(CompareNegate);
CompareNegate(1,0x80000000);
assertOptimized(CompareNegate);
assertEquals(x, CompareNegate(1,0x80000000));
