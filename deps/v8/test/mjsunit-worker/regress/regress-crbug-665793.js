// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-665793.js");
	}
}
function foo() {
  return 'x'[1];
}
assertEquals(undefined, foo());
%OptimizeFunctionOnNextCall(foo);
assertEquals(undefined, foo());
