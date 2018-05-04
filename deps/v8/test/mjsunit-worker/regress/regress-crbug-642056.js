// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-642056.js");
	}
}
function f(o) {
  return o.x instanceof Array;
}

var o = { x : 1.5 };
o.x = 0;

f(o);
f(o);
%OptimizeFunctionOnNextCall(f);
f(o);
