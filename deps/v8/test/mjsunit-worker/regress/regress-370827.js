// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --expose-gc --heap-stats

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-370827.js");
	}
}
function g(dummy, x) {
  var start = "";
  if (x) { start = x + " - "; }
  start = start + "array length";
};

function f() {
  gc();
  g([0.1]);
}

f();
%OptimizeFunctionOnNextCall(f);
f();
f();
