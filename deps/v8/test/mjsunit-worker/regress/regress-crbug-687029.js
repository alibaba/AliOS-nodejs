// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-687029.js");
	}
}
function foo(x) {
  x = Math.clz32(x);
  return "a".indexOf("a", x);
}
foo(1);
foo(1);
%OptimizeFunctionOnNextCall(foo);
foo();
