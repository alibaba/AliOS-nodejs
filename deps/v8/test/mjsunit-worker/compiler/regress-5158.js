// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-5158.js");
	}
}
function foo(x) {
  x = +x;
  return (x > 0) ? x : 0 - x;
}

foo(1);
foo(-1);
foo(0);
%OptimizeFunctionOnNextCall(foo);
assertEquals(2147483648, foo(-2147483648));
