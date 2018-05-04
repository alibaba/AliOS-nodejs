// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-664117.js");
	}
}
function foo() {
  return v.length + 1;
}

var v = [];
foo();
v.length = 0xFFFFFFFF;

%OptimizeFunctionOnNextCall(foo);
assertEquals(4294967296, foo());
