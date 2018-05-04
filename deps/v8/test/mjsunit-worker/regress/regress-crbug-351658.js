// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-351658.js");
	}
}
try {
  var f = eval("(function(){0 = y + y})");
  %OptimizeFunctionOnNextCall(f);
  f();
  assertUnreachable();
} catch(e) {
  assertTrue(e instanceof ReferenceError);
}
