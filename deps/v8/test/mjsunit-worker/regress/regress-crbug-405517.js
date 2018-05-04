// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --gc-interval=203

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-405517.js");
	}
}
function f() {
 var e = [0];
 Object.preventExtensions(e);
 for (var i = 0; i < 4; i++) e.shift();
}

f();
f();
%OptimizeFunctionOnNextCall(f);
f();
