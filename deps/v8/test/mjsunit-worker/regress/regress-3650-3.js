// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --opt  --no-stress-fullcodegen

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-3650-3.js");
	}
}
function foo(a) {
  for (var d in a) {
    delete a[1];
  }
}

foo([1,2,3]);
foo([2,3,4]);
%OptimizeFunctionOnNextCall(foo);
foo([1,2,3]);
assertOptimized(foo);
