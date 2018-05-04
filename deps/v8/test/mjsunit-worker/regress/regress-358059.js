// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-358059.js");
	}
}
function f(a, b) { return b + (a.x++); }
var o = {};
o.__defineGetter__('x', function() { return 1; });
assertEquals(4, f(o, 3));
assertEquals(4, f(o, 3));
%OptimizeFunctionOnNextCall(f);
assertEquals(4, f(o, 3));
