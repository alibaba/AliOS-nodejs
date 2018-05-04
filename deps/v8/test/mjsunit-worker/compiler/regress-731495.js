// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-731495.js");
	}
}
function foo() {
  global = "";
  global = global + "bar";
  return global;
};

assertEquals(foo(), "bar");
%OptimizeFunctionOnNextCall(foo);
assertEquals(foo(), "bar");
